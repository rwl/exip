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
 * @file eventsEXI.c
 * @brief Defines events related functions
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "eventsEXI.h"
#include "streamEncode.h"

#define UNUSED_CODE_PART_VALUE UINT_MAX

errorCode writeEventCode(EXIStream* strm, GrammarRule* currentRule, unsigned char codeLength, size_t codeLastPart)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char i = 0;
	for(i = 0; i < codeLength - 1; i++)
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, currentRule->bits[i], (unsigned int) currentRule->prodCounts[i]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = encodeNBitUnsignedInteger(strm, currentRule->bits[codeLength - 1], (unsigned int) codeLastPart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

EXIEvent getEventDefType(EventType eType)
{
	EXIEvent event;
	event.eventType = eType;
	event.valueType = VALUE_TYPE_NONE;
	return event;
}

unsigned char eventsIdentical(EXIEvent e1, EXIEvent e2)
{
	return e1.eventType == e2.eventType && e1.valueType == e2.valueType;
}

unsigned char valueTypeClassesEqual(ValueType t1, ValueType t2)
{
	return t1 == t2 || (t1 >= VALUE_TYPE_INTEGER && t1 >= VALUE_TYPE_INTEGER);
}
