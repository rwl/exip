/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * Configuration parameters of the EXIP library for the Mulle platform
 *
 * @date Sep 13, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "ip_malloc.h"
#include "elib.h"

#ifndef EXIPCONFIG_H_
#define EXIPCONFIG_H_

#define ON  1
#define OFF 0

#define EXIP_DEBUG  	   ON
#define EXIP_DEBUG_LEVEL INFO

#define DEBUG_STREAM_IO   OFF
#define DEBUG_COMMON      OFF
#define DEBUG_CONTENT_IO   ON
#define DEBUG_GRAMMAR      ON
#define DEBUG_GRAMMAR_GEN OFF
#define DEBUG_STRING_TBLS OFF

#define DEBUG_CHAR_OUTPUT(character)	do {_printf ("%c", character);} while(0)
#define DEBUG_OUTPUT(msg)	do {_printf msg;} while(0)

/**
 * Define the memory allocation and freeing functions
 */
#define EXIP_MALLOC malloc
#define EXIP_REALLOC realloc
#define EXIP_MFREE free

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

/* NO support for strtol(), strtoll() and bsearch(),
 * Not used in our use cases so just ignore them */
#define EXIP_STRTOLL(buff, endPointer, b) 0
#define strtol(buff, endPointer, b) 0
#define LLONG_MAX -1
#define LLONG_MIN -1
#define bsearch(key, base, nmemb, size, fn_t) NULL

#endif /* EXIPCONFIG_H_ */
