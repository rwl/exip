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

// Content Handler API
char xsd_fatalError(const char code, const char* msg);
char xsd_startDocument();
char xsd_endDocument();
char xsd_startElement(QName qname);
char xsd_endElement();
char xsd_attribute(QName qname);
char xsd_stringData(const StringType value);
char xsd_exiHeader(const EXIheader* header);

static struct globalSchemaProps* props;

static ContextStack* contextStack;

static ProtoGrammarsStack* pGrammarStack;

static DynArray* elNotResolvedArray;

static DynArray* attributeUses;

static URITable* metaURITable;

static DynArray* regProdQname;

static DynArray* globalElements;

static EXIStream* tmpStrm;

ExipSchema* exipSchemaLocal;

static void pushElemContext(ContextStack** cStack, struct elementDescr* elem);

static void popElemContext(ContextStack** cStack, struct elementDescr** elem);

static void initElemContext(struct elementDescr* elem);

// Handling of schema elements

static errorCode handleAttributeEl();

static errorCode handleExtentionEl();

static errorCode handleSimpleContentEl();

static errorCode handleComplexTypeEl();

static errorCode handleElementEl();

errorCode generateSchemaInformedGrammars(char* binaryStream, uint32_t bufLen, unsigned char schemaFormat,
										EXIStream* strm, ExipSchema* exipSchema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ContentHandler xsdHandler;
	struct globalSchemaProps localProps;

	if(schemaFormat != SCHEMA_FORMAT_XSD_EXI)
		return NOT_IMPLEMENTED_YET;

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

	localProps.propsStat = 0;
	localProps.expectAttributeData = 0;
	localProps.attributeFormDefault = 3;
	localProps.elementFormDefault = 3;
	localProps.charDataPointer = NULL;
	props = &localProps;

	contextStack = NULL;

	pGrammarStack = NULL;

	tmp_err_code = createDynArray(&elNotResolvedArray, sizeof(struct elementNotResolved), 10, strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&attributeUses, sizeof(struct EXIGrammar), 5, strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createURITable(&metaURITable, strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&regProdQname, sizeof(struct productionQname), 20, strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&globalElements, sizeof(struct globalElementId), 10, strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmpStrm = strm;

	tmp_err_code = createGrammarPool(&(exipSchema->ePool));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = createGrammarPool(&(exipSchema->tPool));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	exipSchemaLocal = exipSchema;

	// Parse the EXI stream
	parseEXI(binaryStream, bufLen, &xsdHandler);

	return ERR_OK;
}

char xsd_fatalError(const char code, const char* msg)
{
	DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Fatal error occurred during schema processing\n"));
	return EXIP_HANDLER_STOP;
}

char xsd_startDocument()
{
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Start XML Schema parsing\n"));
	return EXIP_HANDLER_OK;
}

char xsd_endDocument()
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int i = 0;
	unsigned char is_found = 0;
	struct EXIGrammar* result;
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End XML Schema parsing\n"));

	exipSchemaLocal->glElems.elems = globalElements->elements;
	exipSchemaLocal->glElems.count = globalElements->elementCount;

// Only for debugging purposes
#if DEBUG_GRAMMAR_GEN == ON
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
#endif

	return EXIP_HANDLER_OK;
}

char xsd_startElement(QName qname)
{
	if(!props->propsStat) // This should be the first <schema> element
	{
		if(strEqualToAscii(*qname.uri, "http://www.w3.org/2001/XMLSchema") &&
				strEqualToAscii(*qname.localName, "schema"))
		{
			props->propsStat = 1;
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
		if(props->propsStat != 2) // This is the first element after the <schema>
		{
			props->propsStat = 2; // All attributes of the <schema> element are already parsed
			if(props->elementFormDefault == 3)
				props->elementFormDefault = 0; // The default value is unqualified
			if(props->attributeFormDefault == 3)
				props->attributeFormDefault = 0; // The default value is unqualified
		}

		if(!strEqualToAscii(*qname.uri, "http://www.w3.org/2001/XMLSchema"))
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid namespace of XML Schema element\n"));
			return EXIP_HANDLER_STOP;
		}

		elem = (struct elementDescr*) memManagedAllocate(tmpStrm, sizeof(struct elementDescr));
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

		pushElemContext(&contextStack, elem);
	}

	return EXIP_HANDLER_OK;
}

char xsd_endElement()
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(contextStack == NULL) // No elements stored in the stack. That is </schema>
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </schema> element\n"));
		tmp_err_code = ERR_OK;
	}
	else
	{
		if(contextStack->element == ELEMENT_ATTRIBUTE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </attribute> element\n"));
			tmp_err_code = handleAttributeEl();
		}
		else if(contextStack->element == ELEMENT_EXTENSION)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </extension> element\n"));
			tmp_err_code = handleExtentionEl();
		}
		else if(contextStack->element == ELEMENT_SIMPLE_CONTENT)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </simpleContent> element\n"));
			tmp_err_code = handleSimpleContentEl();
		}
		else if(contextStack->element == ELEMENT_COMPLEX_TYPE)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </complexType> element\n"));
			tmp_err_code = handleComplexTypeEl();
		}
		else if(contextStack->element == ELEMENT_ELEMENT)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </element> element\n"));
			tmp_err_code = handleElementEl();
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

char xsd_attribute(QName qname)
{
	if(props->propsStat == 1) // <schema> element attribute
	{
		if(strEqualToAscii(*qname.localName, "targetNamespace"))
		{
			props->charDataPointer = &(props->targetNamespace);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |targetNamespace| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "elementFormDefault"))
		{
			props->elementFormDefault = 2;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |elementFormDefault| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "attributeFormDefault"))
		{
			props->attributeFormDefault = 2;
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
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_NAME]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |name| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "type"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_TYPE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |type| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "ref"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_REF]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |ref| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "minOccurs"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_MIN_OCCURS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |minOccurs| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "maxOccurs"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_MAX_OCCURS]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |maxOccurs| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "form"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_FORM]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |form| \n"));
		}
		else if(strEqualToAscii(*qname.localName, "base"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_BASE]);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |base| \n"));
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element attribute\n"));
		}
	}
	props->expectAttributeData = 1;
	return EXIP_HANDLER_OK;
}

char xsd_stringData(const StringType value)
{
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">String data:\n"));

#if	DEBUG_GRAMMAR_GEN == ON
	printString(&value);
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n"));
#endif

	if(props->expectAttributeData)
	{
		if(props->propsStat == 1) // <schema> element attribute data
		{
			if(props->charDataPointer != NULL)
			{
				*(props->charDataPointer) = value;
				props->charDataPointer = NULL;
			}
			else if(props->elementFormDefault == 2) // expecting value for elementFormDefault
			{
				if(strEqualToAscii(value, "qualified"))
					props->elementFormDefault = 1;
				else if(strEqualToAscii(value, "unqualified"))
					props->elementFormDefault = 0;
				else
				{
					DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid value for elementFormDefault attribute\n"));
					return EXIP_HANDLER_STOP;
				}
			}
			else if(props->attributeFormDefault == 2) // expecting value for attributeFormDefault
			{
				if(strEqualToAscii(value, "qualified"))
					props->attributeFormDefault = 1;
				else if(strEqualToAscii(value, "unqualified"))
					props->attributeFormDefault = 0;
				else
				{
					DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid value for elementFormDefault attribute\n"));
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
			if(props->charDataPointer != NULL)
			{
				*(props->charDataPointer) = value;
				props->charDataPointer = NULL;
			}
			else
			{
				DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element attribute value\n"));
			}
		}

		props->expectAttributeData = 0;
	}
	else
	{
		if(props->charDataPointer != NULL)
		{
			*(props->charDataPointer) = value;
			props->charDataPointer = NULL;
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element value\n"));
		}
	}

	return EXIP_HANDLER_OK;
}

char xsd_exiHeader(const EXIheader* header)
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

static errorCode handleAttributeEl()
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char required = 0;
	StringType target_ns;
	QName simpleType;
	QName scope;
	struct EXIGrammar* attrUseGrammar;
	uint32_t attrUseGrammarID;
	struct elementDescr* elemDesc;

	popElemContext(&contextStack, &elemDesc);

	if (!isStrEmpty(&(elemDesc->attributePointers[ATTRIBUTE_USE])) &&
			strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_USE], "required"))
	{
		required = 1;
	}
	if(props->attributeFormDefault == 1 || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace

		target_ns = props->targetNamespace;
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

	tmp_err_code = createAttributeUseGrammar(tmpStrm, required, elemDesc->attributePointers[ATTRIBUTE_NAME],
											 target_ns, simpleType, scope, &attrUseGrammar, metaURITable, regProdQname);
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

	tmp_err_code = addDynElement(attributeUses, attrUseGrammar, &attrUseGrammarID, tmpStrm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleExtentionEl()
{
	// TODO: this implementation is just experimental.
	//       It only creates simple type grammar depending on the value of base attribute
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QName simpleType;
	struct EXIGrammar* simpleTypeGrammar;
	struct elementDescr* elemDesc;

	popElemContext(&contextStack, &elemDesc);

	simpleType.localName = &(elemDesc->attributePointers[ATTRIBUTE_BASE]);
	simpleType.uri = NULL;

	tmp_err_code = createSimpleTypeGrammar(tmpStrm, simpleType, &simpleTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = pushGrammar((EXIGrammarStack**) &pGrammarStack, simpleTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode handleSimpleContentEl()
{
	// TODO: For now just skip this element. The simpleTypeGrammar should already be created
	struct elementDescr* elemDesc;

	popElemContext(&contextStack, &elemDesc);
	return ERR_OK;
}

static errorCode handleComplexTypeEl()
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

	popElemContext(&contextStack, &elemDesc);

	if(props->elementFormDefault == 1 || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace

		target_ns = props->targetNamespace;
	}
	else
	{
		target_ns.length = 0;
		target_ns.str = NULL;
	}
	typeName = elemDesc->attributePointers[ATTRIBUTE_NAME];

	tmp_err_code = popGrammar((EXIGrammarStack**) &pGrammarStack, &contentTypeGrammar);
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

	tmp_err_code = createComplexTypeGrammar(tmpStrm, typeName, target_ns,
			(struct EXIGrammar*) attributeUses->elements, attributeUses->elementCount,
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

	tmp_err_code = normalizeGrammar(tmpStrm, resultComplexGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	//TODO: the attributeUses array must be emptied here

	if(isStrEmpty(&typeName))  // The name is empty i.e. anonymous complex type
	{
		// Put the ComplexTypeGrammar on top of the pGrammarStack
		// There should be a parent <element> declaration for this grammar
		tmp_err_code = pushGrammar((EXIGrammarStack**) &pGrammarStack, resultComplexGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // Named complex type - put it directly in the Type Grammar pool
	{
		if(!lookupURI(metaURITable, target_ns, &uriRowId))
		{
			tmp_err_code = addURIRow(metaURITable, target_ns, &uriRowId, tmpStrm);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(metaURITable->rows[uriRowId].lTable == NULL)
		{
			tmp_err_code = createLocalNamesTable(&metaURITable->rows[uriRowId].lTable, tmpStrm);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(!lookupLN(metaURITable->rows[uriRowId].lTable, typeName, &lnRowId))
		{
			tmp_err_code = addLNRow(metaURITable->rows[uriRowId].lTable, typeName, &lnRowId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		tmp_err_code = addGrammarInPool(exipSchemaLocal->tPool, uriRowId,
												lnRowId, resultComplexGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

static errorCode handleElementEl()
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

	popElemContext(&contextStack, &elemDesc);
	type = elemDesc->attributePointers[ATTRIBUTE_TYPE];

	if(contextStack == NULL) // Global element
		isGlobal = 1;

	if(isGlobal || props->elementFormDefault == 1 || strEqualToAscii(elemDesc->attributePointers[ATTRIBUTE_FORM], "qualified"))
	{
		//TODO: must take into account the parent element target namespace

		target_ns = props->targetNamespace;
	}
	else
	{
		target_ns.length = 0;
		target_ns.str = NULL;
	}
	elName = elemDesc->attributePointers[ATTRIBUTE_NAME];

	if(isGlobal)
	{
		struct globalElementId glEl;
		uint32_t dynArrId = 0;

		if(!lookupURI(metaURITable, target_ns, &uriRowId))
		{
			tmp_err_code = addURIRow(metaURITable, target_ns, &uriRowId, tmpStrm);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(metaURITable->rows[uriRowId].lTable == NULL)
		{
			tmp_err_code = createLocalNamesTable(&metaURITable->rows[uriRowId].lTable, tmpStrm);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(!lookupLN(metaURITable->rows[uriRowId].lTable, elName, &lnRowId))
		{
			tmp_err_code = addLNRow(metaURITable->rows[uriRowId].lTable, elName, &lnRowId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		glEl.uriRowId = uriRowId;
		glEl.lnRowId = lnRowId;

		tmp_err_code = addDynElement(globalElements, &glEl, &dynArrId, tmpStrm);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(isStrEmpty(&type))  // There is no type attribute i.e. there must be some complex type in the pGrammarStack
		{
			tmp_err_code = popGrammar((EXIGrammarStack**) &pGrammarStack, &typeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = addGrammarInPool(exipSchemaLocal->ePool, uriRowId,
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
