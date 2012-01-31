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
 * @file exipConfig.h
 * @brief Configuration parameters of the EXIP library
 * To be defined per application
 *
 * @date Oct 13, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id: exipConfig.h 92 2011-03-29 15:31:01Z kjussakov $
 */
#ifndef EXIPCONFIG_H_
#define EXIPCONFIG_H_

#define ON  1
#define OFF 0

#ifdef EXIP_APP_CONFIG

#  include "app_config.h"

#else  // The default EXIP parameters

#  define EXIP_DEBUG  	     ON //TODO: document this macro #DOCUMENT#
#  define EXIP_DEBUG_LEVEL INFO //TODO: document this macro #DOCUMENT#

#  define DEBUG_STREAM_IO   ON	//TODO: document this macro #DOCUMENT#
#  define DEBUG_COMMON      ON	//TODO: document this macro #DOCUMENT#
#  define DEBUG_CONTENT_IO  ON	//TODO: document this macro #DOCUMENT#
#  define DEBUG_GRAMMAR     ON	//TODO: document this macro #DOCUMENT#
#  define DEBUG_GRAMMAR_GEN ON	//TODO: document this macro #DOCUMENT#
#  define DEBUG_STRING_TBLS ON	//TODO: document this macro #DOCUMENT#

#  define DEBUG_ALL_MODULES (DEBUG_STREAM_IO || DEBUG_COMMON || DEBUG_CONTENT_IO || DEBUG_GRAMMAR || DEBUG_GRAMMAR_GEN || DEBUG_STRING_TBLS) //TODO: document this macro #DOCUMENT#

/**
 * Define the memory allocation functions
 */
#  include <stdlib.h>
#  define EXIP_MALLOC(p) malloc((size_t)(p))   //TODO: document this macro #DOCUMENT#
#  define EXIP_REALLOC(p1, p2) realloc((p1), (size_t)(p2)) //TODO: document this macro #DOCUMENT#

/**
 * Define the memory freeing function
 */
#  define EXIP_MFREE free  //TODO: document this macro #DOCUMENT#

#  define MAX_HASH_TABLE_SIZE 16000 //TODO: document this macro #DOCUMENT#
#  define HASH_TABLE_USE ON

// Some types in procTypes.h
#  include <stdint.h>
#define EXIP_DECIMAL float
#define EXIP_UNSIGNED_INTEGER int32_t
#define EXIP_INTEGER int32_t

struct ThinFloat
{
	int32_t mantissa;
	char exponent;
};

#define EXIP_FLOAT struct ThinFloat

#endif /* EXIP_APP_CONFIG */

#endif /* EXIPCONFIG_H_ */
