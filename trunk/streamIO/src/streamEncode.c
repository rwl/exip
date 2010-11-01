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

errorCode encodeNBitUnsignedInteger(EXIStream* strm, uint32_t int_val)
{
	if(strm->opts->compression == 0 && strm->opts->alignment == BIT_PACKED)
	{
		return writeBits(strm, int_val);
	}
	else
	{
		int nbits = getBitsNumber(int_val);
		int byte_number = nbits / 8 + (nbits % 8 != 0);
		int tmp_byte_buf = 0;
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		int i = 0;
		for(i = 0; i < byte_number; i++)
		{
			tmp_byte_buf = (int_val & (255ul << (i * 8))) >> (i * 8);

			tmp_err_code = writeNBits(strm, 8, tmp_byte_buf);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	return ERR_OK;
}

errorCode encodeBoolean(EXIStream* strm, unsigned char bool_val)
{
	//TODO:  when pattern facets are available in the schema datatype - handle it differently
	return writeNextBit(strm, bool_val);
}

errorCode encodeUnsignedInteger(EXIStream* strm, uint32_t int_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int nbits = getBitsNumber(int_val);
	int nbyte7 = nbits / 7 + (nbits % 7 != 0);
	int tmp_byte_buf = 0;
	int i = 0;
	for(i = 0; i < nbyte7; i++)
	{
		tmp_byte_buf = (int_val & (127ul << (i * 7))) >> (i * 7);
		if(i == nbyte7 - 1)
			writeNextBit(strm, 0);
		else
			writeNextBit(strm, 1);

		tmp_err_code = writeNBits(strm, 7, tmp_byte_buf);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode encodeBigUnsignedInteger(EXIStream* strm, BigUnsignedInt int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeString(EXIStream* strm, const StringType* string_val)
{
	// Assume no Restricted Character Set is defined
	//TODO: Handle the case when Restricted Character Set is defined

	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = encodeUnsignedInteger(strm, string_val->length);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	uint32_t tmp_val= 0;
	uint32_t i = 0;
	for(; i < string_val->length; i++)
	{
		tmp_err_code = getUCSCodePoint(string_val, i, &tmp_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = encodeUnsignedInteger(strm, tmp_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode encodeBinary(EXIStream* strm, char* binary_val, uint32_t nbytes)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeIntegerValue(EXIStream* strm, int32_t sint_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeBigIntegerValue(EXIStream* strm, BigSignedInt sint_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeDecimalValue(EXIStream* strm, decimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeBigDecimalValue(EXIStream* strm, bigDecimal dec_val)
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
