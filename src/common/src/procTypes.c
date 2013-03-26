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
	opts->schemaIDMode = SCHEMA_ID_ABSENT;
	opts->schemaID.str = NULL;
	opts->schemaID.length = 0;
	opts->drMap = NULL;
}

errorCode checkOptionValues(EXIOptions* opts)
{
	/** Mismatch in the header options.
	 * This error can be due to:
	 * 1) The "alignment" element MUST NOT appear in an EXI options document when the "compression" element is present;
	 * 2) The "strict" element MUST NOT appear in an EXI options document when one of "dtd", "prefixes",
	 * "comments", "pis" or "selfContained" element is present in the same options document. That is only the
	 * element "lexicalValues", from the fidelity options, is permitted to occur in the presence of "strict" element;
	 * 3) The "selfContained" element MUST NOT appear in an EXI options document when one of "compression",
	 * "pre-compression" or "strict" elements are present in the same options document.
	 * 4) The datatypeRepresentationMap option does not take effect when the value of the Preserve.lexicalValues
	 * fidelity option is true (see 6.3 Fidelity Options), or when the EXI stream is a schema-less EXI stream.
	 */
	if(GET_ALIGNMENT(opts->enumOpt) != BIT_PACKED && WITH_COMPRESSION(opts->enumOpt))
	{
		DEBUG_MSG(ERROR, DEBUG_COMMON, ("\n>EXI Options mismatch: both \"alignment\" and \"compression\" set"));
		return HEADER_OPTIONS_MISMATCH;
	}
	else if(WITH_STRICT(opts->enumOpt) &&
			(IS_PRESERVED(opts->preserve, PRESERVE_DTD) ||
			 IS_PRESERVED(opts->preserve, PRESERVE_PREFIXES) ||
			 IS_PRESERVED(opts->preserve, PRESERVE_COMMENTS) ||
			 IS_PRESERVED(opts->preserve, PRESERVE_PIS) ||
			 WITH_SELF_CONTAINED(opts->enumOpt))
			)
	{
		DEBUG_MSG(ERROR, DEBUG_COMMON, ("\n>EXI Options mismatch: both \"strict\" and one or more of \"dtd\", \"prefixes\", \"comments\", \"pis\" or \"selfContained\" are set"));
		return HEADER_OPTIONS_MISMATCH;
	}
	else if(WITH_SELF_CONTAINED(opts->enumOpt) && (WITH_COMPRESSION(opts->enumOpt) || GET_ALIGNMENT(opts->enumOpt) == PRE_COMPRESSION))
	{
		DEBUG_MSG(ERROR, DEBUG_COMMON, ("\n>EXI Options mismatch: both \"selfContained\" and (\"compression\" or \"pre-compression\") are set"));
		return HEADER_OPTIONS_MISMATCH;
	}

	if(opts->drMap != NULL && (IS_PRESERVED(opts->preserve, PRESERVE_LEXVALUES)))
	{
		DEBUG_MSG(WARNING, DEBUG_COMMON, ("\n>The datatypeRepresentationMap option specified but has no effect"));
	}

	return ERR_OK;
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
