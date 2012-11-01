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
	if(rule->part[0].count == rule->part0Dimension) // The dynamic array rule->prods[0] needs to be resized
	{
		void* ptr = EXIP_REALLOC(rule->part[0].prod, sizeof(Production)*(rule->part0Dimension + DEFAULT_PROD_ARRAY_DIM));
		if(ptr == NULL)
			return MEMORY_ALLOCATION_ERROR;

		rule->part[0].prod = ptr;
		rule->part0Dimension += DEFAULT_PROD_ARRAY_DIM;
	}

	rule->part[0].prod[rule->part[0].count].eventType = eventType;
	rule->part[0].prod[rule->part[0].count].typeId = INDEX_MAX;
	rule->part[0].prod[rule->part[0].count].nonTermID = nonTermID;
	rule->part[0].prod[rule->part[0].count].qnameId = *qnameId;

	rule->part[0].count += 1;
	rule->part[0].bits = getBitsNumber(rule->part[0].count - 1 + (rule->part[1].count + rule->part[2].count > 0));
	return ERR_OK;
}

errorCode copyGrammarRule(AllocList* memList, GrammarRule* src, GrammarRule* dest)
{
	unsigned char b;
	Index j = 0;

	for(b = 0; b < 3; b++)
	{
		dest->part[b].count = src->part[b].count;
		dest->part[b].bits = src->part[b].bits;

		if(src->part[b].count != 0)
		{
			dest->part[b].prod = (Production*) memManagedAllocate(memList, sizeof(Production)*dest->part[b].count);
			if(dest->part[b].prod == NULL)
				return MEMORY_ALLOCATION_ERROR;

			for(j = 0;j < dest->part[b].count; j++)
			{
				dest->part[b].prod[j] = src->part[b].prod[j];
				if(src->part[b].prod[j].nonTermID != GR_VOID_NON_TERMINAL)
					dest->part[b].prod[j].nonTermID = src->part[b].prod[j].nonTermID;
			}
		}
		else
		{
			dest->part[b].prod = NULL;
		}
	}

	return ERR_OK;
}

#if EXIP_DEBUG == ON

errorCode printGrammarRule(SmallIndex nonTermID, GrammarRule* rule, EXIPSchema *schema)
{
	Index j = 0;
	unsigned char b = 0;
	Index tmp_prod_indx = 0;

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\n>RULE\n"));
	DEBUG_MSG(INFO, EXIP_DEBUG, ("NT-%u:", (unsigned int) nonTermID));

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\n"));

	for(b = 0; b < 3; b++)
	{
		for(j = 0; j < rule->part[b].count; j++)
		{
			String *localName = NULL;
			tmp_prod_indx = rule->part[b].count - 1 - j;
			DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));
			switch(rule->part[b].prod[tmp_prod_indx].eventType)
			{
				case EVENT_SD:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("SD "));
					break;
				case EVENT_ED:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("ED "));
					break;
				case EVENT_SE_QNAME:
				{
					QNameID *qname = &rule->part[b].prod[tmp_prod_indx].qnameId;
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
					QNameID *qname = &rule->part[b].prod[tmp_prod_indx].qnameId;
					localName = &(GET_LN_URI_P_QNAME(schema->uriTable, qname).lnStr);
					DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (qname %u:%u) [%u]", (unsigned int) rule->part[b].prod[tmp_prod_indx].qnameId.uriId, (unsigned int) rule->part[b].prod[tmp_prod_indx].qnameId.lnId, (unsigned int) rule->part[b].prod[tmp_prod_indx].typeId));
					break;
				}
				case EVENT_AT_URI:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (uri) "));
					break;
				case EVENT_AT_ALL:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (*) [%u]", (unsigned int) rule->part[b].prod[tmp_prod_indx].typeId));
					break;
				case EVENT_CH:
					DEBUG_MSG(INFO, EXIP_DEBUG, ("CH [%u]", (unsigned int) rule->part[b].prod[tmp_prod_indx].typeId));
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
			if(rule->part[b].prod[tmp_prod_indx].nonTermID != GR_VOID_NON_TERMINAL)
			{
				DEBUG_MSG(INFO, EXIP_DEBUG, ("NT-%u", (unsigned int) rule->part[b].prod[tmp_prod_indx].nonTermID));
			}
			DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));
			if(b > 0)
			{
				DEBUG_MSG(INFO, EXIP_DEBUG, ("%u", (unsigned int) rule->part[0].count));
				DEBUG_MSG(INFO, EXIP_DEBUG, ("."));
				if(b > 1)
				{
					DEBUG_MSG(INFO, EXIP_DEBUG, ("%u", (unsigned int) rule->part[1].count));
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
