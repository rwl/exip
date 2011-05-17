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

#define ELEMENT_VOID           255


/** Codes for the attributes found in the schema */
#define ATTRIBUTE_ABSENT     0
#define ATTRIBUTE_NAME       1
#define ATTRIBUTE_TYPE       2
#define ATTRIBUTE_REF        3
#define ATTRIBUTE_MIN_OCCURS 4
#define ATTRIBUTE_MAX_OCCURS 5
#define ATTRIBUTE_FORM       6
#define ATTRIBUTE_BASE       7
#define ATTRIBUTE_USE        8

#define ATTRIBUTE_VOID     255

#define ATTRIBUTE_CONTEXT_ARRAY_SIZE 20


#define INITIAL_STATE         0
#define SCHEMA_ELEMENT_STATE  1
#define SCHEMA_CONTENT_STATE  2

#define FORM_DEF_UNQUALIFIED   0
#define FORM_DEF_QUALIFIED     1
#define FORM_DEF_EXPECTING     2
#define FORM_DEF_INITIAL_STATE 3

/**
 * Global schema properties (found as an attributes of the schema root element in XSD)
 * They should not change over time of processing
 */
struct globalSchemaProps {
	unsigned char propsStat; // 0 - initial state, 1 - <schema> element is parsed expect attributes, 2 - the properties are all set (<schema> attr. parsed)
	unsigned char expectAttributeData;
	StringType* charDataPointer; // Pointer to the expected character data
	StringType targetNamespace;
	uint16_t targetNSMetaID;  // the uri row ID in the metaURI table of the targetNamespace
	unsigned char attributeFormDefault; // 0 unqualified, 1 qualified, 2 expecting value, 3 initial state
	unsigned char elementFormDefault;  // 0 unqualified, 1 qualified, 2 expecting value, 3 initial state
	StringType emptyString; // A holder for the empty string constant used throughout the generation
};

/**
 * An entry of nested schema descriptions. It is used to store the
 * current context when passing through schema document
 */
struct elementDescr {
	unsigned char element;  // represented with codes defined above
	StringType attributePointers[ATTRIBUTE_CONTEXT_ARRAY_SIZE]; // the index is the code of the attribute
	GenericStack* pGrammarStack; // The proto-grammars created so far and connected to this elemDescr
	DynArray* attributeUses; // For complex types/content this array stores the attribute uses
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
	ExipSchema* schema;
};

// Content Handler API
static char xsd_fatalError(const char code, const char* msg, void* app_data);
static char xsd_startDocument(void* app_data);
static char xsd_endDocument(void* app_data);
static char xsd_startElement(QName qname, void* app_data);
static char xsd_endElement(void* app_data);
static char xsd_attribute(QName qname, void* app_data);
static char xsd_stringData(const StringType value, void* app_data);
static char xsd_exiHeader(const EXIheader* header, void* app_data);

static void initElemContext(ElementDescription* elem);

// Handling of schema elements

static errorCode handleAttributeEl(struct xsdAppData* app_data);

static errorCode handleExtentionEl(struct xsdAppData* app_data);

static errorCode handleSimpleContentEl(struct xsdAppData* app_data);

static errorCode handleComplexTypeEl(struct xsdAppData* app_data);

static errorCode handleElementEl(struct xsdAppData* app_data);

static errorCode handleElementSequence(struct xsdAppData* app_data);


//////////// Helper functions

static errorCode addLocalName(uint16_t uriId, struct xsdAppData* app_data, StringType* ln, size_t* lnRowId);

static void sortInitialStringTables(URITable* stringTables);

static int compareLN(const void* lnRow1, const void* lnRow2);

static int compareURI(const void* uriRow1, const void* uriRow2);

static int parseOccuranceAttribute(const StringType occurance);

static errorCode getTypeQName(AllocList* memList, const StringType typeLiteral, QName* qname);

////////////

errorCode generateSchemaInformedGrammars(char* binaryBuf, size_t bufLen, size_t bufContent, IOStream* ioStrm,
										unsigned char schemaFormat, ExipSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ContentHandler xsdHandler;
	struct xsdAppData parsing_data;

	if(schemaFormat != SCHEMA_FORMAT_XSD_EXI)
		return NOT_IMPLEMENTED_YET;

	initAllocList(&parsing_data.tmpMemList);
	initAllocList(&schema->memList);

	initContentHandler(&xsdHandler);
	xsdHandler.fatalError = xsd_fatalError;
	xsdHandler.error = xsd_fatalError;
	xsdHandler.startDocument = xsd_startDocument;
	xsdHandler.endDocument = xsd_endDocument;
	xsdHandler.startElement = xsd_startElement;
	xsdHandler.attribute = xsd_attribute;
	xsdHandler.stringData = xsd_stringData;
	xsdHandler.endElement = xsd_endElement;
	xsdHandler.exiHeader = xsd_exiHeader;

	parsing_data.props.propsStat = INITIAL_STATE;
	parsing_data.props.expectAttributeData = FALSE;
	parsing_data.props.attributeFormDefault = FORM_DEF_INITIAL_STATE;
	parsing_data.props.elementFormDefault = FORM_DEF_INITIAL_STATE;
	parsing_data.props.charDataPointer = NULL;
	parsing_data.props.targetNSMetaID = 0;
	tmp_err_code = getEmptyString(&parsing_data.props.emptyString);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = getEmptyString(&parsing_data.props.targetNamespace);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

	parsing_data.contextStack = NULL;

	tmp_err_code = createDynArray(&parsing_data.elNotResolvedArray, sizeof(struct elementNotResolved), 10, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.globalElemGrammars, sizeof(QNameID), 10, &schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createURITable(&schema->initialStringTables, &schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	createInitialEntries(&schema->memList, schema->initialStringTables, TRUE);

	tmp_err_code = createURITable(&parsing_data.metaStringTables, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	createInitialEntries(&parsing_data.tmpMemList, parsing_data.metaStringTables, TRUE);

	parsing_data.schema = schema;

	// Parse the EXI stream
	parseEXI(binaryBuf, bufLen, bufContent, ioStrm, &xsdHandler, NULL, &parsing_data);

	return ERR_OK;
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
			tmpGrammar = appD->schema->initialStringTables->rows[tmpQnameID->uriRowId].lTable->rows[tmpQnameID->lnRowId].globalGrammar;
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
			tmpGrammar = appD->schema->initialStringTables->rows[i].lTable->rows[j].globalGrammar;
			if(tmpGrammar != NULL)
			{
				for (t = 0; t < tmpGrammar->rulesDimension; t++)
				{
					for (p = 0; p < tmpGrammar->ruleArray[t].prodCounts[0]; p++)
					{
						tmpProd = &tmpGrammar->ruleArray[t].prodArrays[0][p];
						if(tmpProd->uriRowID != UINT16_MAX)
						{
							lookupURI(appD->schema->initialStringTables, appD->metaStringTables->rows[tmpProd->uriRowID].string_val, &uriRowID);
							tmpProd->uriRowID = t;

							lookupLN(appD->schema->initialStringTables->rows[t].lTable, appD->metaStringTables->rows[t].lTable->rows[tmpProd->lnRowID].string_val, &lnRowID);
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
		if(strEqualToAscii(*qname.uri, XML_SCHEMA_NAMESPACE) &&
				strEqualToAscii(*qname.localName, "schema"))
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

			if(!isStrEmpty(&appD->props.targetNamespace)) // Add the declared target namespace in the String Tables
			{
				uint16_t uriID;

				// If the target namespace is not in the initial uri entries add it
				if(!lookupURI(appD->schema->initialStringTables, appD->props.targetNamespace, &uriID))
				{
					StringType tnsClone;
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

		if(!strEqualToAscii(*qname.uri, XML_SCHEMA_NAMESPACE))
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

		if(strEqualToAscii(*qname.localName, "element"))
		{
			element->element = ELEMENT_ELEMENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <element> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "attribute"))
		{
			element->element = ELEMENT_ATTRIBUTE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <attribute> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "choice"))
		{
			element->element = ELEMENT_CHOICE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <choice> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "complexType"))
		{
			element->element = ELEMENT_COMPLEX_TYPE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <complexType> element\n"));
			tmp_err_code = createDynArray(&element->attributeUses, sizeof(ProtoGrammar), 5, &appD->tmpMemList);
			if(tmp_err_code != ERR_OK)
				return EXIP_HANDLER_STOP;
		}
		else if(strEqualToAscii(*qname.localName, "complexContent"))
		{
			element->element = ELEMENT_COMPLEX_CONTENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <complexContent> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "group"))
		{
			element->element = ELEMENT_GROUP;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <group> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "import"))
		{
			element->element = ELEMENT_IMPORT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <import> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "sequence"))
		{
			element->element = ELEMENT_SEQUENCE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <sequence> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "all"))
		{
			element->element = ELEMENT_ALL;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <all> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "extension"))
		{
			element->element = ELEMENT_EXTENSION;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <extension> element\n"));
			tmp_err_code = createDynArray(&element->attributeUses, sizeof(ProtoGrammar), 5, &appD->tmpMemList);
			if(tmp_err_code != ERR_OK)
				return EXIP_HANDLER_STOP;
		}
		else if(strEqualToAscii(*qname.localName, "restriction"))
		{
			element->element = ELEMENT_RESTRICTION;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <restriction> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "simpleContent"))
		{
			element->element = ELEMENT_SIMPLE_CONTENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <simpleContent> element\n"));
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored schema element\n"));
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
		else if(element->element == ELEMENT_SEQUENCE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </sequence> element\n"));
			tmp_err_code = handleElementSequence(appD);
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored closing element\n"));
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
		if(strEqualToAscii(*qname.localName, "targetNamespace"))
		{
			appD->props.charDataPointer = &(appD->props.targetNamespace);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |targetNamespace| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "elementFormDefault"))
		{
			appD->props.elementFormDefault = FORM_DEF_EXPECTING;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |elementFormDefault| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "attributeFormDefault"))
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

		if(strEqualToAscii(*qname.localName, "name"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_NAME]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |name| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "type"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_TYPE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |type| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "ref"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_REF]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |ref| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "minOccurs"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_MIN_OCCURS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |minOccurs| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "maxOccurs"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_MAX_OCCURS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |maxOccurs| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "form"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_FORM]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |form| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "base"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_BASE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |base| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "use"))
		{
			appD->props.charDataPointer = &(element->attributePointers[ATTRIBUTE_USE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |use| \n"));
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element attribute\n"));
		}
	}
	appD->props.expectAttributeData = TRUE;
	return EXIP_HANDLER_OK;
}

static char xsd_stringData(const StringType value, void* app_data)
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
				if(strEqualToAscii(value, "qualified"))
					appD->props.elementFormDefault = FORM_DEF_QUALIFIED;
				else if(strEqualToAscii(value, "unqualified"))
					appD->props.elementFormDefault = FORM_DEF_UNQUALIFIED;
				else
				{
					DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid value for elementFormDefault attribute\n"));
					return EXIP_HANDLER_STOP;
				}
			}
			else if(appD->props.attributeFormDefault == FORM_DEF_EXPECTING) // expecting value for attributeFormDefault
			{
				if(strEqualToAscii(value, "qualified"))
					appD->props.attributeFormDefault = FORM_DEF_QUALIFIED;
				else if(strEqualToAscii(value, "unqualified"))
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

static char xsd_exiHeader(const EXIheader* header, void* app_data)
{
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">XML Schema header parsed\n"));
	return EXIP_HANDLER_OK;
}

static void initElemContext(ElementDescription* elem)
{
	unsigned int i = 0;
	elem->element = ELEMENT_VOID;
	elem->pGrammarStack = NULL;
	elem->attributeUses = NULL;
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
	StringType* target_ns;
	QName simpleType;
	QName scope;
	ProtoGrammar* attrUseGrammar;
	size_t attrUseGrammarID;
	ElementDescription* elemDesc;
	size_t lnRowID;
	uint16_t uriRowID;
	StringType* attrName;

	tmp_err_code = popFromStack(&(app_data->contextStack), (void**) &elemDesc);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if (!isStrEmpty(&(elemDesc->attributePointers[ATTRIBUTE_USE])) &&
			strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_USE], "required"))
	{
		required = 1;
	}
	if(app_data->props.attributeFormDefault == FORM_DEF_QUALIFIED || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
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


	simpleType.localName = &(elemDesc->attributePointers[ATTRIBUTE_TYPE]);
	simpleType.uri =  &app_data->props.emptyString;

	scope.localName = &app_data->props.emptyString;
	scope.uri = &app_data->props.emptyString;

	tmp_err_code = createAttributeUseGrammar(&app_data->tmpMemList, required, attrName,
											 target_ns, simpleType, scope, &attrUseGrammar, uriRowID, lnRowID);
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
	StringType* typeName;
	StringType* target_ns;
	ProtoGrammar* simpleTypeGrammar;
	ElementDescription* elemDesc;
	ProtoGrammar* resultComplexGrammar;

	tmp_err_code = popFromStack(&(app_data->contextStack), (void**) &elemDesc);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	typeName = &app_data->props.emptyString;
	target_ns = &app_data->props.emptyString;

	simpleType.localName = &(elemDesc->attributePointers[ATTRIBUTE_BASE]);
	simpleType.uri = &app_data->props.emptyString;

	tmp_err_code = createSimpleTypeGrammar(&app_data->tmpMemList, simpleType, &simpleTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// TODO: the attributeUses array must be sorted first before calling createComplexTypeGrammar()
	tmp_err_code = createComplexTypeGrammar(&app_data->tmpMemList, typeName, target_ns,
			(ProtoGrammar*) elemDesc->attributeUses->elements, (unsigned int)(elemDesc->attributeUses->elementCount),
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

	tmp_err_code = popFromStack(&(app_data->contextStack), (void**) &elemDesc);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;


	tmp_err_code = popFromStack(&elemDesc->pGrammarStack, (void**) &contentGr);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pGrammarStack), contentGr);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	return ERR_OK;
}

static errorCode handleComplexTypeEl(struct xsdAppData* app_data)
{
	// TODO: The attribute uses must be sorted first

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	StringType* typeName;
	StringType* target_ns;
	ProtoGrammar* contentTypeGrammar;
	ProtoGrammar* resultComplexGrammar;
	ElementDescription* elemDesc;
	size_t lnRowID;
	uint16_t uriRowID;

	tmp_err_code = popFromStack(&(app_data->contextStack), (void**) &elemDesc);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	typeName = &(elemDesc->attributePointers[ATTRIBUTE_NAME]);

	if(app_data->props.elementFormDefault == FORM_DEF_QUALIFIED || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
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

	if(!isStrEmpty(typeName))
	{
		tmp_err_code = addLocalName(uriRowID, app_data, typeName, &lnRowID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}


	tmp_err_code = popFromStack(&(elemDesc->pGrammarStack), (void**) &contentTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// TODO: the attributeUses array must be sorted first before calling createComplexTypeGrammar()

	tmp_err_code = createComplexTypeGrammar(&app_data->tmpMemList, typeName, target_ns,
			(ProtoGrammar*) elemDesc->attributeUses->elements, (unsigned int)(elemDesc->attributeUses->elementCount),
			                           NULL, 0, contentTypeGrammar, &resultComplexGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

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

	if(isStrEmpty(typeName))  // The name is empty i.e. anonymous complex type
	{
		// Put the ComplexTypeGrammar on top of the pGrammarStack
		// There should be a parent <element> declaration for this grammar
		tmp_err_code = pushOnStack(&(((ElementDescription*) app_data->contextStack->element)->pGrammarStack), resultComplexGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // Named complex type - put it directly in the typeGrammars list
	{
		EXIGrammar* complexEXIGrammar;
		tmp_err_code = convertProtoGrammar(&app_data->schema->memList, resultComplexGrammar, &complexEXIGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = assignCodes(complexEXIGrammar);
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

		app_data->schema->initialStringTables->rows[uriRowID].lTable->rows[lnRowID].globalGrammar = complexEXIGrammar;
	}

	return ERR_OK;
}

static errorCode handleElementEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	StringType type;
	ProtoGrammar* typeGrammar;
	StringType* elName;
	StringType* target_ns;
	unsigned char isGlobal = 0;
	uint16_t uriRowId;
	size_t lnRowId;
	EXIGrammar* exiTypeGrammar;

	tmp_err_code = popFromStack(&(app_data->contextStack), (void**) &elemDesc);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	type = elemDesc->attributePointers[ATTRIBUTE_TYPE];

	if(app_data->contextStack == NULL) // Global element
		isGlobal = TRUE;

	if(isGlobal || app_data->props.elementFormDefault == FORM_DEF_QUALIFIED || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
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

	if(isGlobal)
	{
		QNameID globalQnameID;
		size_t dynElID;

		if(isStrEmpty(&type))  // There is no type attribute i.e. there must be some complex type in the pGrammarStack
		{
			tmp_err_code = popFromStack(&(elemDesc->pGrammarStack), (void**) &typeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else // The element has a particular named type
		{
			return NOT_IMPLEMENTED_YET;
		}

		tmp_err_code = convertProtoGrammar(&app_data->schema->memList, typeGrammar, &exiTypeGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = assignCodes(exiTypeGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

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

		app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].globalGrammar = exiTypeGrammar;
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

		if(isStrEmpty(&type))  // There is no type attribute i.e. there must be some complex type in the pGrammarStack
		{
			tmp_err_code = popFromStack(&(elemDesc->pGrammarStack), (void**) &typeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = convertProtoGrammar(&app_data->schema->memList, typeGrammar, &exiTypeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = assignCodes(exiTypeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

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

			app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].globalGrammar = exiTypeGrammar;
		}
		else // The element has a particular named type
		{
			tmp_err_code = getTypeQName(&app_data->tmpMemList, type, &typeQname);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(isStrEmpty(typeQname.uri) || strEqualToAscii(*typeQname.uri, XML_SCHEMA_NAMESPACE)) // This is simple type definition
			{
				tmp_err_code = createSimpleTypeGrammar(&app_data->tmpMemList, typeQname, &typeGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = convertProtoGrammar(&app_data->schema->memList, typeGrammar, &exiTypeGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = assignCodes(exiTypeGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				app_data->schema->initialStringTables->rows[uriRowId].lTable->rows[lnRowId].globalGrammar = exiTypeGrammar;
			}
			else // A complex type name
			{
				return NOT_IMPLEMENTED_YET;
			}
		}

		minOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MIN_OCCURS]);
		maxOccurs = parseOccuranceAttribute(elemDesc->attributePointers[ATTRIBUTE_MAX_OCCURS]);

		tmp_err_code = createElementTermGrammar(&app_data->tmpMemList, elName, target_ns, &elTermGrammar, uriRowId, lnRowId);
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

static errorCode handleElementSequence(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ElementDescription* elemDesc;
	ProtoGrammar* seqGrammar;
	ProtoGrammar* seqPartGrammar;
	unsigned int minOccurs = 1;
	int32_t maxOccurs = 1;

	tmp_err_code = popFromStack(&(app_data->contextStack), (void**) &elemDesc);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

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

// Only adds it if it is not there yet
static errorCode addLocalName(uint16_t uriId, struct xsdAppData* app_data, StringType* ln, size_t* lnRowId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	StringType lnClone;   // The local name string is copied to the schema MemList

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

	return str_compare(r1->string_val, r2->string_val);
}

static int compareURI(const void* uriRow1, const void* uriRow2)
{
	struct URIRow* r1 = (struct URIRow*) uriRow1;
	struct URIRow* r2 = (struct URIRow*) uriRow2;

	return str_compare(r1->string_val, r2->string_val);
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

static int parseOccuranceAttribute(const StringType occurance)
{
	// TODO: Just a temporary implementation. Only works for the ASCII string representation. Fix that!
	char buff[20];

	if(isStrEmpty(&occurance))
		return 1; // The default value

	if(strEqualToAscii(occurance, "unbounded"))
		return -1;

	memcpy(buff, occurance.str, occurance.length);
	buff[occurance.length] = '\0';

	return atoi(buff);
}

static errorCode getTypeQName(AllocList* memList, const StringType typeLiteral, QName* qname)
{
	// TODO: Just a temporary implementation. Only works for the ASCII string representation. Fix that!
	int i;
	StringType* ln;
	StringType* uri;

	ln = memManagedAllocate(memList, sizeof(StringType));
	if(ln == NULL)
		return MEMORY_ALLOCATION_ERROR;

	uri = memManagedAllocate(memList, sizeof(StringType));
	if(uri == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(i = 0; i < (int)(typeLiteral.length); i++)
	{
		if(typeLiteral.str[i] == ':')
		{
			uri->length = i;
			uri->str = typeLiteral.str;

			ln->length = typeLiteral.length - i - 1;
			ln->str = &typeLiteral.str[i + 1];

			qname->localName = ln;
			qname->uri = uri;

			return ERR_OK;
		}
	}

	// Else, there are not ':' character; i.e. no prefix

	if(getEmptyString(uri) != ERR_OK)
		return UNEXPECTED_ERROR;

	ln->length = typeLiteral.length;
	ln->str = typeLiteral.str;

	qname->localName = ln;
	qname->uri = uri;

	return ERR_OK;
}
