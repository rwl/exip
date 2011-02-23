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
#include "normGrammar.h"
#include "string.h"
#include "grammarAugment.h"

struct metaGrammarNode
{
	StringType uri;
	StringType ln;
	struct EXIGrammar* grammar;
	struct metaGrammarNode* nextNode;
};

struct metaGrammarList
{
	struct metaGrammarNode* first;
	struct metaGrammarNode* last;
	unsigned int size;
};

typedef struct metaGrammarList MetaGrammarList;

struct xsdAppData
{
	struct globalSchemaProps props;
	AllocList tmpMemList;   			// Temporary allocations during the schema creation
	ContextStack* contextStack;
	ProtoGrammarsStack* pGrammarStack;
	DynArray* elNotResolvedArray;
	DynArray* attributeUses;
	DynArray* regProdQname;
	MetaGrammarList globalElemGrammars; // Sorting while adding
	MetaGrammarList subElementGrammars;
	MetaGrammarList typeGrammars;
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

static void pushElemContext(ContextStack** cStack, struct elementDescr* elem);

static void popElemContext(ContextStack** cStack, struct elementDescr** elem);

static void initElemContext(struct elementDescr* elem);

// Handling of schema elements

static errorCode handleAttributeEl(struct xsdAppData* app_data);

static errorCode handleExtentionEl(struct xsdAppData* app_data);

static errorCode handleSimpleContentEl(struct xsdAppData* app_data);

static errorCode handleComplexTypeEl(struct xsdAppData* app_data);

static errorCode handleElementEl(struct xsdAppData* app_data);


//////////// Helper functions

static errorCode appendMetaGrammarNode(AllocList* tmpMemList, MetaGrammarList* gList, struct EXIGrammar* grammar, StringType name, StringType ns);

static errorCode orderedAddMetaGrammarNode(AllocList* tmpMemList, MetaGrammarList* gList, struct EXIGrammar* grammar, StringType name, StringType ns);

static errorCode addLocalName(uint16_t uriId, AllocList* memList, URITable* stringTables, StringType ln);

static void sortInitialStringTables(URITable* stringTables);

static int compareLN(const void* lnRow1, const void* lnRow2);

static int compareURI(const void* uriRow1, const void* uriRow2);

////////////

errorCode generateSchemaInformedGrammars(char* binaryStream, size_t bufLen, unsigned char schemaFormat,
										 ExipSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ContentHandler xsdHandler;
	struct xsdAppData parsing_data;

	if(schemaFormat != SCHEMA_FORMAT_XSD_EXI)
		return NOT_IMPLEMENTED_YET;

	initAllocList(&parsing_data.tmpMemList);

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

	parsing_data.contextStack = NULL;

	parsing_data.pGrammarStack = NULL;

	tmp_err_code = createDynArray(&parsing_data.elNotResolvedArray, sizeof(struct elementNotResolved), 10, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.attributeUses, sizeof(struct EXIGrammar), 5, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.regProdQname, sizeof(struct productionQname), 20, &parsing_data.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	parsing_data.props.targetNSMetaID = 0;

	parsing_data.globalElemGrammars.first = NULL;
	parsing_data.globalElemGrammars.last = NULL;
	parsing_data.globalElemGrammars.size = 0;

	parsing_data.subElementGrammars.first = NULL;
	parsing_data.subElementGrammars.last = NULL;
	parsing_data.subElementGrammars.size = 0;

	parsing_data.typeGrammars.first = NULL;
	parsing_data.typeGrammars.last = NULL;
	parsing_data.typeGrammars.size = 0;

	initAllocList(&schema->memList);

	tmp_err_code = createURITable(&schema->initialStringTables, &schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	createInitialEntries(&schema->memList, schema->initialStringTables, TRUE);

	parsing_data.schema = schema;

	// Parse the EXI stream
	parseEXI(binaryStream, bufLen, &xsdHandler, &parsing_data);

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
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i = 0;
	unsigned int j = 0;
	struct productionQname* tmpPQ;
	struct metaGrammarNode* tmpGNode;
	uint16_t uriRowID;
	size_t lnRowID;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End XML Schema parsing\n"));

// Only for debugging purposes
#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		struct metaGrammarNode* tmp = appD->globalElemGrammars.first;

		while(tmp)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\nURI: "));
			printString(&tmp->uri);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\nLN: "));
			printString(&tmp->ln);

			for(t = 0; t < tmp->grammar->rulesDimension; t++)
			{
				tmp_err_code = printGrammarRule(&(tmp->grammar->ruleArray[t]));
				if(tmp_err_code != ERR_OK)
				{
					DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">printGrammarRule() fail\n"));
					return EXIP_HANDLER_STOP;
				}
			}
			tmp = tmp->nextNode;
		}
	}
#endif

	sortInitialStringTables(appD->schema->initialStringTables);

	for(i = 0; i < appD->regProdQname->elementCount; i++)
	{
		tmpPQ = ((struct productionQname*) appD->regProdQname->elements) + i;
		lookupURI(appD->schema->initialStringTables, *(tmpPQ->qname.uri), &uriRowID);
		*(tmpPQ->p_uriRowID) = uriRowID;
		lookupLN(appD->schema->initialStringTables->rows[uriRowID].lTable, *(tmpPQ->qname.localName), &lnRowID);
		*(tmpPQ->p_lnRowID) = lnRowID;
	}

	appD->schema->globalElemGrammars.count = appD->globalElemGrammars.size;
	appD->schema->globalElemGrammars.elems = (GrammarDescr*) memManagedAllocate(&appD->schema->memList, sizeof(GrammarDescr)*appD->globalElemGrammars.size);
	tmpGNode = appD->globalElemGrammars.first;
	for(i = 0; i < appD->globalElemGrammars.size; i++)
	{
		appD->schema->globalElemGrammars.elems[i].grammar.lastNonTermID = tmpGNode->grammar->lastNonTermID;
		appD->schema->globalElemGrammars.elems[i].grammar.nextInStack = tmpGNode->grammar->nextInStack;
		appD->schema->globalElemGrammars.elems[i].grammar.rulesDimension = tmpGNode->grammar->rulesDimension;
		appD->schema->globalElemGrammars.elems[i].grammar.ruleArray = (GrammarRule*) memManagedAllocate(&appD->schema->memList, sizeof(GrammarRule) * tmpGNode->grammar->rulesDimension);
		for(j = 0; j < tmpGNode->grammar->rulesDimension; j++)
		{
			tmp_err_code = copyGrammarRule(&appD->schema->memList, &tmpGNode->grammar->ruleArray[j], &appD->schema->globalElemGrammars.elems[i].grammar.ruleArray[j], 0);
			if(tmp_err_code != ERR_OK)
			{
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Schema parsing error: %d\n", tmp_err_code));
				return EXIP_HANDLER_STOP;
			}
		}
		lookupURI(appD->schema->initialStringTables, tmpGNode->uri, &uriRowID);
		appD->schema->globalElemGrammars.elems[i].uriRowId = uriRowID;
		lookupLN(appD->schema->initialStringTables->rows[uriRowID].lTable, tmpGNode->ln, &lnRowID);
		appD->schema->globalElemGrammars.elems[i].lnRowId = lnRowID;
		tmpGNode = tmpGNode->nextNode;
	}

	// TODO: the same for the type grammars and subelement grammars
	appD->schema->subElementGrammars.count = 0;

	freeAllocList(&appD->tmpMemList);
	return EXIP_HANDLER_OK;
}

static char xsd_startElement(QName qname, void* app_data)
{
	struct xsdAppData* appD = (struct xsdAppData*) app_data;
	if(appD->props.propsStat == INITIAL_STATE) // This should be the first <schema> element
	{
		if(strEqualToAscii(*qname.uri, "http://www.w3.org/2001/XMLSchema") &&
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
		struct elementDescr* elem;
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
				errorCode tmp_err_code = UNEXPECTED_ERROR;

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
				}
				appD->props.targetNSMetaID = uriID;
			}
		}

		if(!strEqualToAscii(*qname.uri, "http://www.w3.org/2001/XMLSchema"))
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid namespace of XML Schema element\n"));
			return EXIP_HANDLER_STOP;
		}

		elem = (struct elementDescr*) memManagedAllocate(&appD->tmpMemList, sizeof(struct elementDescr));
		if(elem == NULL)
			return MEMORY_ALLOCATION_ERROR;

		initElemContext(elem);

		if(strEqualToAscii(*qname.localName, "element"))
		{
			elem->element = ELEMENT_ELEMENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <element> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "attribute"))
		{
			elem->element = ELEMENT_ATTRIBUTE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <attribute> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "choice"))
		{
			elem->element = ELEMENT_CHOICE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <choice> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "complexType"))
		{
			elem->element = ELEMENT_COMPLEX_TYPE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <complexType> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "complexContent"))
		{
			elem->element = ELEMENT_COMPLEX_CONTENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <complexContent> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "group"))
		{
			elem->element = ELEMENT_GROUP;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <group> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "import"))
		{
			elem->element = ELEMENT_IMPORT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <import> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "sequence"))
		{
			elem->element = ELEMENT_SEQUENCE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <sequence> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "all"))
		{
			elem->element = ELEMENT_ALL;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <all> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "extension"))
		{
			elem->element = ELEMENT_EXTENSION;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <extension> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "restriction"))
		{
			elem->element = ELEMENT_RESTRICTION;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <restriction> element\n"));
		}
		else if(strEqualToAscii(*qname.localName, "simpleContent"))
		{
			elem->element = ELEMENT_SIMPLE_CONTENT;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <simpleContent> element\n"));
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored schema element\n"));
		}

		pushElemContext(&(appD->contextStack), elem);
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
		if(appD->contextStack->element == ELEMENT_ATTRIBUTE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </attribute> element\n"));
			tmp_err_code = handleAttributeEl(appD);
		}
		else if(appD->contextStack->element == ELEMENT_EXTENSION)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </extension> element\n"));
			tmp_err_code = handleExtentionEl(appD);
		}
		else if(appD->contextStack->element == ELEMENT_SIMPLE_CONTENT)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </simpleContent> element\n"));
			tmp_err_code = handleSimpleContentEl(appD);
		}
		else if(appD->contextStack->element == ELEMENT_COMPLEX_TYPE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </complexType> element\n"));
			tmp_err_code = handleComplexTypeEl(appD);
		}
		else if(appD->contextStack->element == ELEMENT_ELEMENT)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </element> element\n"));
			tmp_err_code = handleElementEl(appD);
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
		if(strEqualToAscii(*qname.localName, "name"))
		{
			appD->props.charDataPointer = &(appD->contextStack->attributePointers[ATTRIBUTE_NAME]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |name| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "type"))
		{
			appD->props.charDataPointer = &(appD->contextStack->attributePointers[ATTRIBUTE_TYPE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |type| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "ref"))
		{
			appD->props.charDataPointer = &(appD->contextStack->attributePointers[ATTRIBUTE_REF]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |ref| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "minOccurs"))
		{
			appD->props.charDataPointer = &(appD->contextStack->attributePointers[ATTRIBUTE_MIN_OCCURS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |minOccurs| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "maxOccurs"))
		{
			appD->props.charDataPointer = &(appD->contextStack->attributePointers[ATTRIBUTE_MAX_OCCURS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |maxOccurs| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "form"))
		{
			appD->props.charDataPointer = &(appD->contextStack->attributePointers[ATTRIBUTE_FORM]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |form| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "base"))
		{
			appD->props.charDataPointer = &(appD->contextStack->attributePointers[ATTRIBUTE_BASE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |base| \n"));
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

void pushElemContext(ContextStack** cStack, struct elementDescr* elem)
{
	elem->nextInStack = *cStack;
	*cStack = elem;
}

void popElemContext(ContextStack** cStack, struct elementDescr** elem)
{
	*elem = *cStack;
	*cStack = (*cStack)->nextInStack;
	(*elem)->nextInStack = NULL;
}

static void initElemContext(struct elementDescr* elem)
{
	unsigned int i = 0;
	elem->element = ELEMENT_VOID;
	elem->nextInStack = NULL;
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
	StringType target_ns;
	QName simpleType;
	QName scope;
	struct EXIGrammar* attrUseGrammar;
	size_t attrUseGrammarID;
	struct elementDescr* elemDesc;

	popElemContext(&(app_data->contextStack), &elemDesc);

	if (!isStrEmpty(&(elemDesc->attributePointers[ATTRIBUTE_USE])) &&
			strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_USE], "required"))
	{
		required = 1;
	}
	if(app_data->props.attributeFormDefault == FORM_DEF_QUALIFIED || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace - might be different from the global target namespace

		target_ns = app_data->props.targetNamespace; // it is the globally defined target namespace
		tmp_err_code = addLocalName(app_data->props.targetNSMetaID, &app_data->schema->memList, app_data->schema->initialStringTables, elemDesc->attributePointers[ATTRIBUTE_NAME]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		target_ns.length = 0;
		target_ns.str = NULL;
		tmp_err_code = addLocalName(0, &app_data->schema->memList, app_data->schema->initialStringTables, elemDesc->attributePointers[ATTRIBUTE_NAME]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	simpleType.localName = &(elemDesc->attributePointers[ATTRIBUTE_TYPE]);
	simpleType.uri = NULL;

	scope.localName = NULL;
	scope.uri = NULL;

	tmp_err_code = createAttributeUseGrammar(&app_data->tmpMemList, required, elemDesc->attributePointers[ATTRIBUTE_NAME],
											 target_ns, simpleType, scope, &attrUseGrammar, app_data->regProdQname);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		for(t = 0; t < attrUseGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(&(attrUseGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = addDynElement(app_data->attributeUses, attrUseGrammar, &attrUseGrammarID, &app_data->tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleExtentionEl(struct xsdAppData* app_data)
{
	// TODO: this implementation is just experimental.
	//       It only creates simple type grammar depending on the value of base attribute
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QName simpleType;
	struct EXIGrammar* simpleTypeGrammar;
	struct elementDescr* elemDesc;

	popElemContext(&(app_data->contextStack), &elemDesc);

	simpleType.localName = &(elemDesc->attributePointers[ATTRIBUTE_BASE]);
	simpleType.uri = NULL;

	tmp_err_code = createSimpleTypeGrammar(&app_data->tmpMemList, simpleType, &simpleTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = pushGrammar((EXIGrammarStack**) &(app_data->pGrammarStack), simpleTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleSimpleContentEl(struct xsdAppData* app_data)
{
	// TODO: For now just skip this element. The simpleTypeGrammar should already be created
	struct elementDescr* elemDesc;

	popElemContext(&(app_data->contextStack), &elemDesc);
	return ERR_OK;
}

static errorCode handleComplexTypeEl(struct xsdAppData* app_data)
{
	// TODO: The attribute uses must be sorted first
	// TODO: Then the dynamic attribute uses array must be emptied

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	StringType typeName;
	StringType target_ns;
	struct EXIGrammar* contentTypeGrammar;
	struct EXIGrammar* resultComplexGrammar;
	struct elementDescr* elemDesc;

	popElemContext(&(app_data->contextStack), &elemDesc);

	typeName = elemDesc->attributePointers[ATTRIBUTE_NAME];

	if(app_data->props.elementFormDefault == FORM_DEF_QUALIFIED || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace

		target_ns = app_data->props.targetNamespace;
		if(!isStrEmpty(&typeName))
		{
			tmp_err_code = addLocalName(app_data->props.targetNSMetaID, &app_data->schema->memList, app_data->schema->initialStringTables, typeName);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else
	{
		target_ns.length = 0;
		target_ns.str = NULL;
		if(!isStrEmpty(&typeName))
		{
			tmp_err_code = addLocalName(0, &app_data->schema->memList, app_data->schema->initialStringTables, typeName);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	tmp_err_code = popGrammar((EXIGrammarStack**) &(app_data->pGrammarStack), &contentTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t tt = 0;
		for(tt = 0; tt < contentTypeGrammar->rulesDimension; tt++)
		{
			tmp_err_code = printGrammarRule(&(contentTypeGrammar->ruleArray[tt]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	// TODO: the attributeUses array must be sorted first before calling createComplexTypeGrammar()

	tmp_err_code = createComplexTypeGrammar(&app_data->tmpMemList, typeName, target_ns,
			(struct EXIGrammar*) app_data->attributeUses->elements, app_data->attributeUses->elementCount,
			                           NULL, 0, contentTypeGrammar, &resultComplexGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		uint16_t t = 0;
		for(t = 0; t < resultComplexGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(&(resultComplexGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = normalizeGrammar(&app_data->tmpMemList, resultComplexGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = assignCodes(resultComplexGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	//TODO: the attributeUses array must be emptied here

	if(isStrEmpty(&typeName))  // The name is empty i.e. anonymous complex type
	{
		// Put the ComplexTypeGrammar on top of the pGrammarStack
		// There should be a parent <element> declaration for this grammar
		tmp_err_code = pushGrammar((EXIGrammarStack**) &(app_data->pGrammarStack), resultComplexGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // Named complex type - put it directly in the typeGrammars list
	{
		tmp_err_code = appendMetaGrammarNode(&app_data->tmpMemList, &app_data->typeGrammars, resultComplexGrammar, typeName, target_ns);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

static errorCode handleElementEl(struct xsdAppData* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	struct elementDescr* elemDesc;
	StringType type;
	struct EXIGrammar* typeGrammar;
	StringType elName;
	StringType target_ns;
	unsigned char isGlobal = 0;

	popElemContext(&(app_data->contextStack), &elemDesc);
	type = elemDesc->attributePointers[ATTRIBUTE_TYPE];

	if(app_data->contextStack == NULL) // Global element
		isGlobal = 1;

	if(isGlobal || app_data->props.elementFormDefault == FORM_DEF_QUALIFIED || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace

		target_ns = app_data->props.targetNamespace;
		tmp_err_code = addLocalName(app_data->props.targetNSMetaID, &app_data->schema->memList, app_data->schema->initialStringTables, elemDesc->attributePointers[ATTRIBUTE_NAME]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		target_ns.length = 0;
		target_ns.str = NULL;
		tmp_err_code = addLocalName(0, &app_data->schema->memList, app_data->schema->initialStringTables, elemDesc->attributePointers[ATTRIBUTE_NAME]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	elName = elemDesc->attributePointers[ATTRIBUTE_NAME];

	if(isGlobal)
	{
		if(isStrEmpty(&type))  // There is no type attribute i.e. there must be some complex type in the pGrammarStack
		{
			tmp_err_code = popGrammar((EXIGrammarStack**) &(app_data->pGrammarStack), &typeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else // The element has a particular named type
		{
			return NOT_IMPLEMENTED_YET;
		}

		tmp_err_code = orderedAddMetaGrammarNode(&app_data->tmpMemList, &app_data->globalElemGrammars, typeGrammar, elName, target_ns);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else  // Local element definition i.e within complex type
	{
		return NOT_IMPLEMENTED_YET;
	}
	return ERR_OK;
}

static errorCode appendMetaGrammarNode(AllocList* tmpMemList, MetaGrammarList* gList, struct EXIGrammar* grammar, StringType name, StringType ns)
{
	struct metaGrammarNode* node = (struct metaGrammarNode*) memManagedAllocate(tmpMemList, sizeof(struct metaGrammarNode));
	if(node == NULL)
		return MEMORY_ALLOCATION_ERROR;

	node->grammar = grammar;
	node->uri.length = ns.length;
	node->uri.str = ns.str;
	node->ln.length = name.length;
	node->ln.str = name.str;
	node->nextNode = NULL;

	gList->last->nextNode = node;
	gList->last = node;
	gList->size += 1;

	return ERR_OK;
}

static errorCode orderedAddMetaGrammarNode(AllocList* tmpMemList, MetaGrammarList* gList, struct EXIGrammar* grammar, StringType name, StringType ns)
{
	struct metaGrammarNode* newNode = memManagedAllocate(tmpMemList, sizeof(struct metaGrammarNode));
	if(newNode == NULL)
		return MEMORY_ALLOCATION_ERROR;

	newNode->grammar = grammar;
	newNode->uri.length = ns.length;
	newNode->uri.str = ns.str;
	newNode->ln.length = name.length;
	newNode->ln.str = name.str;

	if(gList->first == NULL) // Empty list
	{
		gList->first = newNode;
		gList->last = newNode;
		gList->size = 1;
	}
	else if(qnamesCompare(gList->first->uri, gList->first->ln, ns, name) >= 0) // The added grammar is less or equal to the smallest grammar in the ordered list
	{
		// insert the node at the beginning of the list
		newNode->nextNode = gList->first;
		gList->first = newNode;
		gList->size += 1;
	}
	else if(qnamesCompare(gList->last->uri, gList->last->ln, ns, name) <= 0) // The added grammar is bigger or equal to the biggest grammar in the ordered list
	{
		// insert the node at the end of the list
		newNode->nextNode = NULL;
		gList->last->nextNode = newNode;
		gList->last = newNode;
		gList->size += 1;
	}
	else  // find the right place for the node
	{
		struct metaGrammarNode* tmpNode = gList->first;
		while(tmpNode)
		{
			if(qnamesCompare(tmpNode->nextNode->uri, tmpNode->nextNode->ln, ns, name) >= 0) // The added grammar is less or equal to the currently tested grammar in the ordered list
			{
				// insert the node after the tmpNode
				newNode->nextNode = tmpNode->nextNode;
				tmpNode->nextNode = newNode;
				gList->size += 1;
				break;
			}
			tmpNode = tmpNode->nextNode;
		}
	}
	return ERR_OK;
}

// Only adds it if it is not there yet
static errorCode addLocalName(uint16_t uriId, AllocList* memList, URITable* stringTables, StringType ln)
{
	size_t lnID;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	StringType lnClone;   // The local name string is copied to the schema MemList

	if(stringTables->rows[uriId].lTable == NULL)
	{
		tmp_err_code = createLocalNamesTable(&stringTables->rows[uriId].lTable, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = cloneString(&ln, &lnClone, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = addLNRow(stringTables->rows[uriId].lTable, lnClone, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(!lookupLN(stringTables->rows[uriId].lTable, ln, &lnID))
	{
		tmp_err_code = cloneString(&ln, &lnClone, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = addLNRow(stringTables->rows[uriId].lTable, lnClone, &lnID);
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
