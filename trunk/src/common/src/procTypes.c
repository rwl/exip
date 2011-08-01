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
 * @file procTypes.c
 * @brief Support functions for the common types
 *
 * @date Sep 6, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "procTypes.h"

void makeDefaultOpts(EXIOptions* opts)
{
	opts->enumOpt = 0;
	opts->preserve = 0; // all preserve flags are false by default
	opts->blockSize = 1000000;
	opts->valueMaxLength = SIZE_MAX;
	opts->valuePartitionCapacity = SIZE_MAX;
	opts->user_defined_data = NULL;
	opts->schemaID.str = NULL;
	opts->schemaID.length = 0;
	opts->drMap = NULL;
}

errorCode pushOnStack(GenericStack** stack, void* element)
{
	struct stackNode* node = EXIP_MALLOC(sizeof(struct stackNode));
	if(node == NULL)
		return MEMORY_ALLOCATION_ERROR;

	node->element = element;
	node->nextInStack = *stack;
	*stack = node;
	return ERR_OK;
}

void popFromStack(GenericStack** stack, void** element)
{
	struct stackNode* node;
	if((*stack) == NULL)
	{
		(*element) = NULL;
	}
	else
	{
		node = *stack;
		*stack = (*stack)->nextInStack;

		(*element) = node->element;
		EXIP_MFREE(node);
	}
}
