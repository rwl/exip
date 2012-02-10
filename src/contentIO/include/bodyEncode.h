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
 * @file bodyEncode.h
 * @brief API for encoding EXI stream body
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef BODYENCODE_H_
#define BODYENCODE_H_

#include "errorHandle.h"
#include "procTypes.h"

/**
 * @brief Finds the grammar production based on the event (and eventually the qname in case of SE and AT)
 * @param[in, out] strm EXI stream
 * @param[in] evnt event to be encoded
 * @param[in] qname element or attribute QName in case of SE or AT events; NULL otherwise
 * @param[out] codeLength 1,2 or 3 is the allowed length of EXI event codes
 * @param[out] lastCodePart the last part of the event code
 * @return Error handling code
 */
errorCode lookupProduction(EXIStream* strm, EXIEvent evnt, QName* qname, unsigned char* codeLength, size_t* lastCodePart);

/**
 * @brief Encodes String value into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] strng string to be written
 * @param[in] uriID The URI id in the URI string table
 * @param[in] lnID The ln id in the LN string table
 * @return Error handling code
 */
errorCode encodeStringData(EXIStream* strm, String strng, uint16_t uriID, size_t lnID);

/**
 * @brief Encodes QName into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] qname qname to be written
 * @param[in] eventT (EVENT_SE_ALL or EVENT_AT_ALL) used for error checking purposes:
 * If the given prefix does not exist in the associated partition, the QName MUST be part of an SE event and
 * the prefix MUST be resolved by one of the NS events immediately following the SE event (see resolution rules below).
 * @param[out] uriID the QName uriID
 * @param[out] lnID the QName lnID
 * @return Error handling code
 */
errorCode encodeQName(EXIStream* strm, QName qname, EventType eventT, uint16_t* uriID, size_t* lnID);

/**
 * @brief Encodes URI into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] uri uri to be written
 * @param[out] uriID id of the uri row written in the string table
 * @return Error handling code
 */
errorCode encodeURI(EXIStream* strm, String* uri, uint16_t* uriID);

/**
 * @brief Encodes Local Name into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] ln ln to be written
 * @param[in] uriID id of the uri row of the URI the string table
 * @param[out] lnID id of the LN row written in the string table
 * @return Error handling code
 */
errorCode encodeLocalName(EXIStream* strm, String* ln, uint16_t uriID, size_t* lnID);

/**
 * @brief Encodes the prefix part of the QName into the EXI stream in case the Preserve.prefixes == TRUE
 * @param[in, out] strm EXI stream
 * @param[in] qname qname to be written
 * @param[in] eventT (EVENT_SE_ALL or EVENT_AT_ALL) used for error checking purposes:
 * If the given prefix does not exist in the associated partition, the QName MUST be part of an SE event and
 * the prefix MUST be resolved by one of the NS events immediately following the SE event (see resolution rules below).
 * @param[in] uriID the QName uriID
 * @return Error handling code
 */
errorCode encodePrefixQName(EXIStream* strm, QName* qname, EventType eventT, uint16_t uriID);

/**
 * @brief Encodes the prefix part of the NS event into the EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] uriID id of the uri row of the URI the string table
 * @param[in] prefix prefix to be written
 * @return Error handling code
 */
errorCode encodePrefix(EXIStream* strm, uint16_t uriID, String* prefix);

/**
 * @brief Encodes Integer value into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] int_val integer to be written
 * @param[in] intType how the integer is represented: unsigned int, small int or regular int
 * @return Error handling code
 */
errorCode encodeIntData(EXIStream* strm, Integer int_val, ValueType intType);

#endif /* BODYENCODE_H_ */
