/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file errorHandle.h
 * @brief Error handling codes and function definitions
 *
 * @date Jul 7, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef ERRORHANDLE_H_
#define ERRORHANDLE_H_

#include "exipConfig.h"

#define INFO 1
#define WARNING 2
#define ERROR 3

/**
 * @page debugging How to debug
 *
 * Turning the debugging is done by setting EXIP_DEBUG to ON.
 * You have control over the level of verbosity (INFO, WARNING or ERROR)
 * and the source of debugging information. All these parameters
 * can be configured in the exipConfig.h header that is defined
 * per target platform in build/gcc/[target platform].
 * When turned on, the debugging information is by default
 * printed on the standard output.
 *
 * @section configuration Debug configuration
 * The following macro definitions are used to customize the debugging:
 * <dl>
 *     <dt>EXIP_DEBUG</dt>
 *     		<dd>Switch ON/OFF the debugging</dd>
 *     <dt>DEBUG_CHAR_OUTPUT(ch)</dt>
 *     		<dd>Defines a function to print a single character ch</dd>
 *     <dt>DEBUG_OUTPUT(str)</dt>
 *     		<dd>Defines a function to print a NULL terminated string str</dd>
 *     <dt>EXIP_DEBUG_LEVEL</dt>
 *     		<dd>Verbosity level: one of INFO, WARNING or ERROR</dd>
 *     <dt>DEBUG_MSG(level, module, msg)</dt>
 *     		<dd>Used to print a debug message</dd>
 * </dl>
 *
 * @section error_codes Error codes
 * Most of the functions in the exip library return a 8 bit integer that
 * indicates the status of the execution with 0 being success and
 * non-zero value indication an error condition.
 * <p>
 *  The error codes are defined in errorHandle.h
 * </p>
 *
 * @section debug_output Debug output
 * DEBUG_STREAM_IO provides a detailed stream output and position debug messages. It shows byte-level
 * output on a new line, beginning with '>>'. It indicates position of next write
 * in the form \@B:b, where B is the byte offset and b is the bit offset within
 * the byte.
 *
 * <div>
 *   Below is an example encoding for the EXI equivalent of "<description>new</description>". First,
 *   you see the local name for each production in the rule. Below the rule, the ">> 0x0 (3 bits)"
 *   tells exactly what was written to select the 'description' production at index 0.
 *   Then the "@8:4" shows the position at which the next output occurs. Just above the
 *   output below was "@8:1", where the present output was placed.
 *
 *   To see this sort of output, use DEBUG_STREAM_IO and DEBUG_CONTENT_IO.
 *
 *
 *   <pre>
 *	>RULE
 *	NT-1:
 *		SE (qname: 4:34)    NT-2    0   description
 *		SE (qname: 4:75)    NT-3    1   notes
 *		SE (qname: 4:3)     NT-4    2   activateAt
 *		SE (qname: 4:44)    NT-5    3   expireAt
 *		SE (qname: 4:80)    NT-6    4   panId
 *		SE (qname: 4:111)   NT-7    5   timezoneId
 *		SE (qname: 4:33)    NT-8    6   deploymentDevices
 *		EE      7
 *	>> 0x0 (3 bits)  \@8:4
 *
 *	>Start string data serialization
 *
 *	>RULE
 *	NT-0:
 *		CH [2]  NT-1    0
 *		AT (qname 2:1) [1]  NT-0    1.0
 *	>> 0x0 (1 bits)  \@8:5
 *	>> 0x05  \@8:6  \@9:5
 *
 *	 Write string, len 3: new
 *	>> 0x6E  \@9:6  \@10:5
 *	>> 0x65  \@10:6  \@11:5
 *	>> 0x77  \@11:6  \@12:5
 *	>End element serialization
 *    </pre>
 * </div>
 */
#if EXIP_DEBUG == ON

#  include <stdio.h>

/* Platform specific debugging character output */
#ifndef DEBUG_CHAR_OUTPUT
#  define DEBUG_CHAR_OUTPUT(character)	do {putchar (character);} while(0)
#endif

/* Platform specific debugging output */
#  ifndef DEBUG_OUTPUT
#    define DEBUG_OUTPUT(msg)	do {printf msg;} while(0)
#  endif

#  ifndef EXIP_DEBUG_LEVEL
#    define EXIP_DEBUG_LEVEL INFO
#  endif

#  define DEBUG_MSG(level, module, msg) do { if (level >= EXIP_DEBUG_LEVEL && module == ON) { DEBUG_OUTPUT(msg); } } while(0)
#else
#  define DEBUG_MSG(level, module, msg)
#endif /* EXIP_DEBUG */

typedef char errorCode;

/* Definitions for error constants. */

/** No error, everything is OK. */
#define ERR_OK    0

/** Mismatch in the header options.
 * This error can be due to:
 * 1) The "alignment" element MUST NOT appear in an EXI options document when the "compression" element is present;
 * 2) The "strict" element MUST NOT appear in an EXI options document when one of "dtd", "prefixes",
 * "comments", "pis" or "selfContained" element is present in the same options document. That is only the
 * element "lexicalValues", from the fidelity options, is permitted to occur in the presence of "strict" element;
 * 3) The "selfContained" element MUST NOT appear in an EXI options document when one of "compression",
 * "pre-compression" or "strict" elements are present in the same options document.
 * 4) The  datatypeRepresentationMap option does not take effect when the value of the Preserve.lexicalValues
 * fidelity option is true (see 6.3 Fidelity Options), or when the EXI stream is a schema-less EXI stream.
 * 5) Presence Bit for EXI Options not set and no out-of-band options set
 */
#define HEADER_OPTIONS_MISMATCH 20

#define INVALID_STRING_OPERATION 19

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
