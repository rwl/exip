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
 * @author Ashok Gowtham
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "../include/streamDecode.h"
#include "../include/streamRead.h"
#include "stringManipulate.h"

errorCode decodeNBitUnsignedInteger(EXIStream* strm, unsigned char n, uint32_t* int_val)
{
	if(strm->opts->compression == 0 && strm->opts->alignment == BIT_PACKED)
	{
		return readBits(strm, n, int_val);
	}
	else
	{
		int byte_number = ((int) n) / 8 + (n % 8 != 0);
		int tmp_byte_buf = 0;
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		*int_val = 0;
		int i = 0;
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

errorCode decodeUnsignedInteger(EXIStream* strm, uint32_t* int_val)
{
	// TODO: handle situations where the UnsignedInteger cannot fit into unsigned int type!
	//       In this case it should return BIGGER_TYPE_REQUIRED error and position the
	//       stream pointer at initial position
	int mask_7bits = 127;
	int mask_8th_bit = 128;
	unsigned int initial_multiplier = 1;
	*int_val = 0;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int tmp_byte_buf = 0;
	int more_bytes_to_read = 0;

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

errorCode decodeBigUnsignedInteger(EXIStream* strm, BigUnsignedInt* int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decodeString(EXIStream* strm, StringType* string_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t string_length = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &string_length);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	string_val->length = string_length;

	tmp_err_code = decodeStringOnly(strm, string_length, string_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	return ERR_OK;
}

errorCode decodeStringOnly(EXIStream* strm, uint32_t str_length, StringType* string_val)
{
	// Assume no Restricted Character Set is defined
	//TODO: Handle the case when Restricted Character Set is defined

	// The exact size of the string is known at this point. This means that
	// this is the place to allocate the memory for the  { CharType* str; }!!!
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	tmp_err_code = allocateStringMemory(&(string_val->str), str_length, strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	string_val->length = str_length;

	uint32_t i = 0;
	uint32_t tmp_code_point = 0;
	for(i = 0; i < str_length; i++)
	{
		tmp_err_code = decodeUnsignedInteger(strm, &tmp_code_point);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		writeCharToString(string_val, tmp_code_point, i);
	}
	return ERR_OK;
}

errorCode decodeBinary(EXIStream* strm, char** binary_val, uint32_t* nbytes)
{
	errorCode err;
	uint32_t length=0;
	unsigned int int_val=0;

	err = decodeUnsignedInteger(strm, &length);
	if(err!=ERR_OK) return err;
	*nbytes = length;
	(*binary_val) = EXIP_MALLOC(length); // This memory should be manually freed after the content handler is invoked
	if((*binary_val) == NULL)
		return MEMORY_ALLOCATION_ERROR;

	uint32_t i = 0;
	for(i = 0; i < length; i++)
	{
		err = readBits(strm,8,&int_val);
		if(err!=ERR_OK) return err;
		(*binary_val)[i]=(char)int_val;
	}
	return ERR_OK;
}

errorCode decodeIntegerValue(EXIStream* strm, int32_t* sint_val)
{
	// TODO: If there is associated schema datatype handle differently!
	// TODO: check if the result fit into int type
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char bool_val = 0;
	tmp_err_code = decodeBoolean(strm, &bool_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(bool_val == 0) // A sign value of zero (0) is used to represent positive integers
	{
		tmp_err_code = decodeUnsignedInteger(strm, sint_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(bool_val == 1) // A sign value of one (1) is used to represent negative integers
	{
		tmp_err_code = decodeUnsignedInteger(strm, sint_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		*sint_val = -*sint_val;
	}
	else
		return UNEXPECTED_ERROR;
	return ERR_OK;
}

errorCode decodeBigIntegerValue(EXIStream* strm, BigSignedInt* sint_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decodeDecimalValue(EXIStream* strm, decimal* dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decodeBigDecimalValue(EXIStream* strm, bigDecimal* dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decodeFloatValue(EXIStream* strm, double* double_val)
{
//refer : http://www.linuxquestions.org/questions/programming-9/c-language-inf-and-nan-437323/
	errorCode err;

	double val;	// val = man * 10^exp
	int man;	//mantissa
	int exp;	//exponent

	err = decodeIntegerValue(strm,&man);	//decode mantissa
	if(err!=ERR_OK) return err;
	err = decodeIntegerValue(strm,&exp);	//decode exponent
	if(err!=ERR_OK) return err;

	if(exp == -(0x1<<14))	//if exp == -2^14
	{
		if(man==1)
		{
			//val =  +INF
			val=0x7f800000;
		}
		else if(man==-1)
		{
			//val =  -INF
			val=0xff800000;
		}
		else
		{
			//val = NaN
			val=0x7fc00000;
		}
	}
	else
	{
		val=man;
		//apply the exponent  (man * 10^exp)
		if(exp>0)
		{
			while(exp)
			{
				val*=10;
				exp--;
			}
		}
		else
		{
			while(exp)
			{
				val/=10;
				exp++;
			}
		}
	}
	*double_val=val;
	return ERR_OK;
}

errorCode decodeBigFloatValue(EXIStream* strm, BigFloat* double_val)
{
	return NOT_IMPLEMENTED_YET;
}
