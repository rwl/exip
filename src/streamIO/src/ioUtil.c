/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file ioUtil.c
 * @brief Implements common utilities for StreamIO module
 *
 * @date Oct 26, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "ioUtil.h"

void moveBitPointer(EXIStream* strm, unsigned int bitPositions)
{
	int nbits;

	strm->context.bufferIndx += bitPositions/8;
	nbits = bitPositions % 8;
	if(nbits < 8 - strm->context.bitPointer) // The remaining (0-7) bit positions can be moved within the current byte
	{
		strm->context.bitPointer += nbits;
	}
	else
	{
		strm->context.bufferIndx += 1;
		strm->context.bitPointer = nbits - (8 - strm->context.bitPointer);
	}
}

unsigned char getBitsNumber(uint64_t val)
{
	if(val == 0)
		return 0;
	return log2INT(val) + 1;
}

unsigned int log2INT(uint64_t val)
{
	const uint64_t b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000, 0xFFFFFFFF00000000};
	const unsigned int S[] = {1, 2, 4, 8, 16, 32};
	int i;

	unsigned int r = 0; // result of log2(v) will go here
	for (i = 5; i >= 0; i--) // unroll for speed...
	{
	  if (val & b[i])
	  {
		val >>= S[i];
	    r |= S[i];
	  }
	}
	return r;
}
