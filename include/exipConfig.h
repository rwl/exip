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
 *
 * @date Oct 13, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */
#ifndef EXIPCONFIG_H_
#define EXIPCONFIG_H_

#define INFO 1
#define WARNING 2
#define ERROR 3

#define ON  1
#define OFF 0

#define EXIP_DEBUG  	  OFF    //TODO: document this macro #DOCUMENT#
#define EXIP_DEBUG_LEVEL INFO    //TODO: document this macro #DOCUMENT#

#define DEBUG_STREAM_IO   OFF	//TODO: document this macro #DOCUMENT#
#define DEBUG_COMMON      OFF	//TODO: document this macro #DOCUMENT#
#define DEBUG_CONTENT_IO  OFF	//TODO: document this macro #DOCUMENT#
#define DEBUG_GRAMMAR     OFF	//TODO: document this macro #DOCUMENT#
#define DEBUG_GRAMMAR_GEN OFF	//TODO: document this macro #DOCUMENT#
#define DEBUG_STRING_TBLS OFF	//TODO: document this macro #DOCUMENT#

#define DEBUG_ALL_MODULES (DEBUG_STREAM_IO || DEBUG_COMMON || DEBUG_CONTENT_IO || DEBUG_GRAMMAR || DEBUG_GRAMMAR_GEN || DEBUG_STRING_TBLS) //TODO: document this macro #DOCUMENT#

#define TRUE  1
#define FALSE 0

/**
 * Define the memory allocation functions
 */
#include <stdlib.h> //TODO: make it conditional!
#define EXIP_MALLOC malloc   //TODO: document this macro #DOCUMENT#
#define EXIP_REALLOC realloc //TODO: document this macro #DOCUMENT#

/**
 * Define the memory freeing function
 */
#define EXIP_MFREE free  //TODO: document this macro #DOCUMENT#

#define MAX_HASH_TABLE_SIZE 16000 //TODO: document this macro #DOCUMENT#

#endif /* EXIPCONFIG_H_ */
