/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file streamRead.h
 * @brief Interface to a low-level EXI stream reader
 *
 * @date Jul 7, 2010
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id$
 */

#ifndef STREAMREADER_H_
#define STREAMREADER_H_

#include "errorHandle.h"
#include "procTypes.h"

/**
 * @brief Reads the next single bit from a buffer and moves its current bit pointer
 * @param[in] strm EXI stream of bits
 * @param[out] bit_val the value of the next bit: 0 or 1
 * @return Error handling code
 */
errorCode readNextBit(EXIStream* strm, boolean* bit_val);

/**
 * @brief Read the next n bits and return the result as an integer. Moves the stream current bit pointer
 * with the number of bits read
 * @param[in] strm EXI stream of bits
 * @param[in] n The number of bits in the range
 * @param[out] bits_val resulting bits value
 * @return Error handling code
 */
errorCode readBits(EXIStream* strm, unsigned char n, unsigned int* bits_val);

#endif /* STREAMREADER_H_ */
