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
 * @file EXIParser.c
 * @brief Implementation of a parser of EXI streams
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "EXIParser.h"
#include "procTypes.h"
#include "errorHandle.h"
#include "bodyDecode.h"
#include "headerDecode.h"
#include "memManagement.h"

void parseEXI(char* binaryBuf, size_t bufLen, size_t bufContent, IOStream* ioStrm, ContentHandler* handler, ExipSchema* schema, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIStream strm;
	EXIOptions options;

	initAllocList(&strm.memList);
	strm.buffer = binaryBuf;
	strm.context.bitPointer = 0;
	strm.context.bufferIndx = 0;
	strm.bufLen = bufLen;
	strm.header.opts = &options;
	strm.context.nonTermID = GR_DOCUMENT;
	strm.context.curr_lnID = 0;
	strm.context.curr_uriID = 0;
	strm.context.expectATData = 0;
	strm.bufContent = bufContent;
	strm.ioStrm = ioStrm;

	tmp_err_code = decodeHeader(&strm);
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
			handler->fatalError(tmp_err_code, "Error parsing EXI header", app_data);
		freeAllMem(&strm);
		return;
	}
	if(handler->exiHeader != NULL)
	{
		if(handler->exiHeader(&(strm.header), app_data) == EXIP_HANDLER_STOP)
			return;
	}

	decodeBody(&strm, handler, schema, app_data);
}
