/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file streamWrite.h
 * @brief Interface to a low-level EXI stream writer
 *
 * @date Aug 23, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef STREAMWRITE_H_
#define STREAMWRITE_H_

#include "errorHandle.h"
#include "procTypes.h"

/**
 * @brief Writes a single bit to an EXI stream and moves its current bit pointer
 * @param[out] strm EXI stream of bits
 * @param[in] bit_val the value of the next bit: 0 or 1
 * @return Error handling code
 */
errorCode writeNextBit(EXIStream* strm, unsigned char bit_val);

/**
 * @brief Writes an unsigned integer value to an EXI stream with the least possible bits
 * and moves the stream current bit pointer to the last bit written.
 * @param[out] strm EXI stream of bits
 * @param[in] bits_val resulting bits value
 */
errorCode writeBits(EXIStream* strm, unsigned int bits_val);

/**
 * @brief Writes an unsigned integer value to an EXI stream with nbits (possible 0 paddings)
 * and moves the stream current bit pointer to the last bit written.
 * @param[out] strm EXI stream of bits
 * @param[in] nbits number of bits
 * @param[in] bits_val resulting bits value
 */
errorCode writeNBits(EXIStream* strm, unsigned char nbits, unsigned int bits_val);

#endif /* STREAMWRITE_H_ */
