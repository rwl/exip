/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file dynamicArray.c
 * @brief Implementation for untyped dynamic array
 *
 * @date Jan 25, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "dynamicArray.h"
#include "memManagement.h"

errorCode createDynArray(DynArray* dynArray, size_t entrySize, uint16_t chunkEntries, AllocList* memList)
{
	void** base = (void **)(dynArray + 1);
	Index* count = (Index*)(base + 1);

	*base = memManagedAllocatePtr(memList, entrySize*chunkEntries, &dynArray->memPair);
	if(*base == NULL)
		return MEMORY_ALLOCATION_ERROR;

	dynArray->entrySize = entrySize;
	*count = 0;
	dynArray->chunkEntries = chunkEntries;
	dynArray->arrayEntries = chunkEntries;

	return ERR_OK;
}

errorCode addEmptyDynEntry(DynArray* dynArray, void** entry, Index* entryID, AllocList* memList)
{
	void** base;
	Index* count;
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(dynArray == NULL)
		return NULL_POINTER_REF;

	base = (void **)(dynArray + 1);
	count = (Index*)(base + 1);
	if(dynArray->arrayEntries == *count)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate(base, dynArray->entrySize * (*count + dynArray->chunkEntries), dynArray->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		dynArray->arrayEntries = dynArray->arrayEntries + dynArray->chunkEntries;
	}

	*entry = (void*)((unsigned char *)(*base) + (*count * dynArray->entrySize));

	*entryID = *count;

	*count += 1;
	return ERR_OK;
}

errorCode addDynEntry(DynArray* dynArray, void* entry, Index* entryID, AllocList* memList)
{
	errorCode tmp_err_code;
	void *emptyEntry;

	tmp_err_code = addEmptyDynEntry(dynArray, &emptyEntry, entryID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	memcpy(emptyEntry, entry, dynArray->entrySize);
	return ERR_OK;
}

errorCode delDynEntry(DynArray* dynArray, Index entryID)
{
	void** base;
	Index* count;

	if(dynArray == NULL)
		return NULL_POINTER_REF;

	base = (void **)(dynArray + 1);
	count = (Index*)(base + 1);

	if(entryID == *count - 1)
	{
		*count -= 1;
	}
	else if(*count - 1 - entryID >= 0)
	{
		/* Shuffle the array down to fill the removed entry */
		memcpy(((unsigned char *)*base) + entryID * dynArray->entrySize,
			   ((unsigned char *)*base) + entryID * dynArray->entrySize + dynArray->entrySize,
			   (*count - 1 - entryID) * dynArray->entrySize);
		*count -= 1;
	}
	else
		return OUT_OF_BOUND_BUFFER;

	return ERR_OK;
}