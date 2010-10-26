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
 * @file streamEncode.c
 * @brief Implements an interface to a higher-level EXI stream encoder - encode basic EXI types
 *
 * @date Oct 26, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "../include/streamEncode.h"
#include "../include/streamWrite.h"

errorCode encodeNBitUnsignedInteger(EXIStream* strm, unsigned char n, uint32_t int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeBoolean(EXIStream* strm, unsigned char bool_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeUnsignedInteger(EXIStream* strm, uint32_t int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeBigUnsignedInteger(EXIStream* strm, BigUnsignedInt int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeString(EXIStream* strm, const StringType* string_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeBinary(EXIStream* strm, char* binary_val, uint32_t nbytes)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeIntegerValue(EXIStream* strm, int32_t sint_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decodeBigIntegerValue(EXIStream* strm, BigSignedInt sint_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeDecimalValue(EXIStream* strm, decimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decodeBigDecimalValue(EXIStream* strm, bigDecimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeFloatValue(EXIStream* strm, double double_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeBigFloatValue(EXIStream* strm, BigFloat double_val)
{
	return NOT_IMPLEMENTED_YET;
}
