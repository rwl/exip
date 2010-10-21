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
 * @file streamDecode.h
 * @brief Interface to a higher-level EXI stream decoder - decode basic EXI types
 *
 * @date Jul 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef STREAMDECODE_H_
#define STREAMDECODE_H_

#include "errorHandle.h"
#include "procTypes.h"

/**
 * @brief Decode EXI n-bit Unsigned Integer
 * Decodes and returns an n-bit unsigned integer.
 * @param[in] strm EXI stream of bits
 * @param[in] n The number of bits in the range [1,32].
 * @param[out] int_val resulting n-bit unsigned integer value
 * @return Error handling code
 */
errorCode decodeNBitUnsignedInteger(EXIStream* strm, unsigned char n, uint32_t* int_val);

/**
 * @brief Decode EXI Boolean
 * Decode a single boolean value. The value false is represented by the bit
 * (byte) 0, and the value true is represented by the bit (byte) 1.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] bool_val 0-false, 1-true
 * @return Error handling code
 */
errorCode decodeBoolean(EXIStream* strm, unsigned char* bool_val);

/**
 * @brief Decode EXI Unsigned Integer type
 * Decode an arbitrary precision non negative integer using a sequence of
 * octets. The most significant bit of the last octet is set to zero to
 * indicate sequence termination. Only seven bits per octet are used to
 * store the integer's value.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] int_val resulting unsigned integer value
 * @return Error handling code. It returns BIGGER_TYPE_REQUIRED indicating that
 * the integer is bigger than 32 bits. The processor MUST invoke the function
 * that handles larger unsigned integers
 */
errorCode decodeUnsignedInteger(EXIStream* strm, uint32_t* int_val);

/**
 * @brief Decode EXI Unsigned Integer type when the value is bigger than 32 bits
 *
 * @param[in] strm EXI stream of bits
 * @param[out] int_val resulting big unsigned integer value
 * @return Error handling code
 */
errorCode decodeBigUnsignedInteger(EXIStream* strm, BigUnsignedInt* int_val);

/**
 * @brief Decode EXI String type
 * Decode a string as a length-prefixed sequence of UCS codepoints, each of
 * which is encoded as an integer.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] string_val null-terminated decoded string
 * @return Error handling code
 */
errorCode decodeString(EXIStream* strm, StringType* string_val);

/**
 * @brief Decode String with the length of the String specified
 * This function is used for Partitions Optimized for Frequent use of String Literals
 * when there is a local name miss => the Length part is read first.
 * The memory to hold the string data is allocated dynamically
 *
 * @param[in] strm EXI stream of bits
 * @param[in] str_length the length of the string
 * @param[out] string_val null-terminated decoded string
 * @return Error handling code
 */
errorCode decodeStringOnly(EXIStream* strm, unsigned int str_length, StringType* string_val);

/**
 * @brief Decode EXI Binary type
 * Decode a binary value as a length-prefixed sequence of octets.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] binary_val decoded binary value
 * @param[out] bytes length of decoded binary content
 * @return Error handling code
 */
errorCode decodeBinary(EXIStream* strm, char* binary_val, unsigned int* bytes);

/**
 * @brief Decode EXI (signed) Integer type
 * Decode an arbitrary precision integer using a sign bit followed by a
 * sequence of octets. The most significant bit of the last octet is set to
 * zero to indicate sequence termination. Only seven bits per octet are used
 * to store the integer's value.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] sint_val decoded signed integer value
 * @return Error handling code
 */
errorCode decodeIntegerValue(EXIStream* strm, int* sint_val);


/**
 * @brief Decode EXI Decimal type
 * Decode a decimal represented as a Boolean sign followed by two Unsigned
 * Integers. A sign value of zero (0) is used to represent positive Decimal
 * values and a sign value of one (1) is used to represent negative Decimal
 * values The first Integer represents the integral portion of the Decimal
 * value. The second positive integer represents the fractional portion of
 * the decimal with the digits in reverse order to preserve leading zeros.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] dec_val decoded decimal value as float
 * @return Error handling code
 */
errorCode decodeDecimalValue(EXIStream* strm, float* dec_val);


/**
 * @brief Decode EXI Float type
 * Decode a Float represented as two consecutive Integers. The first Integer
 * represents the mantissa of the floating point number and the second
 * Integer represents the 10-based exponent of the floating point number
 *
 * @param[in] strm EXI stream of bits
 * @param[out] dec_val decoded decimal value as float
 * @return Error handling code
 */
errorCode decodeFloatValue(EXIStream* strm, long double* double_val);
#endif /* STREAMDECODE_H_ */
