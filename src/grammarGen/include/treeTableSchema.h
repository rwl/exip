/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file treeTableSchema.h
 * @brief Defining a TreeTable structure for schema definitions;
 * Functions for converting the XML schema definitions to a TreeTable structure, and TreeTable manipulations
 * @date Mar 13, 2012
 * @author Rumen Kyusakov
 * @author Robert Cragie
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef TREETABLESCHEMA_H_
#define TREETABLESCHEMA_H_

/**
 * The process for generating an EXIPSchema object is the following:
 * First the XML schema file(s) is/are converted to a TreeTable structure(s) - treeTableBuild.c
 * Then the TreeTable structures are rationalize and linked - links base types to derived types, elements to types etc. - treeTableManipulate.c
 * Finally the TreeTable structures are converted to EXIPSchema object - treeTableToGrammars.c
 */

#include "procTypes.h"
#include "dynamicArray.h"
#include "protoGrammars.h"
#include "hashtable.h"

/** Enumeration of elements found in the schema */
enum ElemEnum
{
	ELEMENT_ELEMENT          =0,
	ELEMENT_ATTRIBUTE        =1,
	ELEMENT_CHOICE           =2,
	ELEMENT_COMPLEX_TYPE     =3,
	ELEMENT_COMPLEX_CONTENT  =4,
	ELEMENT_GROUP            =5,
	ELEMENT_IMPORT           =6,
	ELEMENT_SEQUENCE         =7,
	ELEMENT_ALL              =8,
	ELEMENT_EXTENSION        =9,
	ELEMENT_RESTRICTION     =10,
	ELEMENT_SIMPLE_CONTENT  =11,
	ELEMENT_ANY             =12,
	ELEMENT_SIMPLE_TYPE     =13,
	ELEMENT_MIN_INCLUSIVE   =14,
	ELEMENT_ANNOTATION      =15,
	ELEMENT_DOCUMENTATION   =16,
	ELEMENT_MAX_LENGTH      =17,
	ELEMENT_MAX_INCLUSIVE   =18,
	ELEMENT_LIST	        =19,
	ELEMENT_UNION		    =20,
	ELEMENT_ATTRIBUTE_GROUP =21,
	ELEMENT_ANY_ATTRIBUTE   =22,
	ELEMENT_ENUMERATION     =23,
	ELEMENT_KEY             =24,
	ELEMENT_SELECTOR        =25,
	ELEMENT_FIELD           =26,
	ELEMENT_NOTATION        =27,
	ELEMENT_INCLUDE         =28,
	ELEMENT_REDEFINE        =29,
	ELEMENT_MIN_EXCLUSIVE   =30,
	ELEMENT_MAX_EXCLUSIVE   =31,
	ELEMENT_TOTAL_DIGITS    =32,
	ELEMENT_FRACTION_DIGITS =33,
	ELEMENT_LENGTH          =34,
	ELEMENT_MIN_LENGTH      =35,
	ELEMENT_WHITE_SPACE     =36,
	ELEMENT_PATTERN         =37,
	ELEMENT_APPINFO         =38,
	ELEMENT_VOID            =39
};

typedef enum ElemEnum ElemEnum;

/** Codes for the attributes found in the schema */
enum AttrEnum
{
	ATTRIBUTE_NAME           =0,
	ATTRIBUTE_TYPE           =1,
	ATTRIBUTE_REF            =2,
	ATTRIBUTE_MIN_OCCURS     =3,
	ATTRIBUTE_MAX_OCCURS     =4,
	ATTRIBUTE_FORM           =5,
	ATTRIBUTE_BASE           =6,
	ATTRIBUTE_USE            =7,
	ATTRIBUTE_NAMESPACE      =8,
	ATTRIBUTE_PROC_CONTENTS  =9,
	ATTRIBUTE_VALUE          =10,
	ATTRIBUTE_NILLABLE       =11,
	ATTRIBUTE_ITEM_TYPE      =12,
	ATTRIBUTE_MEMBER_TYPES   =13,
	ATTRIBUTE_MIXED          =14,
	ATTRIBUTE_CONTEXT_ARRAY_SIZE =15
};

typedef enum AttrEnum AttrEnum;

/** Namespace and prefix defined in a XML schema file */
struct PfxNsEntry
{
	String pfx;
	String ns;
};

typedef struct PfxNsEntry PfxNsEntry;

/** An array of namespaces and prefixes defined in a XML schema file */
struct PfxNsTable
{
	DynArray dynArray;
	PfxNsEntry *pfxNs;
	Index count;
};

typedef struct PfxNsTable PfxNsTable;

/** The possible values of the schema attributes:
 * elementFormDefault and attributeFormDefault */
enum FormType
{
	QUALIFIED          =0,
	UNQUALIFIED        =1
};

typedef enum FormType FormType;

/**
 * Represents a single definition (i.e XML element) from the XML schema.
 * For example:
 * [element] entry, [complexType], [attribute] entry etc.
 * The entry contains all the attribute values defined as well as
 * links to nested entries, parent entries and sibling entries.
 * */
struct TreeTableEntry
{
	/** Child element. It is either a nested element or a link to a global definition.
	 * The global definition is linked during resolveTypeHierarchy(). In this case
	 * a local entry with type="..." (or ref="...") attribute is linked to a
	 * global entry that defines the corresponding type or referenced element.
	 * The linked global entry can be located in different tree (XSD file); */
	struct
	{
		struct TreeTable* treeT;
		struct TreeTableEntry* entry;
	} child;

	/** Next element in group. For example the
	 * [element], [attribute], [sequence] are often sibling definitions in a complex type */
	struct TreeTableEntry* next;

	/** Element corresponding to a supertype.
	 * In case of [extension] or [restriction] entries, this pointer links the
	 * base type to the corresponding global type definition.
	 * The linked global entry can be located in different tree (XSD file); */
	struct
	{
		struct TreeTable* treeT;
		struct TreeTableEntry* entry;
	} supertype;

	/** The XML schema element. Represented with the codes defined above */
	ElemEnum element;

	/** In case of attributes that are part of the definition, their values are stored as a string
	 * in this array. The index is the code of the attribute whose value is stored. */
	String attributePointers[ATTRIBUTE_CONTEXT_ARRAY_SIZE];
};

typedef struct TreeTableEntry TreeTableEntry;

/** 
 * Tree structure to store parsed schema structure and attributes; a schema tree.
 * All the information from the XML schema needed for the grammar generation is stored
 * in this structure. */
struct TreeTable
{
	DynArray dynArray;
	TreeTableEntry* tree;
	Index count;

	struct
	{
		/** Whether the tree corresponds to the main XSD of the schema definitions */
		unsigned char isMain;
		/** targetNamespace for that TreeTable */
		String targetNs;
		/** Identifies the target namespace in the EXIP Schema URI Table */
		SmallIndex targetNsId; 
		/** Qualify namespace for locally declared elements? */
		FormType elemFormDefault;
		/**  Qualify namespace for locally declared attributes? */
		FormType attrFormDefault;
		/** Namespace prefix/URI pairs */
		PfxNsTable pfxNsTable;
	} globalDefs;

	AllocList memList; // Holding the memory allocations for the TreeTable

#if HASH_TABLE_USE == ON
	/**
	 * Used for fast lookups of global types when linking types' base types (extensions & restrictions)
	 * and element types to global types.
	 */
	struct hashtable* typeTbl;

	/**
	 * Used for fast lookups of elements when linking element declarations
	 * referred with the ref schema attribute.
	 */
	struct hashtable* elemTbl;

	/**
	 * Used for fast lookups of global attributes when linking references to global attributes
	 */
	struct hashtable* attrTbl;

	/**
	 * Used for fast lookups of global model groups during linking references to them
	 */
	struct hashtable* groupTbl;

	/**
	 * Used for fast lookups of global attribute groups when linking references to global attribute groups
	 */
	struct hashtable* attrGroupTbl;
#endif
};

typedef struct TreeTable TreeTable;

/**
 * Some schema attributes (e.g. namespace="...")
 * define a list of namespaces that are stored as a string.
 * The string needs to be parsed and each namespace added
 * as an entry in the NsTable dynamic array.
 * The entries are then used to fill the URI partition
 * of the string table and by the createWildcardTermGrammar()
 * and createComplexGrammar()
 * */
struct NsTable
{
	DynArray dynArray;
	String *base;
	size_t count;
};

typedef struct NsTable NsTable;

/**
 * @brief Initialize a TreeTable object
 *
 * @param[in, out] treeT a tree table container
 * @return Error handling code
 */
errorCode initTreeTable(TreeTable* treeT);

/**
 * @brief Destroy a TreeTable object (free the memory allocations)
 *
 * @param[in, out] treeT a tree table object
 * @return Error handling code
 */
void destroyTreeTable(TreeTable* treeT);

/**
 * @brief Performs two things: builds a treeTable and the string tables of the EXIPSchema object
 *
 * -# From the XML schema definitions builds an analogous treeTable
 * -# Fills in the pre-populated entries in the string tables of the EXIPSchema object
 * 
 * The pre-populated entries in the string tables are sorted in generateSchemaInformedGrammars()
 * after all the treeTables are generated.
 * 
 * The schema and treeT objects should be initialized before calling this function.
 * @param[in] buffer an input buffer holding (part of) the representation of the schema
 * @param[in] schemaFormat EXI, XSD, DTD or any other schema representation supported
 * @param[in] opt options used for EXI schemaFormat - otherwise NULL. If options are set then they will be used
 * for processing the EXI XSD stream although no options are specified in the EXI header. If there are
 * options defined in the EXI header of the XSD stream then this parameter must be NULL.
 * @param[out] treeT a memory representation of the XML schema definitions. Must be initialized.
 * @param[out] schema partly built schema information (only the string tables) used for processing EXI streams.
 * Must be initialized.
 * @return Error handling code
 */
errorCode generateTreeTable(BinaryBuffer buffer, unsigned char schemaFormat, EXIOptions* opt, TreeTable* treeT, EXIPSchema* schema);

/**
 * @brief Links derived types to base types, elements to types and references to global elements
 * 
 * In case of:
 * -# type="..." attribute - finds the corresponding type definition and links it to the
 *    child pointer of that entry
 * -# ref="..." attribute - finds the corresponding element definition and links it to the
 *    child pointer of that entry
 * -# base="..." attribute - finds the corresponding type definition and links it to the
 *    supertype pointer of that entry
 *
 * @param[in] schema the EXIPSchema object
 * @param[in, out] treeT an array of tree table objects
 * @param[in] count the number of tree table objects
 * @return Error handling code
 */
errorCode resolveTypeHierarchy(EXIPSchema* schema, TreeTable* treeT, unsigned int count);

/**
 * @brief Given types resolved TreeTable objects that are created from a XML schema files,
 * builds the EXIP grammars in the EXIPSchema object.
 *
 * @param[in] treeT an array of tree table objects
 * @param[in] count the number of tree table objects
 * @param[out] schema schema information used for processing EXI streams in schema mode
 * @return Error handling code
 */
errorCode convertTreeTablesToExipSchema(TreeTable* treeT, unsigned int count, EXIPSchema* schema);

/**
 * @brief Given a type value encoded as QName string in the form "prefix:localname"
 * derives the "namespace" part and the "local name" part.
 * 
 * Used for the value of the "type" attribute of element and attribute use definitions
 *
 * @param[in] schema the EXIPSchema object
 * @param[in] treeT a tree table object
 * @param[in] typeLiteral the type literal
 * @param[out] qname qname of the type
 * @return Error handling code
 */
errorCode getTypeQName(EXIPSchema* schema, TreeTable* treeT, const String typeLiteral, QNameID* qname);

/**
 * @brief Given a namespace="..." attribute containing a list of namespaces as a sting, returns an array of these namespaces
 *
 * This function also resolves the special values "##targetNamespace" and "##local"
 *
 * @param[in] treeT the tree table object
 * @param[in] nsList a sting containing the space separated namespaces
 * @param[out] nsTable an array of namespaces
 * @return Error handling code
 */
errorCode getNsList(TreeTable* treeT, String nsList, NsTable* nsTable);

#if DEBUG_GRAMMAR_GEN == ON
/**
 * @brief Print a tree table entry
 *
 * @param[in] treeTableEntryIn a tree table object
 * @param[in] indentIdx Indent index
 * @param[in] prefix prefix for the C definitions
 */
void printTreeTableEntry(TreeTableEntry* treeTableEntryIn, int indentIdx, char *prefix);
#endif

#endif /* TREETABLESCHEMA_H_ */
