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
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	SET_PROD_EXI_EVENT(newProd->content, eventType);
	newProd->typeId = typeId;
	newProd->qnameId = qnameID;
	SET_PROD_NON_TERM(newProd->content, nonTermID);

	return ERR_OK;
}

errorCode convertProtoGrammar(AllocList* memlist, ProtoGrammar* pg, EXIGrammar* exiGrammar)
{
	Index ruleIter;
	Index prodIter;

	exiGrammar->props = 0;
	SET_SCHEMA_GR(exiGrammar->props);
	SET_CONTENT_INDEX(exiGrammar->props, pg->contentIndex);
	exiGrammar->count = pg->count;

	// #DOCUMENT# one more rule slot is created as it can be needed for addUndeclaredProductions
	exiGrammar->rule = (GrammarRule*) memManagedAllocate(memlist, sizeof(GrammarRule)*(pg->count + 1));
	if(exiGrammar->rule == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(ruleIter = 0; ruleIter < pg->count; ruleIter++)
	{
		/* Part 1 */
		exiGrammar->rule[ruleIter].production = (Production*) memManagedAllocate(memlist, sizeof(Production)*pg->rule[ruleIter].count);
		if(exiGrammar->rule[ruleIter].production == NULL)
			return MEMORY_ALLOCATION_ERROR;

		exiGrammar->rule[ruleIter].pCount = pg->rule[ruleIter].count;

		for(prodIter = 0; prodIter < pg->rule[ruleIter].count; prodIter++)
		{
			exiGrammar->rule[ruleIter].production[prodIter] = pg->rule[ruleIter].prod[prodIter];
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
			tmp_err_code = addProduction(pRuleEntry, GET_PROD_EXI_EVENT(src->rule[i].prod[j].content), src->rule[i].prod[j].typeId, src->rule[i].prod[j].qnameId, GET_PROD_NON_TERM(src->rule[i].prod[j].content));
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
