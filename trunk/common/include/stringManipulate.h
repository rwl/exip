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
 * @brief Translate the UCS [ISO/IEC 10646] code point to CharType
 * Note! The implementation of this function is platform-specific.
 * It depends on the representation of the characters
 * @param[in] code_point  UCS [ISO/IEC 10646] code point
 * @param[out] ch the character corresponding to the code_point
 * @return Error handling code
 */
errorCode UCSToChar(unsigned int code_point, CharType* ch);

/**
 * @brief Creates an empty string
 * Note! The implementation of this function is platform-specific.
 * @param[in, out] emptyStr empty string
 * @return Error handling code
 */
errorCode getEmptyString(StringType emptyStr);

/**
 * @brief Transform a NULL terminated string of ASCII chars to StringType allocating memory for the CharType*.
 * Note! The implementation of this function is platform-specific.
 * @param[in] inStr ASCII stream
 * @param[in, out] outStr resulted string
 * @return Error handling code
 */
errorCode asciiToString(char* inStr, StringType outStr);

#endif /* STRINGMANIPULATE_H_ */
