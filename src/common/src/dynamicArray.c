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

errorCode createDynArray(DynArray* dynArray, size_t entrySize, uint16_t chunkEntries)
{
	void** base = (void **)(dynArray + 1);
	Index* count = (Index*)(base + 1);

	*base = EXIP_MALLOC(entrySize*chunkEntries);
	if(*base == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	dynArray->entrySize = entrySize;
	*count = 0;
	dynArray->chunkEntries = chunkEntries;
	dynArray->arrayEntries = chunkEntries;

	return EXIP_ERR_OK;
}

errorCode addEmptyDynEntry(DynArray* dynArray, void** entry, Index* entryID)
{
	void** base;
	Index* count;

	if(dynArray == NULL)
		return EXIP_NULL_POINTER_REF;

	base = (void **)(dynArray + 1);
	count = (Index*)(base + 1);
	if(dynArray->arrayEntries == *count)   // The dynamic array must be extended first
	{
		void* ptr = EXIP_REALLOC(*base, dynArray->entrySize * (*count + dynArray->chunkEntries));
		if(ptr == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		*base = ptr;
		dynArray->arrayEntries = dynArray->arrayEntries + dynArray->chunkEntries;
	}

	*entry = (void*)((unsigned char *)(*base) + (*count * dynArray->entrySize));

	*entryID = *count;

	*count += 1;
	return EXIP_ERR_OK;
}

errorCode addDynEntry(DynArray* dynArray, void* entry, Index* entryID)
{
	errorCode tmp_err_code;
	void *emptyEntry;

	TRY(addEmptyDynEntry(dynArray, &emptyEntry, entryID));

	memcpy(emptyEntry, entry, dynArray->entrySize);
	return EXIP_ERR_OK;
}

errorCode delDynEntry(DynArray* dynArray, Index entryID)
{
	void** base;
	Index* count;

	if(dynArray == NULL)
		return EXIP_NULL_POINTER_REF;

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
		return EXIP_OUT_OF_BOUND_BUFFER;

	return EXIP_ERR_OK;
}

void destroyDynArray(DynArray* dynArray)
{
	void** base = (void **)(dynArray + 1);
	EXIP_MFREE(*base);
}
