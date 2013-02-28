/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * Configuration parameters of the EXIP library for the contiki operating system
 *
 * @date Sep 13, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "d_mem.h"
#include <stdlib.h>

#ifndef EXIPCONFIG_H_
#define EXIPCONFIG_H_

#define ON  1
#define OFF 0

#define EXIP_DEBUG  	  OFF
#define EXIP_DEBUG_LEVEL INFO

#define DEBUG_STREAM_IO   OFF
#define DEBUG_COMMON      OFF
#define DEBUG_CONTENT_IO  OFF
#define DEBUG_GRAMMAR     OFF
#define DEBUG_GRAMMAR_GEN OFF
#define DEBUG_STRING_TBLS OFF

#define DEBUG_CHAR_OUTPUT(character)	do {_printf ("%c", character);} while(0)
#define DEBUG_OUTPUT(msg)	do {_printf msg;} while(0)

#define assert(ignore)((void) 0)

/**
 * Define the memory allocation and freeing functions
 */
#define EXIP_MALLOC d_malloc
#define EXIP_REALLOC d_realloc
#define EXIP_MFREE d_free

#define HASH_TABLE_USE OFF
#define INITIAL_HASH_TABLE_SIZE 53
#define MAX_HASH_TABLE_SIZE 3000
#define DYN_ARRAY_USE ON


// Some types in procTypes.h
#define EXIP_DECIMAL float
#define EXIP_UNSIGNED_INTEGER uint32_t
#define EXIP_INTEGER int32_t

struct ThinFloat
{
	int32_t mantissa;
	char exponent;
};

#define EXIP_FLOAT struct ThinFloat

#define LLONG_MAX LONG_MAX
#define LLONG_MIN LONG_MIN

#define DEFAULT_GRAMMAR_TABLE         40
#define DEFAULT_SIMPLE_GRAMMAR_TABLE  55

#define EXIP_INDEX size_t
#define EXIP_INDEX_MAX SIZE_MAX

#define EXIP_SMALL_INDEX uint8_t
#define EXIP_SMALL_INDEX_MAX UINT8_MAX

#endif /* EXIPCONFIG_H_ */
