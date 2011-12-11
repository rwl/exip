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

#define INFO 1
#define WARNING 2
#define ERROR 3

#if EXIP_DEBUG == ON // TODO: document this macro #DOCUMENT#

#  include <stdio.h>

/* Platform specific debugging character output */
#ifndef DEBUG_CHAR_OUTPUT	// TODO: document this macro #DOCUMENT#
#  define DEBUG_CHAR_OUTPUT(character)	do {putchar (character);} while(0)
#endif

/* Platform specific debugging output */
#  ifndef DEBUG_OUTPUT	// TODO: document this macro #DOCUMENT#
#    define DEBUG_OUTPUT(msg)	do {printf msg;} while(0)
#  endif

#  ifndef EXIP_DEBUG_LEVEL // TODO: document this macro #DOCUMENT#
#    define EXIP_DEBUG_LEVEL INFO
#  endif

#  define DEBUG_MSG(level, module, msg) do { if (level >= EXIP_DEBUG_LEVEL && module == ON) { DEBUG_OUTPUT(msg); } } while(0)
#else
#  define DEBUG_MSG(level, module, msg)
#endif /* EXIP_DEBUG */

typedef char errorCode;

//TODO: define the rest of the error codes

/* Definitions for error constants. */
/** No error, everything OK. */
#define ERR_OK    0

/** When encoding XML Schema in EXI the prefixes must be preserved:
 * When qualified namesNS are used in the values of AT or CH events in an EXI Stream,
 * the Preserve.prefixes fidelity option SHOULD be turned on to enable the preservation of
 * the NS prefix declarations used by these values. Note, in particular among other cases,
 * that this practice applies to the use of xsi:type attributes in EXI streams when Preserve.lexicalValues
 * fidelity option is set to true. */
#define NO_PREFIXES_PRESERVED_XML_SCHEMA 18

/** The information passed to the EXIP API is invalid */
#define INVALID_EXIP_CONFIGURATION 17

/** The number MAXIMUM_NUMBER_OF_PREFIXES_PER_URI is reached - must be increased in the build */
#define TOO_MUCH_PREFIXES_PER_URI 16

#define PARSING_COMPLETE 15

#define EMPTY_COLLECTION 14

/** Function invocation is invalid for the given arguments */
#define INVALID_OPERATION 13

/** A command to stop the EXI processing received from the application */
#define HANDLER_STOP_RECEIVED 12

/** An event code to be serialized is not found at the current grammar stack */
#define EVENT_CODE_MISSING 11

/** Buffer end reached  */
#define BUFFER_END_REACHED 10

/** Received EXI value type or event encoding that is invalid according to the specification */
#define INVALID_EXI_INPUT 9

/** Processor state is inconsistent with the stream events  */
#define INCONSISTENT_PROC_STATE 8

/** Error in the EXI header */
#define INVALID_EXI_HEADER    7

/** Unsuccessful memory allocation */
#define MEMORY_ALLOCATION_ERROR 6

/** Try to access null pointer */
#define NULL_POINTER_REF 5

/** Array out of bound  */
#define OUT_OF_BOUND_BUFFER 4

/** Hash table error  */
#define HASH_TABLE_ERROR 3

/** Any error that does not fall into the other categories */
#define UNEXPECTED_ERROR 2

/** The code for this function is not yet implemented. */
#define NOT_IMPLEMENTED_YET 1

#endif /* ERRORHANDLE_H_ */
