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

extern const unsigned char BIT_MASK[];

errorCode writeNextBit(EXIStream* strm, unsigned char bit_val)
{
	if(strm->bufLen <= strm->context.bufferIndx) // the whole buffer is filled! flush it!
	{
		size_t numBytesWritten = 0;
		if(strm->ioStrm.readWriteToStream == NULL)
			return BUFFER_END_REACHED;
		numBytesWritten = strm->ioStrm.readWriteToStream(strm->buffer, strm->bufLen, strm->ioStrm.stream);
		if(numBytesWritten < strm->bufLen)
			return BUFFER_END_REACHED;
		strm->context.bitPointer = 0;
		strm->context.bufferIndx = 0;
	}

	if(bit_val == 0)
		strm->buffer[strm->context.bufferIndx] = strm->buffer[strm->context.bufferIndx] & (~(1<<REVERSE_BIT_POSITION(strm->context.bitPointer)));
	else
		strm->buffer[strm->context.bufferIndx] = strm->buffer[strm->context.bufferIndx] | (1<<REVERSE_BIT_POSITION(strm->context.bitPointer));

	moveBitPointer(strm, 1);
	return ERR_OK;
}

errorCode writeBits(EXIStream* strm, unsigned int bits_val)
{
	unsigned char nbits = getBitsNumber(bits_val);
	return writeNBits(strm, nbits, bits_val);
}

errorCode writeNBits(EXIStream* strm, unsigned char nbits, unsigned int bits_val)
{
	unsigned int numBitsWrite = 0; // Number of the bits written so far
	unsigned char tmp = 0;
	int bits_in_byte = 0; // Number of bits written in one iteration
	unsigned int numBytesToBeWritten = ((unsigned int) nbits) / 8 + (8 - strm->context.bitPointer < nbits % 8 );

	if(strm->bufLen <= strm->context.bufferIndx + numBytesToBeWritten)
	{
		// The buffer end is reached: there are fewer than nbits bits left in the buffer
		char leftOverBits;
		size_t numBytesWritten = 0;
		if(strm->ioStrm.readWriteToStream == NULL)
			return BUFFER_END_REACHED;

		leftOverBits = strm->buffer[strm->context.bufferIndx];

		numBytesWritten = strm->ioStrm.readWriteToStream(strm->buffer, strm->context.bufferIndx, strm->ioStrm.stream);
		if(numBytesWritten < strm->context.bufferIndx)
			return BUFFER_END_REACHED;

		strm->buffer[0] = leftOverBits;
		strm->context.bufferIndx = 0;
	}

	while(numBitsWrite < nbits)
	{
		if((unsigned int)(nbits - numBitsWrite) <= (unsigned int)(8 - strm->context.bitPointer)) // The rest of the unwritten bits can be put in the current byte from the stream
			bits_in_byte = nbits - numBitsWrite;
		else // The rest of the unwritten bits are more than the bits in the current byte from the stream
			bits_in_byte = 8 - strm->context.bitPointer;

		tmp = (bits_val >> (nbits - numBitsWrite - bits_in_byte)) & BIT_MASK[bits_in_byte];
		tmp = tmp << (8 - strm->context.bitPointer - bits_in_byte);
		strm->buffer[strm->context.bufferIndx] = strm->buffer[strm->context.bufferIndx] & (~BIT_MASK[8 - strm->context.bitPointer]); // Initialize the unused bits with 0s
		strm->buffer[strm->context.bufferIndx] = strm->buffer[strm->context.bufferIndx] | tmp;

		numBitsWrite += bits_in_byte;
		moveBitPointer(strm, bits_in_byte);
	}

	return ERR_OK;
}
