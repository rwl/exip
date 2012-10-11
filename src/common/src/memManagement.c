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

void* memManagedAllocatePtr(AllocList* list, size_t size, struct reAllocPair* memPair)
{
	void* ptr = memManagedAllocate(list, size);
	if(ptr != NULL)
	{
		memPair->memBlock = list->lastBlock;
		memPair->allocIndx = list->currAllocSlot - 1;
	}
	return ptr;
}

errorCode memManagedReAllocate(void** ptr, size_t size, struct reAllocPair memPair)
{
	void* new_ptr = EXIP_REALLOC(*ptr, size);
	if(new_ptr == NULL)
		return MEMORY_ALLOCATION_ERROR;
	*ptr = new_ptr;

	memPair.memBlock->allocation[memPair.allocIndx] = new_ptr;
	return ERR_OK;
}

void freeAllMem(EXIStream* strm)
{
	if(strm->schema != NULL)
	{
		if(strm->schema->isStatic == TRUE)
		{
			// Reseting the value cross table links to NULL
			Index i;
			Index j;
			for(i = 0; i < strm->schema->uriTable.count; i++)
			{
				for(j = 0; j < strm->schema->uriTable.uri[i].lnTable.count; j++)
				{
					strm->schema->uriTable.uri[i].lnTable.ln[j].vxTable.vx = NULL;
					strm->schema->uriTable.uri[i].lnTable.ln[j].vxTable.count = 0;
				}
			}
		}
		else
			freeAllocList(&strm->schema->memList);
	}
	// Hash tables are freed separately
	// #DOCUMENT#
#if HASH_TABLE_USE == ON
	if(strm->valueTable.hashTbl != NULL)
		hashtable_destroy(strm->valueTable.hashTbl);
#endif

	if(strm->valueTable.value != NULL)
	{
		Index i;
		for(i = 0; i < strm->valueTable.count; i++)
		{
			EXIP_MFREE(strm->valueTable.value[i].valueStr.str);
		}
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
