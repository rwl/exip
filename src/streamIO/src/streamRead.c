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
	unsigned int numBytesToBeRead = 1 + ((n + strm->context.bitPointer - 1) / 8);
	unsigned int byteIndx = 1;
	unsigned char *buf;

	if(strm->buffer.bufContent < strm->context.bufferIndx + numBytesToBeRead)
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

	buf = (unsigned char *) strm->buffer.buf + strm->context.bufferIndx;

	*bits_val = (buf[0] & BIT_MASK[8 - strm->context.bitPointer])<<((numBytesToBeRead-1)*8);

	while(byteIndx < numBytesToBeRead)
	{
		*bits_val += (unsigned int) (buf[byteIndx])<<((numBytesToBeRead-byteIndx-1)*8);
		byteIndx++;
	}
	*bits_val = *bits_val << strm->context.bitPointer;
	*bits_val = *bits_val >> (numBytesToBeRead*8-n);

	n += strm->context.bitPointer;
	strm->context.bufferIndx += n / 8;
	strm->context.bitPointer = n % 8;
	return ERR_OK;
}

