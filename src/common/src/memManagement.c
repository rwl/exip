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
 * @file memManagement.c
 * @brief Implementation of handling memory operations - allocation, deallocation etc.
 * @date Oct 25, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "memManagement.h"
#include "hashtable.h"

void initAllocList(AllocList* list)
{
	list->lastBlock = &(list->firstBlock);
	list->firstBlock.currentAlloc = 0;
	list->firstBlock.nextBlock = NULL;
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
	if(strm->vTable->hashTbl != NULL)
		hashtable_destroy(strm->vTable->hashTbl);

	freeAllocList(&(strm->memList));
}

void freeAllocList(AllocList* list)
{
	struct allocBlock* tmpBlock = &(list->firstBlock);
	struct allocBlock* rmBl;
	unsigned int i = 0;

	for(i = 0; i < tmpBlock->currentAlloc; i++)
		EXIP_MFREE(tmpBlock->allocation[i]);

	tmpBlock = tmpBlock->nextBlock;

	while(tmpBlock != NULL)
	{
		for(i = 0; i < tmpBlock->currentAlloc; i++)
			EXIP_MFREE(tmpBlock->allocation[i]);

		rmBl = tmpBlock;
		tmpBlock = tmpBlock->nextBlock;
		EXIP_MFREE(rmBl);
	}
}
