/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file eventsEXI.c
 * @brief Defines events related functions
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "eventsEXI.h"
#include "streamEncode.h"
#include "stringManipulate.h"
#include "ioUtil.h"

errorCode writeEventCode(EXIStream* strm, GrammarRule* currentRule, unsigned char codeLength, Index codeLastPart)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char lastPartBits = currentRule->bits1;

	if(codeLength > 1)
	{
		// Encode first part here ...
		tmp_err_code = encodeNBitUnsignedInteger(strm, currentRule->bits1, (unsigned int) currentRule->p1Count);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(codeLength == 2)
			lastPartBits = getBitsNumber(currentRule->p2Count - 1);
	}

	if(codeLength > 2)
	{
		// Encode second part here ...
		tmp_err_code = encodeNBitUnsignedInteger(strm, getBitsNumber(currentRule->p2Count), (unsigned int) currentRule->p2Count);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		lastPartBits = getBitsNumber(currentRule->p3Count - 1);
	}

	tmp_err_code = encodeNBitUnsignedInteger(strm, lastPartBits, (unsigned int) codeLastPart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

unsigned char valueTypeClassesEqual(EXIType t1, EXIType t2)
{
	return t1 == t2 || (t1 >= VALUE_TYPE_INTEGER && t1 >= VALUE_TYPE_INTEGER);
}
