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
 * @file EXISerializer.h
 * @brief Interface for serializing an EXI stream
 * Application will use this interface to work with the EXIP serializer
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef EXISERIALIZER_H_
#define EXISERIALIZER_H_

#include "errorHandle.h"
#include "procTypes.h"

struct EXISerializer
{
	// For handling the meta-data (document structure)
	errorCode (*startDocument)(EXIStream* strm);
	errorCode (*endDocument)(EXIStream* strm);
	errorCode (*startElement)(EXIStream* strm, QName qname);
	errorCode (*endElement)(EXIStream* strm);
	errorCode (*attribute)(EXIStream* strm, QName qname, EXIType valueType);

	// For handling the data
	errorCode (*intData)(EXIStream* strm, Integer int_val);
	errorCode (*booleanData)(EXIStream* strm, unsigned char bool_val);
	errorCode (*stringData)(EXIStream* strm, const String str_val);
	errorCode (*floatData)(EXIStream* strm, Float float_val);
	errorCode (*binaryData)(EXIStream* strm, const char* binary_val, size_t nbytes);
	errorCode (*dateTimeData)(EXIStream* strm, struct tm dt_val, uint16_t presenceMask);
	errorCode (*decimalData)(EXIStream* strm, Decimal dec_val);

	// Miscellaneous
	errorCode (*processingInstruction)(EXIStream* strm); // TODO: define the parameters!
	errorCode (*namespaceDeclaration)(EXIStream* strm, const String namespace, const String prefix, unsigned char isLocalElementNS);

	// EXI specific
	errorCode (*exiHeader)(EXIStream* strm);
	errorCode (*selfContained)(EXIStream* strm);  // Used for indexing independent elements for random access

	// EXIP specific
	void (*initHeader)(EXIStream* strm);
	errorCode (*initStream)(EXIStream* strm, char* buf, size_t bufSize, IOStream* ioStrm, EXIPSchema* schema, unsigned char schemaIdMode, String* schemaID);
	errorCode (*closeEXIStream)(EXIStream* strm);
};

typedef struct EXISerializer EXISerializer;

/**** START: Serializer API implementation  ****/

// For handling the meta-data (document structure)
errorCode startDocument(EXIStream* strm);
errorCode endDocument(EXIStream* strm);
errorCode startElement(EXIStream* strm, QName qname);
errorCode endElement(EXIStream* strm);
errorCode attribute(EXIStream* strm, QName qname, EXIType exiType);

// For handling the data
errorCode intData(EXIStream* strm, Integer int_val);
errorCode booleanData(EXIStream* strm, unsigned char bool_val);
errorCode stringData(EXIStream* strm, const String str_val);
errorCode floatData(EXIStream* strm, Float float_val);
errorCode binaryData(EXIStream* strm, const char* binary_val, size_t nbytes);
errorCode dateTimeData(EXIStream* strm, struct tm dt_val, uint16_t presenceMask);
errorCode decimalData(EXIStream* strm, Decimal dec_val);

// Miscellaneous
errorCode processingInstruction(EXIStream* strm); // TODO: define the parameters!
errorCode namespaceDeclaration(EXIStream* strm, const String namespace, const String prefix, unsigned char isLocalElementNS);

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


/**** START: Fast, low level API for schema encoding only ****/

/**
 * To be used by code generation tools such as static XML bindings
 * and when efficiency is of high importance
 *
 * @param[in, out] strm EXI stream
 * @param[in] codeLength 1,2 or 3 is the allowed length of EXI event codes
 * @param[in] lastCodePart the last part of the event code
 * @param[in] qname used only for SE(*), AT(*), SE(uri:*), AT(uri:*) and when
 * a new prefix should be serialized in SE(QName) and AT(QName); NULL otherwise
 */
errorCode serializeEvent(EXIStream* strm, unsigned char codeLength, size_t lastCodePart, QName* qname);

/****  END: Fast, low level API for schema encoding only ****/

#endif /* EXISERIALIZER_H_ */
