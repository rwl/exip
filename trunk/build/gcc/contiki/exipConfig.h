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

#define EXIP_INDEX uint16_t
#define EXIP_INDEX_MAX UINT16_MAX

#define EXIP_SMALL_INDEX uint8_t
#define EXIP_SMALL_INDEX_MAX UINT8_MAX

/**
 * Whether the EXIP library is conforming to the W3C EXI Profile specification.
 * EXI Profile helps reduce RAM and programming memory and can be used
 * only for schema mode processing. Use SCHEMA_ID_EMPTY schemaId switch to enable
 * schema mode for otherwise schema-less streams.
 * The value of OFF disable the EXI Profile mode i.e. full featured EXI processing.
 * Set to ON to enable EXI Profile default (most restrictive) mode of operation where:
 * - maximumNumberOfBuiltInElementGrammars parameter value is set to 0
 * - maximumNumberOfBuiltInProductions parameter value is set to 0
 * - localValuePartitions parameter value is set to 0.
 *
 * Note that encoding/decoding the Profile parameters in the header is not supported
 * and most likely will never be.
 * Use only out-of-band communication of the EXI Profile default mode!
 *
 * @see http://www.w3.org/TR/exi-profile/
 */
#define EXI_PROFILE_DEFAULT OFF

#if EXI_PROFILE_DEFAULT
# define VALUE_CROSSTABLE_USE  OFF
# define BUILD_IN_GRAMMARS_USE OFF
#else
// Configure here which of the EXI features/components to be included in the compilation:

/* Whether to implement the local value string table.
 * NOTE: EXI streams that are not encoded using this option cannot be decoded correctly
 * and will return an error. The opposite is true however - a stream encoded with no
 * local value tables is valid EXI stream can be decoded with full-fledged EXI processor.
 * Disabling the local values indexing is used in EXI Profile
 * and can possibly found use in other application profiles of the EXI spec. */
#define VALUE_CROSSTABLE_USE  ON

/* Whether to enable the use of build-in grammars.
 * NOTE: Only useful for schema STRICT mode processing without schema extension points
 * or so called wildcard elements and attributes. */
#define BUILD_IN_GRAMMARS_USE ON
#endif

#endif /* EXIPCONFIG_H_ */
