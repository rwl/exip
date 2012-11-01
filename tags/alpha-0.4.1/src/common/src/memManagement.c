/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file memManagement.c
 * @brief Implementation of handling memory operations - allocation, deallocation etc.
 * @date Oct 25, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "memManagement.h"
#include "hashtable.h"
#include "dynamicArray.h"
#include "sTables.h"
#include "grammars.h"

errorCode initAllocList(AllocList* list)
{
	list->firstBlock = EXIP_MALLOC(sizeof(struct allocBlock));
	if(list->firstBlock == NULL)
		return MEMORY_ALLOCATION_ERROR;
	list->firstBlock->nextBlock = NULL;
	list->lastBlock = list->firstBlock;
	list->currAllocSlot = 0;

	return ERR_OK;
}

void* memManagedAllocate(AllocList* list, size_t size)
{
	void* ptr = EXIP_MALLOC(size);
	if(ptr != NULL)
	{
		if(list->currAllocSlot == ALLOCATION_ARRAY_SIZE)
		{
			struct allocBlock* newBlock = EXIP_MALLOC(sizeof(struct allocBlock));
			if(newBlock == NULL)
				return NULL;

			newBlock->nextBlock = NULL;
			list->lastBlock->nextBlock = newBlock;
			list->lastBlock = newBlock;
			list->currAllocSlot = 0;
		}

		list->lastBlock->allocation[list->currAllocSlot] = ptr;
		list->currAllocSlot += 1;
	}
	return ptr;
}

void freeAllMem(EXIStream* strm)
{
	Index g, i, j;
	DynGrammarRule* tmp_rule;

	// Explicitly free the memory for any build-in grammars
	for(g = strm->schema->staticGrCount; g < strm->schema->grammarTable.count; g++)
	{
		for(i = 0; i < strm->schema->grammarTable.grammar[g].count; i++)
		{
			tmp_rule = &((DynGrammarRule*) strm->schema->grammarTable.grammar[g].rule)[i];
			for(j = 0; j < 3; j++)
			{
				if(tmp_rule->part[j].prod != NULL) {}
					EXIP_MFREE(tmp_rule->part[j].prod);
			}
		}
		EXIP_MFREE(strm->schema->grammarTable.grammar[g].rule);
	}

	strm->schema->grammarTable.count = strm->schema->staticGrCount;

	// Freeing the value cross tables

	for(i = 0; i < strm->schema->uriTable.count; i++)
	{
		for(j = 0; j < strm->schema->uriTable.uri[i].lnTable.count; j++)
		{
			if(GET_LN_URI_IDS(strm->schema->uriTable, i, j).vxTable.vx != NULL)
			{
				destroyDynArray(&GET_LN_URI_IDS(strm->schema->uriTable, i, j).vxTable.dynArray);
			}
			strm->schema->uriTable.uri[i].lnTable.ln[j].vxTable.vx = NULL;
			strm->schema->uriTable.uri[i].lnTable.ln[j].vxTable.count = 0;
		}
	}

	// Hash tables are freed separately
	// #DOCUMENT#
#if HASH_TABLE_USE == ON
	if(strm->valueTable.hashTbl != NULL)
		hashtable_destroy(strm->valueTable.hashTbl);
#endif

	// Freeing the value table if present

	if(strm->valueTable.value != NULL)
	{
		Index i;
		for(i = 0; i < strm->valueTable.count; i++)
		{
			EXIP_MFREE(strm->valueTable.value[i].valueStr.str);
		}

		destroyDynArray(&strm->valueTable.dynArray);
	}

	// In case a default schema was used for this stream
	if(strm->schema->staticGrCount <= SIMPLE_TYPE_COUNT)
	{
		// No schema-informed grammars. This is an empty EXIPSchema container that needs to be freed
		// Freeing the string tables

		for(i = 0; i < strm->schema->uriTable.count; i++)
		{
			if(strm->schema->uriTable.uri[i].pfxTable != NULL)
				EXIP_MFREE(strm->schema->uriTable.uri[i].pfxTable);

			destroyDynArray(&strm->schema->uriTable.uri[i].lnTable.dynArray);
		}

		destroyDynArray(&strm->schema->uriTable.dynArray);
		destroyDynArray(&strm->schema->grammarTable.dynArray);
		if(strm->schema->simpleTypeTable.sType != NULL)
			destroyDynArray(&strm->schema->simpleTypeTable.dynArray);
		freeAllocList(&strm->schema->memList);
	}

	freeAllocList(&(strm->memList));
}

void freeAllocList(AllocList* list)
{
	struct allocBlock* tmpBlock = list->firstBlock;
	struct allocBlock* rmBl;
	unsigned int i = 0;
	unsigned int allocLimitInBlock;

	while(tmpBlock != NULL)
	{
		if(tmpBlock->nextBlock != NULL)
			allocLimitInBlock = ALLOCATION_ARRAY_SIZE;
		else
			allocLimitInBlock = list->currAllocSlot;

		for(i = 0; i < allocLimitInBlock; i++)
			EXIP_MFREE(tmpBlock->allocation[i]);

		rmBl = tmpBlock;
		tmpBlock = tmpBlock->nextBlock;
		EXIP_MFREE(rmBl);
	}
}
