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

struct xsdAppData
{
	struct globalSchemaProps props;
	ContextStack* contextStack;
	ProtoGrammarsStack* pGrammarStack;
	DynArray* elNotResolvedArray;
	DynArray* attributeUses;
	URITable* metaURITable;
	DynArray* regProdQname;
	DynArray* globalElements;
	DynArray* allElementGrammars;
	DynArray* allTypeGrammars;
	EXIStream* strm;
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

errorCode generateSchemaInformedGrammars(char* binaryStream, uint32_t bufLen, unsigned char schemaFormat,
										 ExipSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ContentHandler xsdHandler;
	struct xsdAppData parsing_data;
	EXIStream strm; // used for temporary memory allocation

	if(schemaFormat != SCHEMA_FORMAT_XSD_EXI)
		return NOT_IMPLEMENTED_YET;

	strm.gStack = NULL;
	initAllocList(&strm.memList);
	tmp_err_code = createGrammarPool(&strm.ePool);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = createGrammarPool(&strm.tPool);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

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
	parsing_data.props.expectAttributeData = 0;
	parsing_data.props.attributeFormDefault = FORM_DEF_INITIAL_STATE;
	parsing_data.props.elementFormDefault = FORM_DEF_INITIAL_STATE;
	parsing_data.props.charDataPointer = NULL;

	parsing_data.contextStack = NULL;

	parsing_data.pGrammarStack = NULL;

	tmp_err_code = createDynArray(&parsing_data.elNotResolvedArray, sizeof(struct elementNotResolved), 10, &strm.memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.attributeUses, sizeof(struct EXIGrammar), 5, &strm.memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createInitialStringTables(&strm, TRUE);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	parsing_data.metaURITable = strm.uriTable;

	tmp_err_code = createDynArray(&parsing_data.regProdQname, sizeof(struct productionQname), 20, &strm.memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.globalElements, sizeof(struct grammarQname), 10, &strm.memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.allElementGrammars, sizeof(struct grammarQname), 10, &strm.memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&parsing_data.allTypeGrammars, sizeof(struct grammarQname), 10, &strm.memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	parsing_data.strm = &strm;

	initAllocList(&schema->memList);

	tmp_err_code = createGrammarPool(&(schema->ePool));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = createGrammarPool(&(schema->tPool));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

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
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End XML Schema parsing\n"));

// Only for debugging purposes
#if DEBUG_GRAMMAR_GEN == ON
	{
		int i = 0;
		unsigned char is_found = 0;
		struct EXIGrammar* result;
		for(i = 0; i < exipSchemaLocal->glElems.count; i++)
		{
			tmp_err_code = checkGrammarInPool(exipSchemaLocal->ePool, exipSchemaLocal->glElems.elems[i].uriRowId,
					exipSchemaLocal->glElems.elems[i].lnRowId, &is_found, &result);
			if(tmp_err_code != ERR_OK)
			{
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">checkGrammarInPool() fail\n"));
				return EXIP_HANDLER_STOP;
			}
			if(is_found)
			{
				int t = 0;
				for(; t < result->rulesDimension; t++)
				{
					tmp_err_code = printGrammarRule(&(result->ruleArray[t]));
					if(tmp_err_code != ERR_OK)
					{
						DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">printGrammarRule() fail\n"));
						return EXIP_HANDLER_STOP;
					}
				}
			}
		}
	}
#endif

	tmp_err_code = stringTablesSorting(&appD->strm->memList, appD->metaURITable, appD->schema, appD->regProdQname);
	if(tmp_err_code != ERR_OK)
	{
		DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">String Tables sorting failed: %d\n", tmp_err_code));
		return EXIP_HANDLER_STOP;
	}

	tmp_err_code = sortGlobalElements(&appD->schema->memList, appD->globalElements, appD->schema);
	if(tmp_err_code != ERR_OK)
	{
		DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Global elements sorting failed: %d\n", tmp_err_code));
		return EXIP_HANDLER_STOP;
	}


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
		}

		if(!strEqualToAscii(*qname.uri, "http://www.w3.org/2001/XMLSchema"))
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid namespace of XML Schema element\n"));
			return EXIP_HANDLER_STOP;
		}

		elem = (struct elementDescr*) memManagedAllocate(&appD->strm->memList, sizeof(struct elementDescr));
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
	appD->props.expectAttributeData = 1;
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

		appD->props.expectAttributeData = 0;
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
	int i = 0;
	elem->element = ELEMENT_VOID;
	elem->nextInStack = NULL;
	for(; i < ATTRIBUTE_CONTEXT_ARRAY_SIZE; i++)
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
	uint32_t attrUseGrammarID;
	struct elementDescr* elemDesc;

	popElemContext(&(app_data->contextStack), &elemDesc);

	if (!isStrEmpty(&(elemDesc->attributePointers[ATTRIBUTE_USE])) &&
			strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_USE], "required"))
	{
		required = 1;
	}
	if(app_data->props.attributeFormDefault == FORM_DEF_QUALIFIED || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace

		target_ns = app_data->props.targetNamespace;
	}
	else
	{
		target_ns.length = 0;
		target_ns.str = NULL;
	}
	simpleType.localName = &(elemDesc->attributePointers[ATTRIBUTE_TYPE]);
	simpleType.uri = NULL;

	scope.localName = NULL;
	scope.uri = NULL;

	tmp_err_code = createAttributeUseGrammar(&app_data->strm->memList, required, elemDesc->attributePointers[ATTRIBUTE_NAME],
											 target_ns, simpleType, scope, &attrUseGrammar, app_data->metaURITable, app_data->regProdQname);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		int t = 0;
		for(; t < attrUseGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(&(attrUseGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = addDynElement(app_data->attributeUses, attrUseGrammar, &attrUseGrammarID, &app_data->strm->memList);
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

	tmp_err_code = createSimpleTypeGrammar(&app_data->strm->memList, simpleType, &simpleTypeGrammar);
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
	uint32_t uriRowId = 0;
	uint32_t lnRowId = 0;
	struct EXIGrammar* contentTypeGrammar;
	struct EXIGrammar* resultComplexGrammar;
	struct elementDescr* elemDesc;

	popElemContext(&(app_data->contextStack), &elemDesc);

	if(app_data->props.elementFormDefault == FORM_DEF_QUALIFIED || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace

		target_ns = app_data->props.targetNamespace;
	}
	else
	{
		target_ns.length = 0;
		target_ns.str = NULL;
	}
	typeName = elemDesc->attributePointers[ATTRIBUTE_NAME];

	tmp_err_code = popGrammar((EXIGrammarStack**) &(app_data->pGrammarStack), &contentTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		int tt = 0;
		for(; tt < contentTypeGrammar->rulesDimension; tt++)
		{
			tmp_err_code = printGrammarRule(&(contentTypeGrammar->ruleArray[tt]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	// TODO: the attributeUses array must be sorted first before calling createComplexTypeGrammar()

	tmp_err_code = createComplexTypeGrammar(&app_data->strm->memList, typeName, target_ns,
			(struct EXIGrammar*) app_data->attributeUses->elements, app_data->attributeUses->elementCount,
			                           NULL, 0, contentTypeGrammar, &resultComplexGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

#if DEBUG_GRAMMAR_GEN == ON
	{
		int t = 0;
		for(; t < resultComplexGrammar->rulesDimension; t++)
		{
			tmp_err_code = printGrammarRule(&(resultComplexGrammar->ruleArray[t]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	tmp_err_code = normalizeGrammar(&app_data->strm->memList, resultComplexGrammar);
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
	else // Named complex type - put it directly in the Type Grammar pool
	{
		if(!lookupURI(app_data->metaURITable, target_ns, &uriRowId))
		{
			tmp_err_code = addURIRow(app_data->metaURITable, target_ns, &uriRowId, &app_data->strm->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(app_data->metaURITable->rows[uriRowId].lTable == NULL)
		{
			tmp_err_code = createLocalNamesTable(&(app_data->metaURITable->rows[uriRowId].lTable), &app_data->strm->memList, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(!lookupLN(app_data->metaURITable->rows[uriRowId].lTable, typeName, &lnRowId))
		{
			tmp_err_code = addLNRow(app_data->metaURITable->rows[uriRowId].lTable, typeName, &lnRowId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		tmp_err_code = addGrammarInPool(app_data->strm->tPool, uriRowId,
												lnRowId, resultComplexGrammar);
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
	uint32_t uriRowId = 0;
	uint32_t lnRowId = 0;
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
	}
	else
	{
		target_ns.length = 0;
		target_ns.str = NULL;
	}
	elName = elemDesc->attributePointers[ATTRIBUTE_NAME];

	if(isGlobal)
	{
		struct grammarQname glEl;
		struct productionQname pqRow;
		struct grammarQname* grQname;
		uint32_t dynArrId = 0;

		if(!lookupURI(app_data->metaURITable, target_ns, &uriRowId))
		{
			tmp_err_code = addURIRow(app_data->metaURITable, target_ns, &uriRowId, &app_data->strm->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(app_data->metaURITable->rows[uriRowId].lTable == NULL)
		{
			tmp_err_code = createLocalNamesTable(&(app_data->metaURITable->rows[uriRowId].lTable), &app_data->strm->memList, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(!lookupLN(app_data->metaURITable->rows[uriRowId].lTable, elName, &lnRowId))
		{
			tmp_err_code = addLNRow(app_data->metaURITable->rows[uriRowId].lTable, elName, &lnRowId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		glEl.uriRowId = uriRowId;
		glEl.lnRowId = lnRowId;

		tmp_err_code = addDynElement(app_data->globalElements, &glEl, &dynArrId, &app_data->strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		grQname = (struct grammarQname*) app_data->globalElements->elements;

		pqRow.p_uriRowID = &grQname[dynArrId].uriRowId;
		pqRow.p_lnRowID = &grQname[dynArrId].lnRowId;
		pqRow.uriRowID_old = glEl.uriRowId;
		pqRow.lnRowID_old = glEl.lnRowId;

		tmp_err_code = addDynElement(app_data->regProdQname, &pqRow, &dynArrId, &app_data->strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addDynElement(app_data->allElementGrammars, &glEl, &dynArrId, &app_data->strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		grQname = (struct grammarQname*) app_data->allElementGrammars->elements;

		pqRow.p_uriRowID = &grQname[dynArrId].uriRowId;
		pqRow.p_lnRowID = &grQname[dynArrId].lnRowId;
		pqRow.uriRowID_old = glEl.uriRowId;
		pqRow.lnRowID_old = glEl.lnRowId;

		tmp_err_code = addDynElement(app_data->regProdQname, &pqRow, &dynArrId, &app_data->strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;


		if(isStrEmpty(&type))  // There is no type attribute i.e. there must be some complex type in the pGrammarStack
		{
			tmp_err_code = popGrammar((EXIGrammarStack**) &(app_data->pGrammarStack), &typeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = addGrammarInPool(app_data->strm->ePool, uriRowId,
															lnRowId, typeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else // The element has a particular named type
		{
			return NOT_IMPLEMENTED_YET;
		}
	}
	else  // Local element definition i.e within complex type
	{
		return NOT_IMPLEMENTED_YET;
	}
	return ERR_OK;
}
