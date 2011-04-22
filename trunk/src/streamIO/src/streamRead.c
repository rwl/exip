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
 * @file streamRead.c
 * @brief Implementing the interface to a low-level EXI stream reader
 *
 * @date Aug 18, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "streamRead.h"
#include "ioUtil.h"

const unsigned char BIT_MASK[] = {	(char) 0x00,	// 0b00000000
									(char) 0x01,	// 0b00000001
									(char) 0x03,	// 0b00000011
									(char) 0x07,	// 0b00000111
									(char) 0x0F,	// 0b00001111
									(char) 0x1F,	// 0b00011111
									(char) 0x3F,	// 0b00111111
									(char) 0x7F,	// 0b01111111
									(char) 0xFF	};	// 0b11111111


errorCode readNextBit(EXIStream* strm, unsigned char* bit_val)
{
	if(strm->bufContent <= strm->bufferIndx) // the whole buffer is parsed! read another portion
	{
		if(strm->ioStrm == NULL || strm->ioStrm->readWriteToStream == NULL)
			return BUFFER_END_REACHED;
		strm->bufContent = strm->ioStrm->readWriteToStream(strm->buffer, strm->bufLen, strm->ioStrm->stream);
		if(strm->bufContent == 0)
			return BUFFER_END_REACHED;
		strm->bitPointer = 0;
		strm->bufferIndx = 0;
	}
	*bit_val = 0;
	*bit_val = (strm->buffer[strm->bufferIndx] & (1<<REVERSE_BIT_POSITION(strm->bitPointer))) != 0;

	return moveBitPointer(strm, 1);
}

errorCode readBits(EXIStream* strm, unsigned char n, uint32_t* bits_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int numBitsRead = 0; // Number of the bits read so far
	int tmp = 0;
	int shift = 0;
	int bits_in_byte = 0; // Number of bits read in one iteration
	unsigned int numBytesToBeRead = ((unsigned int) n) / 8 + (8 - strm->bitPointer < n % 8 );
	*bits_val = 0;

	if(strm->bufContent <= strm->bufferIndx + numBytesToBeRead)
	{
		// The buffer end is reached: there are fewer than n bits left unparsed
		char leftOverBits[8];
		size_t bytesCopied = strm->bufContent - strm->bufferIndx;
		size_t bytesRead = 0;
		if(strm->ioStrm == NULL || strm->ioStrm->readWriteToStream == NULL)
			return BUFFER_END_REACHED;

		memcpy(leftOverBits, strm->buffer + strm->bufferIndx, bytesCopied);

		bytesRead = strm->ioStrm->readWriteToStream(strm->buffer + bytesCopied, strm->bufLen - bytesCopied, strm->ioStrm->stream);
		strm->bufContent = bytesRead + bytesCopied;
		if(strm->bufContent < numBytesToBeRead)
			return BUFFER_END_REACHED;

		memcpy(strm->buffer, leftOverBits, bytesCopied);
		strm->bufferIndx = 0;
	}

	while(numBitsRead < n)
	{
		tmp = 0;
		if((unsigned int)(n - numBitsRead) <= (unsigned int)(8 - strm->bitPointer)) // The rest of the unread bits are located in the current byte from the stream
		{
			int tmp_shift;
			bits_in_byte = n - numBitsRead;
			tmp_shift = 8 - strm->bitPointer - bits_in_byte;
			tmp = (strm->buffer[strm->bufferIndx] & BIT_MASK[bits_in_byte]<<tmp_shift) >> tmp_shift;
		}
		else // The rest of the unread bits are located in multiple bytes from the stream
		{
			bits_in_byte = 8 - strm->bitPointer;
			tmp = strm->buffer[strm->bufferIndx] & BIT_MASK[bits_in_byte];
		}
		numBitsRead += bits_in_byte;
		shift = n - (numBitsRead);
		tmp = tmp << shift;
		*bits_val = *bits_val | tmp;

		tmp_err_code = moveBitPointer(strm, bits_in_byte);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

