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
	list->lastBlock = list->firstBlock;
	list->firstBlock->currentAlloc = 0;
	list->firstBlock->nextBlock = NULL;

	return ERR_OK;
}

void* memManagedAllocate(AllocList* list, size_t size)
{
	void* ptr = EXIP_MALLOC(size);
	if(ptr != NULL)
	{
		if(list->lastBlock->currentAlloc == ALLOCATION_ARRAY_SIZE)
		{
			struct allocBlock* newBlock = EXIP_MALLOC(sizeof(struct allocBlock));
			if(newBlock == NULL)
				return NULL;

			newBlock->currentAlloc = 0;
			newBlock->nextBlock = NULL;

			list->lastBlock->nextBlock = newBlock;
			list->lastBlock = newBlock;
		}

		list->lastBlock->allocation[list->lastBlock->currentAlloc] = ptr;
		list->lastBlock->currentAlloc += 1;
	}
	return ptr;
}

void* memManagedAllocatePtr(AllocList* list, size_t size, struct reAllocPair* memPair)
{
	void* ptr = memManagedAllocate(list, size);
	if(ptr != NULL)
	{
		memPair->memBlock = list->lastBlock;
		memPair->allocIndx = list->lastBlock->currentAlloc - 1;
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

void freeLastManagedAlloc(AllocList* list)
{
	if(list->lastBlock->currentAlloc > 0)
	{
		EXIP_MFREE(list->lastBlock->allocation[list->lastBlock->currentAlloc - 1]);
		list->lastBlock->currentAlloc -= 1;
	}
}


void freeAllMem(EXIStream* strm)
{
	// Hash tables are freed separately
	// #DOCUMENT#
#if HASH_TABLE_USE == ON
	if(strm->valueTable.hashTbl != NULL)
		hashtable_destroy(strm->valueTable.hashTbl);
#endif
	freeAllocList(&(strm->memList));
}

void freeAllocList(AllocList* list)
{
	struct allocBlock* tmpBlock = list->firstBlock;
	struct allocBlock* rmBl;
	unsigned int i = 0;

	if(tmpBlock == NULL) // Empty AllocList
		return;

	for(i = 0; i < tmpBlock->currentAlloc; i++)
		EXIP_MFREE(tmpBlock->allocation[i]);

	tmpBlock = tmpBlock->nextBlock;
	EXIP_MFREE(list->firstBlock);

	while(tmpBlock != NULL)
	{
		for(i = 0; i < tmpBlock->currentAlloc; i++)
			EXIP_MFREE(tmpBlock->allocation[i]);

		rmBl = tmpBlock;
		tmpBlock = tmpBlock->nextBlock;
		EXIP_MFREE(rmBl);
	}
}
