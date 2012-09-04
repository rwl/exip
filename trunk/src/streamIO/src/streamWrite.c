/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file streamWrite.c
 * @brief Implementing the interface to a low-level EXI stream writer
 *
 * @date Aug 23, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "streamWrite.h"
#include "ioUtil.h"

extern const unsigned char BIT_MASK[];

errorCode writeNextBit(EXIStream* strm, unsigned char bit_val)
{
	if(strm->buffer.bufLen <= strm->context.bufferIndx) // the whole buffer is filled! flush it!
	{
		Index numBytesWritten = 0;
		if(strm->buffer.ioStrm.readWriteToStream == NULL)
			return BUFFER_END_REACHED;
		numBytesWritten = strm->buffer.ioStrm.readWriteToStream(strm->buffer.buf, strm->buffer.bufLen, strm->buffer.ioStrm.stream);
		if(numBytesWritten < strm->buffer.bufLen)
			return BUFFER_END_REACHED;
		strm->context.bitPointer = 0;
		strm->context.bufferIndx = 0;
	}

	if(bit_val == 0)
		strm->buffer.buf[strm->context.bufferIndx] = strm->buffer.buf[strm->context.bufferIndx] & (~(1<<REVERSE_BIT_POSITION(strm->context.bitPointer)));
	else
		strm->buffer.buf[strm->context.bufferIndx] = strm->buffer.buf[strm->context.bufferIndx] | (1<<REVERSE_BIT_POSITION(strm->context.bitPointer));

	moveBitPointer(strm, 1);
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, ("  @%u:%u", (unsigned int) strm->context.bufferIndx, strm->context.bitPointer));
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

	if(strm->buffer.bufLen <= strm->context.bufferIndx + numBytesToBeWritten)
	{
		// The buffer end is reached: there are fewer than nbits bits left in the buffer
		char leftOverBits;
		Index numBytesWritten = 0;
		if(strm->buffer.ioStrm.readWriteToStream == NULL)
			return BUFFER_END_REACHED;

		leftOverBits = strm->buffer.buf[strm->context.bufferIndx];

		numBytesWritten = strm->buffer.ioStrm.readWriteToStream(strm->buffer.buf, strm->context.bufferIndx, strm->buffer.ioStrm.stream);
		if(numBytesWritten < strm->context.bufferIndx)
			return BUFFER_END_REACHED;

		strm->buffer.buf[0] = leftOverBits;
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
		strm->buffer.buf[strm->context.bufferIndx] = strm->buffer.buf[strm->context.bufferIndx] & (~BIT_MASK[8 - strm->context.bitPointer]); // Initialize the unused bits with 0s
		strm->buffer.buf[strm->context.bufferIndx] = strm->buffer.buf[strm->context.bufferIndx] | tmp;

		numBitsWrite += bits_in_byte;
		moveBitPointer(strm, bits_in_byte);
	}
	DEBUG_MSG(INFO, DEBUG_STREAM_IO, ("  @%u:%u\n", (unsigned int) strm->context.bufferIndx, strm->context.bitPointer));

	return ERR_OK;
}
