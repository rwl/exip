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
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	Index ruleId;

	TRY(addEmptyDynEntry(&pg->dynArray, (void **) ruleEntry, &ruleId));

	return createDynArray(&((*ruleEntry)->dynArray), sizeof(Production), prodDim);
}

errorCode addProduction(ProtoRuleEntry* ruleEntry, EventType eventType, Index typeId, QNameID qnameID, SmallIndex nonTermID)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	Production *newProd;
	Index newProdId;

	TRY(addEmptyDynEntry(&ruleEntry->dynArray, (void**)&newProd, &newProdId));

	SET_PROD_EXI_EVENT(newProd->content, eventType);
	newProd->typeId = typeId;
	newProd->qnameId = qnameID;
	SET_PROD_NON_TERM(newProd->content, nonTermID);

	return EXIP_ERR_OK;
}

errorCode convertProtoGrammar(AllocList* memlist, ProtoGrammar* pg, EXIGrammar* exiGrammar)
{
	Index ruleIter;
	Index prodIter;
	uint16_t attrCount;
	boolean hasEE;

	exiGrammar->props = 0;
	SET_SCHEMA_GR(exiGrammar->props);
	SET_CONTENT_INDEX(exiGrammar->props, pg->contentIndex);
	exiGrammar->count = pg->count;

	exiGrammar->rule = (GrammarRule*) memManagedAllocate(memlist, sizeof(GrammarRule)*(pg->count));
	if(exiGrammar->rule == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	for(ruleIter = 0; ruleIter < pg->count; ruleIter++)
	{
		attrCount = 0;
		hasEE = FALSE;

		exiGrammar->rule[ruleIter].production = (Production*) memManagedAllocate(memlist, sizeof(Production)*pg->rule[ruleIter].count);
		if(exiGrammar->rule[ruleIter].production == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		exiGrammar->rule[ruleIter].pCount = pg->rule[ruleIter].count;
		exiGrammar->rule[ruleIter].meta = 0;

		for(prodIter = 0; prodIter < pg->rule[ruleIter].count; prodIter++)
		{
			if(GET_PROD_EXI_EVENT_CLASS(pg->rule[ruleIter].prod[prodIter].content) == EVENT_AT_CLASS)
				attrCount++;
			else if(GET_PROD_EXI_EVENT(pg->rule[ruleIter].prod[prodIter].content) == EVENT_EE)
				hasEE = TRUE;
			exiGrammar->rule[ruleIter].production[prodIter] = pg->rule[ruleIter].prod[prodIter];
		}

		RULE_SET_AT_COUNT(exiGrammar->rule[ruleIter].meta, attrCount);
		if(hasEE)
			RULE_SET_CONTAIN_EE(exiGrammar->rule[ruleIter].meta);
	}

	return EXIP_ERR_OK;
}

errorCode cloneProtoGrammar(ProtoGrammar* src, ProtoGrammar* dest)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ProtoRuleEntry* pRuleEntry;
	Index i;
	Index j;

	TRY(createProtoGrammar(src->count, dest));

	dest->contentIndex = src->contentIndex;
	for (i = 0; i < src->count; i++)
	{
		TRY(addProtoRule(dest, src->rule[i].count, &pRuleEntry));

		for (j = 0; j < src->rule[i].count; j++)
		{
			TRY(addProduction(pRuleEntry, GET_PROD_EXI_EVENT(src->rule[i].prod[j].content), src->rule[i].prod[j].typeId, src->rule[i].prod[j].qnameId, GET_PROD_NON_TERM(src->rule[i].prod[j].content)));
		}
	}

	return EXIP_ERR_OK;
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

#if EXIP_DEBUG == ON && DEBUG_GRAMMAR_GEN == ON

errorCode printProtoGrammarRule(SmallIndex nonTermID, ProtoRuleEntry* rule)
{
	Index j = 0;
	Production* tmpProd;

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\n>RULE\n"));
	DEBUG_MSG(INFO, EXIP_DEBUG, ("NT-%u:", (unsigned int) nonTermID));

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\n"));

	for(j = 0; j < rule->count; j++)
	{
		tmpProd = &rule->prod[j];
		DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));

		switch(GET_PROD_EXI_EVENT(tmpProd->content))
		{
			case EVENT_SD:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SD                    "));
				break;
			case EVENT_ED:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("ED                    "));
				break;
			case EVENT_SE_QNAME:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (qname: %u:%u)     ", (unsigned int) tmpProd->qnameId.uriId, (unsigned int) tmpProd->qnameId.lnId));
				break;
			case EVENT_SE_URI:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (uri)              "));
				break;
			case EVENT_SE_ALL:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (*)                "));
				break;
			case EVENT_EE:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("EE                    "));
				break;
			case EVENT_AT_QNAME:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (qname %u:%u) [%d] ", (unsigned int) tmpProd->qnameId.uriId, (unsigned int) tmpProd->qnameId.lnId, (unsigned int) tmpProd->typeId));
				break;
			case EVENT_AT_URI:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (uri)              "));
				break;
			case EVENT_AT_ALL:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (*)                "));
				break;
			case EVENT_CH:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("CH [%d]               ", (unsigned int) tmpProd->typeId));
				break;
			case EVENT_NS:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("NS                    "));
				break;
			case EVENT_CM:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("CM                    "));
				break;
			case EVENT_PI:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("PI                    "));
				break;
			case EVENT_DT:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("DT                    "));
				break;
			case EVENT_ER:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("ER                    "));
				break;
			case EVENT_SC:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SC                    "));
				break;
			case EVENT_VOID:
				DEBUG_MSG(INFO, EXIP_DEBUG, (" "));
				break;
			default:
				return EXIP_UNEXPECTED_ERROR;
		}
		if(GET_PROD_NON_TERM(tmpProd->content) != GR_VOID_NON_TERMINAL)
		{
			DEBUG_MSG(INFO, EXIP_DEBUG, ("NT-%u", (unsigned int) GET_PROD_NON_TERM(tmpProd->content)));
		}

		DEBUG_MSG(INFO, EXIP_DEBUG, ("\n"));
	}
	return EXIP_ERR_OK;
}

errorCode printProtoGrammar(ProtoGrammar* pgr)
{
	Index j;

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\nProtoGrammar (%d):\n", pgr->contentIndex));

	for(j = 0; j < pgr->count; j++)
	{
		printProtoGrammarRule(j, &pgr->rule[j]);
	}

	return EXIP_ERR_OK;
}

#endif // EXIP_DEBUG
