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
 * @file headerEncode.c
 * @brief Implementing the interface of EXI header encoder
 *
 * @date Aug 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "headerEncode.h"
#include "streamWrite.h"

errorCode encodeHeader(EXIStream* strm, EXIheader* header)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start EXI header encoding\n"));
	if(header->has_cookie)
	{
		tmp_err_code = writeNBits(strm, 8, 36); // ASCII code for $ = 00100100  (36)
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = writeNBits(strm, 8, 69); // ASCII code for E = 01000101  (69)
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = writeNBits(strm, 8, 88); // ASCII code for X = 01011000  (88)
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = writeNBits(strm, 8, 73); // ASCII code for I = 01001001  (73)
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encoding the header Distinguishing Bits\n"));
	tmp_err_code = writeNBits(strm, 2, 2);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Write the Presence Bit for EXI Options\n"));
	tmp_err_code = writeNextBit(strm, header->has_options);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encode EXI version\n"));
	tmp_err_code = writeNextBit(strm, header->is_preview_version);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(header->version_number > 15)
	{
		tmp_err_code = writeNBits(strm, 4, 15);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = writeNBits(strm, 4, header->version_number - 15 - 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		tmp_err_code = writeNBits(strm, 4, header->version_number - 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encode EXI options\n"));
	if(header->has_options)
	{
		return NOT_IMPLEMENTED_YET; // TODO: Handle EXI streams with options. This includes Padding Bits in some cases
	}

	return ERR_OK;
}
