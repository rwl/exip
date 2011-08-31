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


errorCode encodeNBitUnsignedInteger(EXIStream* strm, unsigned char n, unsigned int int_val)
{
	if(WITH_COMPRESSION(strm->header.opts.enumOpt) == FALSE && GET_ALIGNMENT(strm->header.opts.enumOpt) == BIT_PACKED)
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

errorCode encodeUnsignedInteger(EXIStream* strm, UnsignedInteger int_val)
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

errorCode encodeString(EXIStream* strm, const String* string_val)
{
	// Assume no Restricted Character Set is defined
	//TODO: Handle the case when Restricted Character Set is defined

	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = encodeUnsignedInteger(strm, (UnsignedInteger)(string_val->length) );
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeStringOnly(strm, string_val);
}

errorCode encodeStringOnly(EXIStream* strm, const String* string_val)
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
		tmp_err_code = encodeUnsignedInteger(strm, (UnsignedInteger) tmp_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode encodeBinary(EXIStream* strm, char* binary_val, size_t nbytes)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t i = 0;

	tmp_err_code = encodeUnsignedInteger(strm, (UnsignedInteger) nbytes);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 0; i < nbytes; i++)
	{
		tmp_err_code = writeNBits(strm, 8, (unsigned int) binary_val[i]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode encodeIntegerValue(EXIStream* strm, Integer sint_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	UnsignedInteger uval;
	unsigned char sign;
	if(sint_val >= 0)
	{
		sign = 0;
		uval = (UnsignedInteger) sint_val;
	}
	else
	{
		uval = (UnsignedInteger) -sint_val;
		sign = 1;
	}
	tmp_err_code = writeNextBit(strm, sign);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	return encodeUnsignedInteger(strm, uval);
}

errorCode encodeDecimalValue(EXIStream* strm, Decimal dec_val)
{
	// TODO: Review this. Probably incorrect in some cases and not efficient. Depends on decimal floating point support!
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char sign = 0;
	UnsignedInteger integr_part = 0;
	UnsignedInteger fract_part_rev = 0;
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

	integr_part = (UnsignedInteger) dec_val;

	tmp_err_code = encodeUnsignedInteger(strm, integr_part);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	dec_val = dec_val - integr_part;

	while(dec_val - ((UnsignedInteger) dec_val) != 0)
	{
		dec_val = dec_val * 10;
		d = (unsigned int) dec_val;
		fract_part_rev = fract_part_rev + d*i;
		i = i*10;
		dec_val = dec_val - (UnsignedInteger) dec_val;
	}

	tmp_err_code = encodeUnsignedInteger(strm, fract_part_rev);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode encodeFloatValue(EXIStream* strm, Float fl_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = encodeIntegerValue(strm, (Integer) fl_val.mantissa);	//encode mantissa
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = encodeIntegerValue(strm, (Integer) fl_val.exponent);	//encode exponent
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}
