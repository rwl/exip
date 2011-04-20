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
#include "schema.h"

struct EXISerializer
{
	// For handling the meta-data (document structure)
	errorCode (*startDocumentSer)(EXIStream* strm);
	errorCode (*endDocumentSer)(EXIStream* strm);
	errorCode (*startElementSer)(EXIStream* strm, QName qname);
	errorCode (*endElementSer)(EXIStream* strm);
	errorCode (*attributeSer)(EXIStream* strm, QName qname);

	// For handling the data
	errorCode (*intDataSer)(EXIStream* strm, int32_t int_val);
	errorCode (*bigIntDataSer)(EXIStream* strm, const BigSignedInt int_val);
	errorCode (*booleanDataSer)(EXIStream* strm, unsigned char bool_val);
	errorCode (*stringDataSer)(EXIStream* strm, const StringType str_val);
	errorCode (*floatDataSer)(EXIStream* strm, double float_val);
	errorCode (*bigFloatDataSer)(EXIStream* strm, BigFloat float_val);
	errorCode (*binaryDataSer)(EXIStream* strm, const char* binary_val, size_t nbytes);
	errorCode (*dateTimeDataSer)(EXIStream* strm, struct tm dt_val, uint16_t presenceMask);
	errorCode (*decimalDataSer)(EXIStream* strm, decimal dec_val);
	errorCode (*bigDecimalDataSer)(EXIStream* strm, bigDecimal dec_val);

	// Miscellaneous
	errorCode (*processingInstructionSer)(EXIStream* strm); // TODO: define the parameters!

	// EXI specific
	errorCode (*exiHeaderSer)(EXIStream* strm, EXIheader* header);
	errorCode (*selfContainedSer)(EXIStream* strm);  // Used for indexing independent elements for random access

	// EXIP specific
	errorCode (*initStream)(EXIStream* strm, char* buf, size_t bufSize, IOStream* ioStrm, struct EXIOptions* opts, ExipSchema* schema);
	errorCode (*closeEXIStream)(EXIStream* strm);
};

typedef struct EXISerializer EXISerializer;

#endif /* EXISERIALIZER_H_ */
