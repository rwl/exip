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
 * @file procTypes.h
 * @brief Common structure types used throughout the project
 *
 * @date Jul 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef PROCTYPES_H_
#define PROCTYPES_H_

#include "errorHandle.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#define REVERSE_BIT_POSITION(p) (7 - p)

/**
 * EXI header related macros
 */
#define BIT_PACKED 0
#define BYTE_ALIGNMENT 1
#define PRE_COMPRESSION 2

/**
 * Since we are working with embedded systems - exclude the UTF-8 support
 */
#ifndef CHAR_TYPE
# define CHAR_TYPE unsigned char
#endif
// TODO: document this macro - it must be set during source build to overwrite the default behavior


typedef CHAR_TYPE CharType;

struct StringType
{
	CharType* str;
	unsigned int length;
};

typedef struct StringType StringType;

/**
 * Define the memory allocation function
 */
#define EXIP_MALLOC malloc  //TODO: document this macro
#include <stdlib.h>         //TODO: make it conditional!

/**
 * Define the memory freeing function
 */
#define EXIP_MFREE free  //TODO: document this macro

/********* BEGIN: String Table Types ***************/

struct ValueRow {
	StringType string_val;
};

struct ValueTable {
	struct ValueRow* rows; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct ValueTable ValueTable;

struct ValueLocalCrossTable {
	unsigned int* valueRowIds; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct ValueLocalCrossTable ValueLocalCrossTable;

struct PrefixRow {
	StringType string_val;
};

struct PrefixTable {
	struct PrefixRow* rows; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct PrefixTable PrefixTable;

struct LocalNamesRow {
	ValueLocalCrossTable* vCrossTable;
	StringType string_val;
};

struct LocalNamesTable {
	struct LocalNamesRow* rows; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct LocalNamesTable LocalNamesTable;

struct URIRow {
	PrefixTable* pTable;
	LocalNamesTable* lTable;
	StringType string_val;
};

struct URITable {
	struct URIRow* rows; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct URITable URITable;

/********* END: String Table Types ***************/


struct QName {
	StringType* uri;       // Pointer to a String value in the string table
	StringType* localName; // Pointer to a String value in the string table
};

typedef struct QName QName;

/**
 * Represents an EXI stream
 */
struct EXIStream
{
	/**
	 * Bit stream representing EXI message
	 */
	char* buffer;

	/**
	 * Current position in the buffer - bytewise
	 */
	int bufferIndx;

	/**
	 * Value between 0 and 7; shows the current position within the current byte
	 * 7 is the least significant bit position in the byte
	 */
	unsigned char bitPointer;

	/**
	 * The EXI Options which are derived from the EXI header. They control the
	 * parsing and serialization of the stream.
	 */
	struct EXIOptions* opts;

	/**
	 * The value string table
	 */
	ValueTable* vTable;

	/**
	 * The URI string table
	 */
	URITable* uriTable;
};

typedef struct EXIStream EXIStream;


struct DatatypeRepresentationMap
{
	void* TODO; //TODO: fill in the information for this structure
};

typedef struct DatatypeRepresentationMap DatatypeRepresentationMap;

struct EXIOptions
{
	/**
	 * The alignment option - BIT-PACKED, BYTE-ALIGNMENT or PRE-COMPRESSION
	 */
	unsigned char alignment;

	/**
	 * 0 - false; 1 - true
	 */
	unsigned char compression;

	/**
	 * Strict interpretation of schemas: 0 - false; 1 - true
	 */
	unsigned char strict;

	/**
	 * EXI fragment instead of an EXI document: 0 - false; 1 - true
	 */
	unsigned char fragment;


	//TODO: define the bit mask of booleans for preserve option
	/**
	 * Specifies whether comments, pis, etc. are preserved - bit mask of booleans
	 */
	unsigned char preserve;

	/**
	 * Enables self-contained elements: 0 - false; 1 - true
	 */
	unsigned char selfContained;

	/**
	 * Identify the schema information, if any, used to encode the body
	 */
	char* schemaID;

	/**
	 * Specify alternate datatype representations for typed values in the EXI body
	 */
	DatatypeRepresentationMap* drMap;

	/**
	 *  Specifies the block size used for EXI compression
	 */
	long blockSize;

	/**
	 * Specifies the maximum string length of value content items to be considered for addition to the string table.
	 * 0 - unbounded
	 */
	long valueMaxLength;

	/**
	 * Specifies the total capacity of value partitions in a string table
	 * 0 - unbounded
	 */
	long valuePartitionCapacity;

	/**
	 * User defined meta-data may be added
	 */
	void* user_defined_data;
};

/**
 * Represents an EXI header
 */
struct EXIheader
{
	/**
	 * Boolean value - 0 for lack of EXI cookie, otherwise 1
	 */
	unsigned char has_cookie;

	/**
	 * Boolean value - 0 for lack of EXI Options, otherwise 1
	 */
	unsigned char has_options;

	/**
	 * EXI stream version
	 */
	int version_number;

	struct EXIOptions* opts;
};

typedef struct EXIheader EXIheader;


/**********************Function definitions************************/

/**
 * @brief Set the EXI options to their default values
 *
 * @param[in] strm EXI stream of bits
 * @return Error handling code
 */
errorCode makeDefaultOpts(struct EXIOptions* opts);

/**
 * @brief Determine the number of bits needed to encode a unsigned integer value
 * @param[in] val unsigned integer value
 *
 * @return The number of bits needed
 */
unsigned char getBitsNumber(unsigned int val);

#endif /* PROCTYPES_H_ */
