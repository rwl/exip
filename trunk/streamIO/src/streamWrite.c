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
 * @file streamWrite.c
 * @brief Implementing the interface to a low-level EXI stream writer
 *
 * @date Aug 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "streamWrite.h"
#include "ioUtil.h"

errorCode writeNextBit(EXIStream* strm, unsigned char bit_val)
{
	if(bit_val == 0)
		strm->buffer[strm->bufferIndx] = strm->buffer[strm->bufferIndx] & (~(1<<REVERSE_BIT_POSITION(strm->bitPointer)));
	else
		strm->buffer[strm->bufferIndx] = strm->buffer[strm->bufferIndx] | (1<<REVERSE_BIT_POSITION(strm->bitPointer));

	return moveBitPointer(strm, 1);
}

errorCode writeBits(EXIStream* strm, uint32_t bits_val)
{
	//TODO: Handle error cases i.e. end of the stream and so on
	//TODO: provide more efficient implementation!
	unsigned char nbits = getBitsNumber(bits_val);
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char bval = 0;
	for(; nbits > 0; nbits--)
	{
		bval = (bits_val & (1ul<<(nbits-1))) != 0;
		tmp_err_code = writeNextBit(strm, bval);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode writeNBits(EXIStream* strm, unsigned char nbits, uint32_t bits_val)
{
	//TODO: Handle error cases i.e. end of the stream and so on
	//TODO: provide more efficient implementation!

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char realNbits = getBitsNumber(bits_val);
	if(realNbits > nbits)
		return INCONSISTENT_PROC_STATE;

	int i = 0;
	for(; i < nbits - realNbits; i++)
	{
		tmp_err_code = writeNextBit(strm, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	unsigned char bval = 0;
	for(; realNbits > 0; realNbits--)
	{
		bval = (bits_val & (1ul<<(realNbits-1))) != 0;
		tmp_err_code = writeNextBit(strm, bval);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}
