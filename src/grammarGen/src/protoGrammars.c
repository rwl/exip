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

errorCode createProtoGrammar(AllocList* memList, Index rulesDim, Index prodDim, ProtoGrammar* pg)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoRuleEntry* ruleEntry;
	Index ruleId;
	Index i;

	tmp_err_code = createDynArray(&pg->dynArray, sizeof(ProtoRuleEntry), rulesDim, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	pg->contentIndex = 0;
	for (i = 0; i < rulesDim; i++)
	{
		tmp_err_code = addEmptyDynEntry(&pg->dynArray, (void **)&ruleEntry, &ruleId, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = createDynArray(&ruleEntry->dynArray, sizeof(Production), prodDim, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode addProduction(AllocList* memList, ProtoRuleEntry* ruleEntry, EventType eventType, Index typeId, QNameID qnameID, SmallIndex nonTermID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production *newProd;
	Index newProdId;

	tmp_err_code = addEmptyDynEntry(&ruleEntry->dynArray, (void**)&newProd, &newProdId, memList);

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

errorCode cloneProtoGrammar(AllocList* memList, ProtoGrammar* src, ProtoGrammar* dest)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoRuleEntry* ruleEntry;
	Index i;
	Index j;

	tmp_err_code = createProtoGrammar(memList, src->count, src->rule->count, dest);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	dest->contentIndex = src->contentIndex;
	dest->count = src->count;
	for (i = 0; i < src->count; i++)
	{
		ruleEntry = &src->rule[i];
		for (j = 0; j < ruleEntry->count; j++)
		{
			tmp_err_code = addProduction(memList, &dest->rule[i], ruleEntry->prod[j].eventType, ruleEntry->prod[j].typeId, ruleEntry->prod[j].qnameId, ruleEntry->prod[j].nonTermID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}
