/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file procTypes.h
 * @brief Common structure types used throughout the project
 *
 * @date Jul 7, 2010
 * @author Rumen Kyusakov
 * @version 0.4
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

/** An item in a Generic Stack structure */
struct stackNode
{
	void* item;
	struct stackNode* nextInStack;
};

typedef struct stackNode GenericStack;

/**
 * @def REVERSE_BIT_POSITION(p)
 * Given a bit position inside an octet (0-7)
 * returns the reversed position index.
 */
#define REVERSE_BIT_POSITION(p) (7 - p)

/**
 * @name EXI options related macros
 * Example usage:
 * @code
 *   	SET_ALIGNMENT(enumOpt, BYTE_ALIGNMENT)
 * @endcode
 */

/**@{*/
#define BIT_PACKED      0x00 // 0b00000000
#define BYTE_ALIGNMENT  0x40 // 0b01000000
#define PRE_COMPRESSION 0x80 // 0b10000000
#define ALIGNMENT       0xc0 // 0b11000000

#define COMPRESSION     0x01 // 0b00000001
#define STRICT          0x02 // 0b00000010
#define FRAGMENT        0x04 // 0b00000100
#define SELF_CONTAINED  0x08 // 0b00001000

#define GET_ALIGNMENT(p)       ((p) & ALIGNMENT)
#define WITH_COMPRESSION(p)    (((p) & COMPRESSION) != 0)
#define WITH_STRICT(p)         (((p) & STRICT) != 0)
#define WITH_FRAGMENT(p)       (((p) & FRAGMENT) != 0)
#define WITH_SELF_CONTAINED(p) (((p) & SELF_CONTAINED) != 0)

#define SET_ALIGNMENT(p, align_const) ((p) = (p) | align_const)
#define SET_COMPRESSION(p)            ((p) = (p) | COMPRESSION)
#define SET_STRICT(p)                 ((p) = (p) | STRICT)
#define SET_FRAGMENT(p)               ((p) = (p) | FRAGMENT)
#define SET_SELF_CONTAINED(p)         ((p) = (p) | SELF_CONTAINED)

/**@}*/

/**
 * @name Handling of SchemaID header field
 *
 * SchemaID option modes:
 * @def SCHEMA_ID_ABSENT
 * 		default,  no statement is made about the schema information
 * @def SCHEMA_ID_SET
 * 		some sting identification of the schema is given
 * @def SCHEMA_ID_NIL
 * 		no schema information is used for processing the EXI body (i.e. a schema-less EXI stream)
 * @def SCHEMA_ID_EMPTY
 * 		no user defined schema information is used for processing the EXI body; however, the built-in XML schema types are available for use in the EXI body
 *
 * @see http://www.w3.org/TR/2011/REC-exi-20110310/#key-schemaIdOption
 */
/**@{*/
#define SCHEMA_ID_ABSENT 0
#define SCHEMA_ID_SET    1
#define SCHEMA_ID_NIL    2
#define SCHEMA_ID_EMPTY  3
/**@}*/

/**
 *	@name Fidelity options handling
 *
 *	Example usage:
 *	@code
 *	  SET_PRESERVED(preserve, PRESERVE_PREFIXES)
 *	@endcode
 *
 *	@see http://www.w3.org/TR/2011/REC-exi-20110310/#fidelityOptions
 **/

/**@{*/
#define PRESERVE_COMMENTS  0x01 // 0b00000001
#define PRESERVE_PIS       0x02 // 0b00000010
#define PRESERVE_DTD       0x04 // 0b00000100
#define PRESERVE_PREFIXES  0x08 // 0b00001000
#define PRESERVE_LEXVALUES 0x10 // 0b00010000

#define IS_PRESERVED(p, mask) (((p) & (mask)) != 0)
#define SET_PRESERVED(p, preserve_const) ((p) = (p) | (preserve_const))
/**@}*/

/**
 * The maximum allowed prefixes per namespace.
 * If there is a possibility that a document defines more than 4 prefixes per namespace
 * i.e. weird XML, MAXIMUM_NUMBER_OF_PREFIXES_PER_URI should be increased
 * @note This will require many changes - for example statically generated grammars from XML schemas needs to be rebuilt
 */
#ifndef MAXIMUM_NUMBER_OF_PREFIXES_PER_URI
# define MAXIMUM_NUMBER_OF_PREFIXES_PER_URI 4
#endif

/**
 * Fractional seconds = value * 10^-(offset+1) seconds;
 * Example:
 * offset = 4
 * value = 123
 * Fractional seconds = 123×10^−5 = 0.00123 seconds = 1.23 milliseconds
 * @note (offset+1) must be greater or equal than the number of digits in value
 */
struct fractionalSecs
{
	unsigned char offset;
	unsigned int value;
};

typedef struct fractionalSecs FractionalSecs;

/** DateTime type in EXIP.
 * Contains a standard "struct tm" combined with
 * fractional seconds and presence mask. */
struct EXIPDateTime
{
	/**
	 * As defined in time.h
	 */
	struct tm dateTime;
	FractionalSecs fSecs;

	/**
	 * Defines which fields of the DateTime are included.
	 * Use SEC_PRESENCE, MIN_PRESENCE etc. (*_PRESENCE) masks
	 * and IS_PRESENT() macro
	 */
	uint16_t presenceMask;
};

typedef struct EXIPDateTime EXIPDateTime;

/**
 * @name Presence of fields in the EXIPDateTime type
 *
 * Example usage:
 * @code
 *   IS_PRESENT(presenceMask, YEAR_PRESENCE)
 * @endcode
 */
/**@{*/
#define SEC_PRESENCE       0x0001 // 0b0000000000000001
#define MIN_PRESENCE       0x0002 // 0b0000000000000010
#define HOUR_PRESENCE      0x0004 // 0b0000000000000100
#define MDAY_PRESENCE      0x0008 // 0b0000000000001000
#define MON_PRESENCE       0x0010 // 0b0000000000010000
#define YEAR_PRESENCE      0x0020 // 0b0000000000100000
#define WDAY_PRESENCE      0x0040 // 0b0000000001000000
#define YDAY_PRESENCE      0x0080 // 0b0000000010000000
#define DST_PRESENCE       0x0100 // 0b0000000100000000
#define TZONE_PRESENCE     0x0200 // 0b0000001000000000
#define FRACT_PRESENCE     0x0400 // 0b0000010000000000

#define IS_PRESENT(p, mask) (((p) & (mask)) != 0)
/**@}*/

#ifndef EXIP_UNSIGNED_INTEGER
# define EXIP_UNSIGNED_INTEGER uint64_t
#endif

typedef EXIP_UNSIGNED_INTEGER UnsignedInteger;

#ifndef EXIP_INTEGER
# define EXIP_INTEGER int64_t
#endif

typedef EXIP_INTEGER Integer;

/**
 * The default Float representation in EXIP.
 * Maps directly to the EXI Float datatype
 *
 * @see http://www.w3.org/TR/2011/REC-exi-20110310/#encodingFloat
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

/**
 * Used for the content handler interface for decimal values.
 * Application which require support for different type of decimal values can
 * override this macro.
 * @see http://gcc.gnu.org/onlinedocs/gcc/Decimal-Float.html#Decimal-Float
 * @see http://speleotrove.com/decimal/
 */
#ifndef EXIP_DECIMAL
# define EXIP_DECIMAL _Decimal64
#endif

typedef EXIP_DECIMAL Decimal;

#ifndef EXIP_INDEX
# define EXIP_INDEX size_t
#endif

typedef EXIP_INDEX Index;

#ifndef EXIP_INDEX_MAX
# define EXIP_INDEX_MAX SIZE_MAX
#endif

#define INDEX_MAX EXIP_INDEX_MAX

#ifndef EXIP_SMALL_INDEX
# define EXIP_SMALL_INDEX size_t
#endif

typedef EXIP_SMALL_INDEX SmallIndex;

#ifndef EXIP_SMALL_INDEX_MAX
# define EXIP_SMALL_INDEX_MAX SIZE_MAX
#endif

#define SMALL_INDEX_MAX EXIP_SMALL_INDEX_MAX

/**
 * Defines the encoding used for characters.
 * It is dependent on the implementation of the stringManipulate.h functions
 * The default is ASCII characters (ASCII_stringManipulate.c)
 */
#ifndef CHAR_TYPE
# define CHAR_TYPE char
#endif

typedef CHAR_TYPE CharType;


#ifndef EXIP_STRTOLL
/** strtoll() function */
# define EXIP_STRTOLL strtoll
#endif

/**
 * Represents the length prefixed strings in EXIP
 */
struct StringType
{
	CharType* str;
	Index length;
};

typedef struct StringType String;

/**
 * Represent a fully qualified name
 */
struct QName {
	/**
	 * Pointer to a String value in the URI string table.
	 * @note It is not allowed to modify the string table content from this pointer.*/
	const String* uri;
	/**
	 * Pointer to a String value in the LN sting table.
	 * @note It is not allowed to modify the string table content from this pointer.
	 */
	const String* localName;
	/**
	 * Pointer to a String value in the Prefix string table if [Preserve prefixes] is TRUE and NULL otherwise.
	 * @note It is not allowed to modify the string table content from this pointer.
	 */
	const String* prefix;
};

typedef struct QName QName;

#define URI_MAX SMALL_INDEX_MAX
#define LN_MAX INDEX_MAX

/**
 * Position of a qname in the string tables
 */
struct QNameID {
	SmallIndex uriId;	// VOID == URI_MAX
	Index lnId;	// VOID == LN_MAX
};

typedef struct QNameID QNameID;

/**
 * @name Memory management definitions
 */
/**@{*/

/** @note Should not be bigger than SMALL_INDEX_MAX - 1 */
#define ALLOCATION_ARRAY_SIZE 100

/** An array of allocation pointers */
struct allocBlock {
	void* allocation[ALLOCATION_ARRAY_SIZE];
	struct allocBlock* nextBlock;
};

/** A list of allocation blocks.
 * Pass to initAllocList() before use. */
struct allocList {
	struct allocBlock* firstBlock;
	struct allocBlock* lastBlock;
	/** The next allocation slot in the allocation array of the lastBlock */
	SmallIndex currAllocSlot;
};

typedef struct allocList AllocList;

/**
 * Meta-data of generic dynamic array.
 * A concrete dynamic array is defined as follow:
 * @code
 *   struct [ConcreteDynamicArray]
 *   {
 *     DynArray dynArray;
 *     [ArrayEntryType]* base;
 *     Index count;
 *   };
 * @endcode
 */
struct dynArray {
	/**
	 * The size of an entry in bytes
	 */
	size_t entrySize;

	/**
	 * The initial size of the dynamic array (in number of entries), 
	 * also the chunk of number of entries to be added each expansion time
	 */
	uint16_t chunkEntries;

	/**
	 * The total number of entries in the array
	 */
	uint16_t arrayEntries; 
};

typedef struct dynArray DynArray;

/**@}*/ //End Memory management definitions


/**
 * @name Grammar Types
 */
/**@{*/

/**
 * @page grammars Representing the EXI grammars in EXIP
 *
 * For a sample grammar G:
 * @code
 *   NonTerminal_0 :
 * 					Terminal_1    NonTerminal_0	0
 * 					Terminal_2    NonTerminal_1	1.0
 * 					Terminal_3    NonTerminal_1	1.1.0
 *
 * 	 NonTerminal_1 :
 * 					Terminal_4    NonTerminal_1	0
 * 					Terminal_5    NonTerminal_1	1
 * 					EE
 * @endcode
 *
 * A grammar production is for example "NonTerminal_0 : Terminal_1    NonTerminal_0	0".
 * All grammar productions with the same left-hand side define
 * a grammar rule.
 * Therefore, the grammar G has 2 grammar rules and 6 grammar productions.
 * <p>
 *   A single grammar production is represented by the struct Production.
 *   The [Terminal] is represented by the event type, the type of the value
 *   content if any and a fully qualified name.
 *   The [NonTerminal] is a non-negative integer value.
 *   The last part of the production (the event code) is implicitly encoded
 *   as a position of the production in the grammar rule.
 * </p>
 *
 * @section e_codes Event codes representation
 * The event codes in a production are represented as follow: <br/>
 * For every grammar rule there are 3 arrays of grammar productions -
 * represented with GrammarRulePart type.
 * The first one is for productions with event codes with length 1 -
 * they have only one part.
 * The second one is for productions with events codes with length 2 and
 * the third for productions with three-part event codes.
 * The last production in a production array has event code
 * 0, the production before it 1 etc. - that is in reverse order.<br/>
 * For example, the third production in the first rule of G with event code
 * 1.1.0 is the only production in the GrammarRulePart for 3 parts
 * productions so the last part is 0.
 * The number of productions in the second GrammarRulePart is also 1
 * so the second part is 1.
 * The number of productions in the first GrammarRulePart is also 1
 * so the first part is 1.
 */

/**
 * Event types that occur in an EXI stream. This is moved from the grammar module because 
 * of the added "Void" element.
 * 
 * <table>
 * <tr><th>Name</th><th>Notation</th><th>Value</th></tr>
 * <tr><td>Start Document</td><td>SD         </td><td> 0</td></tr>
 * <tr><td>End Document  </td><td>ED         </td><td> 1</td></tr>
 * <tr><td>Start Element </td><td>SE( qname )</td><td> 5</td></tr>
 * <tr><td>Start Element </td><td>SE( uri:* )</td><td> 6</td></tr>
 * <tr><td>Start Element </td><td>SE( * )    </td><td> 7</td></tr>
 * <tr><td>End Element   </td><td>EE         </td><td> 8</td></tr>
 * <tr><td>Attribute     </td><td>AT( qname )</td><td> 2</td></tr>
 * <tr><td>Attribute     </td><td>AT( uri:* )</td><td> 3</td></tr>
 * <tr><td>Attribute     </td><td>AT( * )    </td><td> 4</td></tr>
 * <tr><td>Characters    </td><td>CH         </td><td> 9</td></tr>
 * <tr><td>Namespace Decl</td><td>NS         </td><td>10</td></tr>
 * <tr><td>Comment       </td><td>CM         </td><td>11</td></tr>
 * <tr><td>Processng Inst</td><td>PI         </td><td>12</td></tr>
 * <tr><td>DOCTYPE       </td><td>DT         </td><td>13</td></tr>
 * <tr><td>Entity Refrnce</td><td>ER         </td><td>14</td></tr>
 * <tr><td>Self-contained</td><td>SC         </td><td>15</td></tr>
 * <tr><td>Void          </td><td>--         </td><td>16</td></tr>
 * </table>
 */
enum EventType
{
	EVENT_SD       =0,
	EVENT_ED       =1,
	EVENT_SE_QNAME =5,
	EVENT_SE_URI   =6,
	EVENT_SE_ALL   =7,
	EVENT_EE       =8,
	EVENT_AT_QNAME =2,
	EVENT_AT_URI   =3,
	EVENT_AT_ALL   =4,
	EVENT_CH       =9,
	EVENT_NS      =10,
	EVENT_CM      =11,
	EVENT_PI      =12,
	EVENT_DT      =13,
	EVENT_ER      =14,
	EVENT_SC      =15,
	/** Indicates lack of Terminal symbol in proto-grammars */
	EVENT_VOID    =16
};

#ifdef EXIP_SIZE_OPTIMIZE
typedef uint8_t EventType;
#else
// This is better for debugging as it may allow display of the enum type string
typedef enum EventType EventType;
#endif

/** 
 * This is the type of the "value" content of EXI events. It is used when schema is available.
 */
enum EXIType
{
	VALUE_TYPE_NONE              =1,
	VALUE_TYPE_STRING            =2,
	VALUE_TYPE_FLOAT             =3,
	VALUE_TYPE_DECIMAL           =4,
	VALUE_TYPE_DATE_TIME         =5,
	VALUE_TYPE_BOOLEAN           =6,
	VALUE_TYPE_BINARY            =7,
	VALUE_TYPE_LIST              =8,
	VALUE_TYPE_QNAME             =9,
	VALUE_TYPE_UNTYPED          =10,

	VALUE_TYPE_INTEGER          =20,
	VALUE_TYPE_SMALL_INTEGER    =21,
	VALUE_TYPE_NON_NEGATIVE_INT =22
};

#ifdef EXIP_SIZE_OPTIMIZE
typedef uint8_t EXIType;
#else
// This is better for debugging as it may allow display of the enum type string
typedef enum EXIType EXIType;
#endif

/** Specifies the right-hand side for a production rule.
 * The left-hand side is implicitly defined by the
 * parent grammar rule. */
struct Production
{
	/** EXI event type, like SD, CH, or EE. */
	EventType eventType;

	/**
	 * Index of the grammar for the production event, if applicable.
	 * <ul>
	 *   <li> AT, CH: index of simple type, in EXIPSchema's simpleTypeTable </li>
	 *   <li> SE(qname): index of EXI grammar, in EXIPSchema's grammarTable.
	 *   Processes the content of the SE(qname) event.</li>
	 *   <li> Otherwise: INDEX_MAX </li>
	 * </ul>
	 */
	Index typeId;
	
	/**
	 * Identifies the terminal portion, the element/attribute for SE(qname), 
     * SE(uri:*), AT(qname) or AT(uri:*).
	 */
	QNameID qnameId;
	
	/** 
     * Unique identifier of right-hand side non-terminal.
     */
	SmallIndex nonTermID;
};

typedef struct Production Production;

/**
 * @name Built-in Grammars non-terminals
 */
/**@{*/
/**
 * Used to indicate that the production does not have NON_TERMINAL on the right-hand side
 */
#define GR_VOID_NON_TERMINAL SMALL_INDEX_MAX

#define GR_DOCUMENT          0
#define GR_DOC_CONTENT       1
#define GR_DOC_END           2

#define GR_START_TAG_CONTENT 0
#define GR_ELEMENT_CONTENT   1

#define GR_FRAGMENT          0
#define GR_FRAGMENT_CONTENT  1
/**@}*/

/**
 * A collection of production rules with a common event code length.
 */
struct GrammarRulePart
{
     /**
      * Array of grammar productions
      */
     Production* prod;
     /**
      * The number of productions in prod
      */
     Index count;
     /**
      * The number of bits used for the event code enumerations, where 
      * count <= 2 ^ bits
      */
     unsigned char bits;
};

typedef struct GrammarRulePart GrammarRulePart;

/** 
 * A container for production rules for a particular left-hand side non-terminal. 
 * Organizes the rules by event code length.
 */
struct GrammarRule
{
	/**
	 * An 'inner' container for production rules, grouped by a rule's event code length, 
     * with a range of 1 to 3 (index 0 to 2). 
	 */
     GrammarRulePart part[3];
};
typedef struct GrammarRule GrammarRule;

/**
 * Extension to the GrammarRule. In the DynGrammarRule the first production array i.e. the one holding the
 * productions with event code with length 1 (prods[0]) is dynamic array.
 * The dynamic GrammarRule is used for Built-in Element Grammar and Built-in Fragment Grammar
 */
struct DynGrammarRule
{
	/**
	 * Each part includes all productions with a specific event code length.
	 * There are three elements, for event code lengths of 1, 2 and 3.
	 */
    GrammarRulePart part[3];

	// Additional fields
    Index part0Dimension; // The size of the part[0] Dynamic production array /allocated space for Productions in it/
};
typedef struct DynGrammarRule DynGrammarRule;

/**
 * @name EXIGrammar properties handling
 */
/**@{*/
#define IS_NILLABLE(p) 			        ((p & GR_PROP_NILLABLE) != 0)
#define IS_AUGMENTED(p) 		        ((p & GR_PROP_AUGMENTED) != 0)
#define IS_BUILT_IN_ELEM(p) 	        ((p & GR_PROP_TYPE_BUILT_IN_ELEMENT) != 0)
#define IS_SCHEMA(p) 			        ((p & GR_PROP_TYPE_SCHEMA) != 0)
#define HAS_NAMED_SUB_TYPE_OR_UNION(p) 	((p & GR_PROP_NAMED_SUB_TYPE_OR_UNION) != 0)

#define SET_NILLABLE(p)    	            ((p) = (p) | GR_PROP_NILLABLE)
#define SET_AUGMENTED(p)    	        ((p) = (p) | GR_PROP_AUGMENTED)
#define SET_BUILT_IN_ELEM(p)            ((p) = (p) | GR_PROP_TYPE_BUILT_IN_ELEMENT)
#define SET_SCHEMA(p)    		        ((p) = (p) | GR_PROP_TYPE_SCHEMA)
#define SET_NAMED_SUB_TYPE_OR_UNION(p)  ((p) = (p) | GR_PROP_NAMED_SUB_TYPE_OR_UNION)

#define GR_PROP_TYPE_BUILT_IN_ELEMENT    0x01 // 0b00000001
#define GR_PROP_TYPE_SCHEMA              0x02 // 0b00000010
#define GR_PROP_NILLABLE                 0x04 // 0b00000100
#define GR_PROP_AUGMENTED                0x08 // 0b00001000
#define GR_PROP_NAMED_SUB_TYPE_OR_UNION  0x10 // 0b00010000
/**@}*/

/**
 * An EXI regular grammar, organized as a collection of containers for production rules, 
 * and grouped by each rule's left-hand side non-terminal.
 */
struct EXIGrammar
{
	/** 
     * Rule container array, where the index of a container is its left-hand side non-terminal ID.
     */
	GrammarRule* rule;
	/** Size of the array */
	SmallIndex count;
	/**
	 * Bitmap of properties.
	 * Use parenthesized macro to retrieve, or the "SET" form to update.
	 * <ul>
	 *   <li> nillable (IS_NILLABLE(p)) </li>
	 *   <li> <a href="http://www.w3.org/TR/2011/REC-exi-20110310/#undeclaredProductions">augmented</a>
	 *   with productions external to the schema (IS_AUGMENTED(p)) </li>
	 *   <li> built-in grammar (IS_BUILT_IN_ELEM(p)) </li>
	 *   <li> schema-informed grammar (IS_SCHEMA(p)) </li>
	 *   <li> does type of this grammar have named sub-types or is it union? (HAS_NAMED_SUB_TYPE_OR_UNION(p)) </li>
	 * </ul>
	 */
	unsigned char props;
	/** For a type grammar, the index of the first rule that does not contain attribute content. */
	SmallIndex contentIndex;
};

typedef struct EXIGrammar EXIGrammar;

/**
 * Used for the processing grammar stack. */
struct GrammarStackNode
{
	EXIGrammar* grammar;
	SmallIndex lastNonTermID; // Stores the last NonTermID before another grammar is added on top of the stack
	struct GrammarStackNode* nextInStack;
};

typedef struct GrammarStackNode EXIGrammarStack;

/**@}*/ // End Grammar Types


/** @name String Table Types */
/**@{*/

struct VxEntry {
	Index globalId;
};

typedef struct VxEntry VxEntry;

struct ValueEntry {
	struct
	{
		QNameID forQNameId;
		Index vxEntryId;
	} locValuePartition; // A reference to the VxEntry for that global value
	String valueStr;
};

typedef struct ValueEntry ValueEntry;

struct ValueTable {
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	ValueEntry* value;
	Index count;
#if HASH_TABLE_USE == ON
	/**
	 * Hashtable for fast look-up of global values in the table.
	 * Only used when:
	 * serializing &&
	 * valuePartitionCapacity > 50  &&   //for small table full-scan will work better
	 * valueMaxLength > 0 // this is essentially equal to valuePartitionCapacity == 0
	 *
	 */
	struct hashtable* hashTbl;
#endif
	/** @see http://www.w3.org/TR/2011/REC-exi-20110310/#key-globalID */
	Index globalId;
};

typedef struct ValueTable ValueTable;

struct VxTable {
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	VxEntry* vx;
	Index count;
};

typedef struct VxTable VxTable;

struct PfxTable {
	/** The number of entries */
	SmallIndex count;
	String pfxStr[MAXIMUM_NUMBER_OF_PREFIXES_PER_URI];
};

typedef struct PfxTable PfxTable;

struct LnEntry {
	VxTable vxTable;
	String lnStr;
	/** Global element grammar with uriStr:lnStr qname.
	 *  Either Index of a global element grammar in the SchemaGrammarTable OR
	 *  INDEX_MAX if a global element grammar with that qname does not exist
	 */
	Index elemGrammar;
	/** Global type grammar with uriStr:lnStr qname.
	 *  Either Index of a global type grammar in the SchemaGrammarTable OR
	 *  INDEX_MAX if a global type grammar with that qname does not exist
	 */
	Index typeGrammar;
};

typedef struct LnEntry LnEntry;

struct LnTable {
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	LnEntry* ln;
	Index count;
};

typedef struct LnTable LnTable;

struct UriEntry {
	LnTable lnTable;
	PfxTable* pfxTable;
	String uriStr;
};

typedef struct UriEntry UriEntry;

struct UriTable {
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	UriEntry* uri;
	SmallIndex count;
};

typedef struct UriTable UriTable;

/**@}*/ // End String Table Types

/** @name Constraining Facets IDs
 * Used for fine-grained schema validation */
/**@{*/
#define TYPE_FACET_LENGTH               0x0001 // 0b0000000000000001
#define TYPE_FACET_MIN_LENGTH           0x0002 // 0b0000000000000010
#define TYPE_FACET_MAX_LENGTH           0x0004 // 0b0000000000000100
#define TYPE_FACET_PATTERN              0x0008 // 0b0000000000001000
#define TYPE_FACET_ENUMERATION          0x0010 // 0b0000000000010000
#define TYPE_FACET_WHITE_SPACE          0x0020 // 0b0000000000100000
#define TYPE_FACET_MAX_INCLUSIVE        0x0040 // 0b0000000001000000
#define TYPE_FACET_MAX_EXCLUSIVE        0x0080 // 0b0000000010000000
#define TYPE_FACET_MIN_EXCLUSIVE        0x0100 // 0b0000000100000000
#define TYPE_FACET_MIN_INCLUSIVE        0x0200 // 0b0000001000000000
#define TYPE_FACET_TOTAL_DIGITS         0x0400 // 0b0000010000000000
#define TYPE_FACET_FRACTION_DIGITS      0x0800 // 0b0000100000000000
#define TYPE_FACET_NAMED_SUBTYPE_UNION  0x1000 // 0b0001000000000000
/**@}*/

/**
 * Attributes of a schema simple type, including EXI datatype for the simple content. 
 */
struct SimpleType {
	EXIType exiType;
	uint16_t facetPresenceMask;
	/**
	 * either maxInclusive or maxExclusive or maxLength or
	 * when exiType is LIST and TYPE_FACET_LENGTH is set then the
	 * TYPE_FACET_LENGTH of the list as length indicates the
	 * itemType/typeID of the elements(they are mutually exclusive) */
	int64_t max;
	/**
	 * either minInclusive or minExclusive or minLength (they are mutually exclusive) */
	int64_t min;
	/**
	 * either length or (totalDigits and fractionDigits) or typeId, when exiType is LIST (they are mutually exclusive)
	 * The totalDigits (if available) are encoded in the most significant 16 bits as unsigned integer
	 * and fractionDigits (if available) are encoded in the least significant 16 bits as unsigned integer
	 */
	uint32_t length;
};

typedef struct SimpleType SimpleType;

/** Holds all simple types defined for particular EXI processing (build-in + schema-defined) */
struct SimpleTypeTable {
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	SimpleType* sType;
	Index count;
};

typedef struct SimpleTypeTable SimpleTypeTable;

/** @name Simple types IDs */
/**@{*/
#define SIMPLE_TYPE_STRING                0
#define SIMPLE_TYPE_NORMALIZED_STRING     1
#define SIMPLE_TYPE_TOKEN                 2
#define SIMPLE_TYPE_NMTOKEN               3
#define SIMPLE_TYPE_NMTOKENS              4
#define SIMPLE_TYPE_NAME                  5
#define SIMPLE_TYPE_LANGUAGE              6
#define SIMPLE_TYPE_NCNAME                7
#define SIMPLE_TYPE_IDREF                 8
#define SIMPLE_TYPE_IDREFS                9
#define SIMPLE_TYPE_ENTITY               10
#define SIMPLE_TYPE_ENTITIES             11
#define SIMPLE_TYPE_ID                   12
#define SIMPLE_TYPE_DECIMAL              13
#define SIMPLE_TYPE_INTEGER              14
#define SIMPLE_TYPE_NON_POSITIVE_INTEGER 15
#define SIMPLE_TYPE_NEGATIVE_INTEGER     16
#define SIMPLE_TYPE_LONG                 17
#define SIMPLE_TYPE_INT                  18
#define SIMPLE_TYPE_SHORT                19
#define SIMPLE_TYPE_BYTE                 20
#define SIMPLE_TYPE_NON_NEGATIVE_INTEGER 21
#define SIMPLE_TYPE_UNSIGNED_LONG        22
#define SIMPLE_TYPE_UNSIGNED_INT         23
#define SIMPLE_TYPE_UNSIGNED_SHORT       24
#define SIMPLE_TYPE_UNSIGNED_BYTE        25
#define SIMPLE_TYPE_POSITIVE_INTEGER     26
#define SIMPLE_TYPE_BOOLEAN              27
#define SIMPLE_TYPE_BASE64_BINARY        28
#define SIMPLE_TYPE_HEX_BINARY           29
#define SIMPLE_TYPE_FLOAT                30
#define SIMPLE_TYPE_DOUBLE               31
#define SIMPLE_TYPE_ANY_URI              32
#define SIMPLE_TYPE_QNAME                33
#define SIMPLE_TYPE_NOTATION             34
#define SIMPLE_TYPE_DURATION             35
#define SIMPLE_TYPE_DATE_TIME            36
#define SIMPLE_TYPE_TIME                 37
#define SIMPLE_TYPE_DATE                 38
#define SIMPLE_TYPE_GYEAR_MONTH          39
#define SIMPLE_TYPE_GYEAR                40
#define SIMPLE_TYPE_GMONTH_DAY           41
#define SIMPLE_TYPE_GDAY                 42
#define SIMPLE_TYPE_GMONTH               43
#define SIMPLE_TYPE_ANY_SIMPLE_TYPE      44
#define SIMPLE_TYPE_ANY_TYPE             45

#define SIMPLE_TYPE_COUNT                46
/**@}*/

/** Holds all EXI grammars defined for particular EXI processing (build-in + schema-defined) */
struct SchemaGrammarTable {
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	EXIGrammar* grammar;
	Index count;
};

typedef struct SchemaGrammarTable SchemaGrammarTable;

/**
 * Stores the enum values for a particular simple type */
struct enumDefinition
{
	/** Index of the simple type grammar in the simpleTypeTable */
	Index typeId;
	/** Array of enum values */
	void *values;
	/** The number or enum values*/
	SmallIndex count;
};

typedef struct enumDefinition EnumDefinition;

/** All the enumerations defined in the schema.
 * The entries are sorted by qnameid */
struct EnumTable {
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	EnumDefinition* enumDef;
	Index count;
};

typedef struct EnumTable EnumTable;

/**
 * EXIP representation of XML Schema.
 * @todo If the simple types are included in the grammarTable's EXIGrammar structure,
 * the simpleTypeTable and grammarTable can be merged.
 */
struct EXIPSchema
{
	/**
	 * Keep track of allocations within this structure when the
	 * schema object is dynamically created.
	 */
	AllocList memList;

	/**
	 * Initial string tables
	 */
	UriTable uriTable;

	/**
	 * The document/fragment grammar for that schema instance.
	 */
	EXIGrammar docGrammar;

	/**
	 * Array of simple types for this schema
	 */
	SimpleTypeTable	simpleTypeTable;

	/**
	 * Array of all the grammars in the schema.
	 */
	SchemaGrammarTable grammarTable;

	/**
	 * The number of schema-informed grammars in grammarTable
	 */
	Index staticGrCount;

	EnumTable enumTable;
};

typedef struct EXIPSchema EXIPSchema;

struct StreamContext
{
	/**
	 * Current position in the buffer - bytewise
	 */
	Index bufferIndx;

	/**
	 * Value between 0 and 7; shows the current position within the current byte.
	 * 7 is the least significant bit position in the byte.
	 */
	unsigned char bitPointer;

	/**
	 * Current (left-hand side) non-terminal ID that identifies the current grammar rule. 
	 * Defines the context/processor state.
	 */
	SmallIndex currNonTermID;

	/** The qname of the current element being parsed/serialized */
	QNameID currElem;

	/** The qname of the current attribute */
	QNameID currAttr;

	/** Non-zero if expecting attribute data or list items */
	unsigned int expectATData;

	/** Value type of the expected attribute */
	Index attrTypeId;
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
	 *
	 * @see options_defs
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
	 * INDEX_MAX - unbounded
	 */
	Index valueMaxLength;

	/**
	 * Specifies the total capacity of value partitions in a string table
	 * INDEX_MAX - unbounded
	 */
	Index valuePartitionCapacity;

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

struct BinaryBuffer
{
	/**
	 * Read/write memory buffer
	 */
	char* buf;

	/**
	 * The size of the buffer
	 */
	Index bufLen;

	/**
	 * The size of the data stored in the buffer - number of bytes
	 */
	Index bufContent;

	/**
	 * Input/Output Stream used to fill/flush the buffer during processing
	 */
	IOStream ioStrm;
};

typedef struct BinaryBuffer BinaryBuffer;

/**
 * Represents an EXI stream
 */
struct EXIStream
{
	BinaryBuffer buffer;

	/**
	 * EXI Header - the most important field is the EXI Options. They control the
	 * parsing and serialization of the stream.
	 */
	EXIheader header;

	/** Holds the current state of the stream. */
	StreamContext context;

	/**
	 * The value string table
	 */
	ValueTable valueTable;

	/**
	 * The grammar stack used during processing
	 */
	EXIGrammarStack* gStack;

	/**
	 * Stores the information of all the allocated memory for that stream,
	 * except the global sting values that are stored in the ValueTable
	 */
	AllocList memList;

	/**
	 * Schema information for that stream.
	 * It contains the string tables and possibly schema-informed EXI grammars.
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

errorCode pushOnStack(GenericStack** stack, void* item);

void popFromStack(GenericStack** stack, void** item);

int compareEnumDefs(const void* enum1, const void* enum2);

#endif /* PROCTYPES_H_ */
