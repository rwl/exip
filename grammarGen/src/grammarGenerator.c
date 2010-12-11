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
void xsd_fatalError(const char code, const char* msg);
void xsd_startDocument();
void xsd_endDocument();
void xsd_startElement(QName qname);
void xsd_endElement();
void xsd_attribute(QName qname);
void xsd_stringData(const StringType value);
void xsd_exiHeader(const EXIheader* header);

errorCode generateSchemaInformedGrammars(char* binaryStream, uint32_t bufLen, unsigned char schemaFormat,
										EXIGrammarStack* gStack, ElementGrammarPool* gPool)
{
	if(schemaFormat != SCHEMA_FORMAT_XSD_EXI)
		return NOT_IMPLEMENTED_YET;

	ContentHandler xsdHandler;
	initContentHandler(&xsdHandler);
	sampleHandler.fatalError = xsd_fatalError;
	sampleHandler.error = xsd_fatalError;
	sampleHandler.startDocument = xsd_startDocument;
	sampleHandler.endDocument = xsd_endDocument;
	sampleHandler.startElement = xsd_startElement;
	sampleHandler.attribute = xsd_attribute;
	sampleHandler.stringData = xsd_stringData;
	sampleHandler.endElement = xsd_endElement;
	sampleHandler.exiHeader = xsd_exiHeader;

	// Parse the EXI stream
	parseEXI(binaryStream, bufLen, &xsdHandler);

	return ERR_OK;
}

void xsd_fatalError(const char code, const char* msg)
{
	//TODO:
}

void xsd_startDocument()
{

}

void xsd_endDocument()
{

}

void xsd_startElement(QName qname)
{

}

void xsd_endElement()
{

}

void xsd_attribute(QName qname)
{

}

void xsd_stringData(const StringType value)
{

}

void xsd_exiHeader(const EXIheader* header)
{

}
