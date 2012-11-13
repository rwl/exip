/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammarAugment.c
 * @brief Implementation of Event Code Assignment and Undeclared Productions addition
 * @date Feb 3, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "grammarAugment.h"
#include "ioUtil.h"
#include "eventsEXI.h"
#include "grammarRules.h"
#include "memManagement.h"

#define ATTR_PROD_ARRAY_SIZE 30

errorCode augmentDocGrammar(AllocList* memList, unsigned char preserve, EXIGrammar* docGrammar)
{
	unsigned int prod2number = 0;  // number of productions with event codes with length 2
	unsigned int prod3number = 0; // number of productions with event codes with length 3

	SET_AUGMENTED(docGrammar->props);

	/* Rule for document content */

	if(IS_PRESERVED(preserve, PRESERVE_DTD))
		prod2number += 1;
	if(IS_PRESERVED(preserve, PRESERVE_COMMENTS))
		prod3number += 1;
	if(IS_PRESERVED(preserve, PRESERVE_PIS))
		prod3number += 1;

	if(prod2number + prod3number == 0)
		return ERR_OK; // Nothing is preserved, no need for adding productions
	else
	{
		docGrammar->rule[1].bits1 = getBitsNumber(docGrammar->rule[1].p1Count);
	}

	docGrammar->rule[1].p2Count = 0;
	docGrammar->rule[1].p3Count = 0;

	/* Part 2 and 3*/
	if(prod2number + prod3number > 0)
	{
		docGrammar->rule[1].prod23 = (Production*) memManagedAllocate(memList, sizeof(Production)*(prod2number + prod3number));
		if(docGrammar->rule[1].prod23 == NULL)
			return MEMORY_ALLOCATION_ERROR;

		docGrammar->rule[1].prod23->eventType = EVENT_DT;
		docGrammar->rule[1].prod23->typeId = INDEX_MAX;
		docGrammar->rule[1].prod23->nonTermID = GR_DOC_CONTENT;
		docGrammar->rule[1].p2Count = 1;

		/* Part 3 */
		if(prod3number > 0)
		{
			if(IS_PRESERVED(preserve, PRESERVE_COMMENTS))
			{
				docGrammar->rule[1].prod23[prod2number + prod3number - 1].eventType = EVENT_CM;
				docGrammar->rule[1].prod23[prod2number + prod3number - 1].typeId = INDEX_MAX;
				docGrammar->rule[1].prod23[prod2number + prod3number - 1].nonTermID = GR_DOC_CONTENT;
			}

			if(IS_PRESERVED(preserve, PRESERVE_PIS))
			{
				docGrammar->rule[1].prod23[prod2number].eventType = EVENT_PI;
				docGrammar->rule[1].prod23[prod2number].typeId = INDEX_MAX;
				docGrammar->rule[1].prod23[prod2number].nonTermID = GR_DOC_CONTENT;
			}
			docGrammar->rule[1].p3Count = prod3number;
		}
	}
	/* Rule for document end */

	prod2number = 0;
	if(IS_PRESERVED(preserve, PRESERVE_COMMENTS))
		prod2number += 1;
	if(IS_PRESERVED(preserve, PRESERVE_PIS))
		prod2number += 1;

	if(prod2number == 0)
		return ERR_OK; // CM and PI are not preserved, no need for adding productions
	else
	{
		docGrammar->rule[2].bits1 = 1;
	}

	docGrammar->rule[2].prod23 = (Production*) memManagedAllocate(memList, sizeof(Production)*prod2number);
	if(docGrammar->rule[2].prod23 == NULL)
		return MEMORY_ALLOCATION_ERROR;

	if(IS_PRESERVED(preserve, PRESERVE_COMMENTS))
	{
		docGrammar->rule[2].prod23[prod2number - 1].eventType = EVENT_CM;
		docGrammar->rule[2].prod23[prod2number - 1].typeId = INDEX_MAX;
		docGrammar->rule[2].prod23[prod2number - 1].nonTermID = GR_DOC_CONTENT;
	}

	if(IS_PRESERVED(preserve, PRESERVE_PIS))
	{
		docGrammar->rule[2].prod23[0].eventType = EVENT_PI;
		docGrammar->rule[2].prod23[0].typeId = INDEX_MAX;
		docGrammar->rule[2].prod23[0].nonTermID = GR_DOC_CONTENT;
	}
	docGrammar->rule[2].p2Count = prod2number;

	return ERR_OK;
}

errorCode augmentFragGrammar(AllocList* memList, unsigned char preserve, EXIGrammar* fragGrammar)
{
	unsigned int prod2number = 0;  // number of productions with event codes with length 2

	SET_AUGMENTED(fragGrammar->props);

	if(IS_PRESERVED(preserve, PRESERVE_COMMENTS))
		prod2number += 1;
	if(IS_PRESERVED(preserve, PRESERVE_PIS))
		prod2number += 1;

	if(prod2number == 0)
		return ERR_OK; // CM and PI are not preserved, no need for adding productions
	else
	{
		fragGrammar->rule[1].bits1 = getBitsNumber(fragGrammar->rule[1].p1Count);
	}

	fragGrammar->rule[1].prod23 = (Production*) memManagedAllocate(memList, sizeof(Production)*prod2number);
	if(fragGrammar->rule[1].prod23 == NULL)
		return MEMORY_ALLOCATION_ERROR;

	if(IS_PRESERVED(preserve, PRESERVE_COMMENTS))
	{
		fragGrammar->rule[1].prod23[prod2number - 1].eventType = EVENT_CM;
		fragGrammar->rule[1].prod23[prod2number - 1].typeId = INDEX_MAX;
		fragGrammar->rule[1].prod23[prod2number - 1].nonTermID = GR_DOC_CONTENT;
	}

	if(IS_PRESERVED(preserve, PRESERVE_PIS))
	{
		fragGrammar->rule[1].prod23[0].eventType = EVENT_PI;
		fragGrammar->rule[1].prod23[0].typeId = INDEX_MAX;
		fragGrammar->rule[1].prod23[0].nonTermID = GR_DOC_CONTENT;
	}

	fragGrammar->rule[1].p2Count = prod2number;
	fragGrammar->rule[1].p3Count = 0;

	return ERR_OK;
}

errorCode addUndeclaredProductions(AllocList* memList, unsigned char strict, unsigned char selfContained, unsigned char preserve, EXIGrammar* grammar, SimpleTypeTable* simpleTypeTable)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SmallIndex i = 0;
	Index j = 0;
	uint16_t att = 0;
	int si;
	unsigned int prod2number = 0;  // number of productions with event codes with length 2
	unsigned int prod3number = 0; // number of productions with event codes with length 3

	if(strict == FALSE)
	{
		unsigned char prodEEFound = FALSE;
		Production* attrProdArray[ATTR_PROD_ARRAY_SIZE];
		Index tmp_prod2_indx;
		Index tmp_prod3_indx;

		// #DOCUMENT# IMPORTANT! It must be assured that the schema informed grammars have one empty slot for the rule:  Element i, content2
		grammar->count += 1;
		tmp_err_code = copyGrammarRule(memList, &grammar->rule[grammar->contentIndex], &grammar->rule[grammar->count-1]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for(i = 0; i <= grammar->contentIndex; i++)
		{
			prod2number = 3;   // always add: (AT (*) Element i, j), (SE (*) Element i, content2) and (CH [untyped value] Element i, content2)
			prod3number = 1;   // always add: AT (*) [untyped value] Element i, j
			att = 0;
			prodEEFound = FALSE;
			tmp_prod2_indx = 0;
			tmp_prod3_indx = 0;

			if(IS_PRESERVED(preserve, PRESERVE_DTD))
				prod2number += 1;   // ER Element i, content2

			if(IS_PRESERVED(preserve, PRESERVE_COMMENTS))
				prod3number += 1;   // CM Element i, content2

			if(IS_PRESERVED(preserve, PRESERVE_PIS))
				prod3number += 1;   // PI Element i, content2

			if(i == 0)
			{
				prod2number += 2; //FOR AT(xsi:type) Element i, 0 and AT(xsi:nil) Element i, 0

				if(IS_PRESERVED(preserve, PRESERVE_PREFIXES))  // Element i, 0 : NS Element i, 0
					prod2number += 1;

				if(selfContained == TRUE)
					prod2number += 1;  // Element i, 0 : SC Fragment
			}

			prod2number += 1; // Element i, j : EE - later substracted if EE-production is found

			for(j = 0; j < grammar->rule[i].p1Count; j++)
			{
				if(prodEEFound == FALSE && grammar->rule[i].prod1[j].nonTermID == GR_VOID_NON_TERMINAL && grammar->rule[i].prod1[j].eventType == EVENT_EE)
				{
					prodEEFound = TRUE;
					prod2number -= 1;
				}

				if(grammar->rule[i].prod1[j].eventType == EVENT_AT_QNAME)
				{
					if(att >= ATTR_PROD_ARRAY_SIZE)
						return INCONSISTENT_PROC_STATE;

					attrProdArray[att] = &(grammar->rule[i].prod1[j]);
					att++;
				}
			}

			prod3number += att;

			grammar->rule[i].bits1 = getBitsNumber(grammar->rule[i].p1Count);

			grammar->rule[i].p2Count = prod2number;

			grammar->rule[i].prod23 = (Production*) memManagedAllocate(memList, sizeof(Production)*(prod2number + prod3number));
			if(grammar->rule[i].prod23 == NULL)
				return MEMORY_ALLOCATION_ERROR;

			tmp_prod2_indx = prod2number - 1;

			if(prodEEFound == FALSE) //	There is no production Gi,0 : EE so add one
			{
				grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_EE;
				grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
				grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = GR_VOID_NON_TERMINAL;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
				tmp_prod2_indx --;
			}

			if(i == 0)  // AT(xsi:type) Element i, 0 and AT(xsi:nil) Element i, 0
			{
				grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_AT_QNAME;
				grammar->rule[i].prod23[tmp_prod2_indx].typeId = SIMPLE_TYPE_QNAME;
				grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = 0;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = XML_SCHEMA_INSTANCE_ID; // "http://www.w3.org/2001/XMLSchema-instance"
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = XML_SCHEMA_INSTANCE_TYPE_ID; // type
				tmp_prod2_indx --;

				grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_AT_QNAME;
				grammar->rule[i].prod23[tmp_prod2_indx].typeId = SIMPLE_TYPE_BOOLEAN;
				grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = 0;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = XML_SCHEMA_INSTANCE_ID; // "http://www.w3.org/2001/XMLSchema-instance"
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = XML_SCHEMA_INSTANCE_NIL_ID; // nil
				tmp_prod2_indx --;
			}

			// 	Element i, j : AT (*) Element i, j
			grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_AT_ALL;
			grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = i;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
			tmp_prod2_indx --;

			grammar->rule[i].p3Count = prod3number;

			tmp_prod3_indx = prod3number - 1;
			for(si = att - 1; si >= 0; si--)
			{
				// EVENT_AT_QNAME and VALUE_TYPE_UNTYPED
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].eventType = EVENT_AT_QNAME;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].typeId = INDEX_MAX;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].nonTermID = attrProdArray[si]->nonTermID;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.uriId = attrProdArray[si]->qnameId.uriId;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.lnId = attrProdArray[si]->qnameId.lnId;
				tmp_prod3_indx --;
			}

			// EVENT_AT_ALL and VALUE_TYPE_UNTYPED
			grammar->rule[i].prod23[prod2number+tmp_prod3_indx].eventType = EVENT_AT_ALL;
			grammar->rule[i].prod23[prod2number+tmp_prod3_indx].typeId = INDEX_MAX;
			grammar->rule[i].prod23[prod2number+tmp_prod3_indx].nonTermID = i;
			grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.uriId = URI_MAX;
			grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.lnId = LN_MAX;
			tmp_prod3_indx --;

			if(i == 0)
			{
				if(IS_PRESERVED(preserve, PRESERVE_PREFIXES)) // Element i, 0 : NS Element i, 0
				{
					grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_NS;
					grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
					grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = 0;
					grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
					grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
					tmp_prod2_indx --;
				}

				if(selfContained == TRUE) // Element i, 0 : SC Fragment
				{
					grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_SC;
					grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
					grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = GR_FRAGMENT;
					grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
					grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
					tmp_prod2_indx --;
				}
			}

			// Element i, j : SE (*) Element i, content2
			grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_SE_ALL;
			grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = grammar->count - 1;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
			tmp_prod2_indx --;

			// Element i, j : CH [untyped value] Element i, content2
			grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_CH;
			grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = grammar->count - 1;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
			tmp_prod2_indx --;

			if(IS_PRESERVED(preserve, PRESERVE_DTD)) // Element i, j : ER Element i, content2
			{
				grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_ER;
				grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
				grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = grammar->count - 1;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
				tmp_prod2_indx --;
			}

			if(IS_PRESERVED(preserve, PRESERVE_COMMENTS)) // Element i, j : CM Element i, content2
			{
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].eventType = EVENT_CM;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].typeId = INDEX_MAX;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].nonTermID = grammar->count - 1;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.uriId = URI_MAX;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.lnId = LN_MAX;
				tmp_prod3_indx --;
			}
			if(IS_PRESERVED(preserve, PRESERVE_PIS))  // Element i, j : PI Element i, content2
			{
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].eventType = EVENT_PI;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].typeId = INDEX_MAX;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].nonTermID = grammar->count - 1;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.uriId = URI_MAX;
				grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.lnId = LN_MAX;
				tmp_prod3_indx --;
			}
		}

		for(i = grammar->contentIndex + 1; i < grammar->count; i++)
		{
			prod2number = 2;   // always add: SE (*) Element i, j	and   CH [untyped value] Element i, j
			prod3number = 0;
			prodEEFound = FALSE;
			tmp_prod2_indx = 0;
			tmp_prod3_indx = 0;

			if(IS_PRESERVED(preserve, PRESERVE_DTD))
				prod2number += 1;   // ER Element i, j

			if(IS_PRESERVED(preserve, PRESERVE_COMMENTS))
				prod3number += 1;   // CM Element i, j

			if(IS_PRESERVED(preserve, PRESERVE_PIS))
				prod3number += 1;   // PI Element i, j


			prod2number += 1; // Element i, j : EE - later substracted if EE-production is found

			for(j = 0; j < grammar->rule[i].p1Count; j++)
			{
				if(grammar->rule[i].prod1[j].nonTermID == GR_VOID_NON_TERMINAL && grammar->rule[i].prod1[j].eventType == EVENT_EE)
				{
					prodEEFound = TRUE;
					prod2number -= 1;
					break;
				}
			}

			grammar->rule[i].bits1 = getBitsNumber(grammar->rule[i].p1Count);

			grammar->rule[i].p2Count = prod2number;
			grammar->rule[i].prod23 = (Production*) memManagedAllocate(memList, sizeof(Production)*(prod2number + prod3number));
			if(grammar->rule[i].prod23 == NULL)
				return MEMORY_ALLOCATION_ERROR;

			tmp_prod2_indx = prod2number - 1;

			if(prodEEFound == FALSE) //	There is no production Gi,0 : EE so add one
			{
				grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_EE;
				grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
				grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = GR_VOID_NON_TERMINAL;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
				tmp_prod2_indx --;
			}

			//  Element i, j : SE (*) Element i, j
			grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_SE_ALL;
			grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = i;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
			tmp_prod2_indx --;

			//  Element i, j : CH [untyped value] Element i, j
			grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_CH;
			grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = i;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
			grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
			tmp_prod2_indx --;

			if(IS_PRESERVED(preserve, PRESERVE_DTD))  // Element i, j : ER Element i, j
			{
				grammar->rule[i].prod23[tmp_prod2_indx].eventType = EVENT_ER;
				grammar->rule[i].prod23[tmp_prod2_indx].typeId = INDEX_MAX;
				grammar->rule[i].prod23[tmp_prod2_indx].nonTermID = i;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.uriId = URI_MAX;
				grammar->rule[i].prod23[tmp_prod2_indx].qnameId.lnId = LN_MAX;
				tmp_prod2_indx --;
			}

			if(prod3number > 0)
			{
				grammar->rule[i].p3Count = prod3number;

				tmp_prod3_indx = prod3number - 1;

				if(IS_PRESERVED(preserve, PRESERVE_COMMENTS)) // Element i, j : CM Element i, j
				{
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].eventType = EVENT_CM;
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].typeId = INDEX_MAX;
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].nonTermID = i;
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.uriId = URI_MAX;
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.lnId = LN_MAX;
					tmp_prod3_indx --;
				}

				if(IS_PRESERVED(preserve, PRESERVE_PIS)) // Element i, j : PI Element i, j
				{
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].eventType = EVENT_PI;
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].typeId = INDEX_MAX;
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].nonTermID = i;
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.uriId = URI_MAX;
					grammar->rule[i].prod23[prod2number+tmp_prod3_indx].qnameId.lnId = LN_MAX;
					tmp_prod3_indx --;
				}
			}
			else
			{
				grammar->rule[i].p3Count = 0;
			}
		}
	}
	else // strict == TRUE
	{
		unsigned char subTypeFound = FALSE;

		if(HAS_NAMED_SUB_TYPE_OR_UNION(grammar->props))
		{
			grammar->rule[0].prod23 = (Production*) memManagedAllocate(memList, sizeof(Production) * (1 + IS_NILLABLE(grammar->props)));
			if(grammar->rule[0].prod23 == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar->rule[0].p2Count = 1 + IS_NILLABLE(grammar->props);
			grammar->rule[0].bits1 = getBitsNumber(grammar->rule[0].p1Count);

			grammar->rule[0].prod23[0].eventType = EVENT_AT_QNAME;
			grammar->rule[0].prod23[0].typeId = INDEX_MAX;
			grammar->rule[0].prod23[0].nonTermID = 0;

			// "http://www.w3.org/2001/XMLSchema-instance" = 2
			grammar->rule[0].prod23[0].qnameId.uriId = XML_SCHEMA_INSTANCE_ID;
			// type = 1
			grammar->rule[0].prod23[0].qnameId.lnId = XML_SCHEMA_INSTANCE_TYPE_ID;
			subTypeFound = TRUE;
		}

		if(IS_NILLABLE(grammar->props))
		{
			unsigned char prodIndex = 0;
			if(subTypeFound)
			{
				prodIndex = 1;
			}
			else
			{
				grammar->rule[0].prod23 = (Production*) memManagedAllocate(memList, sizeof(Production));
				if(grammar->rule[0].prod23 == NULL)
					return MEMORY_ALLOCATION_ERROR;

				grammar->rule[0].p2Count = 1;
				grammar->rule[0].bits1 = getBitsNumber(grammar->rule[0].p1Count);
			}

			grammar->rule[0].prod23[prodIndex].eventType = EVENT_AT_QNAME;
			grammar->rule[0].prod23[prodIndex].typeId = SIMPLE_TYPE_BOOLEAN;
			grammar->rule[0].prod23[prodIndex].nonTermID = 0;

			// "http://www.w3.org/2001/XMLSchema-instance" = 2
			grammar->rule[0].prod23[prodIndex].qnameId.uriId = XML_SCHEMA_INSTANCE_ID;
			// nil = 0
			grammar->rule[0].prod23[prodIndex].qnameId.lnId = XML_SCHEMA_INSTANCE_NIL_ID;
		}
	}

	SET_AUGMENTED(grammar->props);

	return ERR_OK;
}

errorCode addUndeclaredProductionsToAll(AllocList* memList, EXIPSchema* schema, EXIOptions* opts)
{
	unsigned int i = 0;
	EXIGrammar* tmpGrammar = NULL;
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	for (i = 0; i < schema->grammarTable.count; i++)
	{
		tmpGrammar = &schema->grammarTable.grammar[i];
		// TODO: check if this if() is really needed
		if(IS_AUGMENTED(tmpGrammar->props) == FALSE)
		{
			tmp_err_code = addUndeclaredProductions(memList, WITH_STRICT(opts->enumOpt), WITH_SELF_CONTAINED(opts->enumOpt), opts->preserve, tmpGrammar, &schema->simpleTypeTable);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}
