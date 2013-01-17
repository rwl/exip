/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file pc/exipConfig.h
 * @brief Configuration parameters of the EXIP library
 * To be defined per application
 *
 * @date Oct 13, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include <stdlib.h>

#ifndef EXIPCONFIG_H_
#define EXIPCONFIG_H_

#define ON  1
#define OFF 0

/**
 * @def EXIP_DEBUG
 * 		Turn the debugging ON/OFF
 * @def EXIP_DEBUG_LEVEL
 * 		Sets the debugging level (INFO, WARNING or ERROR)
 *
 * @def DEBUG_STREAM_IO
 * 		Turns the debugging ON/OFF for the STREAM_IO module
 * @def DEBUG_COMMON
 * 		Turns the debugging ON/OFF for the COMMON module
 * @def DEBUG_CONTENT_IO
 * 		Turns the debugging ON/OFF for the CONTENT_IO module
 * @def DEBUG_GRAMMAR
 * 		Turns the debugging ON/OFF for the GRAMMAR module
 * @def DEBUG_GRAMMAR_GEN
 * 		Turns the debugging ON/OFF for the GRAMMAR_GEN module
 * @def DEBUG_STRING_TBLS
 * 		Turns the debugging ON/OFF for the STRING_TBLS module
 * @ref debugging */
#define EXIP_DEBUG  	   ON
#define EXIP_DEBUG_LEVEL INFO

#define DEBUG_STREAM_IO   OFF
#define DEBUG_COMMON      OFF
#define DEBUG_CONTENT_IO  OFF
#define DEBUG_GRAMMAR     OFF
#define DEBUG_GRAMMAR_GEN OFF
#define DEBUG_STRING_TBLS OFF

/**
 * @name mem_group Define the memory allocation functions and freeing functions
 *
 * @def EXIP_MALLOC
 * 		malloc function
 * @def EXIP_REALLOC
 * 		realloc function
 * @def EXIP_MFREE
 * 		free function
 */
#define EXIP_MALLOC malloc
#define EXIP_REALLOC realloc
#define EXIP_MFREE free

/** @def HASH_TABLE_USE
 * 		Whether to use hash table for value partition table when in encoding mode
 * 	@def INITIAL_HASH_TABLE_SIZE
 * 		The initial capacity of the hash tables
 * 	@def MAX_HASH_TABLE_SIZE
 * 		The maximum capacity of the hash tables
 */
#define HASH_TABLE_USE ON
#define INITIAL_HASH_TABLE_SIZE 6151
#define MAX_HASH_TABLE_SIZE 32000

/** Whether to use dynamic arrays */
#define DYN_ARRAY_USE ON

#endif /* EXIPCONFIG_H_ */
