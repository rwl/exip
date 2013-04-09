/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file streamDecode.c
 * @brief Implementing the interface to a higher-level EXI stream decoder - decode basic EXI types
 *
 * @date Aug 18, 2010
 * @author Rumen Kyusakov
 * @version 0.4
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
		unsigned int tmp_byte_buf = 0;
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		unsigned int i = 0;

		*int_val = 0;
		for(i = 0; i < byte_number; i++)
		{
			TRY(readBits(strm, 8, &tmp_byte_buf));
			tmp_byte_buf = tmp_byte_buf << (i * 8);
			*int_val = *int_val | tmp_byte_buf;
		}
	}
	return ERR_OK;
}

errorCode decodeBoolean(EXIStream* strm, boolean* bool_val)
{
	//TODO:  when pattern facets are available in the schema datatype - handle it differently
	return readNextBit(strm, bool_val);
}

errorCode decodeUnsignedInteger(EXIStream* strm, UnsignedInteger* int_val)
{
	unsigned int mask_7bits = 0x7F;
	unsigned int mask_8th_bit = 0x80;
	unsigned int i = 0;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_byte_buf = 0;
	unsigned int more_bytes_to_read = 0;
	*int_val = 0;

	do
	{
		TRY(readBits(strm, 8, &tmp_byte_buf));

		more_bytes_to_read = tmp_byte_buf & mask_8th_bit;
		tmp_byte_buf = tmp_byte_buf & mask_7bits;
		*int_val += ((UnsignedInteger) tmp_byte_buf) << (7*i);
		i++;
	}
	while(more_bytes_to_read != 0);

	return ERR_OK;
}

errorCode decodeString(EXIStream* strm, String* string_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	UnsignedInteger string_length = 0;
	TRY(decodeUnsignedInteger(strm, &string_length));

	TRY(allocateStringMemoryManaged(&(string_val->str),(Index) string_length, &strm->memList));

	return decodeStringOnly(strm,(Index)  string_length, string_val);
}

errorCode decodeStringOnly(EXIStream* strm, Index str_length, String* string_val)
{
	// Assume no Restricted Character Set is defined
	//TODO: Handle the case when Restricted Character Set is defined

	// The exact size of the string is known at this point. This means that
	// this is the place to allocate the memory for the  { CharType* str; }!!!
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Index i = 0;
	Index writerPosition = 0;
	UnsignedInteger tmp_code_point = 0;

	string_val->length = str_length;

	for(i = 0; i < str_length; i++)
	{
		TRY(decodeUnsignedInteger(strm, &tmp_code_point));
		TRY(writeCharToString(string_val, (uint32_t) tmp_code_point, &writerPosition));
	}
	return ERR_OK;
}

errorCode decodeBinary(EXIStream* strm, char** binary_val, Index* nbytes)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	UnsignedInteger length = 0;
	unsigned int int_val = 0;
	UnsignedInteger i = 0;

	TRY(decodeUnsignedInteger(strm, &length));
	*nbytes = (Index) length;
	(*binary_val) = (char*) EXIP_MALLOC(length); // This memory should be manually freed after the content handler is invoked
	if((*binary_val) == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(i = 0; i < length; i++)
	{
		tmp_err_code = readBits(strm, 8, &int_val);
		if(tmp_err_code != ERR_OK)
		{
			EXIP_MFREE(*binary_val);
			return tmp_err_code;
		}
		(*binary_val)[i]=(char) int_val;
	}
	return ERR_OK;
}

errorCode decodeIntegerValue(EXIStream* strm, Integer* sint_val)
{
	// TODO: If there is associated schema datatype handle differently!
	// TODO: check if the result fit into int type
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	boolean bool_val = 0;
	UnsignedInteger val;
	TRY(decodeBoolean(strm, &bool_val));

	TRY(decodeUnsignedInteger(strm, &val));

	if(bool_val == 0) // A sign value of zero (0) is used to represent positive integers
		*sint_val = (Integer) val;
	else if(bool_val == 1) // A sign value of one (1) is used to represent negative integers
	{
		val += 1;
		*sint_val = -((Integer) val);
	}
	else
		return UNEXPECTED_ERROR;
	return ERR_OK;
}

errorCode decodeDecimalValue(EXIStream* strm, Decimal* dec_val)
{
	// TODO: Review this. Probably can be more efficient. Depends on decimal floating point support!
	// Ref: http://gcc.gnu.org/onlinedocs/gccint/Decimal-float-library-routines.html
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	boolean sign;
	UnsignedInteger integr_part = 0;
	UnsignedInteger fract_part = 0;
	unsigned int fraction_digits = 1;
	UnsignedInteger fract_part_rev = 0;

	TRY(decodeBoolean(strm, &sign));

	TRY(decodeUnsignedInteger(strm, &integr_part));

	TRY(decodeUnsignedInteger(strm, &fract_part));

	fract_part_rev = 0;
	while(fract_part > 0)
	{
		fract_part_rev = fract_part_rev*10 + fract_part%10;
		fract_part = fract_part/10;
		fraction_digits = fraction_digits*10;
	}
	*dec_val = (Decimal)fract_part_rev;

	if(sign == TRUE) // negative number
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

	TRY(decodeIntegerValue(strm, &mantissa));	//decode mantissa
	TRY(decodeIntegerValue(strm, &exponent));	//decode exponent

	DEBUG_MSG(ERROR, DEBUG_STREAM_IO, (">Float value: %ldE%ld\n", (long int)mantissa, (long int)exponent));

//  TODO: Improve the below validation: it should be independent of how the Float is defined
//
//	if(exponent >= (1 << 14) || exponent < -(1 << 14)
//			|| mantissa >= ((uint64_t) 1 << 63) ||
//			(mantissa < 0 && -mantissa > ((uint64_t) 1 << 63))
//			)
//	{
//		DEBUG_MSG(ERROR, DEBUG_STREAM_IO, (">Invalid float value: %lldE%lld\n", mantissa, exponent));
//		return INVALID_EXI_INPUT;
//	}

	fl_val->mantissa = mantissa;
	fl_val->exponent = (int16_t)exponent; /* TODO not using exip_config.h */

	return ERR_OK;
}

errorCode decodeDateTimeValue(EXIStream* strm, EXIPDateTime* dt_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Integer year;
	unsigned int monDay = 0;
	unsigned int timeVal = 0;
	boolean presence = FALSE;

	dt_val->presenceMask = 0;


	// TODO: currently only the xs:dateTime is implemented.
	//       The other types (gYear, gYearMonth, date, dateTime etc.)
	//       must be known here for correct encoding.

	/* Year component */
	TRY(decodeIntegerValue(strm, &year));

	dt_val->presenceMask = dt_val->presenceMask | YEAR_PRESENCE;
	dt_val->dateTime.tm_year = (int)year - 100;

	/* MonthDay component */
	TRY(decodeNBitUnsignedInteger(strm, 9, &monDay));

	dt_val->presenceMask = dt_val->presenceMask | MON_PRESENCE;
	dt_val->dateTime.tm_mon = monDay / 32 - 1;

	dt_val->presenceMask = dt_val->presenceMask | MDAY_PRESENCE;
	dt_val->dateTime.tm_mday = monDay % 32;

	/* Time component */
	TRY(decodeNBitUnsignedInteger(strm, 17, &timeVal));

	dt_val->presenceMask = dt_val->presenceMask | HOUR_PRESENCE;
	dt_val->dateTime.tm_hour = (timeVal / 64) / 64;

	dt_val->presenceMask = dt_val->presenceMask | MIN_PRESENCE;
	dt_val->dateTime.tm_min = (timeVal / 64) % 64;

	dt_val->presenceMask = dt_val->presenceMask | SEC_PRESENCE;
	dt_val->dateTime.tm_sec = timeVal % 64;

	/* FractionalSecs presence component */
	TRY(decodeBoolean(strm, &presence));

	if(presence)
	{
		UnsignedInteger fSecs = 0;
		unsigned int tmp = 0;

		dt_val->presenceMask = dt_val->presenceMask | FRACT_PRESENCE;
		dt_val->fSecs.offset = 0;
		dt_val->fSecs.value = 0;

		/* FractionalSecs component */
		TRY(decodeUnsignedInteger(strm, &fSecs));

		while(fSecs != 0)
		{
			tmp = fSecs % 10;
			dt_val->fSecs.offset++;

			if(tmp != 0)
			{
				dt_val->fSecs.value = dt_val->fSecs.value*10 + tmp;
			}

			fSecs = fSecs / 10;
		}
		dt_val->fSecs.offset -= 1;
	}

	/* TimeZone presence component */
	TRY(decodeBoolean(strm, &presence));

	if(presence)
	{
		return NOT_IMPLEMENTED_YET;
	}

	return ERR_OK;
}
