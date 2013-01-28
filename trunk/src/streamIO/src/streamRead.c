/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file streamRead.c
 * @brief Implementing the interface to a low-level EXI stream reader
 *
 * @date Aug 18, 2010
 * @author Rumen Kyusakov
 * @version 0.4
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


errorCode readNextBit(EXIStream* strm, boolean* bit_val)
{
	if(strm->buffer.bufContent <= strm->context.bufferIndx) // the whole buffer is parsed! read another portion
	{
		if(strm->buffer.ioStrm.readWriteToStream == NULL)
			return BUFFER_END_REACHED;
		strm->buffer.bufContent = strm->buffer.ioStrm.readWriteToStream(strm->buffer.buf, strm->buffer.bufLen, strm->buffer.ioStrm.stream);
		if(strm->buffer.bufContent == 0)
			return BUFFER_END_REACHED;
		strm->context.bitPointer = 0;
		strm->context.bufferIndx = 0;
	}
	*bit_val = 0;
	*bit_val = (strm->buffer.buf[strm->context.bufferIndx] & (1<<REVERSE_BIT_POSITION(strm->context.bitPointer))) != 0;

	moveBitPointer(strm, 1);
	return ERR_OK;
}

errorCode readBits(EXIStream* strm, unsigned char n, unsigned int* bits_val)
{
	unsigned int numBitsRead = 0; // Number of the bits read so far
	unsigned int tmp = 0;
	unsigned int bits_in_byte = 0; // Number of bits read in one iteration
	unsigned int numBytesToBeRead = ((unsigned int) n) / 8 + (8 - strm->context.bitPointer < n % 8 );
	*bits_val = 0;

	if(strm->buffer.bufContent <= strm->context.bufferIndx + numBytesToBeRead)
	{
		// The buffer end is reached: there are fewer than n bits left unparsed
		char leftOverBits[16];
		Index bytesCopied = strm->buffer.bufContent - strm->context.bufferIndx;
		Index bytesRead = 0;
		if(strm->buffer.ioStrm.readWriteToStream == NULL)
			return BUFFER_END_REACHED;

		memcpy(leftOverBits, strm->buffer.buf + strm->context.bufferIndx, bytesCopied);

		bytesRead = strm->buffer.ioStrm.readWriteToStream(strm->buffer.buf + bytesCopied, strm->buffer.bufLen - bytesCopied, strm->buffer.ioStrm.stream);
		strm->buffer.bufContent = bytesRead + bytesCopied;
		if(strm->buffer.bufContent < numBytesToBeRead)
			return BUFFER_END_REACHED;

		memcpy(strm->buffer.buf, leftOverBits, bytesCopied);
		strm->context.bufferIndx = 0;
	}

	while(numBitsRead < n)
	{
		tmp = 0;
		if((unsigned int)(n - numBitsRead) <= (unsigned int)(8 - strm->context.bitPointer)) // The rest of the unread bits are located in the current byte from the stream
		{
			int tmp_shift;
			bits_in_byte = n - numBitsRead;
			tmp_shift = 8 - strm->context.bitPointer - bits_in_byte;
			tmp = (strm->buffer.buf[strm->context.bufferIndx] >> tmp_shift) & BIT_MASK[bits_in_byte];
		}
		else // The rest of the unread bits are located in multiple bytes from the stream
		{
			bits_in_byte = 8 - strm->context.bitPointer;
			tmp = strm->buffer.buf[strm->context.bufferIndx] & BIT_MASK[bits_in_byte];
		}
		numBitsRead += bits_in_byte;
		tmp = tmp << (n - numBitsRead);
		*bits_val = *bits_val | tmp;

		moveBitPointer(strm, bits_in_byte);
	}
	return ERR_OK;
}

