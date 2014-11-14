/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file ioUtil.h
 * @brief Common utilities for StreamIO module
 *
 * @date Oct 26, 2010
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id$
 */

#ifndef IOUTIL_H_
#define IOUTIL_H_

#include "procTypes.h"
#include "errorHandle.h"

/**
 * @brief Moves the BitPointer with certain positions. Takes care of byteIndex increasing when
 *        the movement cross a byte boundary
 * @param[in] strm EXI stream of bits
 * @param[in] bitPositions the number of bit positions to move the pointer
 */
void moveBitPointer(EXIStream* strm, unsigned int bitPositions);

/**
 * @brief Determine the number of bits needed to encode an unsigned integer value
 * ⌈ log 2 m ⌉ from the spec is equal to getBitsNumber(m - 1)
 *
 * @param[in] val unsigned integer value
 *
 * @return The number of bits needed
 */
unsigned char getBitsNumber(uint64_t val);

/**
 * @brief Log2 function. Used to determine the number of bits needed to encode a unsigned integer value
 * The code taken from: http://www-graphics.stanford.edu/~seander/bithacks.html#IntegerLog
 * @param[in] val uint32_t value
 *
 * @return The number of bits needed
 */
unsigned int log2INT(uint64_t val);

/**
 * @brief Reads an EXI stream chunk using buffer.ioStrm.readWriteToStream if available
 * @param[in] strm EXI stream of bits
 * @param[in] numBytesToBeRead the number of bites that are requested for parsing
 *
 * @return The number of bits needed
 */
errorCode readEXIChunkForParsing(EXIStream* strm, unsigned int numBytesToBeRead);

#endif /* IOUTIL_H_ */
