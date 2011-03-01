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
#include <stdint.h>
#include <time.h>
#include "exipConfig.h"

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
 *	Fidelity option	Effect
 *---------------------------------------------
 *	Preserve.comments	CM events are preserved
 *	Preserve.pis	PI events are preserved
 *	Preserve.dtd	DOCTYPE and ER events are preserved
 *	Preserve.prefixes	NS events and namespace prefixes are preserved
 *	Preserve.lexicalValues	Lexical form of element and attribute values is preserved in value content items
 *
 **/
#define PRESERVE_COMMENTS  0b00000001
#define PRESERVE_PIS       0b00000010
#define PRESERVE_DTD       0b00000100
#define PRESERVE_PREFIXES  0b00001000
#define PRESERVE_LEXVALUES 0b00010000

#define IS_PRESERVED(p, mask) ((p & mask) != 0)

/**
 * For handling the DATE-TIME type (structure tm from time.h)
 */
#define SEC_PRESENCE       0b0000000000000001
#define MIN_PRESENCE       0b0000000000000010
#define HOUR_PRESENCE      0b0000000000000100
#define MDAY_PRESENCE      0b0000000000001000
#define MON_PRESENCE       0b0000000000010000
#define YEAR_PRESENCE      0b0000000000100000
#define WDAY_PRESENCE      0b0000000001000000
#define YDAY_PRESENCE      0b0000000010000000
#define DST_PRESENCE       0b0000000100000000
#define TZONE_PRESENCE     0b0000001000000000

#define IS_PRESENT(p, mask) ((p & mask) != 0)

/**
 * EXI processors SHOULD support arbitrarily large Unsigned Integer values.
 * EXI processors MUST support Unsigned Integer values less than 2147483648.
 * This macro is used to support unsigned integers bigger than 32 bits.
 * Applications which require support for larger than 64 bits unsigned integers must
 * override this macro
 */
#ifndef BIG_UNSIGNED_INT
# define BIG_UNSIGNED_INT uint64_t
#endif

typedef BIG_UNSIGNED_INT BigUnsignedInt;

/*
 * Used for the content handler interface for signed integers bigger than 32 bits
 * Application which require support for larger than 64 bits signed integers must
 * override this macro
 */
#ifndef BIG_SIGNED_INT
# define BIG_SIGNED_INT int64_t
#endif

typedef BIG_SIGNED_INT BigSignedInt;

/*
 * Used for the content handler interface for bigger than double floats
 * Application which require support for larger than long double must
 * override this macro
 */
#ifndef BIG_FLOAT
# define BIG_FLOAT long double
#endif

typedef BIG_FLOAT BigFloat;

/**
 * Represents decimal values. Consists of an integral part, a fractional part and a sign
 * A sign value of zero (0) is used to represent positive Decimal values and a sign value
 * of one (1) is used to represent negative Decimal values
 */
struct decimalEXIP {
	unsigned char sign;
	uint32_t integral;
	uint32_t fraction;
};

/**
 * Represents big decimal values. Consists of an integral part, a fractional part and a sign
 * A sign value of zero (0) is used to represent positive Decimal values and a sign value
 * of one (1) is used to represent negative Decimal values
 */
struct bigDecimalEXIP {
	unsigned char sign;
	BIG_UNSIGNED_INT integral;
	BIG_UNSIGNED_INT fraction;
};

/*
 * Used for the content handler interface for decimal values
 * Application which require support for different type of decimal values can
 * override this macro
 */
#ifndef DECIMAL
# define DECIMAL struct decimalEXIP
#endif

typedef DECIMAL decimal;

/*
 * Used for the content handler interface for big decimal values
 * Application which require support for different type of decimal values can
 * override this macro
 */
#ifndef BIG_DECIMAL
# define BIG_DECIMAL struct bigDecimalEXIP
#endif

typedef BIG_DECIMAL bigDecimal;

/**
 * Defines the encoding used for characters.
 * It is dependent on the implementation of the stringManipulate.h functions
 */
#ifndef CHAR_TYPE  // #DOCUMENT#
# define CHAR_TYPE unsigned char
#endif
// TODO: document this macro - it must be set during source build to overwrite the default behavior


typedef CHAR_TYPE CharType;

struct StringType
{
	CharType* str;
	size_t length;
};

typedef struct StringType StringType;

/********* START: Memory management definitions ***************/

#define ALLOCATION_ARRAY_SIZE 100 // #DOCUMENT#

struct allocBlock {
	void* allocation[ALLOCATION_ARRAY_SIZE];
	unsigned int currentAlloc;
	struct allocBlock* nextBlock;
};

struct allocList {
	struct allocBlock firstBlock;
	struct allocBlock* lastBlock;
};

typedef struct allocList AllocList;

// Used by the memoryManager when there is reallocation
struct reAllocPair {
	struct allocBlock* memBlock;
	unsigned int allocIndx;
};

/********* END: Memory management definitions ***************/

/********* BEGIN: String Table Types ***************/

struct ValueRow {
	StringType string_val;
};

struct ValueTable {
	struct ValueRow* rows; // Dynamic array
	size_t rowCount; // The number of rows
	size_t arrayDimension; // The size of the Dynamic array
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation
};

typedef struct ValueTable ValueTable;

struct ValueLocalCrossTable {
	size_t* valueRowIds; // Dynamic array
	uint16_t rowCount; // The number of rows
	uint16_t arrayDimension; // The size of the Dynamic array
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation
};

typedef struct ValueLocalCrossTable ValueLocalCrossTable;

struct PrefixRow {
	StringType string_val;
};

struct PrefixTable {
	struct PrefixRow* rows; // Dynamic array
	uint16_t rowCount; // The number of rows
	uint16_t arrayDimension; // The size of the Dynamic array
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation
};

typedef struct PrefixTable PrefixTable;

struct LocalNamesRow {
	ValueLocalCrossTable* vCrossTable;
	StringType string_val;
};

struct LocalNamesTable {
	struct LocalNamesRow* rows; // Dynamic array
	size_t rowCount; // The number of rows
	size_t arrayDimension; // The size of the Dynamic array
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation
};

typedef struct LocalNamesTable LocalNamesTable;

struct URIRow {
	PrefixTable* pTable;
	LocalNamesTable* lTable;
	StringType string_val;
};

struct URITable {
	struct URIRow* rows; // Dynamic array
	uint16_t rowCount; // The number of rows
	uint16_t arrayDimension; // The size of the Dynamic array
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation
};

typedef struct URITable URITable;

/********* END: String Table Types ***************/


struct QName {
	const StringType* uri;       // Pointer to a String value in the string table. It is not allowed to modify the string table content from this pointer.
	const StringType* localName; // Pointer to a String value in the string table. It is not allowed to modify the string table content from this pointer.
};

typedef struct QName QName;

/********* START: Grammar Types ***************/
/**
 * This is moved from the grammar module because of the EXIStream grammar element added
 */

/****************************************
 * Name           |   Notation   | Value
 * -------------------------------------
 * Start Document |      SD      |  0
 * End Document   |      ED      |  1
 * Start Element  |  SE( qname ) |  5
 * Start Element  |  SE( uri:* ) |  6
 * Start Element  |  SE( * )	 |  7
 * End Element	  |      EE      |  8
 * Attribute	  |  AT( qname ) |  2
 * Attribute      |  AT( uri:* ) |  3
 * Attribute      |  AT( * )     |  4
 * Characters	  |      CH      |  9
 * Nm-space Decl  |	     NS	     | 10
 * Comment	      |      CM      | 11
 * Proc. Instr.   |      PI      | 12
 * DOCTYPE	      |      DT      | 13
 * Entity Ref.    |      ER      | 14
 * Self Contained |      SC      | 15
 * Void           |      --      | 16     // Used to indicate lack of Terminal symbol in proto-grammars
 ****************************************/
typedef unsigned char EventType;

#define EVENT_SD       0
#define EVENT_ED       1
#define EVENT_SE_QNAME 5
#define EVENT_SE_URI   6
#define EVENT_SE_ALL   7
#define EVENT_EE       8
#define EVENT_AT_QNAME 2
#define EVENT_AT_URI   3
#define EVENT_AT_ALL   4
#define EVENT_CH       9
#define EVENT_NS      10
#define EVENT_CM      11
#define EVENT_PI      12
#define EVENT_DT      13
#define EVENT_ER      14
#define EVENT_SC      15
#define EVENT_VOID    16

/** This is the type of the "value" content of EXI events.
 *  It is used when schema is available.
 * 0 - there is no value content for the event
 * 1 - the type is String
 * 2 - Integer
 * 3 - Float
 * 4 - Decimal
 * 5 - Date-Time
 * 6 - Boolean
 * 7 - Binary
 * */
typedef unsigned char ValueType;

#define VALUE_TYPE_NONE      0
#define VALUE_TYPE_STRING    1
#define VALUE_TYPE_INTEGER   2
#define VALUE_TYPE_FLOAT     3
#define VALUE_TYPE_DECIMAL   4
#define VALUE_TYPE_DATE_TIME 5
#define VALUE_TYPE_BOOLEAN   6
#define VALUE_TYPE_BINARY    7

struct EXIEvent
{
	EventType eventType;
	ValueType valueType;
};

typedef struct EXIEvent EXIEvent;

struct EventCode
{
	unsigned int code[3];
	unsigned char size; // The number of integers constituting the EventCode
};

typedef struct EventCode EventCode;

struct Production
{
	EventCode code;
	EXIEvent event;
	unsigned int nonTermID; // unique identifier of right-hand side Non-terminal

	/**
	 * For SE(qname), SE(uri:*), AT(qname) and AT(uri:*). Points to the qname or its local name
	 * of the element/attribute
	 */
	uint16_t uriRowID;
	size_t lnRowID;
};

typedef struct Production Production;

// TODO: The NON_TERMINAL Ids should be assigned in such way that they are equal to the RULE array index
//       In this way the rule search based on a NON_TERMINAL will be a constant time operation!
//       This will affect in great extend the GrammarGen module

// Define Built-in Grammars non-terminals
#define GR_VOID_NON_TERMINAL 0 // Used to indicate that the production does not have NON_TERMINAL on the right-hand side
#define GR_DOCUMENT          1
#define GR_DOC_CONTENT       2
#define GR_DOC_END           3
#define GR_START_TAG_CONTENT 4
#define GR_ELEMENT_CONTENT   5
#define GR_FRAGMENT          6
#define GR_FRAGMENT_CONTENT  7

#define GR_SCHEMA_GRAMMARS_FIRST 100
/* The non-terminals IDs of all other grammars should start from value GR_SCHEMA_GRAMMARS_FIRST.
 * The value GR_SCHEMA_GRAMMARS_FIRST indicates the NON_TERMINAL describing the content of the entire grammar */

struct GrammarRule
{
	unsigned int nonTermID; // unique identifier of left-hand side Non-terminal
	Production* prodArray; // Array of grammar productions included in that rule
	uint16_t prodCount; // The number of productions in this Grammar Rule
	uint16_t prodDimension; // The size of the productions' array /allocated space for Productions/
	unsigned char bits[3]; // The number of bits used for the integers constituting the EventCode
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation
};

typedef struct GrammarRule GrammarRule;

struct EXIGrammar
{
	GrammarRule* ruleArray; // Array of grammar rules which constitute that grammar
	uint16_t rulesDimension; // The size of the array
	struct EXIGrammar* nextInStack;
	unsigned int lastNonTermID; // Stores the last NonTermID before another grammar is added on top of the stack
};

typedef struct EXIGrammar EXIGrammarStack; // Used to differentiate between single grammar (nextInStack == NULL) and stack of grammars

typedef struct hashtable GrammarPool; // Element grammar pool or Type grammar pool

/*********** END: Grammar Types ***************/

struct StringTablesContext
{
	uint16_t curr_uriID;
	size_t curr_lnID;
	unsigned char expectATData; // 1- Expecting value for an attribute, 0 - otherwise
};

/**
 * Representation of an Input/Output Stream
 */
struct ioStream
{
	/**
	 * When parsing: A function pointer used to fill the EXI buffer when emptied by reading from "stream" "size" number of bytes
	 * When serializing: A function pointer used to write "size" number of bytes of the buffer to the stream
	 * Return the number of bytes read/write
	 */
	size_t (*readWriteToStream)(void* buf, size_t size, void* stream);
	/**
	 * The input stream to be passed to the readInput function pointer
	 */
	void* stream;
};

typedef struct ioStream IOStream;

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

	/** Boolean value - 1 preview version, 0 final version */
	unsigned char is_preview_version;

	/**
	 * EXI stream version
	 */
	int16_t version_number;

	struct EXIOptions* opts;
};

typedef struct EXIheader EXIheader;

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
	 * The size of the buffer
	 */
	size_t bufLen;

	/**
	 * The size of the data stored in buffer - number of bytes
	 */
	size_t bufContent;

	/**
	 * Input stream used to fill the buffer when parsed
	 */
	IOStream* ioStrm;

	/**
	 * Current position in the buffer - bytewise
	 */
	size_t bufferIndx;

	/**
	 * Value between 0 and 7; shows the current position within the current byte
	 * 7 is the least significant bit position in the byte
	 */
	unsigned char bitPointer;

	/**
	 * EXI Header - the most important field is the EXI Options. They control the
	 * parsing and serialization of the stream.
	 */
	EXIheader header;

	/**
	 * The value string table
	 */
	ValueTable* vTable;

	/**
	 * The URI string table
	 */
	URITable* uriTable;

	/**
	 * The grammar stack used during processing
	 */
	EXIGrammarStack* gStack;

	/**
	 * The grammar pool of Element Grammars used during processing
	 */
	GrammarPool* ePool;

	/**
	 * The grammar pool of Type Grammars used during processing
	 */
	GrammarPool* tPool;

	/**
	 * Current (Left-hand side) Non terminal ID (Define the context/processor state)
	 */
	unsigned int nonTermID;

	/**
	 * Current position in the string tables
	 */
	struct StringTablesContext sContext;

	/**
	 * Stores the information of all the allocated memory for that stream
	 */
	AllocList memList;
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

	/**
	 * Specifies whether comments, pis, etc. are preserved - bit mask of booleans
	 * Use IS_PRESERVED macro to retrieve the values different preserve options
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
	uint32_t blockSize;

	/**
	 * Specifies the maximum string length of value content items to be considered for addition to the string table.
	 * 0 - unbounded
	 */
	uint32_t valueMaxLength;

	/**
	 * Specifies the total capacity of value partitions in a string table
	 * 0 - unbounded
	 */
	uint32_t valuePartitionCapacity;

	/**
	 * User defined meta-data may be added
	 */
	void* user_defined_data;
};

/**********************Function definitions************************/

/**
 * @brief Set the EXI options to their default values
 *
 * @param[in] strm EXI stream of bits
 * @return Error handling code
 */
errorCode makeDefaultOpts(struct EXIOptions* opts);

#endif /* PROCTYPES_H_ */
