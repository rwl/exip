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
 * @file streamDecode.c
 * @brief Implementing the interface to a higher-level EXI stream decoder - decode basic EXI types
 *
 * @date Aug 18, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "streamDecode.h"
#include "streamRead.h"
#include "stringManipulate.h"
#include <math.h>

errorCode decodeNBitUnsignedInteger(EXIStream* strm, unsigned char n, unsigned int* int_val)
{
	if(WITH_COMPRESSION(strm->header.opts.enumOpt) == FALSE && GET_ALIGNMENT(strm->header.opts.enumOpt) == BIT_PACKED)
	{
		return readBits(strm, n, int_val);
	}
	else
	{
		unsigned int byte_number = ((unsigned int) n) / 8 + (n % 8 != 0);
		uint32_t tmp_byte_buf = 0;
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		unsigned int i = 0;

		*int_val = 0;
		for(i = 0; i < byte_number; i++)
		{
			tmp_err_code = readBits(strm, 8, &tmp_byte_buf);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			tmp_byte_buf = tmp_byte_buf << (i * 8);
			*int_val = *int_val | tmp_byte_buf;
		}
	}
	return ERR_OK;
}

errorCode decodeBoolean(EXIStream* strm, unsigned char* bool_val)
{
	//TODO:  when pattern facets are available in the schema datatype - handle it differently
	return readNextBit(strm, bool_val);
}

errorCode decodeUnsignedInteger(EXIStream* strm, UnsignedInteger* int_val)
{
	int mask_7bits = 127;
	int mask_8th_bit = 128;
	unsigned int initial_multiplier = 1;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_byte_buf = 0;
	int more_bytes_to_read = 0;
	*int_val = 0;

	do
	{
		tmp_err_code = readBits(strm, 8, &tmp_byte_buf);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		more_bytes_to_read = tmp_byte_buf & mask_8th_bit;
		tmp_byte_buf = tmp_byte_buf & mask_7bits;
		*int_val = *int_val + (initial_multiplier * tmp_byte_buf);
		initial_multiplier = initial_multiplier * 128;
	}
	while(more_bytes_to_read != 0);

	return ERR_OK;
}

errorCode decodeString(EXIStream* strm, String* string_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	UnsignedInteger string_length = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &string_length);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	string_val->length = (size_t) string_length;

	tmp_err_code = decodeStringOnly(strm, string_val->length, string_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	return ERR_OK;
}

errorCode decodeStringOnly(EXIStream* strm, size_t str_length, String* string_val)
{
	// Assume no Restricted Character Set is defined
	//TODO: Handle the case when Restricted Character Set is defined

	// The exact size of the string is known at this point. This means that
	// this is the place to allocate the memory for the  { CharType* str; }!!!
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t i = 0;
	size_t UCSCharPosition = 0;
	UnsignedInteger tmp_code_point = 0;
	tmp_err_code = allocateStringMemory(&(string_val->str), str_length, &strm->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	string_val->length = str_length;

	for(i = 0; i < str_length; i++)
	{
		tmp_err_code = decodeUnsignedInteger(strm, &tmp_code_point);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		writeCharToString(string_val, (uint32_t) tmp_code_point, &UCSCharPosition);
	}
	return ERR_OK;
}

errorCode decodeBinary(EXIStream* strm, char** binary_val, size_t* nbytes)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	UnsignedInteger length = 0;
	unsigned int int_val = 0;
	UnsignedInteger i = 0;

	tmp_err_code = decodeUnsignedInteger(strm, &length);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	*nbytes = (size_t) length;
	(*binary_val) = (char*) EXIP_MALLOC(length); // This memory should be manually freed after the content handler is invoked
	if((*binary_val) == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(i = 0; i < length; i++)
	{
		tmp_err_code = readBits(strm, 8, &int_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		(*binary_val)[i]=(char) int_val;
	}
	return ERR_OK;
}

errorCode decodeIntegerValue(EXIStream* strm, Integer* sint_val)
{
	// TODO: If there is associated schema datatype handle differently!
	// TODO: check if the result fit into int type
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char bool_val = 0;
	UnsignedInteger val;
	tmp_err_code = decodeBoolean(strm, &bool_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = decodeUnsignedInteger(strm, &val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(bool_val == 0) // A sign value of zero (0) is used to represent positive integers
		*sint_val = (Integer) val;
	else if(bool_val == 1) // A sign value of one (1) is used to represent negative integers
		*sint_val = -((Integer) val);
	else
		return UNEXPECTED_ERROR;
	return ERR_OK;
}

errorCode decodeDecimalValue(EXIStream* strm, Decimal* dec_val)
{
	// TODO: Review this. Probably can be more efficient. Depends on decimal floating point support!
	// Ref: http://gcc.gnu.org/onlinedocs/gccint/Decimal-float-library-routines.html
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char sign;
	UnsignedInteger integr_part = 0;
	UnsignedInteger fract_part = 0;
	unsigned int fraction_digits = 1;
	UnsignedInteger fract_part_rev = 0;

	tmp_err_code = decodeBoolean(strm, &sign);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = decodeUnsignedInteger(strm, &integr_part);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = decodeUnsignedInteger(strm, &fract_part);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	fract_part_rev = 0;
	while(fract_part > 0)
	{
		fract_part_rev = fract_part_rev*10 + fract_part%10;
		fract_part = fract_part/10;
		fraction_digits = fraction_digits*10;
	}
	*dec_val = fract_part_rev;

	if(sign == 1) // negative number
		*dec_val = -*dec_val;

	*dec_val = *dec_val / fraction_digits;

	*dec_val = *dec_val + integr_part;

	return ERR_OK;
}

errorCode decodeFloatValue(EXIStream* strm, Float* fl_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Integer mantissa;
	Integer exponent;

	tmp_err_code = decodeIntegerValue(strm, &mantissa);	//decode mantissa
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = decodeIntegerValue(strm, &exponent);	//decode exponent
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(exponent >= (1 << 14) || exponent < -(1 << 14)
			|| mantissa >= ((uint64_t) 1 << 63) ||
			(mantissa < 0 && -mantissa > ((uint64_t) 1 << 63))
			)
	{
		return INVALID_EXI_INPUT;
	}

	fl_val->mantissa = mantissa;
	fl_val->exponent = exponent;

	return ERR_OK;
}
