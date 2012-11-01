/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file protoGrammars.c
 * @brief EXI Proto-Grammars implementation
 * @date May 11, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#define RULE_EXTENSION_FACTOR 20
#define PRODUTION_EXTENSION_FACTOR 20

#include "protoGrammars.h"
#include "memManagement.h"
#include "ioUtil.h"
#include "dynamicArray.h"

errorCode createProtoGrammar(Index rulesDim, ProtoGrammar* pg)
{
	pg->contentIndex = 0;

	return createDynArray(&pg->dynArray, sizeof(ProtoRuleEntry), rulesDim);
}

errorCode addProtoRule(ProtoGrammar* pg, Index prodDim, ProtoRuleEntry** ruleEntry)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Index ruleId;

	tmp_err_code = addEmptyDynEntry(&pg->dynArray, (void **) ruleEntry, &ruleId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return createDynArray(&((*ruleEntry)->dynArray), sizeof(Production), prodDim);
}

errorCode addProduction(ProtoRuleEntry* ruleEntry, EventType eventType, Index typeId, QNameID qnameID, SmallIndex nonTermID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production *newProd;
	Index newProdId;

	tmp_err_code = addEmptyDynEntry(&ruleEntry->dynArray, (void**)&newProd, &newProdId);

	newProd->eventType = eventType;
	newProd->typeId = typeId;
	newProd->qnameId = qnameID;
	newProd->nonTermID = nonTermID;

	return ERR_OK;
}

errorCode convertProtoGrammar(AllocList* memlist, ProtoGrammar* pg, EXIGrammar* exiGrammar)
{
	Index ruleIter;
	Index prodIter;

	exiGrammar->props = 0;
	SET_SCHEMA(exiGrammar->props);
	exiGrammar->contentIndex = pg->contentIndex;
	exiGrammar->count = pg->count;

	// #DOCUMENT# one more rule slot is created as it can be needed for addUndeclaredProductions
	exiGrammar->rule = (GrammarRule*) memManagedAllocate(memlist, sizeof(GrammarRule)*(pg->count + 1));
	if(exiGrammar->rule == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(ruleIter = 0; ruleIter < pg->count; ruleIter++)
	{
		/* Initialize Part 2 */
		exiGrammar->rule[ruleIter].part[1].prod = NULL;
		exiGrammar->rule[ruleIter].part[1].count = 0;
		exiGrammar->rule[ruleIter].part[1].bits = 0;

		/* Initialize Part 3 */
		exiGrammar->rule[ruleIter].part[2].prod = NULL;
		exiGrammar->rule[ruleIter].part[2].count = 0;
		exiGrammar->rule[ruleIter].part[2].bits = 0;

		/* Part 1 */
		exiGrammar->rule[ruleIter].part[0].prod = (Production*) memManagedAllocate(memlist, sizeof(Production)*pg->rule[ruleIter].count);
		if(exiGrammar->rule[ruleIter].part[0].prod == NULL)
			return MEMORY_ALLOCATION_ERROR;

		exiGrammar->rule[ruleIter].part[0].count = pg->rule[ruleIter].count;
		exiGrammar->rule[ruleIter].part[0].bits = getBitsNumber(pg->rule[ruleIter].count - 1);

		for(prodIter = 0; prodIter < pg->rule[ruleIter].count; prodIter++)
		{
			exiGrammar->rule[ruleIter].part[0].prod[prodIter] = pg->rule[ruleIter].prod[prodIter];
		}
	}

	return ERR_OK;
}

errorCode cloneProtoGrammar(ProtoGrammar* src, ProtoGrammar* dest)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoRuleEntry* pRuleEntry;
	Index i;
	Index j;

	tmp_err_code = createProtoGrammar(src->count, dest);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	dest->contentIndex = src->contentIndex;
	for (i = 0; i < src->count; i++)
	{
		tmp_err_code = addProtoRule(dest, src->rule[i].count, &pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for (j = 0; j < src->rule[i].count; j++)
		{
			tmp_err_code = addProduction(pRuleEntry, src->rule[i].prod[j].eventType, src->rule[i].prod[j].typeId, src->rule[i].prod[j].qnameId, src->rule[i].prod[j].nonTermID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}


void destroyProtoGrammar(ProtoGrammar* pg)
{
	Index i;
	for (i = 0; i < pg->count; i++)
	{
		destroyDynArray(&pg->rule[i].dynArray);
	}
	destroyDynArray(&pg->dynArray);
}
