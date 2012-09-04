/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file procTypes.c
 * @brief Support functions for the common types
 *
 * @date Sep 6, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "procTypes.h"
#include "memManagement.h"

void makeDefaultOpts(EXIOptions* opts)
{
	opts->enumOpt = 0;
	opts->preserve = 0; // all preserve flags are false by default
	opts->blockSize = 1000000;
	opts->valueMaxLength = INDEX_MAX;
	opts->valuePartitionCapacity = INDEX_MAX;
	opts->user_defined_data = NULL;
	opts->schemaID.str = NULL;
	opts->schemaID.length = 0;
	opts->drMap = NULL;
}

errorCode pushOnStack(GenericStack** stack, void* item)
{
	struct stackNode* node = (struct stackNode*)EXIP_MALLOC(sizeof(struct stackNode));
	if(node == NULL)
		return MEMORY_ALLOCATION_ERROR;

	node->item = item;
	node->nextInStack = *stack;
	*stack = node;
	return ERR_OK;
}

void popFromStack(GenericStack** stack, void** item)
{
	struct stackNode* node;
	if((*stack) == NULL)
	{
		(*item) = NULL;
	}
	else
	{
		node = *stack;
		*stack = (*stack)->nextInStack;

		(*item) = node->item;
		EXIP_MFREE(node);
	}
}

int compareEnumDefs(const void* enum1, const void* enum2)
{
	EnumDefinition* e1 = (EnumDefinition*) enum1;
	EnumDefinition* e2 = (EnumDefinition*) enum2;

	if(e1->typeId < e2->typeId)
		return -1;
	else if(e1->typeId > e2->typeId)
		return 1;

	return 0;
}

errorCode pushOnStackPersistent(GenericStack** stack, void* item, AllocList* memList)
{
	struct stackNode* node = (struct stackNode*)memManagedAllocate(memList, sizeof(struct stackNode));
	if(node == NULL)
		return MEMORY_ALLOCATION_ERROR;

	node->item = item;
	node->nextInStack = *stack;
	*stack = node;
	return ERR_OK;
}

void popFromStackPersistent(GenericStack** stack, void** item)
{
	struct stackNode* node;
	if((*stack) == NULL)
	{
		(*item) = NULL;
	}
	else
	{
		node = *stack;
		*stack = (*stack)->nextInStack;

		(*item) = node->item;
	}
}
