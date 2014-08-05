/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file streamEncode.c
 * @brief Implements an interface to a higher-level EXI stream encoder - encode basic EXI types
 *
 * @date Oct 26, 2010
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id$
 */

#include "streamEncode.h"
#include "streamWrite.h"
#include "stringManipulate.h"
#include "ioUtil.h"
#include <math.h>


errorCode encodeNBitUnsignedInteger(EXIStream* strm, unsigned char n, unsigned int int_val)
{
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, (">> %d [0x%X] (%u bits)", int_val, int_val, n));
	if(WITH_COMPRESSION(strm->header.opts.enumOpt) == FALSE && GET_ALIGNMENT(strm->header.opts.enumOpt) == BIT_PACKED)
	{
		return writeNBits(strm, n, int_val);
	}
	else
	{
		unsigned int byte_number = n / 8 + (n % 8 != 0);
		int tmp_byte_buf = 0;
		errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
		unsigned int i = 0;
		for(i = 0; i < byte_number; i++)
		{
			tmp_byte_buf = (int_val >> (i * 8)) & 0xFF;
			TRY(writeNBits(strm, 8, tmp_byte_buf));
		}
	}
	return EXIP_OK;
}

errorCode encodeBoolean(EXIStream* strm, boolean bool_val)
{
	//TODO:  when pattern facets are available in the schema datatype - handle it differently
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, (">> 0x%X (bool)", bool_val));
	return encodeNBitUnsignedInteger(strm, 1, bool_val);
}

errorCode encodeUnsignedInteger(EXIStream* strm, UnsignedInteger int_val)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned int nbits = getBitsNumber(int_val);
	unsigned int nbyte7 = nbits / 7 + (nbits % 7 != 0);
	unsigned int tmp_byte_buf = 0;
	unsigned int i = 0;
	
#if DEBUG_STREAM_IO == ON
	if (nbyte7 > 1)
		DEBUG_MSG(INFO, DEBUG_STREAM_IO, (" Write %lu (unsigned)\n", (long unsigned int)int_val));
#endif
	if(nbyte7 == 0)
		nbyte7 = 1;  // the 0 Unsigned Integer is encoded with one 7bit byte
	for(i = 0; i < nbyte7; i++)
	{
		tmp_byte_buf = (unsigned int) ((int_val >> (i * 7)) & 0x7F);
		if(i == nbyte7 - 1)
		{
			DEBUG_MSG(INFO, DEBUG_STREAM_IO, (">> 0x%.2X", tmp_byte_buf & 0x7f));
			writeNextBit(strm, 0);
		}
		else
		{
			DEBUG_MSG(INFO, DEBUG_STREAM_IO, (">> 0x%.2X", tmp_byte_buf | 0x80));
			writeNextBit(strm, 1);
		}

		TRY(writeNBits(strm, 7, tmp_byte_buf));
	}
	return EXIP_OK;
}

errorCode encodeString(EXIStream* strm, const String* string_val)
{
	// Assume no Restricted Character Set is defined
	//TODO: Handle the case when Restricted Character Set is defined

	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_STREAM_IO, (" Prepare to write string"));
	TRY(encodeUnsignedInteger(strm, (UnsignedInteger)(string_val->length)));

	return encodeStringOnly(strm, string_val);
}

errorCode encodeStringOnly(EXIStream* strm, const String* string_val)
{
	// Assume no Restricted Character Set is defined
	//TODO: Handle the case when Restricted Character Set is defined

	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	uint32_t tmp_val = 0;
	Index i = 0;
	Index readerPosition = 0;
#if DEBUG_STREAM_IO == ON && EXIP_DEBUG_LEVEL == INFO
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, ("\n Write string, len %u: ", (unsigned int) string_val->length));
	printString(string_val);
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, ("\n"));
#endif
	
	for(i = 0; i < string_val->length; i++)
	{
		tmp_val = readCharFromString(string_val, &readerPosition);

		TRY(encodeUnsignedInteger(strm, (UnsignedInteger) tmp_val));
	}

	return EXIP_OK;
}

errorCode encodeBinary(EXIStream* strm, char* binary_val, Index nbytes)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	Index i = 0;

	TRY(encodeUnsignedInteger(strm, (UnsignedInteger) nbytes));

	DEBUG_MSG(INFO, DEBUG_STREAM_IO, (" Write %u (binary bytes)\n", (unsigned int) nbytes));
	for(i = 0; i < nbytes; i++)
	{
		TRY(writeNBits(strm, 8, (unsigned int) binary_val[i]));
	}
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, ("\n"));
	return EXIP_OK;
}

errorCode encodeIntegerValue(EXIStream* strm, Integer sint_val)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	UnsignedInteger uval;
	unsigned char sign;
	if(sint_val >= 0)
	{
		sign = 0;
		uval = (UnsignedInteger) sint_val;
	}
	else
	{
		sint_val += 1;
		uval = (UnsignedInteger) -sint_val;
		sign = 1;
	}
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, (" Write %ld (signed)", (long int)sint_val));
	TRY(writeNextBit(strm, sign));
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, ("\n"));
	return encodeUnsignedInteger(strm, uval);
}

errorCode encodeDecimalValue(EXIStream* strm, Decimal dec_val)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	boolean sign;
	UnsignedInteger integr_part = 0;
	UnsignedInteger fract_part_rev = 0;
	UnsignedInteger m;
	int e = dec_val.exponent;

	if(dec_val.mantissa >= 0)
	{
		sign = FALSE;
		integr_part = (UnsignedInteger) dec_val.mantissa;
	}
	else
	{
		sign = TRUE;
		integr_part = (UnsignedInteger) -dec_val.mantissa;
	}

	m = integr_part;

	TRY(encodeBoolean(strm, sign));

	if(dec_val.exponent > 0)
	{
		while(e)
		{
			integr_part = integr_part*10;
			e--;
		}
	}
	else if(dec_val.exponent < 0)
	{
		while(e)
		{
			integr_part = integr_part/10;
			e++;
		}
	}

	TRY(encodeUnsignedInteger(strm, integr_part));
	e = dec_val.exponent;
	while(e < 0)
	{
		fract_part_rev = fract_part_rev*10 + m%10;
		m = m/10;
		e++;
	}

	TRY(encodeUnsignedInteger(strm, fract_part_rev));

	return EXIP_OK;
}

errorCode encodeFloatValue(EXIStream* strm, Float fl_val)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	DEBUG_MSG(ERROR, DEBUG_STREAM_IO, (">Float value: %ldE%d\n", (long int)fl_val.mantissa, fl_val.exponent));

	TRY(encodeIntegerValue(strm, (Integer) fl_val.mantissa));	//encode mantissa
	TRY(encodeIntegerValue(strm, (Integer) fl_val.exponent));	//encode exponent

	return EXIP_OK;
}

errorCode encodeDateTimeValue(EXIStream* strm, EXIType dtType, EXIPDateTime dt_val)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	if(dtType == VALUE_TYPE_DATE_TIME || dtType == VALUE_TYPE_DATE || dtType == VALUE_TYPE_YEAR)
	{
		/* Year component */
		TRY(encodeIntegerValue(strm, (Integer) dt_val.dateTime.tm_year - 100));
	}

	if(dtType == VALUE_TYPE_DATE_TIME || dtType == VALUE_TYPE_DATE || dtType == VALUE_TYPE_MONTH)
	{
		/* MonthDay component */
		unsigned int monDay = 0;

		monDay = dt_val.dateTime.tm_mon + 1;
		monDay = monDay * 32;

		monDay += dt_val.dateTime.tm_mday;

		TRY(encodeNBitUnsignedInteger(strm, 9, monDay));
	}

	if(dtType == VALUE_TYPE_DATE_TIME || dtType == VALUE_TYPE_TIME)
	{
		/* Time component */
		unsigned int timeVal = 0;

		timeVal += dt_val.dateTime.tm_hour;
		timeVal = timeVal * 64;
		timeVal += dt_val.dateTime.tm_min;
		timeVal = timeVal * 64;
		timeVal += dt_val.dateTime.tm_sec;

		TRY(encodeNBitUnsignedInteger(strm, 17, timeVal));

		if(IS_PRESENT(dt_val.presenceMask, FRACT_PRESENCE))
		{
			/* FractionalSecs component */
			UnsignedInteger fSecs = 0;
			unsigned int tmp;
			unsigned int i = 1;
			unsigned int j = 0;

			tmp = dt_val.fSecs.value;

			while(tmp != 0)
			{
				fSecs = fSecs*i + (tmp % 10);
				tmp = tmp / 10;

				i = 10;
				j++;
			}

			for(i = 0; i < dt_val.fSecs.offset + 1 - j; j++)
			{
				fSecs = fSecs*10;
			}

			TRY(encodeBoolean(strm, TRUE));
			TRY(encodeUnsignedInteger(strm, fSecs));
		}
		else
		{
			TRY(encodeBoolean(strm, FALSE));
		}

	}

	if(IS_PRESENT(dt_val.presenceMask, TZONE_PRESENCE))
	{
		TRY(encodeBoolean(strm, TRUE));
		TRY(encodeNBitUnsignedInteger(strm, 11, dt_val.TimeZone));
	}
	else
	{
		TRY(encodeBoolean(strm, FALSE));
	}

	return EXIP_OK;
}

errorCode writeEventCode(EXIStream* strm, EventCode ec)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	int i;

	for(i = 0; i < ec.length; i++)
	{
		TRY(encodeNBitUnsignedInteger(strm, ec.bits[i], (unsigned int) ec.part[i]));
	}

	return EXIP_OK;
}
