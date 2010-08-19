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

#include "../include/streamRead.h"

/**
 * @brief Moves the BitPointer with certain positions. Takes care of byteIndex increasing when
 *        the movement cross a byte boundary
 * @param[in] strm EXI stream of bits
 * @param[in] bitPositions the number of bit positions to move the pointer
 * @return Error handling code
 */
static errorCode moveBitPointer(EXIStream* strm, int bitPositions);

const unsigned char BIT_MASK[] = {(char) 0b00000000,
								  (char) 0b00000001,
								  (char) 0b00000011,
							  	  (char) 0b00000111,
								  (char) 0b00001111,
								  (char) 0b00011111,
								  (char) 0b00111111,
								  (char) 0b01111111,
								  (char) 0b11111111};

static errorCode moveBitPointer(EXIStream* strm, int bitPositions)
{
	//TODO: Handle error cases i.e. end of the stream and so on
	strm->bufferIndx = strm->bufferIndx + bitPositions/8;
	int nbits = 0;
	if(bitPositions < 8)
		nbits = bitPositions;
	else
		nbits = bitPositions % 8;
	if(nbits < 8 - strm->bitPointer) // The remaining (0-7) bit positions can be moved within the current byte
	{
		strm->bitPointer += nbits;
	}
	else
	{
		strm->bufferIndx += 1;
		strm->bitPointer = nbits - (8 - strm->bitPointer);
	}
	return ERR_OK;
}


errorCode readNextBit(EXIStream* strm, unsigned char* bit_val)
{
	//TODO: Handle error cases i.e. end of the stream and so on
	*bit_val = 0;
	*bit_val = (strm->buffer[strm->bufferIndx] & (1<<REVERSE_BIT_POSITION(strm->bitPointer))) != 0;

	moveBitPointer(strm, 1);

	return ERR_OK;
}

errorCode readBits(EXIStream* strm, unsigned char n, int* bits_val)
{
	//TODO: Handle error cases i.e. end of the stream and so on
	*bits_val = 0;
	int numBitsRead = 0; // Number of the bits read so far
	int tmp = 0;
	int shift = 0;
	int bits_in_byte = 0; // Number of bits read in one iteration
	while(numBitsRead < n)
	{
		tmp = 0;
		if(n - numBitsRead <= 8 - strm->bitPointer) // The rest of the unread bits are located in the current byte from the stream
		{
			bits_in_byte = n - numBitsRead;
			int tmp_shift = 8 - strm->bitPointer - (n - numBitsRead);
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

		moveBitPointer(strm, bits_in_byte);
	}
	return ERR_OK;
}

