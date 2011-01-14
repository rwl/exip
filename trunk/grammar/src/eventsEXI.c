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

#define UNUSED_CODE_PART_VALUE 9999

EventCode getEventCode3(unsigned int first, unsigned int second, unsigned int third)
{
	EventCode res;
	res.size = 3;
	res.code[0] = first;
	res.code[1] = second;
	res.code[2] = third;
	return res;
}

EventCode getEventCode2(unsigned int first, unsigned int second)
{
	EventCode res;
	res.size = 2;
	res.code[0] = first;
	res.code[1] = second;
	res.code[2] = UNUSED_CODE_PART_VALUE;
	return res;
}

EventCode getEventCode1(unsigned int first)
{
	EventCode res;
	res.size = 1;
	res.code[0] = first;
	res.code[1] = UNUSED_CODE_PART_VALUE;
	res.code[2] = UNUSED_CODE_PART_VALUE;
	return res;
}

errorCode writeEventCode(EXIStream* strm, EventCode code, unsigned char* bits)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int i = 0;
	for(; i < code.size; i++)
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, bits[i], code.code[i]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

EXIEvent getEventDefType(EventType eType)
{
	EXIEvent event;
	event.eventType = eType;
	event.valueType = VALUE_TYPE_NONE;
	return event;
}

unsigned char eventsEqual(EXIEvent e1, EXIEvent e2)
{
	return e1.eventType == e2.eventType && e1.valueType == e2.valueType;
}
