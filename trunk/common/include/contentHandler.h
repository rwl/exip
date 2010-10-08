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

struct ContentHandler
{
	void (*startDocument)();
	void (*endDocument)();
	void (*startElement)(QName qname);
	void (*endElement)(); // TODO: define the parameters if needed. Most probably not. The element should be known from the context
	void (*attributeString)(QName qname, const StringType value);
	void (*intData)(); // TODO: define the parameters!
	void (*stringData)(); // TODO: define the parameters!
	void (*floatData)(); // TODO: define the parameters!
	void (*binaryData)(); // TODO: define the parameters!

	void (*processingInstruction)(); // TODO: define the parameters!

	void (*warning)(const char code, const char* msg);
	void (*error)(const char code, const char* msg);
	void (*fatalError)(const char code, const char* msg);

	// EXI specific
	void (*selfContained)();  // Used for indexing independent elements for random access
};

typedef struct ContentHandler ContentHandler;


#endif /* CONTENTHANDLER_H_ */
