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
#include "EXIParser.h"

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

static EXIStream* tmpStrm;

static void pushElemContext(ContextStack** cStack, struct elementDescr* elem);

static void popElemContext(ContextStack** cStack, struct elementDescr** elem);

errorCode generateSchemaInformedGrammars(char* binaryStream, uint32_t bufLen, unsigned char schemaFormat,
										EXIStream* strm, ElementGrammarPool* typesGrammarPool)
{
	if(schemaFormat != SCHEMA_FORMAT_XSD_EXI)
		return NOT_IMPLEMENTED_YET;

	ContentHandler xsdHandler;
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

	struct globalSchemaProps localProps;
	localProps.propsStat = 0;
	localProps.expectAttributeData = 0;
	localProps.attributeFormDefault = 3;
	localProps.elementFormDefault = 3;
	localProps.charDataPointer = NULL;
	props = &localProps;

	contextStack = NULL;

	pGrammarStack = NULL;

	errorCode tmp_err_code = createDynArray(&elNotResolvedArray, sizeof(struct elementNotResolved), 10, strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmpStrm = strm;

	// Parse the EXI stream
	parseEXI(binaryStream, bufLen, &xsdHandler);

	return ERR_OK;
}

char xsd_fatalError(const char code, const char* msg)
{
	//TODO:
	return EXIP_HANDLER_STOP;
}

char xsd_startDocument()
{
	DEBUG_MSG(INFO,(">Start XML Schema parsing\n"));
	return EXIP_HANDLER_OK;
}

char xsd_endDocument()
{
	DEBUG_MSG(INFO,(">XML Schema parsing finished\n"));
	return EXIP_HANDLER_OK;
}

char xsd_startElement(QName qname)
{
	if(!props->propsStat) // This should be the first <schema> element
	{
		if(strEqualToAscii(qname.uri, "http://www.w3.org/2001/XMLSchema") &&
				strEqualToAscii(qname.localName, "schema"))
			props->propsStat = 1;
		else
		{
			DEBUG_MSG(ERROR,(">Invalid XML Schema! Missing <schema> root element\n"));
			return EXIP_HANDLER_STOP;
		}
	}
	else
	{
		if(!strEqualToAscii(qname.uri, "http://www.w3.org/2001/XMLSchema"))
		{
			DEBUG_MSG(ERROR,(">Invalid namespace of XML Schema element\n"));
			return EXIP_HANDLER_STOP;
		}

		struct elementDescr* elem = (struct elementDescr*) memManagedAllocate(tmpStrm, sizeof(struct elementDescr));
		if(elem == NULL)
			return MEMORY_ALLOCATION_ERROR;

		if(strEqualToAscii(qname.localName, "element"))
		{
			elem.element = ELEMENT_ELEMENT;
		}
		else if(strEqualToAscii(qname.localName, "attribute"))
		{
			elem.element = ELEMENT_ATTRIBUTE;
		}
		else if(strEqualToAscii(qname.localName, "choice"))
		{
			elem.element = ELEMENT_CHOICE;
		}
		else if(strEqualToAscii(qname.localName, "complexType"))
		{
			elem.element = ELEMENT_COMPLEX_TYPE;
		}
		else if(strEqualToAscii(qname.localName, "complexContent"))
		{
			elem.element = ELEMENT_COMPLEX_CONTENT;
		}
		else if(strEqualToAscii(qname.localName, "group"))
		{
			elem.element = ELEMENT_GROUP;
		}
		else if(strEqualToAscii(qname.localName, "import"))
		{
			elem.element = ELEMENT_IMPORT;
		}
		else if(strEqualToAscii(qname.localName, "sequence"))
		{
			elem.element = ELEMENT_SEQUENCE;
		}
		else if(strEqualToAscii(qname.localName, "all"))
		{
			elem.element = ELEMENT_ALL;
		}
		else
		{
			DEBUG_MSG(WARNING,(">Ignored schema element\n"));
		}

		pushElemContext(&contextStack, elem);
	}
	return EXIP_HANDLER_OK;
}

char xsd_endElement()
{
	if(contextStack == NULL) // No elements stored in the stack. That is </schema>
	{
		props->propsStat = 2; // All attributes of the <schema> element are already parsed
		if(props->elementFormDefault == 3)
			props->elementFormDefault = 0; // The default value is unqualified
		if(props->attributeFormDefault == 3)
			props->attributeFormDefault = 0; // The default value is unqualified
	}
	return EXIP_HANDLER_OK;
}

char xsd_attribute(QName qname)
{
	if(props->propsStat == 1) // <schema> element attribute
	{
		if(strEqualToAscii(qname.localName, "targetNamespace"))
			props->charDataPointer = &(props->targetNamespace);
		else if(strEqualToAscii(qname.localName, "elementFormDefault"))
			props->elementFormDefault = 2;
		else if(strEqualToAscii(qname.localName, "attributeFormDefault"))
			props->attributeFormDefault = 2;
		else
		{
			DEBUG_MSG(WARNING,(">Ignored <schema> attribute\n"));
		}
	}
	else
	{
		if(strEqualToAscii(qname.localName, "name"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_NAME]);
		}
		else if(strEqualToAscii(qname.localName, "type"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_TYPE]);
		}
		else if(strEqualToAscii(qname.localName, "ref"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_REF]);
		}
		else if(strEqualToAscii(qname.localName, "minOccurs"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_MIN_OCCURS]);
		}
		else if(strEqualToAscii(qname.localName, "maxOccurs"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_MAX_OCCURS]);
		}
		else if(strEqualToAscii(qname.localName, "form"))
		{
			props->charDataPointer = &(contextStack->attributePointers[ATTRIBUTE_FORM]);
		}
		else
		{
			DEBUG_MSG(WARNING,(">Ignored element attribute\n"));
		}
	}
	props->expectAttributeData = 1;
	return EXIP_HANDLER_OK;
}

char xsd_stringData(const StringType value)
{
	if(expectAttributeData)
	{
		if(props->propsStat = 1) // <schema> element attribute data
		{
			if(props->charDataPointer != NULL)
			{
				*(props->charDataPointer) = value;
				props->charDataPointer = NULL;
			}
			else if(props->elementFormDefault == 2) // expecting value for elementFormDefault
			{
				if(strEqualToAscii(qname.localName, "qualified"))
					props->elementFormDefault = 1;
				else if(strEqualToAscii(qname.localName, "unqualified"))
					props->elementFormDefault = 0;
				else
				{
					DEBUG_MSG(ERROR,(">Invalid value for elementFormDefault attribute\n"));
					return EXIP_HANDLER_STOP;
				}
			}
			else if(props->attributeFormDefault == 2) // expecting value for attributeFormDefault
			{
				if(strEqualToAscii(qname.localName, "qualified"))
					props->attributeFormDefault = 1;
				else if(strEqualToAscii(qname.localName, "unqualified"))
					props->attributeFormDefault = 0;
				else
				{
					DEBUG_MSG(ERROR,(">Invalid value for elementFormDefault attribute\n"));
					return EXIP_HANDLER_STOP;
				}
			}
			else
			{
				DEBUG_MSG(WARNING,(">Ignored <schema> attribute value\n"));
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
				DEBUG_MSG(WARNING,(">Ignored element attribute value\n"));
			}
		}

		expectAttributeData = 0;
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
			DEBUG_MSG(WARNING,(">Ignored element value\n"));
		}
	}

	return EXIP_HANDLER_OK;
}

char xsd_exiHeader(const EXIheader* header)
{
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
