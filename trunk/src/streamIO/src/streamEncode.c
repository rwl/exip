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

#include "streamEncode.h"
#include "streamWrite.h"
#include "stringManipulate.h"
#include "ioUtil.h"
#include <math.h>


errorCode encodeNBitUnsignedInteger(EXIStream* strm, unsigned char n, uint32_t int_val)
{
	if(strm->header.opts->compression == 0 && strm->header.opts->alignment == BIT_PACKED)
	{
		return writeNBits(strm, n, int_val);
	}
	else
	{
		unsigned int byte_number = n / 8 + (n % 8 != 0);
		int tmp_byte_buf = 0;
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		unsigned int i = 0;
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
	unsigned int nbits = getBitsNumber(int_val);
	int nbyte7 = nbits / 7 + (nbits % 7 != 0);
	int tmp_byte_buf = 0;
	int i = 0;
	if(nbyte7 == 0)
		nbyte7 = 1;  // the 0 Unsigned Integer is encoded with one 7bit byte
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

	tmp_err_code = encodeUnsignedInteger(strm, (uint32_t)(string_val->length) );
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeStringOnly(strm, string_val);
}

errorCode encodeStringOnly(EXIStream* strm, const StringType* string_val)
{
	// Assume no Restricted Character Set is defined
	//TODO: Handle the case when Restricted Character Set is defined

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t tmp_val= 0;
	size_t i = 0;
	for(i = 0; i < string_val->length; i++)
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

errorCode encodeBinary(EXIStream* strm, char* binary_val, size_t nbytes)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t i = 0;

	tmp_err_code = encodeUnsignedInteger(strm, (uint32_t) nbytes);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 0; i < nbytes; i++)
	{
		tmp_err_code = writeNBits(strm, 8, (uint32_t) binary_val[i]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode encodeIntegerValue(EXIStream* strm, int32_t sint_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t uval;
	unsigned char sign;
	if(sint_val >= 0)
	{
		sign = 0;
		uval = (uint32_t) sint_val;
	}
	else
	{
		uval = (uint32_t) -sint_val;
		sign = 1;
	}
	tmp_err_code = writeNextBit(strm, sign);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	return encodeUnsignedInteger(strm, uval);
}

errorCode encodeBigIntegerValue(EXIStream* strm, BigSignedInt sint_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeDecimalValue(EXIStream* strm, decimal dec_val)
{
	// TODO: Review this. Probably incorrect in some cases and not efficient. Depends on decimal floating point support!
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char sign = 0;
	uint32_t integr_part = 0;
	uint32_t fract_part_rev = 0;
	unsigned int i = 1;
	unsigned int d = 0;

	if(dec_val >= 0)
		sign = 0;
	else
	{
		dec_val = -dec_val;
		sign = 1;
	}

	tmp_err_code = encodeBoolean(strm, sign);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	integr_part = (uint32_t) dec_val;

	tmp_err_code = encodeUnsignedInteger(strm, integr_part);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	dec_val = dec_val - integr_part;

	while(dec_val - ((uint32_t) dec_val) != 0)
	{
		dec_val = dec_val * 10;
		d = (unsigned int) dec_val;
		fract_part_rev = fract_part_rev + d*i;
		i = i*10;
		dec_val = dec_val - (uint32_t) dec_val;
	}

	tmp_err_code = encodeUnsignedInteger(strm, fract_part_rev);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode encodeBigDecimalValue(EXIStream* strm, bigDecimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode encodeFloatValue(EXIStream* strm, double double_val)
{
	// TODO: Review this. Probably incorrect in some cases and not efficient. Depends on decimal floating point support!
	// There should be a function from a library that can be reused for the conversion from base 2 to base 10 floating points
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int32_t mant = 0;	//mantissa
	int32_t expt = 0;	//exponent

	if(double_val == INFINITY)
	{
		expt = -(0x1<<14); // expt == -2^14
		mant = 1;
	}
	else if(double_val == -INFINITY)
	{
		expt = -(0x1<<14); // expt == -2^14
		mant = -1;
	}
	else if(double_val == NAN)
	{
		expt = -(0x1<<14); // expt == -2^14
		mant = 5;
	}
	else
	{
		decimal tmp_dec = double_val;
		int tmp_expt = 0;

		while(tmp_dec - ((int32_t) tmp_dec) != 0)
		{
			tmp_dec = tmp_dec*10;
			tmp_expt++;
		}

		mant = (int32_t) tmp_dec;
		expt = tmp_expt;
	}

	tmp_err_code = encodeIntegerValue(strm, mant);	//encode mantissa
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = encodeIntegerValue(strm, expt);	//encode exponent
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode encodeBigFloatValue(EXIStream* strm, BigFloat double_val)
{
	return NOT_IMPLEMENTED_YET;
}
