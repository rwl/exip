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
 * @file contentHandler.h
 * @brief SAX-like interface for parsing the content of an EXI stream
 * The applications should register to this handlers with callback functions
 * invoked when the processor pass through the stream. This interface is lower level than SAX.
 * If you want to use SAX API you should wrap this interface.
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef CONTENTHANDLER_H_
#define CONTENTHANDLER_H_

#include "procTypes.h"

/** Macros for the returning code from the ContentHandler callbacks */
#define EXIP_HANDLER_OK   0
#define EXIP_HANDLER_STOP 1

struct ContentHandler
{
	// For handling the meta-data (document structure)
	char (*startDocument)(void* app_data);
	char (*endDocument)(void* app_data);
	char (*startElement)(QName qname, void* app_data);
	char (*endElement)(void* app_data);
	char (*attribute)(QName qname, void* app_data);

	// For handling the data
	char (*intData)(Integer int_val, void* app_data);
	char (*booleanData)(unsigned char bool_val, void* app_data);
	char (*stringData)(const String str_val, void* app_data);
	char (*floatData)(Float float_val, void* app_data);
	char (*binaryData)(const char* binary_val, size_t nbytes, void* app_data);
	char (*dateTimeData)(struct tm dt_val, uint16_t presenceMask, void* app_data);
	char (*decimalData)(Decimal dec_val, void* app_data);

	// Miscellaneous
	char (*processingInstruction)(void* app_data); // TODO: define the parameters!

	// For error handling
	char (*warning)(const char code, const char* msg, void* app_data);
	char (*error)(const char code, const char* msg, void* app_data);
	char (*fatalError)(const char code, const char* msg, void* app_data);

	// EXI specific
	char (*selfContained)(void* app_data);  // Used for indexing independent elements for random access
};

typedef struct ContentHandler ContentHandler;

/**
 * @brief Initialize the content handler before use
 * @param[in] handler fresh ContentHandler
 *
 */
void initContentHandler(ContentHandler* handler);

#endif /* CONTENTHANDLER_H_ */
