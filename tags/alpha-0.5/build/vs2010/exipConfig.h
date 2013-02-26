/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * Used for the MS VS build
 *
 * @date Oct 13, 2010
 * @author Robert Cragie
 * @version 0.4
 * @par[Revision] $Id: exipConfig.h 92 2011-03-29 15:31:01Z kjussakov $
 */
#ifndef EXIPCONFIG_H_
#define EXIPCONFIG_H_

#define ON  1
#define OFF 0

#  define EXIP_DEBUG  	     ON
#  define EXIP_DEBUG_LEVEL INFO

#  define DEBUG_STREAM_IO   ON
#  define DEBUG_COMMON      ON
#  define DEBUG_CONTENT_IO  ON
#  define DEBUG_GRAMMAR     ON
#  define DEBUG_GRAMMAR_GEN ON
#  define DEBUG_STRING_TBLS ON

/**
 * Define the memory allocation and freeing functions
 */
#ifdef USE_DBG_MALLOC
#  include "dbgMalloc.h"
#  define EXIP_MALLOC(p) dbgMalloc((size_t)(p))
#  define EXIP_REALLOC(p1, p2) dbgRealloc((p1), (size_t)(p2))
#  define EXIP_MFREE dbgFree  //TODO: document this macro #DOCUMENT#
#else
#  include <stdlib.h>
#  define EXIP_MALLOC(p) malloc((size_t)(p))
#  define EXIP_REALLOC(p1, p2) realloc((p1), (size_t)(p2))
#  define EXIP_MFREE free
#endif

#  define MAX_HASH_TABLE_SIZE 16000
#  define HASH_TABLE_USE ON
#  define INITIAL_HASH_TABLE_SIZE 6151
#  define DYN_ARRAY_USE ON

// Some types in procTypes.h
#  include <stdint.h>
#define EXIP_DECIMAL float
#define EXIP_UNSIGNED_INTEGER int64_t
#define EXIP_INTEGER int64_t

#define EXIP_STRTOLL _strtoi64

#define EXIP_INDEX uint16_t
#define EXIP_INDEX_MAX UINT16_MAX

#define EXIP_SMALL_INDEX uint8_t
#define EXIP_SMALL_INDEX_MAX UINT8_MAX

struct ThinFloat
{
	int64_t mantissa;
	int16_t exponent;
};

#define EXIP_FLOAT struct ThinFloat

// NOTE: The GR_VOID_NON_TERMINAL should be set to the maximum 24 bit unsigned value in case the
// SMALL_INDEX_MAX is 32 bits or bigger
#define GR_VOID_NON_TERMINAL 0xFFFFFF

#endif /* EXIPCONFIG_H_ */
