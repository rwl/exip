/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file treeTableBuild.c
 * @brief Implementation of functions for converting a XML schema document to a TreeTable structure containing the schema definitions
 * @date Mar 13, 2012
 * @author Rumen Kyusakov
 * @author Robert Cragie
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "treeTableSchema.h"
#include "dynamicArray.h"
#include "EXIParser.h"
#include "genUtils.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "grammars.h"
#include "initSchemaInstance.h"
#include "grammarGenerator.h"

#define INITIAL_STATE         0
#define SCHEMA_ELEMENT_STATE  1
#define SCHEMA_CONTENT_STATE  2

extern const String XML_SCHEMA_NAMESPACE;

/**
 * State required when parsing a schema to build a TreeTable.
 */
struct TreeTableParsingData
{
	/**
	 * Current status:
	 * - 0 initial state
	 * - 1 [schema] element is parsed; expect attributes
	 * - 2 the properties are all set ([schema] attr. parsed)
	 */
	unsigned char propsStat; 
	boolean expectingAttr;
	/** Pointer to the expected character data */
	String* charDataPtr;
	/**
	 * Whether or not the currently processed element is part of the tree table.
	 * - 0 (FALSE) - the element is not ignored and will be an entry in the tree table
	 * - > 0 - the number of ignored elements on the stack
	 */
	unsigned int ignoredElement;
	/** Stack of TreeTableEntry */
	GenericStack* contextStack; 
	/** Default is UNQUALIFIED. */
	String attributeFormDefault;
	/** Default is UNQUALIFIED. */
	String elementFormDefault;
	/** The index in the uriTable (before sorting) of the targetNamespace */
	SmallIndex targetNsId;
	TreeTable* treeT;
	EXIPSchema* schema;
};


// Content Handler API
static errorCode xsd_fatalError(const errorCode code, const char* msg, void* app_data);
static errorCode xsd_startDocument(void* app_data);
static errorCode xsd_endDocument(void* app_data);
static errorCode xsd_startElement(QName qname, void* app_data);
static errorCode xsd_endElement(void* app_data);
static errorCode xsd_attribute(QName qname, void* app_data);
static errorCode xsd_stringData(const String value, void* app_data);
static errorCode xsd_namespaceDeclaration(const String ns, const String prefix, boolean isLocalElementNS, void* app_data);

//////////// Helper functions

static void initEntryContext(TreeTableEntry* entry);

////////////

static const char* elemStrings[] =
{
	"element",
	"attribute",
	"choice",
	"complexType",
	"complexContent",
	"group",
	"import",
	"sequence",
	"all",
	"extension",
	"restriction",
	"simpleContent",
	"any",
	"simpleType",
	"minInclusive",
	"annotation",
	"documentation",
	"maxLength",
	"maxInclusive",
	"list",
	"union",
	"attributeGroup",
	"anyAttribute",
	"enumeration",
	"key",
	"selector",
	"field",
	"notation",
	"include",
	"redefine",
	"minExclusive",
	"maxExclusive",
	"totalDigits",
	"fractionDigits",
	"length",
	"minLength",
	"whiteSpace",
	"pattern",
	"appinfo"
};

static const char* attrStrings[] =
{
	"name",
	"type",
	"ref",
	"minOccurs",
	"maxOccurs",
	"form",
	"base",
	"use",
	"namespace",
	"processContents",
	"value",
	"nillable",
	"itemType",
	"memberTypes",
	"mixed"
};

errorCode generateTreeTable(BinaryBuffer buffer, SchemaFormat schemaFormat, EXIOptions* opt, TreeTable* treeT, EXIPSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Parser xsdParser;
	struct TreeTableParsingData ttpd;

	if(schemaFormat != SCHEMA_FORMAT_XSD_EXI)
		return NOT_IMPLEMENTED_YET;

	TRY(initParser(&xsdParser, buffer, &ttpd));

	xsdParser.handler.fatalError = xsd_fatalError;
	xsdParser.handler.error = xsd_fatalError;
	xsdParser.handler.startDocument = xsd_startDocument;
	xsdParser.handler.endDocument = xsd_endDocument;
	xsdParser.handler.startElement = xsd_startElement;
	xsdParser.handler.attribute = xsd_attribute;
	xsdParser.handler.stringData = xsd_stringData;
	xsdParser.handler.endElement = xsd_endElement;
	xsdParser.handler.namespaceDeclaration = xsd_namespaceDeclaration;

	ttpd.propsStat = INITIAL_STATE;
	ttpd.expectingAttr = FALSE;
	ttpd.charDataPtr = NULL;
	ttpd.ignoredElement = FALSE;
	getEmptyString(&ttpd.attributeFormDefault);
	getEmptyString(&ttpd.elementFormDefault);
	getEmptyString(&treeT->globalDefs.targetNs);
	ttpd.contextStack = NULL;

	ttpd.treeT = treeT;
	ttpd.schema = schema;

	// Parse the EXI stream

	if(opt != NULL)
	{
		xsdParser.strm.header.opts = *opt;
		TRY(parseHeader(&xsdParser, TRUE));
	}
	else
	{
		TRY(parseHeader(&xsdParser, FALSE));
	}

	if(IS_PRESERVED(xsdParser.strm.header.opts.preserve, PRESERVE_PREFIXES) == FALSE)
	{
		/* When qualified namesNS are used in the values of AT or CH events in an EXI Stream,
		 * the Preserve.prefixes fidelity option SHOULD be turned on to enable the preservation of
		 * the NS prefix declarations used by these values. Note, in particular among other cases,
		 * that this practice applies to the use of xsi:type attributes in EXI streams when Preserve.lexicalValues
		 * fidelity option is set to true.*/
		DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">XML Schema must be EXI encoded with the prefixes preserved\n"));
		destroyParser(&xsdParser);
		return NO_PREFIXES_PRESERVED_XML_SCHEMA;
	}

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">XML Schema header parsed\n"));

	// TODO: add the XSD schema here - will be used when the EXI encoded XSD is in schema mode
 	TRY(setSchema(&xsdParser, NULL));

	while(tmp_err_code == ERR_OK)
	{
		tmp_err_code = parseNext(&xsdParser);
	}

	destroyParser(&xsdParser);

	if(tmp_err_code == PARSING_COMPLETE)
		return ERR_OK;

	return tmp_err_code;
}

static errorCode xsd_fatalError(const errorCode code, const char* msg, void* app_data)
{
	DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Fatal error occurred during schema processing\n"));
	return EXIP_HANDLER_STOP;
}

static errorCode xsd_startDocument(void* app_data)
{
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Start XML Schema parsing\n"));
	return ERR_OK;
}

static errorCode xsd_endDocument(void* app_data)
{
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End XML Schema parsing\n"));
	return ERR_OK;
}

static errorCode xsd_startElement(QName qname, void* app_data)
{
	struct TreeTableParsingData* ttpd = (struct TreeTableParsingData*) app_data;
	if(ttpd->propsStat == INITIAL_STATE) // This should be the first <schema> element
	{
		if(stringEqual(*qname.uri, XML_SCHEMA_NAMESPACE) &&
				stringEqualToAscii(*qname.localName, "schema"))
		{
			ttpd->propsStat = SCHEMA_ELEMENT_STATE;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <schema> element\n"));
		}
		else
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid XML Schema! Missing <schema> root element\n"));
			return EXIP_HANDLER_STOP;
		}
	}
	else
	{
		TreeTableEntry* treeTableEntry;
		TreeTableEntry* prevEntry = NULL;
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Index treeTableId;
		int i;

		if(ttpd->propsStat == SCHEMA_ELEMENT_STATE)
		{
			/** The next element after the <schema>
			 *  The targetNamespace should be defined by now, so inserted in the schema->uriTable
			 */
			if(!lookupUri(&ttpd->schema->uriTable, ttpd->treeT->globalDefs.targetNs, &ttpd->targetNsId))
			{
				String clonedTargetNS;

				TRY(cloneStringManaged(&ttpd->treeT->globalDefs.targetNs, &clonedTargetNS, &ttpd->schema->memList));

				// Add the target namespace to the schema string tables
				TRY(addUriEntry(&ttpd->schema->uriTable, clonedTargetNS, &ttpd->targetNsId));
			}

			// Setting up the tree table globalDefs

			if(stringEqualToAscii(ttpd->elementFormDefault, "qualified"))
				ttpd->treeT->globalDefs.elemFormDefault = QUALIFIED;
			if(stringEqualToAscii(ttpd->attributeFormDefault, "qualified"))
				ttpd->treeT->globalDefs.attrFormDefault = QUALIFIED;

			ttpd->propsStat = SCHEMA_CONTENT_STATE;
		}

		/**** Normal element processing starts here ****/

		if(!stringEqual(*qname.uri, XML_SCHEMA_NAMESPACE))
		{
			if(ttpd->ignoredElement == 0)
			{
				// If it is not within an ignored element
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid namespace of XML Schema element\n"));
				return EXIP_HANDLER_STOP;
			}
			else
			{
				// If it is within an ignored element - ignore this one as well
				ttpd->ignoredElement += 1;
				return ERR_OK;
			}
		}

		/**** Ignore certain elements. These elements will not be part of the resulting tree table. ****/

		if(stringEqualToAscii(*qname.localName, elemStrings[ELEMENT_ANNOTATION]))
		{
			ttpd->ignoredElement += 1;
			return ERR_OK;
		}
		else if(stringEqualToAscii(*qname.localName, elemStrings[ELEMENT_DOCUMENTATION]))
		{
			ttpd->ignoredElement += 1;
			return ERR_OK;
		}
		else if(stringEqualToAscii(*qname.localName, elemStrings[ELEMENT_APPINFO]))
		{
			ttpd->ignoredElement += 1;
			return ERR_OK;
		}
		else
			ttpd->ignoredElement = FALSE;

		if(ttpd->contextStack != NULL)
			prevEntry = (TreeTableEntry*) ttpd->contextStack->item;

		if(prevEntry == NULL)
		{
			/* At the root level, so create an entry */
			TRY(addEmptyDynEntry(&ttpd->treeT->dynArray, (void**)&treeTableEntry, &treeTableId));
		}
		else
		{
			/* Nested entry */
			treeTableEntry = (TreeTableEntry*) memManagedAllocate(&ttpd->treeT->memList, sizeof(TreeTableEntry));
			if(treeTableEntry == NULL)
			{
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Memory allocation error\n"));
				return EXIP_HANDLER_STOP;
			}

			if(prevEntry->child.entry == NULL)
			{
				/* Nesting one level deeper */
				prevEntry->child.entry = treeTableEntry;
				prevEntry->child.treeT = ttpd->treeT;
			}
			else /* Append across */
			{
				TreeTableEntry* lastEntry;

				lastEntry = prevEntry->child.entry->next;
				if(lastEntry == NULL)
				{
					prevEntry->child.entry->next = treeTableEntry;
				}
				else
				{
					while(lastEntry->next != NULL)
					{
						lastEntry = lastEntry->next;
					}
					lastEntry->next = treeTableEntry;
				}
			}
		}

		initEntryContext(treeTableEntry);

		for(i = (int) ELEMENT_ELEMENT; i < (int) ELEMENT_VOID; i++)
		{
			/* 
			 * See what sort of stream element this actually is.
			 * Tag the stream element to direct processing at the end of the stream element
			 */
			if(stringEqualToAscii(*qname.localName, elemStrings[i]))
			{
				treeTableEntry->element = (ElemEnum) i;
				DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Starting <%s> element\n", elemStrings[i]));
				break;
			}
		}
		if (i == (int) ELEMENT_VOID)
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored schema element\n"));
#if EXIP_DEBUG == ON && DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL <= WARNING
			printString(qname.localName);
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, ("\n"));
#endif
			return EXIP_HANDLER_STOP;
		}

		/* Push the stream element onto the context stack associated with the stream */
		TRY(pushOnStack(&(ttpd->contextStack), treeTableEntry));
	}

	return ERR_OK;
}

static errorCode xsd_endElement(void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	struct TreeTableParsingData* ttpd = (struct TreeTableParsingData*) app_data;

	if(ttpd->ignoredElement != FALSE)
	{
		ttpd->ignoredElement -= 1;
		return ERR_OK;
	}

	if(ttpd->contextStack == NULL) // No elements stored in the stack. That is </schema>
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">End </schema> element\n"));
	else
	{
		TreeTableEntry* entry;
		String* elName;
		String clonedName;

		/* Pop the stream element from the stack*/
		popFromStack(&ttpd->contextStack, (void**) &entry);

		elName = &entry->attributePointers[ATTRIBUTE_NAME];

		if(!isStringEmpty(elName))
		{
			Index lnId;
			SmallIndex uriId = 0; // URI	0	"" [empty string]

			if(ttpd->contextStack == NULL) // If the schema definition is global
			{
				// it is always in the target namespace
				uriId = ttpd->targetNsId;
			}
			else
			{
				// local scope so look for form attribute value
				if(entry->element == ELEMENT_ELEMENT)
				{
					if(ttpd->treeT->globalDefs.elemFormDefault == QUALIFIED || stringEqualToAscii(entry->attributePointers[ATTRIBUTE_FORM], "qualified"))
						uriId = ttpd->targetNsId;

				}
				else if(entry->element == ELEMENT_ATTRIBUTE)
				{
					if(ttpd->treeT->globalDefs.attrFormDefault == QUALIFIED || stringEqualToAscii(entry->attributePointers[ATTRIBUTE_FORM], "qualified"))
						uriId = ttpd->targetNsId;
				}
			}

			if(!lookupLn(&ttpd->schema->uriTable.uri[uriId].lnTable, *elName, &lnId))
			{
				TRY(cloneStringManaged(elName, &clonedName, &ttpd->schema->memList));

				/* Add the element name to the schema string tables. Note this table persists beyond the tree table */
				TRY(addLnEntry(&ttpd->schema->uriTable.uri[uriId].lnTable, clonedName, &lnId));
			}

			if(entry->element == ELEMENT_ANY || entry->element == ELEMENT_ANY_ATTRIBUTE)
			{
				NsTable nsTable;
				size_t i;

				TRY(createDynArray(&nsTable.dynArray, sizeof(String), 5));

				if(ERR_OK != getNsList(ttpd->treeT, entry->attributePointers[ATTRIBUTE_NAMESPACE], &nsTable))
					return	EXIP_HANDLER_STOP;

				for(i = 0; i < nsTable.count; i++)
				{
					if(!lookupUri(&ttpd->schema->uriTable, nsTable.base[i], &uriId))
					{
						TRY(cloneStringManaged(&nsTable.base[i], &clonedName, &ttpd->schema->memList));

						/* Add the namespace to the schema URI string table. Note this table persists beyond the tree table */
						TRY(addUriEntry(&ttpd->schema->uriTable, clonedName, &uriId));
					}
				}
			}

#if HASH_TABLE_USE == ON
			/*
			 * The hash tables are used to look up global definitions easily when it comes to resolving the tree table
			 * For example, an element local to another element may be declared with a type definition which is global.
			 * When it comes to linking that type in as a child or supertype, then the hash table can easily look up the
			 * appropriate tree table entry
			 */

			if(ttpd->contextStack == NULL) // If the schema definition is global
			{
				String lnNameStr = GET_LN_URI_IDS(ttpd->schema->uriTable, uriId, lnId).lnStr;

				if(entry->element == ELEMENT_ELEMENT)
				{
					if(ttpd->treeT->elemTbl != NULL)
					{
						if(hashtable_search(ttpd->treeT->elemTbl, lnNameStr) != INDEX_MAX)
						{
							DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("ERROR: Duplicate global element name definition\n"));
							return EXIP_HANDLER_STOP;
						}

						if(hashtable_insert(ttpd->treeT->elemTbl, lnNameStr, ttpd->treeT->count - 1) != ERR_OK)
							return EXIP_HANDLER_STOP;
					}
				}
				else if(entry->element == ELEMENT_SIMPLE_TYPE || entry->element == ELEMENT_COMPLEX_TYPE)
				{
					if(ttpd->treeT->typeTbl != NULL)
					{
						if(hashtable_search(ttpd->treeT->typeTbl, lnNameStr) != INDEX_MAX)
						{
							DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("ERROR: Duplicate global type name definition\n"));
							return EXIP_HANDLER_STOP;
						}

						if(hashtable_insert(ttpd->treeT->typeTbl, lnNameStr, ttpd->treeT->count - 1) != ERR_OK)
							return EXIP_HANDLER_STOP;
					}
				}
				else if(entry->element == ELEMENT_ATTRIBUTE)
				{
					if(ttpd->treeT->attrTbl != NULL)
					{
						if(hashtable_search(ttpd->treeT->attrTbl, lnNameStr) != INDEX_MAX)
						{
							DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("ERROR: Duplicate global attribute name definition\n"));
							return EXIP_HANDLER_STOP;
						}

						if(hashtable_insert(ttpd->treeT->attrTbl, lnNameStr, ttpd->treeT->count - 1) != ERR_OK)
							return EXIP_HANDLER_STOP;
					}
				}
				else if(entry->element == ELEMENT_GROUP)
				{
					if(ttpd->treeT->groupTbl != NULL)
					{
						if(hashtable_search(ttpd->treeT->groupTbl, lnNameStr) != INDEX_MAX)
						{
							DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("ERROR: Duplicate global group name definition\n"));
							return EXIP_HANDLER_STOP;
						}

						if(hashtable_insert(ttpd->treeT->groupTbl, lnNameStr, ttpd->treeT->count - 1) != ERR_OK)
							return EXIP_HANDLER_STOP;
					}
				}
				else if(entry->element == ELEMENT_ATTRIBUTE_GROUP)
				{
					if(ttpd->treeT->attrGroupTbl != NULL)
					{
						if(hashtable_search(ttpd->treeT->attrGroupTbl, lnNameStr) != INDEX_MAX)
						{
							DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("ERROR: Duplicate global attribute group name definition\n"));
							return EXIP_HANDLER_STOP;
						}

						if(hashtable_insert(ttpd->treeT->attrGroupTbl, lnNameStr, ttpd->treeT->count - 1) != ERR_OK)
							return EXIP_HANDLER_STOP;
					}
				}
			}
#endif
		}

	}

	return ERR_OK;
}

static errorCode xsd_attribute(QName qname, void* app_data)
{
	struct TreeTableParsingData* ttpd = (struct TreeTableParsingData*) app_data;

	if(ttpd->ignoredElement != FALSE)
		return ERR_OK;

	if(ttpd->propsStat == SCHEMA_ELEMENT_STATE) // <schema> element attribute
	{
		if(stringEqualToAscii(*qname.localName, "targetNamespace"))
		{
			ttpd->charDataPtr = &(ttpd->treeT->globalDefs.targetNs);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |targetNamespace| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "elementFormDefault"))
		{
			ttpd->charDataPtr = &(ttpd->elementFormDefault);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |elementFormDefault| \n"));
		}
		else if(stringEqualToAscii(*qname.localName, "attributeFormDefault"))
		{
			ttpd->charDataPtr = &(ttpd->attributeFormDefault);
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |attributeFormDefault| \n"));
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored <schema> attribute\n"));
#if EXIP_DEBUG == ON && DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL <= WARNING
			printString(qname.localName);
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, ("\n"));
#endif
		}
	}
	else
	{
		TreeTableEntry* currEntry = (TreeTableEntry*) ttpd->contextStack->item;
		int i;

		for(i = (int)ATTRIBUTE_NAME; i < (int)ATTRIBUTE_CONTEXT_ARRAY_SIZE; i++)
		{
			/* 
			 * See what sort of attribute this actually is.
			 * Tag the data ptr to the right attribute placeholder
			 */
			if(stringEqualToAscii(*qname.localName, attrStrings[i]))
			{
				ttpd->charDataPtr = &currEntry->attributePointers[i];
				DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Attribute |%s|\n", attrStrings[i]));
				break;
			}
		}
		if (i == ATTRIBUTE_CONTEXT_ARRAY_SIZE)
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element attribute\n"));
#if EXIP_DEBUG == ON && DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL <= WARNING
			printString(qname.localName);
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, ("\n"));
#endif
		}
	}
	ttpd->expectingAttr = TRUE;
	return ERR_OK;
}

static errorCode xsd_stringData(const String value, void* app_data)
{
	struct TreeTableParsingData* ttpd = (struct TreeTableParsingData*) app_data;
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(ttpd->ignoredElement != FALSE)
		return ERR_OK;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">String data:\n"));

#if	DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	printString(&value);
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n"));
#endif

	if(ttpd->expectingAttr)
	{
		if(ttpd->charDataPtr != NULL)
		{
			TRY(cloneStringManaged(&value, ttpd->charDataPtr, &ttpd->treeT->memList));
			ttpd->charDataPtr = NULL;
		}
		else
		{
			DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, (">Ignored element attribute value\n"));
		}
		ttpd->expectingAttr = FALSE;
	}
	else
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Ignored element character data"));
	}

	return ERR_OK;
}

static errorCode xsd_namespaceDeclaration(const String ns, const String pfx, boolean isLocalElementNS, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	struct TreeTableParsingData* ttpd = (struct TreeTableParsingData*) app_data;
	PfxNsEntry pfxNsEntry;
	Index entryID;

	if(ttpd->ignoredElement != FALSE)
		return ERR_OK;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (">Namespace declaration\n"));
#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("  pfx: "));
	printString(&pfx);
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("  ns: "));
	printString(&ns);
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n"));
#endif
	
	TRY(cloneStringManaged(&ns, &pfxNsEntry.ns, &ttpd->treeT->memList));
	TRY(cloneStringManaged(&pfx, &pfxNsEntry.pfx, &ttpd->treeT->memList));
	TRY(addDynEntry(&ttpd->treeT->globalDefs.pfxNsTable.dynArray, &pfxNsEntry, &entryID));

	return ERR_OK;
}

static void initEntryContext(TreeTableEntry* entry)
{
	unsigned int i = 0;
	entry->next = NULL;
	entry->supertype.entry = NULL;
	entry->supertype.treeT = NULL;
	entry->child.entry = NULL;
	entry->child.treeT = NULL;
	entry->element = ELEMENT_VOID;
	entry->loopDetection = 0;

	for(i = 0; i < ATTRIBUTE_CONTEXT_ARRAY_SIZE; i++)
	{
		entry->attributePointers[i].length = 0;
		entry->attributePointers[i].str = NULL;
	}
}

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO

static void printNameTypeBase(String *attrPtrs, char* indent)
{
	if(attrPtrs[ATTRIBUTE_NAME].str != NULL)
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("%sName: ", indent));
		printString(&attrPtrs[ATTRIBUTE_NAME]);
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n"));
	}
	if(attrPtrs[ATTRIBUTE_TYPE].str != NULL)
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("%sType: ", indent));
		printString(&attrPtrs[ATTRIBUTE_TYPE]);
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n"));
	}
	if(attrPtrs[ATTRIBUTE_BASE].str != NULL)
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("%sBase: ", indent));
		printString(&attrPtrs[ATTRIBUTE_BASE]);
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n"));
	}
}

#define FULL_EXPANSION

void printTreeTableEntry(TreeTableEntry* treeTableEntryIn, int indentIdx, char *prefix)
{
	char indent[101] = "                                                                                                    ";

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("%s%s[%s]\n", indent + (100 - indentIdx), prefix, elemStrings[treeTableEntryIn->element]));
	printNameTypeBase(treeTableEntryIn->attributePointers, indent + (100 - indentIdx));

#ifdef FULL_EXPANSION
	if(treeTableEntryIn->supertype.entry != NULL)
		printTreeTableEntry(treeTableEntryIn->supertype.entry, indentIdx + 4, "");

	if(treeTableEntryIn->child.entry != NULL)
		printTreeTableEntry(treeTableEntryIn->child.entry, indentIdx, "");
#else
	if(treeTableEntryIn->child.entry != NULL)
		printTreeTableEntry(treeTableEntryIn->child.entry, indentIdx+2, "");
#endif

	if(treeTableEntryIn->next != NULL)
		printTreeTableEntry(treeTableEntryIn->next, indentIdx, "...");

}
#endif
