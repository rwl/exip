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
 * @file bodyDecode.h
 * @brief API for decoding EXI stream body
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef BODYDECODE_H_
#define BODYDECODE_H_

#include "contentHandler.h"

/**
 * @brief Decodes a QName from the EXI stream
 *
 * @param[in, out] strm EXI stream representation
 * @param[out] qname the QName decoded
 * @return Error handling code
 */
errorCode decodeQName(EXIStream* strm, QName* qname);

/**
 * @brief Decodes the URI part of a QName from the EXI stream
 *
 * @param[in, out] strm EXI stream representation
 * @param[out] uriID The URI id in the URI string table
 * @return Error handling code
 */
errorCode decodeURI(EXIStream* strm, uint16_t* uriID);

/**
 * @brief Decodes the Local name part of a QName from the EXI stream
 *
 * @param[in, out] strm EXI stream representation
 * @param[in] uriID The URI id in the URI string table
 * @param[out] lnID The LN id in the Local name string table
 * @return Error handling code
 */
errorCode decodeLocalName(EXIStream* strm, uint16_t uriID, size_t* lnID);

/**
 * @brief Decodes the prefix component of the QName from the EXI stream
 * In case of Preserve.prefixes the QName type contains a prefix also
 *
 * @param[in, out] strm EXI stream representation
 * @param[in] uriID The URI id in the URI string table
 * @param[out] qname qname that holds the prefix
 * @return Error handling code
 */
errorCode decodePrefixQname(EXIStream* strm, QName* qname);

/**
 * @brief Decodes the URI part of a QName from the EXI stream
 *
 * @param[in, out] strm EXI stream representation
 * @param[in] uriID The URI id in the URI string table
 * @param[out] prfxID The prefix id in the Prefix string table
 * @return Error handling code
 */
errorCode decodePrefix(EXIStream* strm, uint16_t uriID, unsigned int* prfxID);

/**
 * @brief Decodes a string value from the EXI stream
 * @param[in, out] strm EXI stream representation
 * @param[out] value the string decoded
 * @param[out] freeable if TRUE the value can be freed immediately afterwards using freeLastManagedAlloc()
 * @return Error handling code
 */
errorCode decodeStringValue(EXIStream* strm, String* value, unsigned char* freeable);

/**
 * @brief Decodes the content of EXI event
 * @param[in, out] strm EXI stream representation
 * @param[in] event the event which content will be decoded
 * @param[in] handler application content handler; stores the callback functions
 * @param[out] nonTermID_out nonTerminal ID after the content decoding
 * @param[in] currRule the current grammar rule in use for the event
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode decodeEventContent(EXIStream* strm, EXIEvent event, ContentHandler* handler,
									size_t* nonTermID_out, GrammarRule* currRule, void* app_data);

#endif /* BODYDECODE_H_ */
