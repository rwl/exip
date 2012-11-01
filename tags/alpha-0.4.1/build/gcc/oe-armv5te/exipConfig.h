/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * Used for OpenEmbedded on ARMv5te CPUs
 *
 * @date Apr 6, 2012
 * @author Ken Bannister
 * @version 0.4
 * @par[Revision] $Id$
 */

#include <stdlib.h>

#ifndef EXIPCONFIG_H_
#define EXIPCONFIG_H_

#define ON  1
#define OFF 0

#define EXIP_DEBUG  	   ON
#define EXIP_DEBUG_LEVEL INFO

#define DEBUG_STREAM_IO   OFF
#define DEBUG_COMMON      OFF
#define DEBUG_CONTENT_IO  OFF
#define DEBUG_GRAMMAR     OFF
#define DEBUG_GRAMMAR_GEN OFF
#define DEBUG_STRING_TBLS OFF

/**
 * Define the memory allocation functions and freeing functions
 */
#define EXIP_MALLOC malloc
#define EXIP_REALLOC realloc
#define EXIP_MFREE free

// Whether to use hash table for value partition table when in encoding mode
#define HASH_TABLE_USE ON
#define MAX_HASH_TABLE_SIZE 32000
#define DYN_ARRAY_USE ON

// Some types in procTypes.h
// GCC rejects EXIP's default for decimal
#define EXIP_DECIMAL double

#endif /* EXIPCONFIG_H_ */
