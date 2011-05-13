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
 * @file protoGrammars.c
 * @brief EXI Proto-Grammars implementation
 * @date May 11, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#define RULE_EXTENSION_FACTOR 20
#define PRODUTION_EXTENSION_FACTOR 20

#include "protoGrammars.h"

errorCode createProtoGrammar(AllocList* memlist, unsigned int rulesDim, unsigned int prodDim, ProtoGrammar** result)
{
	unsigned int i;

	(*result) = (ProtoGrammar*) memManagedAllocate(memlist, sizeof(ProtoGrammar));
	if((*result) == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->contentIndex = 0;
	(*result)->prodCount = (unsigned int*) memManagedAllocate(memlist, sizeof(unsigned int)*rulesDim);
	if((*result)->prodCount == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->prodDim = (unsigned int*) memManagedAllocate(memlist, sizeof(unsigned int)*rulesDim);
	if((*result)->prodDim == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->prodMemPair = (struct reAllocPair*) memManagedAllocate(memlist, sizeof(struct reAllocPair)*rulesDim);
	if((*result)->prodMemPair == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->prods = (Production**) memManagedAllocatePtr(memlist, sizeof(Production*)*rulesDim, &((*result)->rulesMemPair));
	if((*result)->prods == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->rulesCount = 0;
	(*result)->rulesDim = rulesDim;

	for(i = 0; i < rulesDim; i++)
	{
		(*result)->prodCount[i] = 0;
		(*result)->prodDim[i] = prodDim;
		(*result)->prods[i] = (Production*) memManagedAllocatePtr(memlist, sizeof(Production)*prodDim, &((*result)->prodMemPair[i]));
		if((*result)->prods[i] == NULL)
			return MEMORY_ALLOCATION_ERROR;
	}

	return ERR_OK;
}

errorCode addProtoRule(AllocList* memlist, ProtoGrammar* pg)
{
	if(pg->rulesDim == pg->rulesCount) // An extension is needed
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		unsigned int i = 0;
		unsigned int* oldProdCount = pg->prodCount;
		unsigned int* oldProdDim = pg->prodDim;
		struct reAllocPair* oldProdMemPair = pg->prodMemPair;

		pg->rulesDim += RULE_EXTENSION_FACTOR;
		pg->prodCount = (unsigned int*) memManagedAllocate(memlist, sizeof(unsigned int)*(pg->rulesDim));
		if(pg->prodCount == NULL)
			return MEMORY_ALLOCATION_ERROR;

		pg->prodDim = (unsigned int*) memManagedAllocate(memlist, sizeof(unsigned int)*(pg->rulesDim));
		if(pg->prodDim == NULL)
			return MEMORY_ALLOCATION_ERROR;

		pg->prodMemPair = (struct reAllocPair*) memManagedAllocate(memlist, sizeof(struct reAllocPair)*(pg->rulesDim));
		if(pg->prodMemPair == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_err_code = memManagedReAllocate((void *) pg->prods, sizeof(Production*)*(pg->rulesDim), pg->rulesMemPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for(i = 0; i < pg->rulesCount; i++)
		{
			pg->prodCount[i] = oldProdCount[i];
			pg->prodDim[i] = oldProdDim[i];
			pg->prodMemPair[i] = oldProdMemPair[i];
		}
	}

	pg->rulesCount += 1;

	return ERR_OK;
}

errorCode addProductionToAProtoRule(AllocList* memlist, ProtoGrammar* pg, unsigned int ruleIndex, EXIEvent event, uint16_t uriRowID, size_t lnRowID, size_t nonTermID)
{
	if(pg->prodCount[ruleIndex] == pg->prodDim[ruleIndex]) // An extension is needed
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;

		pg->prodDim[ruleIndex] = pg->prodCount[ruleIndex] + PRODUTION_EXTENSION_FACTOR;
		tmp_err_code = memManagedReAllocate((void *) pg->prods[ruleIndex], sizeof(Production)*(pg->prodDim[ruleIndex]), pg->prodMemPair[ruleIndex]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	pg->prods[ruleIndex][pg->prodCount[ruleIndex]].event = event;
	pg->prods[ruleIndex][pg->prodCount[ruleIndex]].lnRowID = lnRowID;
	pg->prods[ruleIndex][pg->prodCount[ruleIndex]].uriRowID = uriRowID;
	pg->prods[ruleIndex][pg->prodCount[ruleIndex]].nonTermID = nonTermID;

	pg->prodCount[ruleIndex] += 1;

	return ERR_OK;
}
