/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file streamDecode.h
 * @brief Interface to a higher-level EXI stream decoder - decode basic EXI types
 *
 * @date Jul 7, 2010
 * @author Rumen Kyusakov
 * @version 0.4
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
errorCode decodeNBitUnsignedInteger(EXIStream* strm, unsigned char n, unsigned int* int_val);

/**
 * @brief Decode EXI Boolean
 * Decode a single boolean value. The value false is represented by the bit
 * (byte) 0, and the value true is represented by the bit (byte) 1.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] bool_val 0-false, 1-true
 * @return Error handling code
 */
errorCode decodeBoolean(EXIStream* strm, boolean* bool_val);

/**
 * @brief Decode EXI Unsigned Integer type
 * Decode an arbitrary precision non negative integer using a sequence of
 * octets. The most significant bit of the last octet is set to zero to
 * indicate sequence termination. Only seven bits per octet are used to
 * store the integer's value.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] int_val resulting unsigned integer value
 * @return Error handling code.
 */
errorCode decodeUnsignedInteger(EXIStream* strm, UnsignedInteger* int_val);

/**
 * @brief Decode EXI String type
 * Decode a string as a length-prefixed sequence of UCS codepoints, each of
 * which is encoded as an integer.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] string_val decoded string
 * @return Error handling code
 */
errorCode decodeString(EXIStream* strm, String* string_val);

/**
 * @brief Decode String with the length of the String specified
 * This function is used for Partitions Optimized for Frequent use of String Literals
 * when there is a local name miss => the Length part is read first.
 * The memory to hold the string data should be allocated before
 * calling this function.
 *
 * @param[in] strm EXI stream of bits
 * @param[in] str_length the length of the string
 * @param[out] string_val decoded string
 * @return Error handling code
 */
errorCode decodeStringOnly(EXIStream* strm, Index str_length, String* string_val);

/**
 * @brief Decode EXI Binary type
 * Decode a binary value as a length-prefixed sequence of octets.
 * Dynamically allocate a memory for the buffer
 *
 * @param[in] strm EXI stream of bits
 * @param[out] binary_val decoded binary value
 * @param[out] nbytes length of decoded binary content
 * @return Error handling code
 */
errorCode decodeBinary(EXIStream* strm, char** binary_val, Index* nbytes);

/**
 * @brief Decode EXI (signed) Integer type
 * Decode an arbitrary precision integer using a sign bit followed by a
 * sequence of octets. The most significant bit of the last octet is set to
 * zero to indicate sequence termination. Only seven bits per octet are used
 * to store the integer's value.
 *
 * @param[in] strm EXI stream of bits
 * @param[out] sint_val decoded signed integer value
 * @return Error handling code.
 */
errorCode decodeIntegerValue(EXIStream* strm, Integer* sint_val);

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
 * @param[out] dec_val decoded decimal value
 * @return Error handling code.
 */
errorCode decodeDecimalValue(EXIStream* strm, Decimal* dec_val);

/**
 * @brief Decode EXI Float type
 * Decode a Float represented as two consecutive Integers. The first Integer
 * represents the mantissa of the floating point number and the second
 * Integer represents the 10-based exponent of the floating point number
 *
 * @param[in] strm EXI stream of bits
 * @param[out] fl_val decoded float value as double
 * @return Error handling code.
 */
errorCode decodeFloatValue(EXIStream* strm, Float* fl_val);

/**
 * @brief Decode DateTime type
 * Included Components: Year, MonthDay, Time, presence, [FractionalSecs], presence, [TimeZone]
 *
 * @param[in] strm EXI stream of bits
 * @param[out] dt_val decoded dateTime value as EXIPDateTime
 * @return Error handling code.
 */
errorCode decodeDateTimeValue(EXIStream* strm, EXIPDateTime* dt_val);

#endif /* STREAMDECODE_H_ */
