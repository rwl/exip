/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammarGenerator.c
 * @brief Implementation of functions for generating Schema-informed Grammar definitions
 * @date Nov 22, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "grammarGenerator.h"
#include "treeTableSchema.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "initSchemaInstance.h"
#include "sTables.h"


static int compareLn(const void* lnRow1, const void* lnRow2);
static int compareUri(const void* uriRow1, const void* uriRow2);

/**
 * Sorts the pre-populated entries in the string tables according to the spec.
 */
static void sortUriTable(UriTable* uriTable);

errorCode generateSchemaInformedGrammars(BinaryBuffer* buffers, unsigned int bufCount, SchemaFormat schemaFormat, EXIOptions* opt, EXIPSchema* schema,
		errorCode (*loadSchemaHandler) (String* namespace, String* schemaLocation, BinaryBuffer** buffers, unsigned int* bufCount, SchemaFormat* schemaFormat, EXIOptions** opt))
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	TreeTable* treeT;
	unsigned int treeTCount = bufCount;
	unsigned int i = 0;

	// TODO: again in error cases all the memory must be released

	treeT = (TreeTable*) EXIP_MALLOC(sizeof(TreeTable)*bufCount);
	if(treeT == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(i = 0; i < bufCount; i++)
	{
		TRY(initTreeTable(&treeT[i]));
	}

	TRY(initSchema(schema, INIT_SCHEMA_SCHEMA_ENABLED));

	for(i = 0; i < bufCount; i++)
	{
		TRY(generateTreeTable(buffers[i], schemaFormat, opt, &treeT[i], schema));
	}

	TRY(resolveIncludeImportReferences(&treeT, &treeTCount, loadSchemaHandler));

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	{
		unsigned int j;
		for(i = 0; i < treeTCount; i++)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\nElement tree %d before resolving:\n", i));
			for(j = 0; j < treeT[i].count; j++)
			{
				DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>(Before) Tree Table Entry %d:\n", j));
				printTreeTableEntry(&treeT[i].tree[j], 0, "");
			}

			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("\nPrefix Namespace Table\n"));
			for(j = 0; j < treeT[i].globalDefs.pfxNsTable.count; j++)
			{
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("index: %u\n", j));
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("prf: "));
				printString(&treeT[i].globalDefs.pfxNsTable.pfxNs[j].pfx);
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("\n"));
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("uri: "));
				printString(&treeT[i].globalDefs.pfxNsTable.pfxNs[j].ns);
				DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("\n"));
			}
		}

		DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("\nURI Table\n"));
		for(i = 0; i < schema->uriTable.count; i++)
		{
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("index: %u\n", i));
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("uri: "));
			printString(&schema->uriTable.uri[i].uriStr);
			DEBUG_MSG(ERROR, DEBUG_GRAMMAR_GEN, ("\n"));
		}
	}
#endif

	// Sort the string tables
	sortUriTable(&schema->uriTable);

	// Find the correct targetNsId in the string tables for each TreeTable
	for(i = 0; i < treeTCount; i++)
	{
		if(!lookupUri(&schema->uriTable, treeT[i].globalDefs.targetNs, &treeT[i].globalDefs.targetNsId))
			return UNEXPECTED_ERROR;
	}

	TRY(resolveTypeHierarchy(schema, treeT, treeTCount));

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	{
		unsigned int j;

		for(i = 0; i < treeTCount; i++)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\nElement tree %d after resolving:\n", i));
			for(j = 0; j < treeT[i].count; j++)
			{
				DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>(After) Tree Table Entry %d:\n", j));
				printTreeTableEntry(&treeT[i].tree[j], 0, "");
			}
		}
	}
#endif

	TRY(convertTreeTablesToExipSchema(treeT, treeTCount, schema));

	for(i = 0; i < treeTCount; i++)
	{
		destroyTreeTable(&treeT[i]);
	}

	EXIP_MFREE(treeT);

	return tmp_err_code;
}

void destroySchema(EXIPSchema* schema)
{
	Index i;

	// Freeing the string tables

	for(i = 0; i < schema->uriTable.count; i++)
	{
		if(schema->uriTable.uri[i].pfxTable != NULL)
			EXIP_MFREE(schema->uriTable.uri[i].pfxTable);

		destroyDynArray(&schema->uriTable.uri[i].lnTable.dynArray);
	}

	destroyDynArray(&schema->uriTable.dynArray);
	destroyDynArray(&schema->grammarTable.dynArray);
	destroyDynArray(&schema->simpleTypeTable.dynArray);
	destroyDynArray(&schema->enumTable.dynArray);
	freeAllocList(&schema->memList);
}


static int compareLn(const void* lnRow1, const void* lnRow2)
{
	LnEntry* lnEntry1 = (LnEntry*)lnRow1;
	LnEntry* lnEntry2 = (LnEntry*)lnRow2;

	return stringCompare(lnEntry1->lnStr, lnEntry2->lnStr);
}

static int compareUri(const void* uriRow1, const void* uriRow2)
{
	UriEntry* uriEntry1 = (UriEntry*) uriRow1;
	UriEntry* uriEntry2 = (UriEntry*) uriRow2;

	return stringCompare(uriEntry1->uriStr, uriEntry2->uriStr);
}

static void sortUriTable(UriTable* uriTable)
{
	uint16_t i = 0;

	// First sort the local name tables

	for (i = 0; i < uriTable->count; i++)
	{
		unsigned int initialEntries = 0;

		//	The initialEntries entries in "http://www.w3.org/XML/1998/namespace",
		//	"http://www.w3.org/2001/XMLSchema-instance" and "http://www.w3.org/2001/XMLSchema"
		//  are not sorted
		if(i == XML_NAMESPACE_ID) // "http://www.w3.org/XML/1998/namespace"
		{
			initialEntries = 4;
		}
		else if(i == XML_SCHEMA_INSTANCE_ID) // "http://www.w3.org/2001/XMLSchema-instance"
		{
			initialEntries = 2;
		}
		else if(i == XML_SCHEMA_NAMESPACE_ID) // "http://www.w3.org/2001/XMLSchema"
		{
			initialEntries = 46;
		}

		if(uriTable->uri[i].lnTable.ln != NULL)
			qsort(&uriTable->uri[i].lnTable.ln[initialEntries], uriTable->uri[i].lnTable.count - initialEntries, sizeof(LnEntry), compareLn);
	}

	// Then sort the uri tables

	//	The first four initial entries are not sorted
	//	URI	0	"" [empty string]
	//	URI	1	"http://www.w3.org/XML/1998/namespace"
	//	URI	2	"http://www.w3.org/2001/XMLSchema-instance"
	//	URI	3	"http://www.w3.org/2001/XMLSchema"
	qsort(&uriTable->uri[4], uriTable->count - 4, sizeof(UriEntry), compareUri);
}
