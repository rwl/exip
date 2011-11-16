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

/**** START: Serializer API implementation  ****/

// For handling the meta-data (document structure)
errorCode startDocument(EXIStream* strm, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode endDocument(EXIStream* strm, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode startElement(EXIStream* strm, QName qname, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode endElement(EXIStream* strm, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode attribute(EXIStream* strm, QName qname, EXIType exiType, unsigned char fastSchemaMode, size_t schemaProduction);

// For handling the data
errorCode intData(EXIStream* strm, Integer int_val, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode booleanData(EXIStream* strm, unsigned char bool_val, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode stringData(EXIStream* strm, const String str_val, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode floatData(EXIStream* strm, Float float_val, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode binaryData(EXIStream* strm, const char* binary_val, size_t nbytes, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode dateTimeData(EXIStream* strm, struct tm dt_val, uint16_t presenceMask, unsigned char fastSchemaMode, size_t schemaProduction);
errorCode decimalData(EXIStream* strm, Decimal dec_val, unsigned char fastSchemaMode, size_t schemaProduction);

// Miscellaneous
errorCode processingInstruction(EXIStream* strm); // TODO: define the parameters!

// EXI specific
errorCode selfContained(EXIStream* strm);  // Used for indexing independent elements for random access

// EXIP specific
void initHeader(EXIStream* strm);

/**
 * @brief Encodes String value into EXI stream
 *
 * @param[in, out] strm EXI stream
 * @param[in, out] buf binary buffer for storing the encodded EXI stream
 * @param[in] bufSize the size of the buffer in bytes
 * @param[in] ioStrm defines an output stream to be used to flush the binary buffer when full, NULL if no such output stream exists
 * @param[in] schema a compiled schema information to be used for schema enabled processing, NULL if no schema is available
 * @param[in] schemaIdMode one of SCHEMA_ID_ABSENT, SCHEMA_ID_SET, SCHEMA_ID_NIL or SCHEMA_ID_EMPTY
 * @param[in] schemaID if in SCHEMA_ID_SET a valid string representing the schemaID, NULL otherwise
 * @return Error handling code
 */
errorCode initStream(EXIStream* strm, char* buf, size_t bufSize, IOStream* ioStrm, EXIPSchema *schema,
					unsigned char schemaIdMode, String* schemaID);


errorCode closeEXIStream(EXIStream* strm);

/****  END: Serializer API implementation  ****/


/**
 * @brief Encodes String value into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] strng string to be written
 * @return Error handling code
 */
errorCode encodeStringData(EXIStream* strm, String strng);

/**
 * @brief Encodes SD, ED, EE, CH events
 * @param[in, out] strm EXI stream
 * @param[in] event event to be encoded
 * @param[in] fastSchemaMode - TRUE/FALSE, require valid schemaProduction order number
 * @param[in] schemaProduction the order number of the schema production (starting from 0), only needed if fastSchemaMode == TRUE
 * @param[out] prodType the valueType of the production hit in the grammar
 * @return Error handling code
 */
errorCode encodeSimpleEXIEvent(EXIStream* strm, EXIEvent event, unsigned char fastSchemaMode, size_t schemaProduction, ValueType* prodType);

/**
 * @brief Encodes SE, AT events
 * @param[in, out] strm EXI stream
 * @param[in] qname element or attribute QName
 * @param[in] event_all EVENT_SE_ALL or EVENT_AT_ALL
 * @param[in] event_uri EVENT_SE_URI or EVENT_AT_URI
 * @param[in] event_qname EVENT_SE_QNAME or EVENT_AT_QNAME
 * @param[in] exiType used for AT events
 * @param[in] fastSchemaMode - TRUE/FALSE, require valid schemaProduction order number
 * @param[in] schemaProduction the order number of the schema production (starting from 0), only needed if fastSchemaMode == TRUE
 * @return Error handling code
 */
errorCode encodeComplexEXIEvent(EXIStream* strm, QName qname, EventType event_all, EventType event_uri,
						EventType event_qname, EXIType exiType, unsigned char fastSchemaMode, size_t schemaProduction);

/**
 * @brief Encodes QName into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] qname qname to be written
 * @param[in] eventT (EVENT_SE_ALL or EVENT_AT_ALL) used for error checking purposes:
 * If the given prefix does not exist in the associated partition, the QName MUST be part of an SE event and
 * the prefix MUST be resolved by one of the NS events immediately following the SE event (see resolution rules below).
 * @return Error handling code
 */
errorCode encodeQName(EXIStream* strm, QName qname, EventType eventT);

/**
 * @brief Encodes the prefix part of the QName into the EXI stream in case the Preserve.prefixes == TRUE
 * @param[in, out] strm EXI stream
 * @param[in] qname qname to be written
 * @param[in] eventT (EVENT_SE_ALL or EVENT_AT_ALL) used for error checking purposes:
 * If the given prefix does not exist in the associated partition, the QName MUST be part of an SE event and
 * the prefix MUST be resolved by one of the NS events immediately following the SE event (see resolution rules below).
 * @return Error handling code
 */
errorCode encodePrefixQName(EXIStream* strm, QName qname, EventType eventT);

/**
 * @brief Encodes Integer value into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] int_val integer to be written
 * @param[in] intType how the integer is represented: unsigned int, small int or regular int
 * @return Error handling code
 */
errorCode encodeIntData(EXIStream* strm, Integer int_val, ValueType intType);

#endif /* BODYENCODE_H_ */
