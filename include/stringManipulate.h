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
 * @file stringManipulate.h
 * @brief String manipulation functions
 * @date Sep 3, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef STRINGMANIPULATE_H_
#define STRINGMANIPULATE_H_

#include "procTypes.h"
#include "errorHandle.h"

/**
 * @brief Allocate a memory for a string with UCSchars number of UCS characters
 * Note! The implementation of this function is platform-specific.
 * It depends on the representation of the characters
 *
 * @param[in, out] str a pointer to the uninitialized string
 * @param[in] UCSchars the number of characters (as described by UCS [ISO/IEC 10646])
 * @param[in, out] memList A list storing the memory allocations
 * @return Error handling code
 */
errorCode allocateStringMemory(CharType** str, size_t UCSchars, AllocList* memList);

/**
 * @brief Writes a UCS [ISO/IEC 10646] code point to a string
 * Note! The implementation of this function is platform-specific.
 * It depends on the representation of the characters.
 * The memory needed for str should be allocated before the invocation
 * of this function
 *
 * @param[in, out] str string to be written on
 * @param[in] code_point UCS [ISO/IEC 10646] code point
 * @param[in, out] UCSposition the position of the code point relatively to the beginning of the string.
 * 				   If the string is empty, then 0 should be passed. The position of the next character is
 * 				   returned in the same parameter. This is needed because in some encodings (ex. UTF-8)
 * 				   there is no direct correspondence between the character in UCS and the number of CharTypes that it
 * 			       takes in the encoding. For example 1 UCS character may require 2 or 3  CharTypes to be encoded.
 * @return Error handling code
 */
errorCode writeCharToString(StringType* str, uint32_t code_point, size_t* UCSposition);

/**
 * @brief Creates an empty string
 * Note! The implementation of this function is platform-specific.
 * @param[in, out] emptyStr empty string
 * @return Error handling code
 */
errorCode getEmptyString(StringType* emptyStr);

/**
 * @brief Checks if an string is empty
 * Note! The implementation of this function is platform-specific.
 * @param[in] str string to check
 * @return 0 if not empty, 1 if empty
 */
char isStringEmpty(const StringType* str);

/**
 * @brief Transform a NULL terminated string of ASCII chars to StringType allocating memory for the CharType*.
 * Note! The implementation of this function is platform-specific.
 * @param[in] inStr ASCII stream
 * @param[in, out] outStr resulted string
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] clone Boolean indicating if outStr should reuse the memory allocated for inStr if possible.
 * 					0 - if StringType implementation allows it - do not allocate new memory for the string
 * 					1 - always allocate fresh memory for outStr and copy inStr there
 * @return Error handling code
 */
errorCode asciiToString(const char* inStr, StringType* outStr, AllocList* memList, unsigned char clone);

/**
 * @brief Tests if two strings are equal
 * Note! The implementation of this function is platform-specific.
 * @param[in] str1 string to compare
 * @param[in] str2 string to compare
 * @return 1 if the strings are equal, 0 - otherwise
 */
char stringEqual(const StringType str1, const StringType str2);

/**
 * @brief Compare two strings lexicographically
 * Note! The implementation of this function is platform-specific.
 * @param[in] str1 string to compare
 * @param[in] str2 string to compare
 * @return 0 when the strings are equal; negative int when str1<str2; positive when  str1>str2
 */
int stringCompare(const StringType str1, const StringType str2);

/**
 * @brief Checks if a StringType string and ASCII string are equal
 * Note! The implementation of this function is platform-specific.
 * @param[in] str1 string to compare
 * @param[in] str2 null terminated string to compare
 * @return 1 if the strings are equal, 0 - otherwise
 */
char stringEqualToAscii(const StringType str1, const char* str2);

/**
 * @brief Returns the UCS [ISO/IEC 10646] code point at particular index from a String
 * Note! The implementation of this function is platform-specific.
 * @param[in] str string
 * @param[in] charIndex character index within the string
 * @param[out] UCScp the returned UCS code point
 * @return 1 if the strings are equal, 0 - otherwise
 */
errorCode getUCSCodePoint(const StringType* str, size_t charIndex, uint32_t* UCScp);

/**
 * @brief Makes a copy of the string in a new location
 * Note! The implementation of this function is platform-specific.
 * @param[in] src the string to be copied
 * @param[in] newStr will point to the newly allocated memory with scr->CharType* copied there
 * @param[in, out] memList A list storing the memory allocations
 * @return Error handling code
 */
errorCode cloneString(const StringType* src, StringType* newStr, AllocList* memList);

/**
 * @brief Split a string into multiple tokens/strings based on a particular character
 * @param[in] src the source string
 * @param[in] separator a pattern used to split the src string
 * @param[out] tokensArray an array of tokens
 * @param[out] tokensCount the number of tokens
 * @param[in, out] memList A list storing the memory allocations
 * @return Error handling code
 */
errorCode splitStringByChar(const StringType* src, CharType separator, StringType** tokensArray, unsigned int* tokensCount, AllocList* memList);

#if EXIP_DEBUG == ON

/**
 * @brief Prints out a StringType
 * Note! The implementation of this function is platform-specific.
 * Used for debugging purposes.
 * @param[in] inStr Input string to be printed
 */
void printString(const StringType* inStr);

#endif /* EXIP_DEBUG */

#endif /* STRINGMANIPULATE_H_ */
