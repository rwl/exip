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
#include <string.h>
#include "exipConfig.h"
#include <limits.h>

#define TRUE  1
#define FALSE 0

#ifndef NULL
#define NULL ((void *)0)
#endif

struct stackNode
{
	void* element;
	struct stackNode* nextInStack;
};

typedef struct stackNode GenericStack;

#define REVERSE_BIT_POSITION(p) (7 - p)

/**
 * EXI options related macros
 */
#define BIT_PACKED      0b00000000
#define BYTE_ALIGNMENT  0b01000000
#define PRE_COMPRESSION 0b10000000
#define ALIGNMENT       0b11000000

#define COMPRESSION     0b00000001
#define STRICT          0b00000010
#define FRAGMENT        0b00000100
#define SELF_CONTAINED  0b00001000

#define GET_ALIGNMENT(p)       (p & ALIGNMENT)
#define WITH_COMPRESSION(p)    ((p & COMPRESSION) != 0)
#define WITH_STRICT(p)         ((p & STRICT) != 0)
#define WITH_FRAGMENT(p)       ((p & FRAGMENT) != 0)
#define WITH_SELF_CONTAINED(p) ((p & SELF_CONTAINED) != 0)

#define SET_ALIGNMENT(p, align_const) (p = p | align_const)
#define SET_COMPRESSION(p)            (p = p | COMPRESSION)
#define SET_STRICT(p)                 (p = p | STRICT)
#define SET_FRAGMENT(p)               (p = p | FRAGMENT)
#define SET_SELF_CONTAINED(p)         (p = p | SELF_CONTAINED)

// SchemaID option modes (http://www.w3.org/TR/2011/REC-exi-20110310/#key-schemaIdOption):
// SCHEMA_ID_ABSENT - default,  no statement is made about the schema information
// SCHEMA_ID_SET - some sting identification of the schema is given
// SCHEMA_ID_NIL - no schema information is used for processing the EXI body (i.e. a schema-less EXI stream)
// SCHEMA_ID_EMPTY - no user defined schema information is used for processing the EXI body; however, the built-in XML schema types are available for use in the EXI body
#define SCHEMA_ID_ABSENT 0
#define SCHEMA_ID_SET    1
#define SCHEMA_ID_NIL    2
#define SCHEMA_ID_EMPTY  3

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
#define SET_PRESERVED(p, preserve_const) (p = p | preserve_const)

// #DOCUMENT# If there is a possibility that a document defines more than 4 prefixes per namespace i.e. something insane, this should be increased
// Note that will require many changes - for example statically generated grammars from XML schemas needs to be rebuilt
#ifndef MAXIMUM_NUMBER_OF_PREFIXES_PER_URI
# define MAXIMUM_NUMBER_OF_PREFIXES_PER_URI 4
#endif

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

#ifndef EXIP_UNSIGNED_INTEGER
# define EXIP_UNSIGNED_INTEGER uint64_t
#endif

typedef EXIP_UNSIGNED_INTEGER UnsignedInteger;

#ifndef EXIP_INTEGER
# define EXIP_INTEGER int64_t
#endif

typedef EXIP_INTEGER Integer;

/**
 * The EXI Float datatype representation is two consecutive Integers (see 7.1.5 Integer).
 * The first Integer represents the mantissa of the floating point number and the second
 * Integer represents the base-10 exponent of the floating point number.
 * The range of the mantissa is - (263) to 263-1 and the range of the exponent is - (214-1) to 214-1.
 * The exponent value -(214) is used to indicate one of the special values: infinity,
 * negative infinity and not-a-number (NaN). An exponent value -(214) with mantissa
 * values 1 and -1 represents positive infinity (INF) and negative infinity (-INF) respectively.
 * An exponent value -(214) with any other mantissa value represents NaN.
 */
struct EXIFloat
{
	int64_t mantissa;
	int16_t exponent;
};

#ifndef EXIP_FLOAT
# define EXIP_FLOAT struct EXIFloat
#endif

typedef EXIP_FLOAT Float;

/*
 * Used for the content handler interface for decimal values
 * Application which require support for different type of decimal values can
 * override this macro.
 * Refs: http://gcc.gnu.org/onlinedocs/gcc/Decimal-Float.html#Decimal-Float
 * http://speleotrove.com/decimal/
 */
#ifndef EXIP_DECIMAL
# define EXIP_DECIMAL _Decimal64
#endif

typedef EXIP_DECIMAL Decimal;

/**
 * Defines the encoding used for characters.
 * It is dependent on the implementation of the stringManipulate.h functions
 */
#ifndef CHAR_TYPE  // #DOCUMENT#
# define CHAR_TYPE char
#endif
// TODO: document this macro - it must be set during source build to overwrite the default behavior

typedef CHAR_TYPE CharType;

struct StringType
{
	CharType* str;
	size_t length;
};

typedef struct StringType String;

/********* START: Memory management definitions ***************/

#define ALLOCATION_ARRAY_SIZE 100 // #DOCUMENT#

struct allocBlock {
	void* allocation[ALLOCATION_ARRAY_SIZE];
	unsigned int currentAlloc;
	struct allocBlock* nextBlock;
};

struct allocList {
	struct allocBlock* firstBlock;
	struct allocBlock* lastBlock;
};

typedef struct allocList AllocList;

// Used by the memoryManager when there is reallocation
struct reAllocPair {
	struct allocBlock* memBlock;
	unsigned int allocIndx;
};

/********* END: Memory management definitions ***************/


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
 * 1 - there is no value content for the event
 * 2 - the type is String
 * 3 - Float
 * 4 - Decimal
 * 5 - Date-Time
 * 6 - Boolean
 * 7 - Binary
 * 8 - List
 * 9 - QName
 * 10 - Untyped
 * 20 - Integer
 * 21 - Small int
 * 22 - Unsigned int
 * */
typedef unsigned char EXIType;

#define VALUE_TYPE_NONE              1
#define VALUE_TYPE_STRING            2
#define VALUE_TYPE_FLOAT             3
#define VALUE_TYPE_DECIMAL           4
#define VALUE_TYPE_DATE_TIME         5
#define VALUE_TYPE_BOOLEAN           6
#define VALUE_TYPE_BINARY            7
#define VALUE_TYPE_LIST              8
#define VALUE_TYPE_QNAME             9
#define VALUE_TYPE_UNTYPED          10

#define VALUE_TYPE_INTEGER          20
#define VALUE_TYPE_SMALL_INTEGER    21
#define VALUE_TYPE_NON_NEGATIVE_INT 22

struct ValueType
{
	EXIType exiType;
	uint16_t simpleTypeID; // An index of the simple type in the schema SimpleTypeArray if any, UINT16_MAX otherwise
};

typedef struct ValueType ValueType;

struct EXIEvent
{
	EventType eventType;
	ValueType valueType;
};

typedef struct EXIEvent EXIEvent;

struct Production
{
	EXIEvent event;
	/**
	 * For SE(qname), SE(uri:*), AT(qname) and AT(uri:*). Points to the qname or its local name
	 * of the element/attribute
	 */
	uint16_t uriRowID; // VOID == UINT16_MAX
	size_t lnRowID; // VOID == SIZE_MAX

	size_t nonTermID; // unique identifier of right-hand side Non-terminal
};

typedef struct Production Production;

// Define Built-in Grammars non-terminals
#define GR_VOID_NON_TERMINAL SIZE_MAX // Used to indicate that the production does not have NON_TERMINAL on the right-hand side

#define GR_DOCUMENT          0
#define GR_DOC_CONTENT       1
#define GR_DOC_END           2

#define GR_START_TAG_CONTENT 0
#define GR_ELEMENT_CONTENT   1

#define GR_FRAGMENT          0
#define GR_FRAGMENT_CONTENT  1

struct GrammarRule
{
	Production* prodArrays[3]; // 3 arrays of grammar productions that have event codes of length 1 (array prodArrays[0]), 2 (prodArrays[1]) and 3 (prodArrays[2])
	size_t prodCounts[3]; // The number of productions in the prodArrays
	unsigned char bits[3]; // The number of bits used for the integers constituting the EventCode
};

typedef struct GrammarRule GrammarRule;

/**
 * Extension to the GrammarRule. In the DynGrammarRule the first production array i.e. the one holding the
 * productions with event code with length 1 (prodArrays[0]) is dynamic array.
 * The dynamic GrammarRule is used for Built-in Element Grammar and Built-in Fragment Grammar
 */
struct DynGrammarRule
{
	Production* prodArrays[3]; // 3 arrays of grammar productions that have event codes of length 1 (Dynamic array prodArrays[0]), 2 (prodArrays[1]) and 3 (prodArrays[2])
	size_t prodCounts[3]; // The number of productions in the prodArrays
	unsigned char bits[3]; // The number of bits used for the integers constituting the EventCode

	// Additional fields
	size_t prod1Dimension; // The size of the prodArrays[0] Dynamic production array /allocated space for Productions in it/
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation for prodArrays[0]
};

typedef struct DynGrammarRule DynGrammarRule;

#define GR_TYPE_BUILD_IN_DOC       0
#define GR_TYPE_BUILD_IN_FRAG      1
#define GR_TYPE_BUILD_IN_ELEM      2

#define GR_TYPE_SCHEMA_DOC        10
#define GR_TYPE_SCHEMA_FRAG       11
#define GR_TYPE_SCHEMA_ELEM_FRAG  12
#define GR_TYPE_SCHEMA_ELEM       13
#define GR_TYPE_SCHEMA_TYPE       14
#define GR_TYPE_SCHEMA_EMPTY_TYPE 15

/**
 * The rule index in the ruleArray is the left hand side nonTermID of the particular grammar Rule
 */
struct EXIGrammar
{
	GrammarRule* ruleArray; // Array of grammar rules which constitute that grammar
	size_t rulesDimension; // The size of the array
	unsigned char grammarType;
	unsigned char isNillable;
	size_t contentIndex;
};

typedef struct EXIGrammar EXIGrammar;

struct GrammarStackNode
{
	EXIGrammar* grammar;
	size_t lastNonTermID; // Stores the last NonTermID before another grammar is added on top of the stack
	struct GrammarStackNode* nextInStack;
};

typedef struct GrammarStackNode EXIGrammarStack;

/*********** END: Grammar Types ***************/


/********* BEGIN: String Table Types ***************/

struct ValueRow {
	String string_val;
	size_t* valueLocalCrossTableRowPointer;
};

struct ValueTable {
	struct ValueRow* rows; // Dynamic array
	size_t rowCount; // The number of rows
	size_t arrayDimension; // The size of the Dynamic array
	size_t globalID; // http://www.w3.org/TR/2011/REC-exi-20110310/#key-globalID
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation

	// #DOCUMENT#
	// Hashtable for fast look-up of global values in the table.
	// Only used when:
	// serializing &&
	// valuePartitionCapacity > 50  &&   //for small table full-scan will work better
	// valueMaxLength > 0 // this is essentially equal to valuePartitionCapacity == 0
	struct hashtable *hashTbl;
};

typedef struct ValueTable ValueTable;

struct ValueLocalCrossTable {
	size_t* valueRowIds; // Dynamic array; If the value of an element is SIZE_MAX, then this compact identifier is permanently unassigned
	uint16_t rowCount; // The number of rows
	uint16_t arrayDimension; // The size of the Dynamic array
	struct reAllocPair memPair; // Used by the memoryManager when there is reallocation
};

typedef struct ValueLocalCrossTable ValueLocalCrossTable;

struct PrefixTable {
	String string_val[MAXIMUM_NUMBER_OF_PREFIXES_PER_URI];
	unsigned char rowCount; // The number of rows
};

typedef struct PrefixTable PrefixTable;

struct LocalNamesRow {
	ValueLocalCrossTable* vCrossTable;
	String string_val;
	EXIGrammar* typeGrammar;
	EXIGrammar* emptyTypeGrammar;
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
	String string_val;
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
	const String* uri;       // Pointer to a String value in the string table. It is not allowed to modify the string table content from this pointer.
	const String* localName; // Pointer to a String value in the string table. It is not allowed to modify the string table content from this pointer.
};

typedef struct QName QName;

struct QNameID {
	uint16_t uriRowId;
	size_t lnRowId;
};

typedef struct QNameID QNameID;

// Constraining Facets IDs. Used for fine-grained schema validation
#define TYPE_FACET_LENGTH             0b0000000000000001
#define TYPE_FACET_MIN_LENGTH         0b0000000000000010
#define TYPE_FACET_MAX_LENGTH         0b0000000000000100
#define TYPE_FACET_PATTERN            0b0000000000001000
#define TYPE_FACET_ENUMERATION        0b0000000000010000
#define TYPE_FACET_WHITE_SPACE        0b0000000000100000
#define TYPE_FACET_MAX_INCLUSIVE      0b0000000001000000
#define TYPE_FACET_MAX_EXCLUSIVE      0b0000000010000000
#define TYPE_FACET_MIN_EXCLUSIVE      0b0000000100000000
#define TYPE_FACET_MIN_INCLUSIVE      0b0000001000000000
#define TYPE_FACET_TOTAL_DIGITS       0b0000010000000000
#define TYPE_FACET_FRACTION_DIGITS    0b0000100000000000
#define TYPE_FACET_NAMED_SUBTYPE      0b0001000000000000
#define TYPE_FACET_SIMPLE_UNION_TYPE  0b0010000000000000

struct SimpleType {
	uint16_t facetPresenceMask;
	int maxInclusive;
	int minInclusive;
	unsigned int maxLength;
};

typedef struct SimpleType SimpleType;

#define SIMPLE_TYPE_STRING                0
#define SIMPLE_TYPE_NORMALIZED_STRING     1
#define SIMPLE_TYPE_TOKEN                 2
#define SIMPLE_TYPE_NMTOKEN               3
#define SIMPLE_TYPE_NAME                  4
#define SIMPLE_TYPE_LANGUAGE              5
#define SIMPLE_TYPE_NCNAME                6
#define SIMPLE_TYPE_IDREF                 7
#define SIMPLE_TYPE_IDREFS                8
#define SIMPLE_TYPE_ENTITY                9
#define SIMPLE_TYPE_ENTITIES             10
#define SIMPLE_TYPE_ID                   11
#define SIMPLE_TYPE_DECIMAL              12
#define SIMPLE_TYPE_INTEGER              13
#define SIMPLE_TYPE_NON_POSITIVE_INTEGER 14
#define SIMPLE_TYPE_NEGATIVE_INTEGER     15
#define SIMPLE_TYPE_LONG                 16
#define SIMPLE_TYPE_INT                  17
#define SIMPLE_TYPE_SHORT                18
#define SIMPLE_TYPE_BYTE                 19
#define SIMPLE_TYPE_NON_NEGATIVE_INTEGER 20
#define SIMPLE_TYPE_UNSIGNED_LONG        21
#define SIMPLE_TYPE_UNSIGNED_INT         22
#define SIMPLE_TYPE_UNSIGNED_SHORT       23
#define SIMPLE_TYPE_UNSIGNED_BYTE        24
#define SIMPLE_TYPE_POSITIVE_INTEGER     25
#define SIMPLE_TYPE_BOOLEAN              26
#define SIMPLE_TYPE_BASE64_BINARY        27
#define SIMPLE_TYPE_HEX_BINARY           28
#define SIMPLE_TYPE_FLOAT                29
#define SIMPLE_TYPE_DOUBLE               30
#define SIMPLE_TYPE_ANY_URI              31
#define SIMPLE_TYPE_QNAME                32
#define SIMPLE_TYPE_NOTATION             33
#define SIMPLE_TYPE_DURATION             34
#define SIMPLE_TYPE_DATE_TIME            35
#define SIMPLE_TYPE_TIME                 36
#define SIMPLE_TYPE_DATE                 37
#define SIMPLE_TYPE_GYEAR_MONTH          38
#define SIMPLE_TYPE_GYEAR                39
#define SIMPLE_TYPE_GMONTH_DAY           40
#define SIMPLE_TYPE_GDAY                 41
#define SIMPLE_TYPE_GMONTH               42
#define SIMPLE_TYPE_ANY_SIMPLE_TYPE      43
#define SIMPLE_TYPE_ANY_TYPE             44

#define SIMPLE_TYPE_COUNT                45

struct EXIPSchema
{
	URITable* initialStringTables;
	QNameID* globalElemGrammars;  // Sorted
	unsigned int globalElemGrammarsCount;
	SimpleType* simpleTypeArray;
	uint16_t sTypeArraySize;
	unsigned char isAugmented;
	unsigned char isStatic;

	AllocList memList; // Stores the information for all memory allocations for that schema
};

typedef struct EXIPSchema EXIPSchema;

struct StreamContext
{
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
	 * Current (Left-hand side) Non terminal ID (Define the context/processor state)
	 */
	size_t nonTermID;

	uint16_t curr_uriID;
	size_t curr_lnID;
	unsigned char expectATData; // 0 - FALSE, otherwise the ValueType of the expected data
};

typedef struct StreamContext StreamContext;

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

struct DatatypeRepresentationMap
{
	void* TODO; //TODO: fill in the information for this structure
};

typedef struct DatatypeRepresentationMap DatatypeRepresentationMap;

struct EXIOptions
{
	/**
	 * Use the macros GET_ALIGNMENT(p), WITH_COMPRESSION(p), WITH_STRICT,
	 * WITH_FRAGMENT(p), WITH_SELF_CONTAINED(p) to extract the options:
	 * alignment, compression, strict, fragment and selfContained
	 */
	unsigned char enumOpt;

	/**
	 * Specifies whether comments, pis, etc. are preserved - bit mask of booleans
	 * Use IS_PRESERVED macro to retrieve the values different preserve options
	 */
	unsigned char preserve;

	/**
	 * Identify the schema information, if any, used to encode the body
	 */
	String schemaID;

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
	 * SIZE_MAX - unbounded
	 */
	size_t valueMaxLength;

	/**
	 * Specifies the total capacity of value partitions in a string table
	 * SIZE_MAX - unbounded
	 */
	size_t valuePartitionCapacity;

	/**
	 * User defined meta-data may be added
	 */
	void* user_defined_data;
};

typedef struct EXIOptions EXIOptions;

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

	EXIOptions opts;
};

typedef struct EXIheader EXIheader;

/**
 * Represents an EXI stream
 */
struct EXIStream
{
	/**
	 * Read/write buffer
	 */
	char* buffer;

	/**
	 * The size of the buffer
	 */
	size_t bufLen;

	/**
	 * The size of the data stored in the buffer - number of bytes
	 */
	size_t bufContent;

	/**
	 * Input/Output Stream used to fill/flush the buffer when parsed
	 */
	IOStream ioStrm;

	/**
	 * EXI Header - the most important field is the EXI Options. They control the
	 * parsing and serialization of the stream.
	 */
	EXIheader header;

	/** Holds the current state of the stream*/
	StreamContext context;

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
	 * Stores the information of all the allocated memory for that stream
	 */
	AllocList memList;

	/**
	 * Schema information for that stream if any; NULL otherwise
	 */
	EXIPSchema* schema;
};

typedef struct EXIStream EXIStream;


/**********************Function definitions************************/

/**
 * @brief Set the EXI options to their default values
 *
 * @param[in, out] opts EXI options structure
 */
void makeDefaultOpts(EXIOptions* opts);


errorCode pushOnStack(GenericStack** stack, void* element);

void popFromStack(GenericStack** stack, void** element);

errorCode pushOnStackPersistent(GenericStack** stack, void* element, AllocList* memList);

void popFromStackPersistent(GenericStack** stack, void** element);

#endif /* PROCTYPES_H_ */
