/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammarRules.c
 * @brief Defines grammar rules related functions
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "grammarRules.h"
#include "eventsEXI.h"
#include "memManagement.h"
#include "ioUtil.h"
#include "sTables.h"
#include "stringManipulate.h"

errorCode insertZeroProduction(DynGrammarRule* rule, EventType eventType, SmallIndex nonTermID, QNameID* qnameId)
{
	if(rule->p1Count == rule->prod1Dim) // The dynamic array rule->prod1 needs to be resized
	{
		void* ptr = EXIP_REALLOC(rule->prod1, sizeof(Production)*(rule->prod1Dim + DEFAULT_PROD_ARRAY_DIM));
		if(ptr == NULL)
			return MEMORY_ALLOCATION_ERROR;

		rule->prod1 = ptr;
		rule->prod1Dim += DEFAULT_PROD_ARRAY_DIM;
	}

	rule->prod1[rule->p1Count].eventType = eventType;
	rule->prod1[rule->p1Count].typeId = INDEX_MAX;
	rule->prod1[rule->p1Count].nonTermID = nonTermID;
	rule->prod1[rule->p1Count].qnameId = *qnameId;

	rule->p1Count += 1;
	rule->bits1 = getBitsNumber(rule->p1Count - 1 + (rule->p2Count + rule->p3Count > 0));
	return ERR_OK;
}

errorCode copyGrammarRule(AllocList* memList, GrammarRule* src, GrammarRule* dest)
{
	Index j = 0;

	dest->bits1 = src->bits1;
	dest->p1Count = src->p1Count;
	dest->p2Count = src->p2Count;
	dest->p3Count = src->p3Count;

	if(dest->p1Count != 0)
	{
		dest->prod1 = (Production*) memManagedAllocate(memList, sizeof(Production)*dest->p1Count);
		if(dest->prod1 == NULL)
			return MEMORY_ALLOCATION_ERROR;

		for(j = 0; j < dest->p1Count; j++)
			dest->prod1[j] = src->prod1[j];
	}
	else
		dest->prod1 = NULL;

	if(dest->p2Count + dest->p3Count != 0)
	{
		dest->prod23 = (Production*) memManagedAllocate(memList, sizeof(Production)*(dest->p2Count + dest->p3Count));
		if(dest->prod23 == NULL)
			return MEMORY_ALLOCATION_ERROR;

		for(j = 0; j < (dest->p2Count + dest->p3Count); j++)
			dest->prod23[j] = src->prod23[j];
	}
	else
		dest->prod23 = NULL;

	return ERR_OK;
}

#if EXIP_DEBUG == ON

errorCode printGrammarRule(SmallIndex nonTermID, GrammarRule* rule, EXIPSchema *schema)
{
	Index j = 0;
	unsigned char b = 0;
	Index tmp_prod_indx = 0;
	Production* tmpProd;
	Index tmpCount;

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\n>RULE\n"));
	DEBUG_MSG(INFO, EXIP_DEBUG, ("NT-%u:", (unsigned int) nonTermID));

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\n"));

	for(b = 0; b < 3; b++)
	{
		if(b == 0)
		{
			tmpProd = rule->prod1;
			tmpCount = rule->p1Count;
		}
		else if(b == 1)
		{
			tmpProd = rule->prod23;
			tmpCount = rule->p2Count;
		}
		else if(b == 2)
		{
			tmpCount = rule->p3Count;
			if(tmpCount)
				tmpProd = rule->prod23 + rule->p2Count;
		}

		for(j = 0; j < tmpCount; j++)
		{
			String *localName = NULL;
			tmp_prod_indx = tmpCount - 1 - j;
			DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));
			switch(tmpProd[tmp_prod_indx].eventType)
			{
				case EVENT_SD:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("SD "));
					break;
				case EVENT_ED:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("ED "));
					break;
				case EVENT_SE_QNAME:
				{
					QNameID *qname = &tmpProd[tmp_prod_indx].qnameId;
					localName = &(GET_LN_URI_P_QNAME(schema->uriTable, qname).lnStr);
					DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (qname: %u:%u) ", (unsigned int) qname->uriId, (unsigned int) qname->lnId));
					break;
				}
				case EVENT_SE_URI:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (uri) "));
					break;
				case EVENT_SE_ALL:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (*) "));
					break;
				case EVENT_EE:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("EE "));
					break;
				case EVENT_AT_QNAME:
				{
					QNameID *qname = &tmpProd[tmp_prod_indx].qnameId;
					localName = &(GET_LN_URI_P_QNAME(schema->uriTable, qname).lnStr);
					DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (qname %u:%u) [%u]", (unsigned int) tmpProd[tmp_prod_indx].qnameId.uriId, (unsigned int) tmpProd[tmp_prod_indx].qnameId.lnId, (unsigned int) tmpProd[tmp_prod_indx].typeId));
					break;
				}
				case EVENT_AT_URI:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (uri) "));
					break;
				case EVENT_AT_ALL:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (*) [%u]", (unsigned int) tmpProd[tmp_prod_indx].typeId));
					break;
				case EVENT_CH:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("CH [%u]", (unsigned int) tmpProd[tmp_prod_indx].typeId));
					break;
				case EVENT_NS:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("NS "));
					break;
				case EVENT_CM:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("CM "));
					break;
				case EVENT_PI:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("PI "));
					break;
				case EVENT_DT:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("DT "));
					break;
				case EVENT_ER:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("ER "));
					break;
				case EVENT_SC:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("SC "));
					break;
				case EVENT_VOID:
					DEBUG_MSG(INFO, EXIP_DEBUG, (" "));
					break;
				default:
					return UNEXPECTED_ERROR;
			}
			DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));
			if(tmpProd[tmp_prod_indx].nonTermID != GR_VOID_NON_TERMINAL)
			{
				DEBUG_MSG(INFO, EXIP_DEBUG, ("NT-%u", (unsigned int) tmpProd[tmp_prod_indx].nonTermID));
			}
			DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));
			if(b > 0)
			{
				DEBUG_MSG(INFO, EXIP_DEBUG, ("%u", (unsigned int) rule->p1Count));
				DEBUG_MSG(INFO, EXIP_DEBUG, ("."));
				if(b > 1)
				{
					DEBUG_MSG(INFO, EXIP_DEBUG, ("%u", (unsigned int) rule->p2Count));
					DEBUG_MSG(INFO, EXIP_DEBUG, ("."));
				}
			}
			DEBUG_MSG(INFO, EXIP_DEBUG, ("%u", (unsigned int) j));

			if (localName != NULL)
			{
				DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));
				printString(localName);
			}
			DEBUG_MSG(INFO, EXIP_DEBUG, ("\n"));
		}
	}
	return ERR_OK;
}

#endif // EXIP_DEBUG
