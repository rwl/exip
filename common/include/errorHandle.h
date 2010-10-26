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
 * @file errorHandle.h
 * @brief Error handling codes and function definitions
 *
 * @date Jul 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef ERRORHANDLE_H_
#define ERRORHANDLE_H_

#include "exipConfig.h"

/* Platform specific debugging output */
#ifndef DEBUG_OUTPUT	// TODO: document this macro #DOCUMENT#
#  define DEBUG_OUTPUT(msg)	do {printf msg;} while(0)
#endif

/* Platform specific debugging character output */
#ifndef DEBUG_CHAR_OUTPUT	// TODO: document this macro #DOCUMENT#
#  define DEBUG_CHAR_OUTPUT(character)	do {putchar (character);} while(0)
#endif

#ifdef EXIP_DEBUG // TODO: document this macro #DOCUMENT#

#include <stdio.h>

#ifndef EXIP_DEBUG_LEVEL // TODO: document this macro #DOCUMENT#
# define EXIP_DEBUG_LEVEL INFO
#endif

#  define DEBUG_MSG(level,msg) do { if (level >= EXIP_DEBUG_LEVEL) { DEBUG_OUTPUT(msg); } } while(0)
#else
#  define DEBUG_MSG(level,msg)
#endif /* EXIP_DEBUG */

typedef char errorCode;

//TODO: define the rest of the error codes

/* Definitions for error constants. */
/** No error, everything OK. */
#define ERR_OK    0

/** Buffer end reached  */
#define BUFFER_END_REACHED 4

/** Stream value bigger than a processor type boundary */
#define BIGGER_TYPE_REQUIRED 3

/** Processor state is inconsistent with the stream events  */
#define INCONSISTENT_PROC_STATE 2

/** Error in the EXI header */
#define INVALID_EXI_HEADER    1

/** Unsuccessful memory allocation */
#define MEMORY_ALLOCATION_ERROR -1

/** Try to access null pointer */
#define NULL_POINTER_REF -2

/** Array out of bound  */
#define OUT_OF_BOUND_BUFFER -3

/** Any error that does not fall into the other categories */
#define UNEXPECTED_ERROR -126

/** The code for this function is not yet implemented. */
#define NOT_IMPLEMENTED_YET -127

#endif /* ERRORHANDLE_H_ */
