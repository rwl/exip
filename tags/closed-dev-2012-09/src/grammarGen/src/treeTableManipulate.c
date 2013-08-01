/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file treeTableManipulate.c
 * @brief Manipulation operations on the TreeTable such as rationalize, resolveTypeHierarchy etc.
 * @date Mar 13, 2012
 * @author Robert Cragie
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "treeTableSchema.h"
#include "memManagement.h"
#include "stringManipulate.h"
#include "sTables.h"

#define TREE_TABLE_ENTRY_COUNT 200

#define LOOKUP_ELEMENT_TYPE_TYPE       0
#define LOOKUP_ELEMENT_TYPE_ELEMENT    1
#define LOOKUP_ELEMENT_TYPE_SUPER_TYPE 2

/**
 * Finds a global TreeEntry corresponding to a entry-name eName and links it to the entry child or supertype
 * depending on the elType (LOOKUP_ELEMENT_TYPE_TYPE, LOOKUP_ELEMENT_TYPE_ELEMENT or LOOKUP_ELEMENT_TYPE_SUPER_TYPE)
 */
static errorCode lookupGlobalDefinition(EXIPSchema* schema, TreeTable* treeT, unsigned int count, unsigned int currTreeT, String* eName, unsigned char elType, TreeTableEntry* entry);

/**
 * Check if there exists an <xs:import> with a given namespace attribute
 */
static unsigned char checkForImportWithNs(TreeTable* treeT, String ns);

/**
 * Resolve all the TreeTable entries linked to a global TreeTable entry
 * Performs a Depth-first search (DFS) of the tree formed by the global entry and for all nested entries do:
 * In case of:
 * 1) type="..." attribute - finds the corresponding type definition and links it to the
 * child pointer of that entry
 * 2) ref="..." attribute - finds the corresponding element definition and links it to the
 * child pointer of that entry
 * 3) base="..." attribute - finds the corresponding type definition and links it to the
 * supertype pointer of that entry
 */
static errorCode resolveEntry(EXIPSchema* schema, TreeTable* treeT, unsigned int count, unsigned int currTreeT, TreeTableEntry* entry);

errorCode initTreeTable(TreeTable* treeT)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = initAllocList(&treeT->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&treeT->dynArray, sizeof(TreeTableEntry), TREE_TABLE_ENTRY_COUNT, &treeT->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&treeT->globalDefs.pfxNsTable.dynArray, sizeof(PfxNsEntry), 10, &treeT->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	treeT->globalDefs.isMain = FALSE;
	treeT->globalDefs.attrFormDefault = UNQUALIFIED;
	treeT->globalDefs.elemFormDefault = UNQUALIFIED;
	treeT->count = 0;
	treeT->globalDefs.targetNsId = 0;

#if HASH_TABLE_USE == ON
	// TODO: conditionally create the table, only if the schema is big.
	// How to determine when the schema is big?
	treeT->typeTbl = create_hashtable(INITIAL_HASH_TABLE_SIZE, djbHash, stringEqual);
	if(treeT->typeTbl == NULL)
		return HASH_TABLE_ERROR;

	treeT->elemTbl = create_hashtable(INITIAL_HASH_TABLE_SIZE, djbHash, stringEqual);
	if(treeT->elemTbl == NULL)
		return HASH_TABLE_ERROR;
#endif

	return ERR_OK;
}

void destroyTreeTable(TreeTable* treeT)
{

#if HASH_TABLE_USE == ON
	if(treeT->typeTbl != NULL)
		hashtable_destroy(treeT->typeTbl);

	if(treeT->elemTbl != NULL)
		hashtable_destroy(treeT->elemTbl);
#endif
	freeAllocList(&treeT->memList);
}

errorCode resolveTypeHierarchy(EXIPSchema* schema, TreeTable* treeT, unsigned int count)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Index j;
	unsigned int i;

	/* For every tree table (XSD file) */
	for(i = 0; i < count; i++)
	{
		/* For every three table global entry */
		for(j = 0; j < treeT[i].count; j++)
		{
			tmp_err_code = resolveEntry(schema, treeT, count, i, &treeT[i].tree[j]);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	return ERR_OK;
}

static errorCode resolveEntry(EXIPSchema* schema, TreeTable* treeT, unsigned int count, unsigned int currTreeT, TreeTableEntry* entry)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	/* Recursively resolve entries through children first */
	if(entry->child.entry != NULL)
	{
		tmp_err_code = resolveEntry(schema, treeT, count, currTreeT, entry->child.entry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	/* Then through groups (linked by next pointer) */
	if(entry->next != NULL)
	{
		tmp_err_code = resolveEntry(schema, treeT, count, currTreeT, entry->next);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	/* If elements or attributes have a type, link that in as a child */
	if((entry->element == ELEMENT_ELEMENT) || (entry->element == ELEMENT_ATTRIBUTE))
	{
		if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_TYPE]))
		{
			if(entry->child.entry != NULL) // TODO: add debug info
				return UNEXPECTED_ERROR;

			tmp_err_code = lookupGlobalDefinition(schema, treeT, count, currTreeT, &entry->attributePointers[ATTRIBUTE_TYPE], LOOKUP_ELEMENT_TYPE_TYPE, entry);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_REF]))
		{
			if(entry->child.entry != NULL) // TODO: add debug info
				return UNEXPECTED_ERROR;

			tmp_err_code = lookupGlobalDefinition(schema, treeT, count, currTreeT, &entry->attributePointers[ATTRIBUTE_REF], LOOKUP_ELEMENT_TYPE_ELEMENT, entry);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	/* If there are extensions or restrictions, link in their supertype as a base pointer */
	else if(entry->element == ELEMENT_EXTENSION || entry->element == ELEMENT_RESTRICTION)
	{
		if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_BASE]))
		{
			if(entry->supertype.entry != NULL) // TODO: add debug info
				return UNEXPECTED_ERROR;

			tmp_err_code = lookupGlobalDefinition(schema, treeT, count, currTreeT, &entry->attributePointers[ATTRIBUTE_BASE], LOOKUP_ELEMENT_TYPE_SUPER_TYPE, entry);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

static errorCode lookupGlobalDefinition(EXIPSchema* schema, TreeTable* treeT, unsigned int count, unsigned int currTreeT, String* eName, unsigned char elType, TreeTableEntry* entry)
{
	Index i;
	Index globalIndex = INDEX_MAX;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QNameID typeQnameID;
	unsigned int j;

	tmp_err_code = getTypeQName(schema, &treeT[currTreeT], *eName, &typeQnameID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 0; i < count; i++)
	{
		if(treeT[i].globalDefs.targetNsId == typeQnameID.uriId)
		{
			break;
		}
	}

	/** There is no TreeTable with the requested target namespace*/
	if(i == count)
	{
		if(typeQnameID.uriId == 0 || typeQnameID.uriId > 3)
		{
			// The requested target namespace is not a built-in type namespace
			return UNEXPECTED_ERROR;
		}
	}
	else
	{
#if HASH_TABLE_USE == ON
		/* Use the hash table to do a fast lookup of the tree table index matching the global name */
		if(treeT[i].typeTbl != NULL && treeT[i].elemTbl != NULL)
		{
			if(elType == LOOKUP_ELEMENT_TYPE_ELEMENT)
				globalIndex = hashtable_search(treeT[i].elemTbl, &GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr);
			else
				globalIndex = hashtable_search(treeT[i].typeTbl, &GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr);
		}
		else
#endif
		{
			/* Do a linear search through the tree table entry until we find it */
			for(j = 0; j < treeT[i].count; j++)
			{
				if(stringEqual(treeT[i].tree[j].attributePointers[ATTRIBUTE_NAME], GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr))
				{
					globalIndex = j;
					break;
				}
			}
		}
	}

	if(globalIndex == INDEX_MAX)
	{
		// The type must be build-in or the schema is buggy - do nothing here; it will be checked later
		if(elType == LOOKUP_ELEMENT_TYPE_ELEMENT)
			return UNEXPECTED_ERROR; // TODO: add debug info
	}
	else
	{
		if(elType == LOOKUP_ELEMENT_TYPE_SUPER_TYPE)
		{
			entry->supertype.treeT = &treeT[i];
			entry->supertype.entry = &treeT[i].tree[globalIndex];
		}
		else
		{
			entry->child.treeT = &treeT[i];
			entry->child.entry = &treeT[i].tree[globalIndex];
		}
	}

	return ERR_OK;
}

errorCode getTypeQName(EXIPSchema* schema, TreeTable* treeT, const String typeLiteral, QNameID* qNameID)
{
	Index indx;
	String lnStr;
	String uriStr;
	Index i;
	unsigned char pfxFound = FALSE;

	/*
	 * The type literal string passed in will be in the form of either:
	 *     <prefix>:<type name>
	 * or
	 *     <type name>
	 * See if there is a prefix to separate out
	 */
	indx = getIndexOfChar(&typeLiteral, ':');

	if(indx != INDEX_MAX)
	{
		/* There is a prefix defined. Search the table for a namespace URI which matches the prefix */
		/* 'Borrow' the uriStr for search use */
		uriStr.length = indx;
		uriStr.str = typeLiteral.str;

		for(i = 0; i < treeT->globalDefs.pfxNsTable.count; i++)
		{
			if(stringEqual(uriStr, treeT->globalDefs.pfxNsTable.pfxNs[i].pfx))
			{
				pfxFound = TRUE;
				break;
			}
		}

		if(pfxFound)
		{
			/* Replace with URI of actual namespace */
			uriStr = treeT->globalDefs.pfxNsTable.pfxNs[i].ns;
		}
		else
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid schema base type definition\n"));
			return INVALID_EXI_INPUT;
		}

		/* Adjust the length of the remaining type name and assign the string */
		lnStr.length = typeLiteral.length - indx - 1;
		lnStr.str = typeLiteral.str + indx + 1;
	}
	else
	{
		/* There is no prefix defined. Search the table for a namespace URI with a void prefix */
		for(i = 0; i < treeT->globalDefs.pfxNsTable.count; i++)
		{
			if(isStringEmpty(&treeT->globalDefs.pfxNsTable.pfxNs[i].pfx))
			{
				pfxFound = TRUE;
				break;
			}
		}

		if(pfxFound)
		{
			/* Replace with URI of actual namespace (void in this case) */
			uriStr = treeT->globalDefs.pfxNsTable.pfxNs[i].ns;
		}
		else
		{
			/* Replace with URI with empty string */
			getEmptyString(&uriStr);
		}

		/* The type name is the whole string */
		lnStr.length = typeLiteral.length;
		lnStr.str = typeLiteral.str;
	}

	if(!lookupUri(&schema->uriTable, uriStr, &qNameID->uriId))
		return INVALID_EXI_INPUT;
	if(!lookupLn(&schema->uriTable.uri[qNameID->uriId].lnTable, lnStr, &qNameID->lnId))
		return INVALID_EXI_INPUT;

	// http://www.w3.org/TR/xmlschema11-1/#sec-src-resolve
	// Validation checks:
	//	The appropriate case among the following must be true:
	//		4.1 If the namespace name of the QName is absent, then one of the following must be true:
	//		4.1.1 The <schema> element information item of the schema document containing the QName has no targetNamespace [attribute].
	//		4.1.2 The <schema> element information item of the that schema document contains an <import> element information item with no namespace [attribute].
	//		4.2 otherwise the namespace name of the QName is the same as one of the following:
	//		4.2.1 The actual value of the targetNamespace [attribute] of the <schema> element information item of the schema document containing the QName.
	//		4.2.2 The actual value of the namespace [attribute] of some <import> element information item contained in the <schema> element information item of that schema document.
	//		4.2.3 http://www.w3.org/2001/XMLSchema.
	//		4.2.4 http://www.w3.org/2001/XMLSchema-instance.

	if(isStringEmpty(&uriStr)) // 4.1
	{
		// Check 4.1.1 and 4.1.2
		if(treeT->globalDefs.targetNsId != 0 && !checkForImportWithNs(treeT, uriStr))
		{
			return INVALID_EXI_INPUT;
		}
	}
	else if(!stringEqualToAscii(uriStr, XML_SCHEMA_NAMESPACE) && !stringEqualToAscii(uriStr, XML_SCHEMA_INSTANCE)) // 4.2
	{
		// Check 4.2.1 and 4.2.2
		if(treeT->globalDefs.targetNsId != qNameID->uriId && !checkForImportWithNs(treeT, uriStr))
		{
			return INVALID_EXI_INPUT;
		}
	}

	return ERR_OK;
}

static unsigned char checkForImportWithNs(TreeTable* treeT, String ns)
{
	Index i;

	for (i = 0; i < treeT->count; ++i)
	{
		if(treeT->tree[i].element == ELEMENT_INCLUDE ||
				treeT->tree[i].element == ELEMENT_REDEFINE ||
				treeT->tree[i].element == ELEMENT_ANNOTATION)
		{
			continue;
		}
		else if(treeT->tree[i].element == ELEMENT_IMPORT)
		{
			if(stringEqual(treeT->tree[i].attributePointers[ATTRIBUTE_NAMESPACE], ns))
			{
				return TRUE;
			}
		}
		else
		{
			break;
		}
	}

	return FALSE;
}