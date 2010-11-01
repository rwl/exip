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
 * @file streamEncode.h
 * @brief Interface to a higher-level EXI stream encoder - encode basic EXI types
 *
 * @date Oct 26, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef STREAMENCODE_H_
#define STREAMENCODE_H_

#include "errorHandle.h"
#include "procTypes.h"

/**
 * @brief Encode EXI n-bit Unsigned Integer
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] int_val n-bit unsigned integer value
 * @return Error handling code
 */
errorCode encodeNBitUnsignedInteger(EXIStream* strm, uint32_t int_val);

/**
 * @brief Encode EXI Boolean
 * Encode a single boolean value. The value false is represented by the bit
 * (byte) 0, and the value true is represented by the bit (byte) 1.
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] bool_val 0-false, 1-true
 * @return Error handling code
 */
errorCode encodeBoolean(EXIStream* strm, unsigned char bool_val);

/**
 * @brief Encode EXI Unsigned Integer type
 * Encode an arbitrary precision non negative integer using a sequence of
 * octets. The most significant bit of the last octet is set to zero to
 * indicate sequence termination. Only seven bits per octet are used to
 * store the integer's value.
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] int_val unsigned integer value
 * @return Error handling code.
 */
errorCode encodeUnsignedInteger(EXIStream* strm, uint32_t int_val);

/**
 * @brief Encode EXI Unsigned Integer type when the value is bigger than 32 bits
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] int_val big unsigned integer value
 * @return Error handling code
 */
errorCode encodeBigUnsignedInteger(EXIStream* strm, BigUnsignedInt int_val);

/**
 * @brief Encode EXI String type
 * Encode a string as a length-prefixed sequence of UCS codepoints, each of
 * which is encoded as an integer.
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] string_val string to be encoded
 * @return Error handling code
 */
errorCode encodeString(EXIStream* strm, const StringType* string_val);

/**
 * @brief Encode EXI Binary type
 * Encode a binary value as a length-prefixed sequence of octets.
 * Dynamically allocate a memory for the buffer
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] binary_val binary value to be encoded
 * @param[in] nbytes length of the binary content
 * @return Error handling code
 */
errorCode encodeBinary(EXIStream* strm, char* binary_val, uint32_t nbytes);

/**
 * @brief Encode EXI (signed) Integer type
 * Encode an arbitrary precision integer using a sign bit followed by a
 * sequence of octets. The most significant bit of the last octet is set to
 * zero to indicate sequence termination. Only seven bits per octet are used
 * to store the integer's value.
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] sint_val signed integer value to be encoded
 * @return Error handling code.
 */
errorCode encodeIntegerValue(EXIStream* strm, int32_t sint_val);

/**
 * @brief Encode EXI (signed) Integer type bigger than 32 bits
 * Encode an arbitrary precision integer using a sign bit followed by a
 * sequence of octets. The most significant bit of the last octet is set to
 * zero to indicate sequence termination. Only seven bits per octet are used
 * to store the integer's value.
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] sint_val signed integer value to be encoded
 * @return Error handling code.
 */
errorCode encodeBigIntegerValue(EXIStream* strm, BigSignedInt sint_val);

/**
 * @brief Encode EXI Decimal type
 * Decode a decimal represented as a Boolean sign followed by two Unsigned
 * Integers. A sign value of zero (0) is used to represent positive Decimal
 * values and a sign value of one (1) is used to represent negative Decimal
 * values The first Integer represents the integral portion of the Decimal
 * value. The second positive integer represents the fractional portion of
 * the decimal with the digits in reverse order to preserve leading zeros.
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] dec_val decimal value to be encoded
 * @return Error handling code. It returns BIGGER_TYPE_REQUIRED indicating that
 * the decimal is bigger than the parameter type provided. The processor MUST
 * invoke the function that handles larger decimals
 */
errorCode encodeDecimalValue(EXIStream* strm, decimal dec_val);

/**
 * @brief Encode big Decimal value
 * Encode a decimal represented as a Boolean sign followed by two Unsigned
 * Integers. A sign value of zero (0) is used to represent positive Decimal
 * values and a sign value of one (1) is used to represent negative Decimal
 * values The first Integer represents the integral portion of the Decimal
 * value. The second positive integer represents the fractional portion of
 * the decimal with the digits in reverse order to preserve leading zeros.
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] dec_val decimal value to be encoded
 * @return Error handling code.
 */
errorCode encodeBigDecimalValue(EXIStream* strm, bigDecimal dec_val);


/**
 * @brief Encode EXI Float type
 * Encode a Float represented as two consecutive Integers. The first Integer
 * represents the mantissa of the floating point number and the second
 * Integer represents the 10-based exponent of the floating point number
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] double_val float value to be encoded
 * @return Error handling code.
 */
errorCode encodeFloatValue(EXIStream* strm, double double_val);

/**
 * @brief Encode EXI Float type, bigger than double
 * Encode a Float represented as two consecutive Integers. The first Integer
 * represents the mantissa of the floating point number and the second
 * Integer represents the 10-based exponent of the floating point number
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] double_val big float to be encoded
 * @return Error handling code
 */
errorCode encodeBigFloatValue(EXIStream* strm, BigFloat double_val);

#endif /* STREAMENCODE_H_ */