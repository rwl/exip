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
 * @version 0.5
 * @par[Revision] $Id$
 */

#include "treeTableSchema.h"
#include "memManagement.h"
#include "stringManipulate.h"
#include "sTables.h"

#define TREE_TABLE_ENTRY_COUNT 200

#define LOOKUP_TYPE       0
#define LOOKUP_REF        1
#define LOOKUP_SUPER_TYPE 2
#define LOOKUP_SUBSTITUTION 3

extern const String XML_NAMESPACE;
extern const String XML_SCHEMA_INSTANCE;
extern const String XML_SCHEMA_NAMESPACE;

extern const String URI_1_PFX; // The 'xml' prefix string

#define MAX_INCLUDE_COUNT 50

/**
 * Finds a global TreeEntry corresponding to a entry-name eName and links it to the entry child or supertype
 * depending on the elType (LOOKUP_ELEMENT_TYPE_TYPE, LOOKUP_ELEMENT_TYPE_ELEMENT, LOOKUP_ELEMENT_TYPE_SUPER_TYPE or LOOKUP_SUBSTITUTION)
 */
static errorCode lookupGlobalDefinition(EXIPSchema* schema, TreeTable* treeT, unsigned int count, unsigned int currTreeT, String* eName, unsigned char elType, TreeTableEntry* entry, SubstituteTable* subsTbl);

/**
 * Check if there exists an <xs:import> with a given namespace attribute
 */
static boolean checkForImportWithNs(TreeTable* treeT, String ns);

errorCode resolveIncludeImportReferences(EXIPSchema* schema, TreeTable** treeT, unsigned int* count,
		errorCode (*loadSchemaHandler) (String* namespace, String* schemaLocation, BinaryBuffer** buffers, unsigned int* bufCount, SchemaFormat* schemaFormat, EXIOptions** opt))
{
	// check if all <include> & <import> are refering to treeTable instances. If not call loadSchemaHandler
	// for each unresolved referece. Build new corresponding treeTable incstance and add it to treeT array
	// Use realloc and update the array count argument

	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned int i, j;
	boolean treeTfound = FALSE;
	Index g;
	BinaryBuffer* newBuffers;
	unsigned int bufCount = 0;
	SchemaFormat schemaFormat;
	EXIOptions* options;
	unsigned int includeTblIndex[MAX_INCLUDE_COUNT];
	unsigned int includeCnt, tmpInclCnt;


	for(i = 0; i < *count; i++)
	{
		includeCnt = 0;
		for (g = 0; g < (*treeT)[i].count; ++g)
		{
			if((*treeT)[i].tree[g].element == ELEMENT_REDEFINE ||
				(*treeT)[i].tree[g].element == ELEMENT_ANNOTATION)
			{
				continue;
			}
			else if((*treeT)[i].tree[g].element == ELEMENT_INCLUDE)
			{
				if(includeCnt >= MAX_INCLUDE_COUNT)
					return EXIP_OUT_OF_BOUND_BUFFER;
				includeTblIndex[includeCnt] = g;
				includeCnt++;
			}
			else if((*treeT)[i].tree[g].element == ELEMENT_IMPORT)
			{
				treeTfound = FALSE;
				for(j = 0; j < *count; j++)
				{
					if(j != i && stringEqual((*treeT)[i].tree[g].attributePointers[ATTRIBUTE_NAMESPACE], (*treeT)[j].globalDefs.targetNs))
					{
						treeTfound = TRUE;
						break;
					}
				}

				if(treeTfound == FALSE)
				{
					if(loadSchemaHandler != NULL)
					{
						TRY(loadSchemaHandler(&(*treeT)[i].tree[g].attributePointers[ATTRIBUTE_NAMESPACE], &(*treeT)[i].tree[g].attributePointers[ATTRIBUTE_SCHEMA_LOCATION], &newBuffers,
								&bufCount, &schemaFormat, &options));

						if(bufCount > 0)
						{
							void *tmpPtr;
							unsigned int n;

							tmpPtr = EXIP_REALLOC(*treeT, sizeof(TreeTable)*(*count + bufCount));
							if(tmpPtr == NULL)
								return EXIP_MEMORY_ALLOCATION_ERROR;

							*treeT = tmpPtr;

							for(n = *count; n < *count + bufCount; n++)
							{
								TRY(initTreeTable(&(*treeT[n])));
							}

							for(n = *count; n < *count + bufCount; n++)
							{
								TRY(generateTreeTable(newBuffers[n-*count], schemaFormat, options, &(*treeT[n]), schema));
							}

							*count = (*count + bufCount);
						}
						else
						{
							DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, ("> loadSchemaHandler returns an empty list of BinaryBuffers"));
						}
					}
					else
					{
						DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("> An <import> statement in schema cannot be resolved"));
						return EXIP_INVALID_EXIP_CONFIGURATION;
					}
				}
			}
			else
			{
				break;
			}
		}

		if(includeCnt > 0)
		{
			treeTfound = FALSE;
			tmpInclCnt = 0;
			for(j = 0; j < *count; j++)
			{
				if(j != i && stringEqual((*treeT)[i].globalDefs.targetNs, (*treeT)[j].globalDefs.targetNs))
				{
					tmpInclCnt++;
					if(tmpInclCnt < includeCnt)
						continue;
					else
					{
						treeTfound = TRUE;
						break;
					}
				}
			}

			if(treeTfound == FALSE)
			{
				if(loadSchemaHandler != NULL)
				{
					for(j = tmpInclCnt; j < includeCnt; j++)
					{
						TRY(loadSchemaHandler(NULL, &(*treeT)[i].tree[includeTblIndex[j]].attributePointers[ATTRIBUTE_SCHEMA_LOCATION], &newBuffers,
								&bufCount, &schemaFormat, &options));

						if(bufCount > 0)
						{
							void *tmpPtr;
							unsigned int n;

							tmpPtr = EXIP_REALLOC(*treeT, sizeof(TreeTable)*(*count + bufCount));
							if(tmpPtr == NULL)
								return EXIP_MEMORY_ALLOCATION_ERROR;

							*treeT = tmpPtr;

							for(n = *count; n < *count + bufCount; n++)
							{
								TRY(initTreeTable(&(*treeT[n])));
							}

							for(n = *count; n < *count + bufCount; n++)
							{
								TRY(generateTreeTable(newBuffers[n-*count], schemaFormat, options, &(*treeT[n]), schema));
							}

							*count = (*count + bufCount);
						}
						else
						{
							DEBUG_MSG(WARNING, DEBUG_GRAMMAR_GEN, ("> loadSchemaHandler returns an empty list of BinaryBuffers"));
						}
					}
				}
				else
				{
					DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("> An <include> statement in schema cannot be resolved"));
					return EXIP_INVALID_EXIP_CONFIGURATION;
				}
			}
		}
	}

	return EXIP_OK;
}


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
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	TRY(initAllocList(&treeT->memList));

	TRY(createDynArray(&treeT->dynArray, sizeof(TreeTableEntry), TREE_TABLE_ENTRY_COUNT));
	TRY(createDynArray(&treeT->globalDefs.pfxNsTable.dynArray, sizeof(PfxNsEntry), 10));

	treeT->globalDefs.attrFormDefault = UNQUALIFIED;
	treeT->globalDefs.elemFormDefault = UNQUALIFIED;
	treeT->count = 0;
	treeT->globalDefs.targetNsId = 0;

#if HASH_TABLE_USE
	// TODO: conditionally create the table, only if the schema is big.
	// How to determine when the schema is big?
	treeT->typeTbl = create_hashtable(INITIAL_HASH_TABLE_SIZE, djbHash, stringEqual);
	if(treeT->typeTbl == NULL)
		return EXIP_HASH_TABLE_ERROR;

	treeT->elemTbl = create_hashtable(INITIAL_HASH_TABLE_SIZE, djbHash, stringEqual);
	if(treeT->elemTbl == NULL)
		return EXIP_HASH_TABLE_ERROR;

	treeT->attrTbl = create_hashtable(INITIAL_HASH_TABLE_SIZE, djbHash, stringEqual);
	if(treeT->attrTbl == NULL)
		return EXIP_HASH_TABLE_ERROR;

	treeT->groupTbl = create_hashtable(INITIAL_HASH_TABLE_SIZE, djbHash, stringEqual);
	if(treeT->groupTbl == NULL)
		return EXIP_HASH_TABLE_ERROR;

	treeT->attrGroupTbl = create_hashtable(INITIAL_HASH_TABLE_SIZE, djbHash, stringEqual);
	if(treeT->attrGroupTbl == NULL)
		return EXIP_HASH_TABLE_ERROR;
#endif

	return EXIP_OK;
}

void destroyTreeTable(TreeTable* treeT)
{
	destroyDynArray(&treeT->dynArray);
	destroyDynArray(&treeT->globalDefs.pfxNsTable.dynArray);

#if HASH_TABLE_USE
	if(treeT->typeTbl != NULL)
		hashtable_destroy(treeT->typeTbl);

	if(treeT->elemTbl != NULL)
		hashtable_destroy(treeT->elemTbl);

	if(treeT->attrTbl != NULL)
		hashtable_destroy(treeT->attrTbl);

	if(treeT->groupTbl != NULL)
		hashtable_destroy(treeT->groupTbl);

	if(treeT->attrGroupTbl != NULL)
		hashtable_destroy(treeT->attrGroupTbl);
#endif
	freeAllocList(&treeT->memList);
}

errorCode resolveTypeHierarchy(EXIPSchema* schema, TreeTable* treeT, unsigned int count, SubstituteTable* subsTbl)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	Index j;
	unsigned int i;

	/* For every tree table (XSD file) */
	for(i = 0; i < count; i++)
	{
		/* For every three table global entry */
		for(j = 0; j < treeT[i].count; j++)
		{
			/* If element and substitution group defined -> add the substitute to the head in SubstituteTable.
			 * This check is here as the substitution group elements are only global elements by definition */
			if(treeT[i].tree[j].element == ELEMENT_ELEMENT && !isStringEmpty(&treeT[i].tree[j].attributePointers[ATTRIBUTE_SUBSTITUTION_GROUP]))
			{
			#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
				DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Substitution group "));
				printString(&treeT[i].tree[j].attributePointers[ATTRIBUTE_SUBSTITUTION_GROUP]);
				DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("  added: "));
				printString(&treeT[i].tree[j].attributePointers[ATTRIBUTE_NAME]);
			#endif

				TRY(lookupGlobalDefinition(schema, treeT, count, i, &treeT[i].tree[j].attributePointers[ATTRIBUTE_SUBSTITUTION_GROUP], LOOKUP_SUBSTITUTION, &treeT[i].tree[j], subsTbl));
			}

			TRY(resolveEntry(schema, treeT, count, i, &treeT[i].tree[j]));
		}
	}
	return EXIP_OK;
}

static errorCode resolveEntry(EXIPSchema* schema, TreeTable* treeT, unsigned int count, unsigned int currTreeT, TreeTableEntry* entry)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	/* Recursively resolve entries through children first */
	if(entry->child.entry != NULL)
	{
		TRY(resolveEntry(schema, treeT, count, currTreeT, entry->child.entry));
	}

	/* Then through groups (linked by next pointer) */
	if(entry->next != NULL)
	{
		TRY(resolveEntry(schema, treeT, count, currTreeT, entry->next));
	}

	/* If elements, attributes, groups or attributeGroups -> link type to global types or
	 * link ref to global definitions. */
	if((entry->element == ELEMENT_ELEMENT) ||
		(entry->element == ELEMENT_ATTRIBUTE) ||
		(entry->element == ELEMENT_GROUP) ||
		(entry->element == ELEMENT_ATTRIBUTE_GROUP))
	{
		if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_TYPE]))
		{
			if(entry->child.entry != NULL) // TODO: add debug info
				return EXIP_UNEXPECTED_ERROR;

			TRY(lookupGlobalDefinition(schema, treeT, count, currTreeT, &entry->attributePointers[ATTRIBUTE_TYPE], LOOKUP_TYPE, entry, NULL));
		}
		else if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_REF]))
		{
			if(entry->child.entry != NULL) // TODO: add debug info
				return EXIP_UNEXPECTED_ERROR;

			TRY(lookupGlobalDefinition(schema, treeT, count, currTreeT, &entry->attributePointers[ATTRIBUTE_REF], LOOKUP_REF, entry, NULL));
		}
	}
	/* If there are extensions or restrictions, link their base type to the supertype pointer */
	else if(entry->element == ELEMENT_EXTENSION || entry->element == ELEMENT_RESTRICTION)
	{
		if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_BASE]))
		{
			if(entry->supertype.entry != NULL) // TODO: add debug info
				return EXIP_UNEXPECTED_ERROR;

			TRY(lookupGlobalDefinition(schema, treeT, count, currTreeT, &entry->attributePointers[ATTRIBUTE_BASE], LOOKUP_SUPER_TYPE, entry, NULL));
		}
	}
	/* If there is a list, link its itemType to the supertype pointer */
	else if(entry->element == ELEMENT_LIST)
	{
		if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_ITEM_TYPE]))
		{
			if(entry->supertype.entry != NULL) // TODO: add debug info
				return EXIP_UNEXPECTED_ERROR;

			TRY(lookupGlobalDefinition(schema, treeT, count, currTreeT, &entry->attributePointers[ATTRIBUTE_ITEM_TYPE], LOOKUP_SUPER_TYPE, entry, NULL));
		}
	}

	return EXIP_OK;
}

static errorCode lookupGlobalDefinition(EXIPSchema* schema, TreeTable* treeT, unsigned int count, unsigned int currTreeT, String* eName, unsigned char elType, TreeTableEntry* entry, SubstituteTable* subsTbl)
{
	Index i;
	Index globalIndex = INDEX_MAX;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	QNameID typeQnameID;
	unsigned int j;

	TRY(getTypeQName(schema, &treeT[currTreeT], *eName, &typeQnameID));

	for(i = 0; i < count; i++)
	{
		if(treeT[i].globalDefs.targetNsId == typeQnameID.uriId)
		{
#if HASH_TABLE_USE
			/* Use the hash table to do a fast lookup of the tree table index matching the global name */
			if(treeT[i].typeTbl != NULL && treeT[i].elemTbl != NULL && treeT[i].attrTbl != NULL &&
					treeT[i].groupTbl != NULL && treeT[i].attrGroupTbl != NULL)
			{
				if(elType == LOOKUP_TYPE || elType == LOOKUP_SUPER_TYPE)
					globalIndex = hashtable_search(treeT[i].typeTbl, GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr);
				else if(entry->element == ELEMENT_ELEMENT) // LOOKUP_REF ELEMENT
					globalIndex = hashtable_search(treeT[i].elemTbl, GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr);
				else if(entry->element == ELEMENT_ATTRIBUTE) // LOOKUP_REF ATTRIBUTE
					globalIndex = hashtable_search(treeT[i].attrTbl, GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr);
				else if(entry->element == ELEMENT_GROUP) // LOOKUP_REF GROUP
					globalIndex = hashtable_search(treeT[i].groupTbl, GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr);
				else if(entry->element == ELEMENT_ATTRIBUTE_GROUP) // LOOKUP_REF ATTRIBUTE_GROUP
					globalIndex = hashtable_search(treeT[i].attrGroupTbl, GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr);

			}
			else
#endif
			{
				/* Do a linear search through the tree table entry until we find it */
				for(j = 0; j < treeT[i].count; j++)
				{
					if(stringEqual(treeT[i].tree[j].attributePointers[ATTRIBUTE_NAME], GET_LN_URI_QNAME(schema->uriTable, typeQnameID).lnStr))
					{
						/* If search for global type and the found entry is indeed a simple or complex type */
						if((elType == LOOKUP_TYPE || elType == LOOKUP_SUPER_TYPE) && (treeT[i].tree[j].element == ELEMENT_SIMPLE_TYPE || treeT[i].tree[j].element == ELEMENT_COMPLEX_TYPE))
						{
							globalIndex = j;
							break;
						}
						/* Or if search for REF to element/attr/group/groupAttr or substitutionHead => the entry type must be the same as the one we found*/
						else if((elType == LOOKUP_REF || elType == LOOKUP_SUBSTITUTION) || (treeT[i].tree[j].element == entry->element))
						{
							globalIndex = j;
							break;
						}
					}
				}
			}
			if(globalIndex != INDEX_MAX)
				break;
		}
	}

	/** There is no TreeTable with the requested target namespace */
	if(i == count)
	{
		if(typeQnameID.uriId == 0 || typeQnameID.uriId > 3)
		{
			// The requested target namespace is not a built-in type namespace
			return EXIP_UNEXPECTED_ERROR;
		}
	}

	if(globalIndex == INDEX_MAX)
	{
		// For types search: The type must be build-in or the schema is buggy - do nothing here; it will be checked later
		if(elType == LOOKUP_REF || elType == LOOKUP_SUBSTITUTION)
			return EXIP_UNEXPECTED_ERROR; // TODO: add debug info
	}
	else
	{
		if(elType == LOOKUP_SUPER_TYPE)
		{
			entry->supertype.treeT = &treeT[i];
			entry->supertype.entry = &treeT[i].tree[globalIndex];
		}
		else if (elType == LOOKUP_SUBSTITUTION)
		{
			Index s;
			boolean isHeadFound = FALSE;
			QualifiedTreeTableEntry qEntry;

			assert(subsTbl != NULL);

			/* I> check if the head already exists in subsTbl*/
			for(s = 0; s < subsTbl->count; s++)
			{
				if(subsTbl->head[s].headId.uriId == typeQnameID.uriId && subsTbl->head[s].headId.lnId == typeQnameID.lnId)
				{
					isHeadFound = TRUE;
					break;
				}
			}

			/* II> if no such head exists create one */
			if(!isHeadFound)
			{
				SubtGroupHead newHead;
				newHead.headId = typeQnameID;
				TRY(createDynArray(&newHead.dynArray, sizeof(QualifiedTreeTableEntry), 5));

				TRY(addDynEntry(&subsTbl->dynArray, (void*) &newHead, &s));
			}

			/* III> add the substitute to the head */
			qEntry.treeT = &treeT[currTreeT];
			qEntry.entry = entry;
			TRY(addDynEntry(&subsTbl->head[s].dynArray, (void*) &qEntry, &s));
		}
		else
		{
			entry->child.treeT = &treeT[i];
			entry->child.entry = &treeT[i].tree[globalIndex];
		}
	}

	return EXIP_OK;
}

errorCode getTypeQName(EXIPSchema* schema, TreeTable* treeT, const String typeLiteral, QNameID* qNameID)
{
	Index indx;
	String lnStr;
	String uriStr;
	Index i;
	boolean pfxFound = FALSE;

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
			// see http://www.w3.org/XML/1998/namespace
			if(stringEqual(uriStr, URI_1_PFX))
			{
				uriStr = XML_NAMESPACE;
			}
			else
			{
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, (">Invalid schema base type definition\n"));
				return EXIP_INVALID_EXI_INPUT;
			}
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
		return EXIP_INVALID_EXI_INPUT;
	if(!lookupLn(&schema->uriTable.uri[qNameID->uriId].lnTable, lnStr, &qNameID->lnId))
		return EXIP_INVALID_EXI_INPUT;

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
			return EXIP_INVALID_EXI_INPUT;
		}
	}
	else if(!stringEqual(uriStr, XML_SCHEMA_NAMESPACE) && !stringEqual(uriStr, XML_SCHEMA_INSTANCE)) // 4.2
	{
		// Check 4.2.1 and 4.2.2
		if(treeT->globalDefs.targetNsId != qNameID->uriId && !checkForImportWithNs(treeT, uriStr))
		{
			return EXIP_INVALID_EXI_INPUT;
		}
	}

	return EXIP_OK;
}

static boolean checkForImportWithNs(TreeTable* treeT, String ns)
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

errorCode getNsList(TreeTable* treeT, String nsList, NsTable* nsTable)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	Index dummy_elemID;

	if(isStringEmpty(&nsList))
		return EXIP_OK;
	else
	{
		Index sChIndex;
		String attrNamespece;

		attrNamespece.length = nsList.length;
		attrNamespece.str = nsList.str;
		sChIndex = getIndexOfChar(&attrNamespece, ' ');

		while(sChIndex != INDEX_MAX)
		{
			String tmpNS;
			tmpNS.length = sChIndex;
			tmpNS.str = attrNamespece.str;

			if(stringEqualToAscii(tmpNS, "##targetNamespace"))
			{
				tmpNS = treeT->globalDefs.targetNs;
			}
			if(stringEqualToAscii(tmpNS, "##local"))
			{
				getEmptyString(&tmpNS);
			}

			TRY(addDynEntry(&nsTable->dynArray, &tmpNS, &dummy_elemID));

			attrNamespece.length = attrNamespece.length - sChIndex - 1;
			attrNamespece.str = attrNamespece.str + sChIndex + 1;

			sChIndex = getIndexOfChar(&attrNamespece, ' ');
		}

		TRY(addDynEntry(&nsTable->dynArray, &attrNamespece, &dummy_elemID));
	}
	return EXIP_OK;
}
