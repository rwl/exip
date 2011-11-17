/*==================================================================================*\
|                                                                                    |
|                    EXIP - Efficient XML Interchange Processor                      |
|                                                                                    |
|------------------------------------------------------------------------------------|
| Copyright (c) 2010, EISLAB - Luleå University of Technology                        |
| All rights reserved.                                                               |
|                                                                                    |
| Redistribution and use in source and binary forms, with or without                 |
| modification, are permitted provided that the following conditions are met:        |
|     * Redistributions of source code must retain the above copyright               |
|       notice, this list of conditions and the following disclaimer.                |
|     * Redistributions in binary form must reproduce the above copyright            |
|       notice, this list of conditions and the following disclaimer in the          |
|       documentation and/or other materials provided with the distribution.         |
|     * Neither the name of the EISLAB - Luleå University of Technology nor the      |
|       names of its contributors may be used to endorse or promote products         |
|       derived from this software without specific prior written permission.        |
|                                                                                    |
| THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND    |
| ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED      |
| WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             |
| DISCLAIMED. IN NO EVENT SHALL EISLAB - LULEÅ UNIVERSITY OF TECHNOLOGY BE LIABLE    |
| FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES |
| (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;       |
| LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND        |
| ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT         |
| (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS      |
| SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                       |
|                                                                                    |
|                                                                                    |
|                                                                                    |
\===================================================================================*/

/**
 * @file grammarGenerator.c
 * @brief Implementation of functions for generating Schema-informed Grammar definitions
 * @date Nov 22, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "grammarGenerator.h"
#include "dynamicArray.h"
#include "EXIParser.h"
#include "genUtils.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "grammars.h"
#include "grammarAugment.h"

#define XML_SCHEMA_NAMESPACE "http://www.w3.org/2001/XMLSchema"

/** Form Choice values */
#define FORM_CHOICE_UNQUALIFIED           0
#define FORM_CHOICE_QUALIFIED             1
#define FORM_CHOICE_ABSENT                2

/** Codes for the elements found in the schema */
#define ELEMENT_ELEMENT          0
#define ELEMENT_ATTRIBUTE        1
#define ELEMENT_CHOICE           2
#define ELEMENT_COMPLEX_TYPE     3
#define ELEMENT_COMPLEX_CONTENT  4
#define ELEMENT_GROUP            5
#define ELEMENT_IMPORT           6
#define ELEMENT_SEQUENCE         7
#define ELEMENT_ALL              8
#define ELEMENT_EXTENSION        9
#define ELEMENT_RESTRICTION     10
#define ELEMENT_SIMPLE_CONTENT  11
#define ELEMENT_ANY             12
#define ELEMENT_SIMPLE_TYPE     13
#define ELEMENT_MIN_INCLUSIVE   14
#define ELEMENT_ANNOTATION      15
#define ELEMENT_DOCUMENTATION   16
#define ELEMENT_MAX_LENGTH      17

#define ELEMENT_VOID           255


/** Codes for the attributes found in the schema */
#define ATTRIBUTE_ABSENT         0
#define ATTRIBUTE_NAME           1
#define ATTRIBUTE_TYPE           2
#define ATTRIBUTE_REF            3
#define ATTRIBUTE_MIN_OCCURS     4
#define ATTRIBUTE_MAX_OCCURS     5
#define ATTRIBUTE_FORM           6
#define ATTRIBUTE_BASE           7
#define ATTRIBUTE_USE            8
#define ATTRIBUTE_NAMESPACE      9
#define ATTRIBUTE_PROC_CONTENTS 10
#define ATTRIBUTE_VALUE         11
#define ATTRIBUTE_NILLABLE      12

#define ATTRIBUTE_VOID     255

#define ATTRIBUTE_CONTEXT_ARRAY_SIZE 20

#define INITIAL_STATE         0
#define SCHEMA_ELEMENT_STATE  1
#define SCHEMA_CONTENT_STATE  2

#define FORM_DEF_UNQUALIFIED   0
#define FORM_DEF_QUALIFIED     1
#define FORM_DEF_EXPECTING     2
#define FORM_DEF_INITIAL_STATE 3

struct prefixNS
{
	String prefix;
	String ns;
};

/**
 * Global schema properties (found as an attributes of the schema root element in XSD)
 * They should not change over time of processing
 */
struct globalSchemaProps {
	unsigned char propsStat; // 0 - initial state, 1 - <schema> element is parsed expect attributes, 2 - the properties are all set (<schema> attr. parsed)
	unsigned char expectAttributeData;
	String* charDataPointer; // Pointer to the expected character data
	String targetNamespace;
	uint16_t targetNSMetaID;  // the uri row ID in the metaURI table of the targetNamespace
	unsigned char attributeFormDefault; // 0 unqualified, 1 qualified, 2 expecting value, 3 initial state
	unsigned char elementFormDefault;  // 0 unqualified, 1 qualified, 2 expecting value, 3 initial state
	DynArray* prefixNamespaces; // array of struct prefixNS: prefix, namespace pairs
	String emptyString; // A holder for the empty string constant used throughout the generation
};

/**
 * An entry of nested schema descriptions. It is used to store the
 * current context when passing through schema document
 */
struct elementDescr {
	unsigned char element;  // represented with codes defined above
	String attributePointers[ATTRIBUTE_CONTEXT_ARRAY_SIZE]; // the index is the code of the attribute
	GenericStack* pGrammarStack; // The proto-grammars created so far and connected to this elemDescr
	GenericStack* pEmptyGrammarStack; // The Empty proto-grammars created so far and connected to this elemDescr
	DynArray* attributeUses; // For complex types/content this array stores the attribute uses
	GenericStack* pTypeFacets; // The Constraining Facets attached to this element if any
};

typedef struct elementDescr ElementDescription;

/**
 * Represents an element declaration with attribute "type" and the
 * value of the type that cannot be found in the TypeGrammar pool.
 * That is, the definition of the type is still not reached.
 * These elements are put in a dynamic array
 * */
struct elementNotResolved {
	QName element;
	QName type;
};

struct xsdAppData
{
	struct globalSchemaProps props;
	AllocList tmpMemList;   			// Temporary allocations during the schema creation
	GenericStack* contextStack; // Stack of ElementDescriptions
	DynArray* elNotResolvedArray;
	URITable* metaStringTables;
	DynArray* globalElemGrammars; // QNameID* of globalElemGrammars
	DynArray* simpleTypesArray; // A temporary array of simple type definitions
	EXIPSchema* schema;
};

struct TypeFacet
{
	uint16_t facetID;
	int value;
};

typedef struct TypeFacet TypeFacet;

// Content Handler API
static char xsd_fatalError(const char code, const char* msg, void* app_data);
static char xsd_startDocument(void* app_data);
static char xsd_endDocument(void* app_data);
static char xsd_startElement(QName qname, void* app_data);
static char xsd_endElement(void* app_data);
static char xsd_attribute(QName qname, void* app_data);
static char xsd_stringData(const String value, void* app_data);
static char xsd_namespaceDeclaration(const String namespace, const String prefix, unsigned char isLocalElementNS, void* app_data);

static void initElemContext(ElementDescription* elem);

// Handling of schema elements

static errorCode handleAttributeEl(struct xsdAppData* app_data);

static errorCode handleExtentionEl(struct xsdAppData* app_data);

static errorCode handleSimpleContentEl(struct xsdAppData* app_data);

static errorCode handleComplexTypeEl(struct xsdAppData* app_data);

static errorCode handleElementEl(struct xsdAppData* app_data);

static errorCode handleAnyEl(struct xsdAppData* app_data);

static errorCode handleElementSequence(struct xsdAppData* app_data);

static errorCode handleChoiceEl(struct xsdAppData* app_data);

static errorCode handleSimpleTypeEl(struct xsdAppData* app_data);

static errorCode handleRestrictionEl(struct xsdAppData* app_data);

static errorCode handleMinInclusiveEl(struct xsdAppData* app_data);

static errorCode handleMaxLengthEl(struct xsdAppData* app_data);

//////////// Helper functions

static errorCode addURIString(struct xsdAppData* app_data, String* uri, uint16_t* uriRowId);

static errorCode addLocalName(uint16_t uriId, struct xsdAppData* app_data, String* ln, size_t* lnRowId);

static void sortInitialStringTables(URITable* stringTables);

static int compareLN(const void* lnRow1, const void* lnRow2);

static int compareURI(const void* uriRow1, const void* uriRow2);

static int parseOccuranceAttribute(const String occurance);

static errorCode getTypeQName(struct xsdAppData* app_data, const String typeLiteral, QName* qname);

////////////

errorCode generateSchemaInformedGrammars(char* binaryBuf, size_t bufLen, size_t bufContent, IOStream* ioStrm,
										unsigned char schemaFormat, EXIPSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Parser xsdParser;
	struct xsdAppData parsing_data;

	if(schemaFormat != SCHEMA_FORMAT_XSD_EXI)
		return NOT_IMPLEMENTED_YET;

	tmp_err_code = initAllocList(&parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = initAllocList(&schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	schema->isStatic = FALSE;

	tmp_err_code = initParser(&xsdParser, binaryBuf, bufLen, bufContent, ioStrm, NULL, &parsing_data);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	xsdParser.handler.fatalError = xsd_fatalError;
	xsdParser.handler.error = xsd_fatalError;
	xsdParser.handler.startDocument = xsd_startDocument;
	xsdParser.handler.endDocument = xsd_endDocument;
	xsdParser.handler.startElement = xsd_startElement;
	xsdParser.handler.attribute = xsd_attribute;
	xsdParser.handler.stringData = xsd_stringData;
	xsdParser.handler.endElement = xsd_endElement;
	xsdParser.handler.namespaceDeclaration = xsd_namespaceDeclaration;

	parsing_data.props.propsStat = INITIAL_STATE;
	parsing_data.props.expectAttributeData = FALSE;
	parsing_data.props.attributeFormDefault = FORM_DEF_INITIAL_STATE;
	parsing_data.props.elementFormDefault = FORM_DEF_INITIAL_STATE;
	parsing_data.props.charDataPointer = NULL;
	parsing_data.props.targetNSMetaID = 0;
	getEmptyString(&parsing_data.props.emptyString);
	getEmptyString(&parsing_data.props.targetNamespace);

	parsing_data.contextStack = NULL;

	tmp_err_code = createDynArray(&parsing_data.props.prefixNamespaces, sizeof(struct prefixNS), 10, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.elNotResolvedArray, sizeof(struct elementNotResolved), 10, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.globalElemGrammars, sizeof(QNameID), 10, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.simpleTypesArray, sizeof(SimpleType), 50, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createBuildInTypesDefinitions(parsing_data.simpleTypesArray, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createURITable(&schema->initialStringTables, &schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createInitialEntries(&schema->memList, schema->initialStringTables, TRUE);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createURITable(&parsing_data.metaStringTables, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createInitialEntries(&parsing_data.tmpMemList, parsing_data.metaStringTables, TRUE);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	parsing_data.schema = schema;

	tmp_err_code = generateBuildInTypesGrammars(schema->initialStringTables, &schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Parse the EXI stream

	tmp_err_code = parseHeader(&xsdParser);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">XML Schema header parsed\n"));

	while(tmp_err_code == ERR_OK)
	{
		tmp_err_code = parseNext(&xsdParser);
	}

	destroyParser(&xsdParser);

	if(tmp_err_code == PARSING_COMPLETE)
		return ERR_OK;

	return tmp_err_code;
}

static char xsd_fatalError(const char code, const char* msg, void* app_data)
{
	DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Fatal error occurred during schema processing\n"));
	return EXIP_HANDLER_STOP;
}

static char xsd_startDocument(void* app_data)
{
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Start XML Schema parsing\n"));
	return EXIP_HANDLER_OK;
}

static char xsd_endDocument(void* app_data)
{
	struct xsdAppData* appD = (struct xsdAppData*) app_data;
	unsigned int i = 0;
	size_t j = 0;
	uint16_t uriRowID;
	size_t lnRowID;
	QNameID* tmpQnameID;
	EXIGrammar* tmpGrammar = NULL;
	Production* tmpProd;
	size_t t = 0;
	size_t p = 0;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End XML Schema parsing\n"));

// Only for debugging purposes
#if DEBUG_GRAMMAR_GEN == ON
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\nList of global element grammars:"));

		for(i = 0; i < appD->globalElemGrammars->elementCount; i++)
		{
			tmpQnameID = ((QNameID*) appD->globalElemGrammars->elements) + i;
			tmpGrammar = appD->schema->initialStringTables->rows[tmpQnameID->uriRowId].lTable->rows[tmpQnameID->lnRowId].typeGrammar;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\nURI: "));
			printString(&appD->schema->initialStringTables->rows[tmpQnameID->uriRowId].string_val);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\nLN: "));
			printString(&appD->schema->initialStringTables->rows[tmpQnameID->uriRowId].lTable->rows[tmpQnameID->lnRowId].string_val);
			for(t = 0; t < tmpGrammar->rulesDimension; t++)
			{
				if(printGrammarRule(t, &(tmpGrammar->ruleArray[t])) != ERR_OK)
				{
					DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">printGrammarRule() fail\n"));
					return EXIP_HANDLER_STOP;
				}
			}
		}
	}
#endif

	sortInitialStringTables(appD->schema->initialStringTables);

	appD->schema->sTypeArraySize = appD->simpleTypesArray->elementCount;
	appD->schema->simpleTypeArray = memManagedAllocate(&appD->schema->memList, sizeof(SimpleType) * appD->schema->sTypeArraySize);
	if(appD->schema->simpleTypeArray == NULL)
	{
		DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Memory allocation error\n"));
		return EXIP_HANDLER_STOP;
	}
	for(i = 0; i < appD->schema->sTypeArraySize; i++)
	{
		appD->schema->simpleTypeArray[i].facetPresenceMask = ((SimpleType*) appD->simpleTypesArray->elements)[i].facetPresenceMask;
		appD->schema->simpleTypeArray[i].maxInclusive = ((SimpleType*) appD->simpleTypesArray->elements)[i].maxInclusive;
		appD->schema->simpleTypeArray[i].minInclusive = ((SimpleType*) appD->simpleTypesArray->elements)[i].minInclusive;
	}

	appD->schema->globalElemGrammarsCount = appD->globalElemGrammars->elementCount;
	appD->schema->globalElemGrammars = (QNameID*) memManagedAllocate(&appD->schema->memList, sizeof(QNameID)*appD->schema->globalElemGrammarsCount);
	if(appD->schema->globalElemGrammars == NULL)
	{
		DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Memory allocation error\n"));
		return EXIP_HANDLER_STOP;
	}

	for(i = 0; i < appD->schema->globalElemGrammarsCount; i++)
	{
		tmpQnameID = ((QNameID*) appD->globalElemGrammars->elements) + i;

		lookupURI(appD->schema->initialStringTables, appD->metaStringTables->rows[tmpQnameID->uriRowId].string_val, &uriRowID);
		appD->schema->globalElemGrammars[i].uriRowId = uriRowID;

		lookupLN(appD->schema->initialStringTables->rows[uriRowID].lTable, appD->metaStringTables->rows[uriRowID].lTable->rows[tmpQnameID->lnRowId].string_val, &lnRowID);
		appD->schema->globalElemGrammars[i].lnRowId = lnRowID;
	}

	for (i = 0; i < appD->schema->initialStringTables->rowCount; i++)
	{
		for (j = 0; j < appD->schema->initialStringTables->rows[i].lTable->rowCount; j++)
		{
			tmpGrammar = appD->schema->initialStringTables->rows[i].lTable->rows[j].typeGrammar;
			if(tmpGrammar != NULL)
			{
				for (t = 0; t < tmpGrammar->rulesDimension; t++)
				{
					for (p = 0; p < tmpGrammar->ruleArray[t].prodCounts[0]; p++)
					{
						tmpProd = &tmpGrammar->ruleArray[t].prodArrays[0][p];
						if(tmpProd->uriRowID != UINT16_MAX)
						{
							if(!lookupURI(appD->schema->initialStringTables, appD->metaStringTables->rows[tmpProd->uriRowID].string_val, &uriRowID))
							{
								DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Error in the schema generation\n"));
								return EXIP_HANDLER_STOP;
							}
							tmpProd->uriRowID = uriRowID;

							if(!lookupLN(appD->schema->initialStringTables->rows[uriRowID].lTable, appD->metaStringTables->rows[uriRowID].lTable->rows[tmpProd->lnRowID].string_val, &lnRowID))
							{
								DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Error in the schema generation\n"));
								return EXIP_HANDLER_STOP;
							}
							tmpProd->lnRowID = lnRowID;
						}
					}
				}
			}
		}
	}

	freeAllocList(&appD->tmpMemList);
	return EXIP_HANDLER_OK;
}

static char xsd_startElement(QName qname, void* app_data)
{
	struct xsdAppData* appD = (struct xsdAppData*) app_data;
	if(appD->props.propsStat == INITIAL_STATE) // This should be the first <schema> element
	{
		if(stringEqualToAscii(*qname.uri, XML_SCHEMA_NAMESPACE) &&
				stringEqualToAscii(*qname.localName, "schema"))
		{
			appD->props.propsStat = SCHEMA_ELEMENT_STATE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <schema> element\n"));
		}
		else
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid XML Schema! Missing <schema> root element\n"));
			return EXIP_HANDLER_STOP;
		}
	}
	else
	{
		ElementDescription* element;
		errorCode tmp_err_code = UNEXPECTED_ERROR;

		if(appD->props.propsStat != SCHEMA_CONTENT_STATE) // This is the first element after the <schema>
		{
			appD->props.propsStat = SCHEMA_CONTENT_STATE; // All attributes of the <schema> element are already parsed
			if(appD->props.elementFormDefault == FORM_DEF_INITIAL_STATE)
				appD->props.elementFormDefault = FORM_DEF_UNQUALIFIED; // The default value is unqualified
			if(appD->props.attributeFormDefault == FORM_DEF_INITIAL_STATE)
				appD->props.attributeFormDefault = FORM_DEF_UNQUALIFIED; // The default value is unqualified

			if(!isStringEmpty(&appD->props.targetNamespace)) // Add the declared target namespace in the String Tables
			{
				uint16_t uriID;

				// If the target namespace is not in the initial uri entries add it
				if(!lookupURI(appD->schema->initialStringTables, appD->props.targetNamespace, &uriID))
				{
					String tnsClone;
					tmp_err_code = cloneString(&appD->props.targetNamespace, &tnsClone, &appD->schema->memList);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					tmp_err_code = addURIRow(appD->schema->initialStringTables, tnsClone, &uriID, &appD->schema->memList);
					if(tmp_err_code != ERR_OK)
					{
						DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Schema parsing error: %d\n", tmp_err_code));
						return EXIP_HANDLER_STOP;
					}

					tmp_err_code = addURIRow(appD->metaStringTables, appD->props.targetNamespace, &uriID, &appD->tmpMemList);
					if(tmp_err_code != ERR_OK)
					{
						DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Schema parsing error: %d\n", tmp_err_code));
						return EXIP_HANDLER_STOP;
					}
				}
				appD->props.targetNSMetaID = uriID;
			}
		}

		if(!stringEqualToAscii(*qname.uri, XML_SCHEMA_NAMESPACE))
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid namespace of XML Schema element\n"));
			return EXIP_HANDLER_STOP;
		}

		element = (ElementDescription*) memManagedAllocate(&appD->tmpMemList, sizeof(ElementDescription));
		if(element == NULL)
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Memory allocation error\n"));
			return EXIP_HANDLER_STOP;
		}

		initElemContext(element);

		if(stringEqualToAscii(*qname.localName, "element"))
		{
			element->element = ELEMENT_ELEMENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <element> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "attribute"))
		{
			element->element = ELEMENT_ATTRIBUTE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <attribute> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "choice"))
		{
			element->element = ELEMENT_CHOICE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <choice> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "complexType"))
		{
			element->element = ELEMENT_COMPLEX_TYPE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <complexType> element\n"));
			tmp_err_code = createDynArray(&element->attributeUses, sizeof(ProtoGrammar), 5, &appD->tmpMemList);
			if(tmp_err_code != ERR_OK)
				return EXIP_HANDLER_STOP;
		}
		else if(stringEqualToAscii(*qname.localName, "complexContent"))
		{
			element->element = ELEMENT_COMPLEX_CONTENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <complexContent> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "group"))
		{
			element->element = ELEMENT_GROUP;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <group> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "import"))
		{
			element->element = ELEMENT_IMPORT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <import> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "sequence"))
		{
			element->element = ELEMENT_SEQUENCE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <sequence> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "all"))
		{
			element->element = ELEMENT_ALL;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <all> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "extension"))
		{
			element->element = ELEMENT_EXTENSION;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <extension> element\n"));
			tmp_err_code = createDynArray(&element->attributeUses, sizeof(ProtoGrammar), 5, &appD->tmpMemList);
			if(tmp_err_code != ERR_OK)
				return EXIP_HANDLER_STOP;
		}
		else if(stringEqualToAscii(*qname.localName, "restriction"))
		{
			element->element = ELEMENT_RESTRICTION;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <restriction> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "simpleContent"))
		{
			element->element = ELEMENT_SIMPLE_CONTENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <simpleContent> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "any"))
		{
			element->element = ELEMENT_ANY;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <any> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "simpleType"))
		{
			element->element = ELEMENT_SIMPLE_TYPE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <simpleType> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "minInclusive"))
		{
			element->element = ELEMENT_MIN_INCLUSIVE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <minInclusive> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "annotation"))
		{
			element->element = ELEMENT_ANNOTATION;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <annotation> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "documentation"))
		{
			element->element = ELEMENT_DOCUMENTATION;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <documentation> element\n"));
		}
		else if(stringEqualToAscii(*qname.localName, "maxLength"))
		{
			element->element = ELEMENT_MAX_LENGTH;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <maxLength> element\n"));
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored schema element\n"));
			return EXIP_HANDLER_STOP;
		}

		tmp_err_code = pushOnStack(&(appD->contextStack), element);
		if(tmp_err_code != ERR_OK)
			return EXIP_HANDLER_STOP;
	}

	return EXIP_HANDLER_OK;
}

static char xsd_endElement(void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	struct xsdAppData* appD = (struct xsdAppData*) app_data;
	if(appD->contextStack == NULL) // No elements stored in the stack. That is </schema>
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </schema> element\n"));
		tmp_err_code = ERR_OK;
	}
	else
	{
		ElementDescription* element = (ElementDescription*) appD->contextStack->element;
		if(element->element == ELEMENT_ATTRIBUTE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </attribute> element\n"));
			tmp_err_code = handleAttributeEl(appD);
		}
		else if(element->element == ELEMENT_EXTENSION)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </extension> element\n"));
			tmp_err_code = handleExtentionEl(appD);
		}
		else if(element->element == ELEMENT_SIMPLE_CONTENT)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </simpleContent> element\n"));
			tmp_err_code = handleSimpleContentEl(appD);
		}
		else if(element->element == ELEMENT_COMPLEX_TYPE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </complexType> element\n"));
			tmp_err_code = handleComplexTypeEl(appD);
		}
		else if(element->element == ELEMENT_ELEMENT)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </element> element\n"));
			tmp_err_code = handleElementEl(appD);
		}
		else if(element->element == ELEMENT_ANY)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </any> element\n"));
			tmp_err_code = handleAnyEl(appD);
		}
		else if(element->element == ELEMENT_SEQUENCE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </sequence> element\n"));
			tmp_err_code = handleElementSequence(appD);
		}
		else if(element->element == ELEMENT_CHOICE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </choice> element\n"));
			tmp_err_code = handleChoiceEl(appD);
		}
		else if(element->element == ELEMENT_SIMPLE_TYPE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </simpleType> element\n"));
			tmp_err_code = handleSimpleTypeEl(appD);
		}
		else if(element->element == ELEMENT_RESTRICTION)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </restriction> element\n"));
			tmp_err_code = handleRestrictionEl(appD);
		}
		else if(element->element == ELEMENT_MIN_INCLUSIVE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </minInclusive> element\n"));
			tmp_err_code = handleMinInclusiveEl(appD);
		}
		else if(element->element == ELEMENT_ANNOTATION)
		{
			ElementDescription* elemDesc;
			popFromStack(&(appD->contextStack), (void**) &elemDesc);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </annotation> element\n"));
			tmp_err_code = ERR_OK;
		}
		else if(element->element == ELEMENT_DOCUMENTATION)
		{
			ElementDescription* elemDesc;
			popFromStack(&(appD->contextStack), (void**) &elemDesc);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </documentation> element\n"));
			tmp_err_code = ERR_OK;
		}
		else if(element->element == ELEMENT_MAX_LENGTH)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </maxLength> element\n"));
			tmp_err_code = handleMaxLengthEl(appD);
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored closing element\n"));
			return EXIP_HANDLER_STOP;
		}
	}

	if(tmp_err_code != ERR_OK)
	{
		DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Schema parsing error: %d\n", tmp_err_code));
		return EXIP_HANDLER_STOP;
	}
	return EXIP_HANDLER_OK;
}

static char xsd_attribute(QName qname, void* app_data)
{
	struct xsdAppData* appD = (struct xsdAppData*) app_data;
	if(appD->props.propsStat == SCHEMA_ELEMENT_STATE) // <schema> element attribute
	{
		if(stringEqualToAscii(*qname.localName, "targetNamespace"))
		{
			appD->props.charDataPointer = &(appD->props.targetNamespace);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |targetNamespace| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "elementFormDefault"))
		{
			appD->props.elementFormDefault = FORM_DEF_EXPECTING;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |elementFormDefault| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "attributeFormDefault"))
		{
			appD->props.attributeFormDefault = FORM_DEF_EXPECTING;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |attributeFormDefault| \n"));
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored <schema> attribute\n"));
		}
	}
	else
	{
		ElementDescription* element = (ElementDescription*) appD->contextStack->element;

		if(stringEqualToAscii(*qname.localName, "name"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_NAME]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |name| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "type"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_TYPE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |type| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "ref"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_REF]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |ref| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "minOccurs"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_MIN_OCCURS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |minOccurs| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "maxOccurs"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_MAX_OCCURS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |maxOccurs| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "form"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_FORM]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |form| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "base"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_BASE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |base| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "use"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_USE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |use| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "namespace"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_NAMESPACE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |namespace| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "processContents"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_PROC_CONTENTS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |processContents| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "value"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_VALUE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |value| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "nillable"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_NILLABLE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |value| \n"));
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element attribute\n"));
		}
	}
	appD->props.expectAttributeData = TRUE;
	return EXIP_HANDLER_OK;
}

static char xsd_stringData(const String value, void* app_data)
{
	struct xsdAppData* appD = (struct xsdAppData*) app_data;
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">String data:\n"));

#if	DEBUG_GRAMMAR_GEN == ON
	printString(&value);
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n"));
#endif

	if(appD->props.expectAttributeData)
	{
		if(appD->props.propsStat == SCHEMA_ELEMENT_STATE) // <schema> element attribute data
		{
			if(appD->props.charDataPointer != NULL)
			{
				*(appD->props.charDataPointer) = value;
				appD->props.charDataPointer = NULL;
			}
			else if(appD->props.elementFormDefault == FORM_DEF_EXPECTING) // expecting value for elementFormDefault
			{
				if(stringEqualToAscii(value, "qualified"))
					appD->props.elementFormDefault = FORM_DEF_QUALIFIED;
				else if(stringEqualToAscii(value, "unqualified"))
					appD->props.elementFormDefault = FORM_DEF_UNQUALIFIED;
				else
				{
					DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid value for elementFormDefault attribute\n"));
					return EXIP_HANDLER_STOP;
				}
			}
			else if(appD->props.attributeFormDefault == FORM_DEF_EXPECTING) // expecting value for attributeFormDefault
			{
				if(stringEqualToAscii(value, "qualified"))
					appD->props.attributeFormDefault = FORM_DEF_QUALIFIED;
				else if(stringEqualToAscii(value, "unqualified"))
					appD->props.attributeFormDefault = FORM_DEF_UNQUALIFIED;
				else
				{
					DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid value for attributeFormDefault attribute\n"));
					return EXIP_HANDLER_STOP;
				}
			}
			else
			{
				DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored <schema> attribute value\n"));
			}
		}
		else
		{
			if(appD->props.charDataPointer != NULL)
			{
				*(appD->props.charDataPointer) = value;
				appD->props.charDataPointer = NULL;
			}
			else
			{
				DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element attribute value\n"));
			}
		}

		appD->props.expectAttributeData = FALSE;
	}
	else
	{
		if(appD->props.charDataPointer != NULL)
		{
			*(appD->props.charDataPointer) = value;
			appD->props.charDataPointer = NULL;
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element value\n"));
		}
	}

	return EXIP_HANDLER_OK;
}

static char xsd_namespaceDeclaration(const String namespace, const String prefix, unsigned char isLocalElementNS, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	struct xsdAppData* appD = (struct xsdAppData*) app_data;
	struct prefixNS prNS;
	size_t elID;
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Namespace declaration\n"));

	prNS.ns.length = namespace.length;
	prNS.ns.str = namespace.str;

	prNS.prefix.length = prefix.length;
	prNS.prefix.str = prefix.str;

	tmp_err_code = addDynElement(appD->props.prefixNamespaces, &prNS, &elID, &appD->tmpMemList);
	if(tmp_err_code != ERR_OK)
	{
		DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Error addDynElement\n"));
		return EXIP_HANDLER_STOP;
	}

	return EXIP_HANDLER_OK;
}

static void initElemContext(ElementDescription* elem)
{
	unsigned int i = 0;
	elem->element = ELEMENT_VOID;
	elem->pGrammarStack = NULL;
	elem->pEmptyGrammarStack = NULL;
	elem->attributeUses = NULL;
	elem->pTypeFacets = NULL;
	for(i = 0; i < ATTRIBUTE_CONTEXT_ARRAY_SIZE; i++)
	{
		elem->attributePointers[i].length = 0;
		elem->attributePointers[i].str = NULL;
	}
}

static errorCode handleAttributeEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char required = 0;
	String* target_ns;
	QName simpleType;
	QNameID simpleTypeID;
	QName scope;
	ProtoGrammar* attrUseGrammar;
	size_t attrUseGrammarID;
	ElementDescription* elemDesc;
	size_t lnRowID;
	uint16_t uriRowID;
	String* attrName;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	if (!isStringEmpty(&(elemDesc->attributePointers[ATTRIBUTE_USE])) &&
			stringEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_USE], "required"))
	{
		required = 1;
	}
	if(app_data->props.attributeFormDefault == FORM_DEF_QUALIFIED || stringEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace - might be different from the global target namespace

		target_ns = &(app_data->props.targetNamespace); // it is the globally defined target namespace
		uriRowID = app_data->props.targetNSMetaID;
	}
	else
	{
		target_ns = &app_data->props.emptyString;
		uriRowID = 0;
	}

	attrName = &(elemDesc->attributePointers[ATTRIBUTE_NAME]);
	tmp_err_code = addLocalName(uriRowID, app_data, attrName, &lnRowID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = getTypeQName(app_data, elemDesc->attributePointers[ATTRIBUTE_TYPE], &simpleType);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(isStringEmpty(simpleType.uri)) // The type does not have defined namespace -> assume http://www.w3.org/2001/XMLSchema
	{
		simpleType.uri = &app_data->metaStringTables->rows[3].string_val;
		simpleTypeID.uriRowId = 3;
	}
	else
	{
		tmp_err_code = addURIString(app_data,(String*) simpleType.uri, &simpleTypeID.uriRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = addLocalName(simpleTypeID.uriRowId, app_data,(String*) simpleType.localName, &simpleTypeID.lnRowId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	scope.localName = &app_data->props.emptyString;
	scope.uri = &app_data->props.emptyString;

	tmp_err_code = createAttributeUseGrammar(&app_data->tmpMemList, required, simpleTypeID, scope, &attrUseGrammar, uriRowID, lnRowID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		EXIGrammar* tmpGrammar;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute proto-grammar:\n"));
		convertProtoGrammar(&app_data->tmpMemList, attrUseGrammar, &tmpGrammar);
		for(t = 0; t < tmpGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(t, &(tmpGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = addDynElement(((ElementDescription*) app_data->contextStack->element)->attributeUses, attrUseGrammar, &attrUseGrammarID, &app_data->tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleExtentionEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QName simpleType;
	QNameID simpleTypeID;
	String* typeName;
	String* target_ns;
	ProtoGrammar* simpleTypeGrammar;
	ElementDescription* elemDesc;
	ProtoGrammar* resultComplexGrammar;
	ValueType vType;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	typeName = &app_data->props.emptyString;
	target_ns = &app_data->props.emptyString;

	tmp_err_code = getTypeQName(app_data, elemDesc->attributePointers[ATTRIBUTE_BASE], &simpleType);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(isStringEmpty(simpleType.uri)) // The type does not have defined namespace -> assume http://www.w3.org/2001/XMLSchema
	{
		simpleType.uri = &app_data->metaStringTables->rows[3].string_val;
		simpleTypeID.uriRowId = 3;
	}
	else
	{
		tmp_err_code = addURIString(app_data,(String*) simpleType.uri, &simpleTypeID.uriRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = addLocalName(simpleTypeID.uriRowId, app_data,(String*) simpleType.localName, &simpleTypeID.lnRowId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = getEXIDataTypeFromSimpleType(simpleTypeID, &vType);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createSimpleTypeGrammar(&app_data->tmpMemList, vType, &simpleTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// TODO: the attributeUses array must be sorted first before calling createComplexTypeGrammar()
	tmp_err_code = createComplexTypeGrammar(&app_data->tmpMemList, (ProtoGrammar*) elemDesc->attributeUses->elements, (unsigned int)(elemDesc->attributeUses->elementCount),
									   NULL, 0, simpleTypeGrammar, &resultComplexGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		EXIGrammar* tmpGrammar;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Extension proto-grammar:\n"));
		convertProtoGrammar(&app_data->tmpMemList, resultComplexGrammar, &tmpGrammar);
		for(t = 0; t < tmpGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(t, &(tmpGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pGrammarStack), (void*) resultComplexGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleSimpleContentEl(struct xsdAppData* app_data)
{
	ElementDescription* elemDesc;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammar* contentGr;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	popFromStack(&elemDesc->pGrammarStack, (void**) &contentGr);

	tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pGrammarStack), contentGr);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	return ERR_OK;
}

static errorCode handleComplexTypeEl(struct xsdAppData* app_data)
{
	// TODO: The attribute uses must be sorted first

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	String* typeName;
	String* target_ns;
	ProtoGrammar* contentTypeGrammar;
	ProtoGrammar* resultComplexGrammar;
	ProtoGrammar* resultComplexEmptyGrammar;
	ElementDescription* elemDesc;
	size_t lnRowID;
	uint16_t uriRowID;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	typeName = &(elemDesc->attributePointers[ATTRIBUTE_NAME]);

	if(app_data->props.elementFormDefault == FORM_DEF_QUALIFIED || stringEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace
		target_ns = &(app_data->props.targetNamespace);
		uriRowID = app_data->props.targetNSMetaID;
	}
	else
	{
		target_ns = &app_data->props.emptyString;
		uriRowID = 0; // ""
	}

	if(!isStringEmpty(typeName))
	{
		tmp_err_code = addLocalName(uriRowID, app_data, typeName, &lnRowID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}


	popFromStack(&(elemDesc->pGrammarStack), (void**) &contentTypeGrammar);

	if(contentTypeGrammar == NULL) // An empty element: <xsd:complexType />
		resultComplexGrammar = NULL;
	else
	{
		// TODO: the attributeUses array must be sorted first before calling createComplexTypeGrammar()

		tmp_err_code = createComplexTypeGrammar(&app_data->tmpMemList, (ProtoGrammar*) elemDesc->attributeUses->elements, (unsigned int)(elemDesc->attributeUses->elementCount),
										   NULL, 0, contentTypeGrammar, &resultComplexGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = createComplexEmptyTypeGrammar(&app_data->tmpMemList, (ProtoGrammar*) elemDesc->attributeUses->elements, (unsigned int)(elemDesc->attributeUses->elementCount),
				NULL, 0, &resultComplexEmptyGrammar);

		//TODO: the attributeUses array must be emptied here

#if DEBUG_GRAMMAR_GEN == ON
		{
			uint16_t t = 0;
			EXIGrammar* tmpGrammar;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Complex type proto-grammar:\n"));
			convertProtoGrammar(&app_data->tmpMemList, resultComplexGrammar, &tmpGrammar);
			for(t = 0; t < tmpGrammar->rulesDimension; t++)
			{
				tmp_err_code = printGrammarRule(t, &(tmpGrammar->ruleArray[t]));
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
#endif
	}

	if(isStringEmpty(typeName))  // The name is empty i.e. anonymous complex type
	{
		// Put the ComplexTypeGrammar on top of the pGrammarStack
		// There should be a parent <element> declaration for this grammar
		tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pGrammarStack), resultComplexGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pEmptyGrammarStack), resultComplexEmptyGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // Named complex type - put it directly in the typeGrammars list
	{
		EXIGrammar* complexEXIGrammar;
		EXIGrammar* complexEXIEmptyGrammar;

		tmp_err_code = assignCodes(resultComplexGrammar, app_data->metaStringTables);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = convertProtoGrammar(&app_data->schema->memList, resultComplexGrammar, &complexEXIGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Complex type grammar:\n"));
		for(t = 0; t < complexEXIGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(t, &(complexEXIGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

		tmp_err_code = assignCodes(resultComplexEmptyGrammar, app_data->metaStringTables);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = convertProtoGrammar(&app_data->schema->memList, resultComplexEmptyGrammar, &complexEXIEmptyGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		app_data->schema->initialStringTables->rows[uriRowID].lTable->rows[lnRowID].typeGrammar = complexEXIGrammar;
		app_data->schema->initialStringTables->rows[uriRowID].lTable->rows[lnRowID].typeEmptyGrammar = complexEXIEmptyGrammar;
	}

	return ERR_OK;
}

static errorCode handleElementEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	String type;
	ProtoGrammar* typeGrammar;
	ProtoGrammar* typeEmptyGrammar;
	String* elName;
	String* target_ns;
	unsigned char isGlobal = 0;
	uint16_t uriRowId;
	size_t lnRowId;
	EXIGrammar* exiTypeGrammar;
	EXIGrammar* exiTypeEmptyGrammar;
	unsigned char isNillable = FALSE;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	type = elemDesc->attributePointers[ATTRIBUTE_TYPE];

	if(app_data->contextStack == NULL) // Global element
		isGlobal = TRUE;

	if(isGlobal || app_data->props.elementFormDefault == FORM_DEF_QUALIFIED || stringEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace
		target_ns = &(app_data->props.targetNamespace);
		uriRowId = app_data->props.targetNSMetaID;
	}
	else
	{
		target_ns = &app_data->props.emptyString;
		uriRowId = 0;
	}

	elName = &(elemDesc->attributePointers[ATTRIBUTE_NAME]);

	tmp_err_code = addLocalName(uriRowId, app_data, elName, &lnRowId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(!isStringEmpty(&elemDesc->attributePointers[ATTRIBUTE_NILLABLE]) &&
		stringEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_NILLABLE], "true"))
	{
		isNillable = TRUE;
	}

	if(isGlobal)
	{
		QNameID globalQnameID;
		size_t dynElID;

		if(isStringEmpty(&type))  // There is no type attribute i.e. there must be some complex/simple type in the pGrammarStack
		{
			popFromStack(&(elemDesc->pGrammarStack), (void**) &typeGrammar);
			popFromStack(&(elemDesc->pEmptyGrammarStack), (void**) &typeEmptyGrammar);
		}
		else // The element has a particular named type
		{
			return NOT_IMPLEMENTED_YET;
		}

		tmp_err_code = assignCodes(typeGrammar, app_data->metaStringTables);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = convertProtoGrammar(&app_data->schema->memList, typeGrammar, &exiTypeGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		exiTypeGrammar->isNillable = isNillable;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Element grammar:\n"));
		for(t = 0; t < exiTypeGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(t, &(exiTypeGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

		tmp_err_code = assignCodes(typeEmptyGrammar, app_data->metaStringTables);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = convertProtoGrammar(&app_data->schema->memList, typeEmptyGrammar, &exiTypeEmptyGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		exiTypeEmptyGrammar->isNillable = isNillable;

		app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].typeGrammar = exiTypeGrammar;
		app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].typeEmptyGrammar = exiTypeEmptyGrammar;
		globalQnameID.uriRowId = uriRowId;
		globalQnameID.lnRowId = lnRowId;
		tmp_err_code = addDynElement(app_data->globalElemGrammars, &globalQnameID, &dynElID, &app_data->schema->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else  // Local element definition i.e within complex type
	{
		ProtoGrammar* elTermGrammar;
		ProtoGrammar* elParticleGrammar;
		unsigned int minOccurs = 1;
		int32_t maxOccurs = 1;
		QName typeQname;

		if(isStringEmpty(&type))  // There is no type attribute i.e. there must be some complex type in the pGrammarStack
		{
			popFromStack(&(elemDesc->pGrammarStack), (void**) &typeGrammar);
			popFromStack(&(elemDesc->pEmptyGrammarStack), (void**) &typeEmptyGrammar);

			if(typeGrammar != NULL) // It is not an empty element: <xsd:complexType />
			{
				tmp_err_code = assignCodes(typeGrammar, app_data->metaStringTables);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = convertProtoGrammar(&app_data->schema->memList, typeGrammar, &exiTypeGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				exiTypeGrammar->isNillable = isNillable;

#if DEBUG_GRAMMAR_GEN == ON
				{
					uint16_t t = 0;
					DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Element grammar:\n"));
					for(t = 0; t < exiTypeGrammar->rulesDimension; t++)
					{
						tmp_err_code = printGrammarRule(t, &(exiTypeGrammar->ruleArray[t]));
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
				}
#endif

				tmp_err_code = assignCodes(typeEmptyGrammar, app_data->metaStringTables);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = convertProtoGrammar(&app_data->schema->memList, typeEmptyGrammar, &exiTypeEmptyGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				exiTypeEmptyGrammar->isNillable = isNillable;

				app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].typeGrammar = exiTypeGrammar;
				app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].typeEmptyGrammar = exiTypeEmptyGrammar;
			}
			else // <xsd:complexType />
			{
				/** Empty element (<xsd:complexType /> )
				 *  Reuse the empty grammar defined for simple types TypeEmpty grammars and not create a multiple empty grammars each time
				 *  there is an empty element declared - the case for EXI options document*/

				app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].typeGrammar = app_data->schema->initialStringTables->rows[3].lTable->rows[0].typeEmptyGrammar;
				app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].typeEmptyGrammar = app_data->schema->initialStringTables->rows[3].lTable->rows[0].typeEmptyGrammar;
			}
		}
		else // The element has a particular named type
		{
			tmp_err_code = getTypeQName(app_data, type, &typeQname);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(isStringEmpty(typeQname.uri) || stringEqualToAscii(*typeQname.uri, XML_SCHEMA_NAMESPACE)) // This is build-in simple type definition
			{
				size_t stypelnRowId;

				if(lookupLN(app_data->metaStringTables->rows[3].lTable, *typeQname.localName, &stypelnRowId) == FALSE)
					return UNEXPECTED_ERROR;

				// URI 3 -> http://www.w3.org/2001/XMLSchema
				app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].typeGrammar = app_data->schema->initialStringTables->rows[3].lTable->rows[stypelnRowId].typeGrammar;
				app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].typeEmptyGrammar = app_data->schema->initialStringTables->rows[3].lTable->rows[stypelnRowId].typeEmptyGrammar;
			}
			else // A complex type name or derived simple type
			{
				return NOT_IMPLEMENTED_YET;
			}
		}

		minOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MIN_OCCURS]);
		maxOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MAX_OCCURS]);

		tmp_err_code = createElementTermGrammar(&app_data->tmpMemList, &elTermGrammar, uriRowId, lnRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = createParticleGrammar(&app_data->tmpMemList, minOccurs, maxOccurs,
											elTermGrammar, &elParticleGrammar);

		tmp_err_code = pushOnStack(&((ElementDescription*) app_data->contextStack->element)->pGrammarStack, (void*) elParticleGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

static errorCode handleAnyEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	String* namespaceArr;
	unsigned int namespaceCount = 0;
	uint16_t uriRowId;
	unsigned int i = 0;
	ProtoGrammar* wildTermGrammar;
	ProtoGrammar* wildParticleGrammar;
	unsigned int minOccurs = 1;
	int32_t maxOccurs = 1;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	if(isStringEmpty(&elemDesc->attributePointers[ATTRIBUTE_NAMESPACE]))
	{
		namespaceCount = 1;
		namespaceArr = &elemDesc->attributePointers[ATTRIBUTE_NAMESPACE];
		tmp_err_code = asciiToString("##any", namespaceArr, &app_data->tmpMemList, FALSE);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		tmp_err_code = splitStringByChar(&elemDesc->attributePointers[ATTRIBUTE_NAMESPACE], ' ', &namespaceArr, &namespaceCount, &app_data->tmpMemList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	for(i = 0; i < namespaceCount; i++)
	{
		if(!stringEqualToAscii(namespaceArr[i], "##any") &&
				!stringEqualToAscii(namespaceArr[i], "##other") &&
				!stringEqualToAscii(namespaceArr[i], "##targetNamespace") &&
				!stringEqualToAscii(namespaceArr[i], "##local"))
		{
			tmp_err_code = addURIString(app_data, &namespaceArr[i], &uriRowId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	minOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MIN_OCCURS]);
	maxOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MAX_OCCURS]);

	tmp_err_code = createWildcardTermGrammar(&app_data->tmpMemList, namespaceArr, namespaceCount, &wildTermGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createParticleGrammar(&app_data->tmpMemList, minOccurs, maxOccurs,
			wildTermGrammar, &wildParticleGrammar);

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		EXIGrammar* tmpGrammar;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Wildcard proto-grammar:\n"));
		convertProtoGrammar(&app_data->tmpMemList, wildParticleGrammar, &tmpGrammar);
		for(t = 0; t < tmpGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(t, &(tmpGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = pushOnStack(&((ElementDescription*) app_data->contextStack->element)->pGrammarStack, (void*) wildParticleGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleElementSequence(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	ProtoGrammar* seqGrammar;
	ProtoGrammar* seqPartGrammar;
	unsigned int minOccurs = 1;
	int32_t maxOccurs = 1;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	minOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MIN_OCCURS]);
	maxOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MAX_OCCURS]);

	tmp_err_code = createSequenceModelGroupsGrammar(&app_data->tmpMemList, elemDesc->pGrammarStack, &seqGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createParticleGrammar(&app_data->tmpMemList, minOccurs, maxOccurs, seqGrammar, &seqPartGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		EXIGrammar* tmpGrammar;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Sequence proto-grammar:\n"));
		convertProtoGrammar(&app_data->tmpMemList, seqPartGrammar, &tmpGrammar);
		for(t = 0; t < tmpGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(t, &(tmpGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = pushOnStack(&((ElementDescription*) app_data->contextStack->element)->pGrammarStack, (void*) seqPartGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleChoiceEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	ProtoGrammar* choiceGrammar;
	ProtoGrammar* choicePartGrammar;
	unsigned int minOccurs = 1;
	int32_t maxOccurs = 1;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	minOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MIN_OCCURS]);
	maxOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MAX_OCCURS]);

	tmp_err_code = createChoiceModelGroupsGrammar(&app_data->tmpMemList, elemDesc->pGrammarStack, &choiceGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createParticleGrammar(&app_data->tmpMemList, minOccurs, maxOccurs, choiceGrammar, &choicePartGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		EXIGrammar* tmpGrammar;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Choice proto-grammar:\n"));
		convertProtoGrammar(&app_data->tmpMemList, choicePartGrammar, &tmpGrammar);
		for(t = 0; t < tmpGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(t, &(tmpGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = pushOnStack(&((ElementDescription*) app_data->contextStack->element)->pGrammarStack, (void*) choicePartGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;

}

static errorCode handleRestrictionEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	ProtoGrammar* simpleRestrictedGrammar;
	QName baseType;
	QNameID baseTypeID;
	TypeFacet* tmpFacet;
	SimpleType newSimpleType;
	ValueType vType;
	size_t elID;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	tmp_err_code = getTypeQName(app_data, elemDesc->attributePointers[ATTRIBUTE_BASE], &baseType);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(isStringEmpty(baseType.uri)) // The type does not have defined namespace -> assume http://www.w3.org/2001/XMLSchema
	{
		baseType.uri = &app_data->metaStringTables->rows[3].string_val;
		baseTypeID.uriRowId = 3;
	}
	else
	{
		tmp_err_code = addURIString(app_data,(String*) baseType.uri, &baseTypeID.uriRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = addLocalName(baseTypeID.uriRowId, app_data,(String*) baseType.localName, &baseTypeID.lnRowId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = getEXIDataTypeFromSimpleType(baseTypeID, &vType);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(vType.simpleTypeID == UINT16_MAX)
	{
		newSimpleType.facetPresenceMask = 0;
		newSimpleType.maxInclusive = 0;
		newSimpleType.minInclusive = 0;
	}
	else
	{
		newSimpleType.facetPresenceMask = ((SimpleType*) app_data->simpleTypesArray->elements)[vType.simpleTypeID].facetPresenceMask;
		// remove the presence of named subtype
		newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask & (~TYPE_FACET_NAMED_SUBTYPE);
		newSimpleType.maxInclusive = ((SimpleType*) app_data->simpleTypesArray->elements)[vType.simpleTypeID].maxInclusive;
		newSimpleType.minInclusive = ((SimpleType*) app_data->simpleTypesArray->elements)[vType.simpleTypeID].minInclusive;
	}

	while(elemDesc->pTypeFacets != NULL)
	{
		popFromStack(&elemDesc->pTypeFacets, (void**) &tmpFacet);

		if(tmpFacet->facetID == TYPE_FACET_MAX_INCLUSIVE)
		{
			newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
			newSimpleType.maxInclusive = tmpFacet->value;
		}
		else if(tmpFacet->facetID == TYPE_FACET_MIN_INCLUSIVE)
		{
			newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
			newSimpleType.maxInclusive = tmpFacet->value;
		}
		else if(tmpFacet->facetID == TYPE_FACET_MAX_LENGTH)
		{
			newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask | TYPE_FACET_MAX_LENGTH;
			newSimpleType.maxLength = (unsigned int) tmpFacet->value;
		}
		else
			return NOT_IMPLEMENTED_YET;
	}

	tmp_err_code = addDynElement(app_data->simpleTypesArray, &newSimpleType, &elID, &app_data->tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	vType.simpleTypeID = (uint16_t) elID;

	tmp_err_code = createSimpleTypeGrammar(&app_data->tmpMemList, vType, &simpleRestrictedGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = pushOnStack(&((ElementDescription*) app_data->contextStack->element)->pGrammarStack, (void*) simpleRestrictedGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleSimpleTypeEl(struct xsdAppData* app_data)
{
	ElementDescription* elemDesc;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammar* simpleTypeGr;
	EXIGrammar* exiTypeGrammar;

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	popFromStack(&elemDesc->pGrammarStack, (void**) &simpleTypeGr);

	if(isStringEmpty(&elemDesc->attributePointers[ATTRIBUTE_NAME]))
	{
		// Anonymous simple type -> there should be a parent element declaration
		tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pGrammarStack), simpleTypeGr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		size_t lnRowID = 0;

		tmp_err_code = assignCodes(simpleTypeGr, app_data->metaStringTables);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = convertProtoGrammar(&app_data->schema->memList, simpleTypeGr, &exiTypeGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLocalName(app_data->props.targetNSMetaID, app_data, &elemDesc->attributePointers[ATTRIBUTE_NAME], &lnRowID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		app_data->schema->initialStringTables->rows[app_data->props.targetNSMetaID].lTable->rows[lnRowID].typeGrammar = exiTypeGrammar;
		app_data->schema->initialStringTables->rows[app_data->props.targetNSMetaID].lTable->rows[lnRowID].typeEmptyGrammar = app_data->schema->initialStringTables->rows[3].lTable->rows[0].typeEmptyGrammar;

		return ERR_OK;
	}

	return ERR_OK;
}

static errorCode handleMinInclusiveEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	TypeFacet* minIncl;
	char buf[100];

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	minIncl = memManagedAllocate(&app_data->tmpMemList, sizeof(TypeFacet));
	if(minIncl == NULL)
		return MEMORY_ALLOCATION_ERROR;

	minIncl->facetID = TYPE_FACET_MIN_INCLUSIVE;
	if(elemDesc->attributePointers[ATTRIBUTE_VALUE].length >= 100)
		return INVALID_OPERATION;
	memcpy(buf, elemDesc->attributePointers[ATTRIBUTE_VALUE].str, elemDesc->attributePointers[ATTRIBUTE_VALUE].length);
	buf[elemDesc->attributePointers[ATTRIBUTE_VALUE].length] = '\0';
	minIncl->value = atoi(buf);

	tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pTypeFacets), minIncl);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleMaxLengthEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	TypeFacet* maxLen;
	char buf[100];

	popFromStack(&(app_data->contextStack), (void**) &elemDesc);

	maxLen = memManagedAllocate(&app_data->tmpMemList, sizeof(TypeFacet));
	if(maxLen == NULL)
		return MEMORY_ALLOCATION_ERROR;

	maxLen->facetID = TYPE_FACET_MAX_LENGTH;
	if(elemDesc->attributePointers[ATTRIBUTE_VALUE].length >= 100)
		return INVALID_OPERATION;
	memcpy(buf, elemDesc->attributePointers[ATTRIBUTE_VALUE].str, elemDesc->attributePointers[ATTRIBUTE_VALUE].length);
	buf[elemDesc->attributePointers[ATTRIBUTE_VALUE].length] = '\0';
	maxLen->value = atoi(buf);

	tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pTypeFacets), maxLen);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

// Only adds it if it is not there yet
static errorCode addURIString(struct xsdAppData* app_data, String* uri, uint16_t* uriRowId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	String uriClone;   // The uri name string is copied to the schema MemList

	if(!lookupURI(app_data->schema->initialStringTables, *uri, uriRowId))
	{
		tmp_err_code = cloneString(uri, &uriClone, &app_data->schema->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = addURIRow(app_data->schema->initialStringTables, uriClone, uriRowId, &app_data->schema->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addURIRow(app_data->metaStringTables, *uri, uriRowId, &app_data->tmpMemList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

// Only adds it if it is not there yet
static errorCode addLocalName(uint16_t uriId, struct xsdAppData* app_data, String* ln, size_t* lnRowId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	String lnClone;   // The local name string is copied to the schema MemList

	if(app_data->schema->initialStringTables->rows[uriId].lTable == NULL)
	{
		tmp_err_code = createLocalNamesTable(&app_data->schema->initialStringTables->rows[uriId].lTable, &app_data->schema->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = cloneString(ln, &lnClone, &app_data->schema->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = addLNRow(app_data->schema->initialStringTables->rows[uriId].lTable, lnClone, lnRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = createLocalNamesTable(&app_data->metaStringTables->rows[uriId].lTable, &app_data->tmpMemList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = addLNRow(app_data->metaStringTables->rows[uriId].lTable, *ln, lnRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(!lookupLN(app_data->schema->initialStringTables->rows[uriId].lTable, *ln, lnRowId))
	{
		tmp_err_code = cloneString(ln, &lnClone, &app_data->schema->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = addLNRow(app_data->schema->initialStringTables->rows[uriId].lTable, lnClone, lnRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLNRow(app_data->metaStringTables->rows[uriId].lTable, *ln, lnRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

static int compareLN(const void* lnRow1, const void* lnRow2)
{
	struct LocalNamesRow* r1 = (struct LocalNamesRow*) lnRow1;
	struct LocalNamesRow* r2 = (struct LocalNamesRow*) lnRow2;

	return stringCompare(r1->string_val, r2->string_val);
}

static int compareURI(const void* uriRow1, const void* uriRow2)
{
	struct URIRow* r1 = (struct URIRow*) uriRow1;
	struct URIRow* r2 = (struct URIRow*) uriRow2;

	return stringCompare(r1->string_val, r2->string_val);
}

static void sortInitialStringTables(URITable* stringTables)
{
	uint16_t i = 0;

	// First sort the local name tables

	for (i = 0; i < stringTables->rowCount; i++)
	{
		unsigned int initialEntries = 0;

		//	The initialEntries entries in "http://www.w3.org/XML/1998/namespace",
		//	"http://www.w3.org/2001/XMLSchema-instance" and "http://www.w3.org/2001/XMLSchema"
		//  are not sorted
		if(i == 1) // "http://www.w3.org/XML/1998/namespace"
		{
			initialEntries = 4;
		}
		else if(i == 2) // "http://www.w3.org/2001/XMLSchema-instance"
		{
			initialEntries = 2;
		}
		else if(i == 3) // "http://www.w3.org/2001/XMLSchema"
		{
			initialEntries = 46;
		}

		if(stringTables->rows[i].lTable != NULL)
			qsort(stringTables->rows[i].lTable->rows + initialEntries, stringTables->rows[i].lTable->rowCount - initialEntries, sizeof(struct LocalNamesRow), compareLN);
	}

	// Then sort the uri tables

	//	The first four initial entries are not sorted
	//	URI	0	"" [empty string]
	//	URI	1	"http://www.w3.org/XML/1998/namespace"
	//	URI	2	"http://www.w3.org/2001/XMLSchema-instance"
	//	URI	3	"http://www.w3.org/2001/XMLSchema"
	qsort(stringTables->rows + 4, stringTables->rowCount - 4, sizeof(struct URIRow), compareURI);
}

static int parseOccuranceAttribute(const String occurance)
{
	// TODO: Just a temporary implementation. Only works for the ASCII string representation. Fix that!
	char buff[20];

	if(isStringEmpty(&occurance))
		return 1; // The default value

	if(stringEqualToAscii(occurance, "unbounded"))
		return -1;

	memcpy(buff, occurance.str, occurance.length);
	buff[occurance.length] = '\0';

	return atoi(buff);
}

static errorCode getTypeQName(struct xsdAppData* app_data, const String typeLiteral, QName* qname)
{
	// TODO: Just a temporary implementation. Only works for the ASCII string representation. Fix that!
	// 		There should be a function in the StringManipulation that search to a character
	int i;
	String* ln;
	String* uri;

	ln = memManagedAllocate(&app_data->tmpMemList, sizeof(String));
	if(ln == NULL)
		return MEMORY_ALLOCATION_ERROR;

	uri = memManagedAllocate(&app_data->tmpMemList, sizeof(String));
	if(uri == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(i = 0; i < (int)(typeLiteral.length); i++)
	{
		if(typeLiteral.str[i] == ':')
		{
			int p;
			unsigned char prefixFound = FALSE;
			struct prefixNS* pns;

			uri->length = i;
			uri->str = typeLiteral.str;

			for(p = 0; p < app_data->props.prefixNamespaces->elementCount; p++)
			{
				pns = &((struct prefixNS*) app_data->props.prefixNamespaces->elements)[p];
				if(stringEqual(*uri, pns->prefix))
				{
					prefixFound = TRUE;
					break;
				}
			}

			if(prefixFound)
			{
				uri = &pns->ns;
			}
			else
			{
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid schema base type definition\n"));
				return INVALID_EXI_INPUT;
			}

			ln->length = typeLiteral.length - i - 1;
			ln->str = &typeLiteral.str[i + 1];

			qname->localName = ln;
			qname->uri = uri;

			return ERR_OK;
		}
	}

	// Else, there are no ':' characters; i.e. no prefix

	getEmptyString(uri);

	ln->length = typeLiteral.length;
	ln->str = typeLiteral.str;

	qname->localName = ln;
	qname->uri = uri;

	return ERR_OK;
}

errorCode generateBuildInTypesGrammars(URITable* sTables, AllocList* memList)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;
	QNameID typeQnameID;
	ValueType vType;
	EXIGrammar* grammar;
	EXIGrammar* typeEmptyGrammar;

	typeEmptyGrammar = memManagedAllocate(memList, sizeof(EXIGrammar));
	if(typeEmptyGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	typeEmptyGrammar->contentIndex = 0;
	typeEmptyGrammar->isNillable = FALSE;
	typeEmptyGrammar->isAugmented = FALSE;
	typeEmptyGrammar->grammarType = GR_TYPE_SCHEMA_EMPTY_TYPE;
	typeEmptyGrammar->rulesDimension = 1;

	// One more rule slot for grammar augmentation when strict == FASLE
	typeEmptyGrammar->ruleArray = memManagedAllocate(memList, sizeof(GrammarRule)*2);
	if(typeEmptyGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	typeEmptyGrammar->ruleArray->bits[0] = 0;
	typeEmptyGrammar->ruleArray->bits[1] = 0;
	typeEmptyGrammar->ruleArray->bits[2] = 0;

	typeEmptyGrammar->ruleArray->prodCounts[0] = 1;
	typeEmptyGrammar->ruleArray->prodCounts[1] = 0;
	typeEmptyGrammar->ruleArray->prodCounts[2] = 0;

	typeEmptyGrammar->ruleArray->prodArrays[0] = memManagedAllocate(memList, sizeof(Production));
	if(typeEmptyGrammar->ruleArray->prodArrays[0] == NULL)
		return MEMORY_ALLOCATION_ERROR;

	typeEmptyGrammar->ruleArray->prodArrays[1] = NULL;
	typeEmptyGrammar->ruleArray->prodArrays[2] = NULL;

	typeEmptyGrammar->ruleArray->prodArrays[0][0].event = getEventDefType(EVENT_EE);
	typeEmptyGrammar->ruleArray->prodArrays[0][0].uriRowID = UINT16_MAX;
	typeEmptyGrammar->ruleArray->prodArrays[0][0].lnRowID = SIZE_MAX;
	typeEmptyGrammar->ruleArray->prodArrays[0][0].nonTermID = GR_VOID_NON_TERMINAL;

	// URI id 3 -> http://www.w3.org/2001/XMLSchema
	typeQnameID.uriRowId = 3;

	for(i = 0; i < sTables->rows[3].lTable->rowCount; i++)
	{
		typeQnameID.lnRowId = i;
		tmp_err_code = getEXIDataTypeFromSimpleType(typeQnameID, &vType);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		grammar = memManagedAllocate(memList, sizeof(EXIGrammar));
		if(grammar == NULL)
			return MEMORY_ALLOCATION_ERROR;

		grammar->contentIndex = 0;
		grammar->grammarType = GR_TYPE_SCHEMA_TYPE;
		grammar->isNillable = FALSE;
		grammar->isAugmented = FALSE;
		grammar->rulesDimension = 2;

		// One more rule slot for grammar augmentation when strict == FASLE
		grammar->ruleArray = memManagedAllocate(memList, sizeof(GrammarRule)*(grammar->rulesDimension + 1));
		if(grammar->ruleArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		grammar->ruleArray[0].bits[0] = 0;
		grammar->ruleArray[0].bits[1] = 0;
		grammar->ruleArray[0].bits[2] = 0;
		grammar->ruleArray[0].prodCounts[0] = 1;
		grammar->ruleArray[0].prodCounts[1] = 0;
		grammar->ruleArray[0].prodCounts[2] = 0;

		grammar->ruleArray[0].prodArrays[0] = memManagedAllocate(memList, sizeof(Production));
		if(grammar->ruleArray[0].prodArrays[0] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		grammar->ruleArray[0].prodArrays[1] = NULL;
		grammar->ruleArray[0].prodArrays[2] = NULL;

		grammar->ruleArray[0].prodArrays[0][0].event.eventType = EVENT_CH;
		grammar->ruleArray[0].prodArrays[0][0].event.valueType = vType;
		grammar->ruleArray[0].prodArrays[0][0].nonTermID = 1;
		grammar->ruleArray[0].prodArrays[0][0].uriRowID = UINT16_MAX;
		grammar->ruleArray[0].prodArrays[0][0].lnRowID = SIZE_MAX;

		grammar->ruleArray[1].bits[0] = 0;
		grammar->ruleArray[1].bits[1] = 0;
		grammar->ruleArray[1].bits[2] = 0;
		grammar->ruleArray[1].prodCounts[0] = 1;
		grammar->ruleArray[1].prodCounts[1] = 0;
		grammar->ruleArray[1].prodCounts[2] = 0;

		grammar->ruleArray[1].prodArrays[0] = memManagedAllocate(memList, sizeof(Production));
		if(grammar->ruleArray[1].prodArrays[0] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		grammar->ruleArray[1].prodArrays[1] = NULL;
		grammar->ruleArray[1].prodArrays[2] = NULL;

		grammar->ruleArray[1].prodArrays[0][0].event.eventType = EVENT_EE;
		grammar->ruleArray[1].prodArrays[0][0].event.valueType.exiType = VALUE_TYPE_NONE;
		grammar->ruleArray[1].prodArrays[0][0].event.valueType.simpleTypeID = UINT16_MAX;
		grammar->ruleArray[1].prodArrays[0][0].nonTermID = GR_VOID_NON_TERMINAL;
		grammar->ruleArray[1].prodArrays[0][0].uriRowID = UINT16_MAX;
		grammar->ruleArray[1].prodArrays[0][0].lnRowID = SIZE_MAX;

		sTables->rows[3].lTable->rows[i].typeGrammar = grammar;
		sTables->rows[3].lTable->rows[i].typeEmptyGrammar = typeEmptyGrammar;
	}

	return ERR_OK;
}
