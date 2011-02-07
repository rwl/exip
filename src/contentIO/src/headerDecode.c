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
 * @file headerDecode.c
 * @brief Implementing the interface of EXI header decoder
 *
 * @date Aug 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "headerDecode.h"
#include "streamDecode.h"
#include "streamRead.h"

errorCode decodeHeader(EXIStream* strm, EXIheader* header)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t bits_val = 0;
	unsigned char smallVal = 0;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start EXI header decoding\n"));
	tmp_err_code = readBits(strm, 2, &bits_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(bits_val == 2)  // The header Distinguishing Bits i.e. no EXI Cookie
	{
		header->has_cookie = 0;
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">No EXI cookie detected\n"));
	}
	else if(bits_val == 0)// ASCII code for $ = 00100100  (36)
	{
		tmp_err_code = readBits(strm, 6, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 36)
			return INVALID_EXI_HEADER;
		tmp_err_code = readBits(strm, 8, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 69)   // ASCII code for E = 01000101  (69)
			return INVALID_EXI_HEADER;
		tmp_err_code = readBits(strm, 8, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 88)   // ASCII code for X = 01011000  (88)
			return INVALID_EXI_HEADER;
		tmp_err_code = readBits(strm, 8, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 73)   // ASCII code for I = 01001001  (73)
			return INVALID_EXI_HEADER;

		header->has_cookie = 1;
		tmp_err_code = readBits(strm, 2, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 2)  // The header Distinguishing Bits are required
			return INVALID_EXI_HEADER;
	}
	else
	{
		return INVALID_EXI_HEADER;
	}

	// Read the Presence Bit for EXI Options
	tmp_err_code = readNextBit(strm, &smallVal);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;


	if(strm->opts == NULL)
		return NULL_POINTER_REF;

	if(smallVal == 1) // There are EXI options
	{
		header->has_options = 1;
		return NOT_IMPLEMENTED_YET; // TODO: Handle EXI streams with options. This includes Padding Bits in some cases
	}
	else // The default values for EXI options
	{
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">No EXI options field in the header\n"));
		header->has_options = 0;
	    makeDefaultOpts(strm->opts);
	}
	header->opts = strm->opts;

	// Read the Version type
	tmp_err_code = readNextBit(strm, &smallVal);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	header->is_preview_version = smallVal;
	header->version_number = 1;

	do
	{
		tmp_err_code = readBits(strm, 4, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		header->version_number += bits_val;
		if(bits_val < 15)
			break;
	} while(1);

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">EXI version: %d\n", header->version_number));
	return ERR_OK;
}
