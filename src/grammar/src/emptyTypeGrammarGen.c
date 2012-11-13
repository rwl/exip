/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file emptyTypeGrammarGen.c
 * @brief Implementing the API for generating emptyTypeGrammars from standard grammars
 * @date May 3, 2012
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "emptyTypeGrammarGen.h"
#include "memManagement.h"
#include "ioUtil.h"
#include "grammarRules.h"

extern Production static_prod_empty_part0[1];

errorCode getEmptyTypeGrammar(EXIStream* strm, EXIGrammar* src, EXIGrammar** dest)
{
	SmallIndex i;
	Index p;
	Index destProdIndx;

	*dest = memManagedAllocate(&strm->memList, sizeof(EXIGrammar));
	if(*dest == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*dest)->contentIndex = src->contentIndex;
	(*dest)->count = src->contentIndex;
	(*dest)->props = src->props;

	(*dest)->rule = memManagedAllocate(&strm->memList, sizeof(GrammarRule)*((*dest)->count));
	if((*dest)->rule == NULL)
		return MEMORY_ALLOCATION_ERROR;

	if(WITH_STRICT(strm->header.opts.enumOpt))
	{
		for(i = 0; i < (*dest)->count - 1; i++)
		{
			// Copy all productions in the rules less than contentIndex i.e. (*dest)->count - 1
			// except the  AT(xsi:type) and AT(xsi:nil)

			(*dest)->rule[i].p1Count = 0;

			if(src->rule[i].p1Count > 0)
			{
				(*dest)->rule[i].prod1 = memManagedAllocate(&strm->memList, sizeof(Production)*(src->rule[i].p1Count));
				if((*dest)->rule[i].prod1 == NULL)
					return MEMORY_ALLOCATION_ERROR;

				destProdIndx = 0;
				for(p = 0; p < src->rule[i].p1Count; p++)
				{
					if(src->rule[i].prod1[p].qnameId.uriId == XML_SCHEMA_INSTANCE_ID &&
						(src->rule[i].prod1[p].qnameId.lnId == XML_SCHEMA_INSTANCE_NIL_ID ||
						 src->rule[i].prod1[p].qnameId.lnId == XML_SCHEMA_INSTANCE_TYPE_ID))
					{
						// In case of AT(xsi:type) and AT(xsi:nil) productions, exclude them
						continue;
					}
					else
					{
						(*dest)->rule[i].prod1[destProdIndx] = src->rule[i].prod1[p];
						destProdIndx++;
					}
				}

				(*dest)->rule[i].p1Count = destProdIndx;
			}

			(*dest)->rule[i].p2Count = 0;
			(*dest)->rule[i].p3Count = 0;

			if(src->rule[i].p2Count + src->rule[i].p3Count > 0)
			{
				(*dest)->rule[i].prod23 = memManagedAllocate(&strm->memList, sizeof(Production)*(src->rule[i].p2Count + src->rule[i].p3Count));
				if((*dest)->rule[i].prod23 == NULL)
					return MEMORY_ALLOCATION_ERROR;

				for(p = 0; p < src->rule[i].p2Count + src->rule[i].p3Count; p++)
				{
					if(src->rule[i].prod23[p].qnameId.uriId == XML_SCHEMA_INSTANCE_ID &&
						(src->rule[i].prod23[p].qnameId.lnId == XML_SCHEMA_INSTANCE_NIL_ID ||
						 src->rule[i].prod23[p].qnameId.lnId == XML_SCHEMA_INSTANCE_TYPE_ID))
					{
						// In case of AT(xsi:type) and AT(xsi:nil) productions, exclude them
						continue;
					}
					else
					{
						(*dest)->rule[i].prod23[destProdIndx] = src->rule[i].prod23[p];
						if(p < src->rule[i].p2Count)
							(*dest)->rule[i].p2Count += 1;
						else
							(*dest)->rule[i].p3Count += 1;
					}
				}
			}

			(*dest)->rule[i].bits1 = getBitsNumber(destProdIndx - 1 + ((*dest)->rule[i].p2Count > 0));
		}

		/* The last rule is an empty rule with a single EE production */
		(*dest)->rule[(*dest)->count - 1].prod1 = static_prod_empty_part0;
		(*dest)->rule[(*dest)->count - 1].bits1 = 0;
		(*dest)->rule[(*dest)->count - 1].p1Count = 1;

		(*dest)->rule[(*dest)->count - 1].prod23 = NULL;
		(*dest)->rule[(*dest)->count - 1].p2Count = 0;
		(*dest)->rule[(*dest)->count - 1].p3Count = 0;
	}
	else
	{	// STRICT FALSE mode
		for(i = 0; i < (*dest)->count - 1; i++)
		{
			// Copy all productions in the rules less than contentIndex i.e. (*dest)->count - 1
			// while taking into account that we do not need the Content2 index added during augmentation

			if(src->rule[i].p1Count > 0)
			{
				(*dest)->rule[i].prod1 = memManagedAllocate(&strm->memList, sizeof(Production)*(src->rule[i].p1Count));
				if((*dest)->rule[i].prod1 == NULL)
					return MEMORY_ALLOCATION_ERROR;

				destProdIndx = 0;
				for(p = 0; p < src->rule[i].p1Count; p++)
				{
					if(src->rule[i].prod1[p].eventType == EVENT_AT_ALL ||
							src->rule[i].prod1[p].eventType == EVENT_AT_QNAME ||
							src->rule[i].prod1[p].eventType == EVENT_AT_URI ||
							src->rule[i].prod1[p].eventType == EVENT_EE
					  )
					{
						(*dest)->rule[i].prod1[destProdIndx] = src->rule[i].prod1[p];
						destProdIndx++;
					}
				}

				(*dest)->rule[i].p1Count = destProdIndx;
			}

			if(src->rule[i].p2Count + src->rule[i].p3Count > 0)
			{
				(*dest)->rule[i].prod23 = memManagedAllocate(&strm->memList, sizeof(Production)*(src->rule[i].p2Count + src->rule[i].p3Count));
				if((*dest)->rule[i].prod23 == NULL)
					return MEMORY_ALLOCATION_ERROR;

				destProdIndx = 0;
				for(p = 0; p < src->rule[i].p2Count + src->rule[i].p3Count; p++)
				{
					if(src->rule[i].prod23[p].eventType == EVENT_AT_ALL ||
							src->rule[i].prod23[p].eventType == EVENT_AT_QNAME ||
							src->rule[i].prod23[p].eventType == EVENT_AT_URI ||
							src->rule[i].prod23[p].eventType == EVENT_EE ||
							src->rule[i].prod23[p].eventType == EVENT_NS ||
							src->rule[i].prod23[p].eventType == EVENT_SC
					  )
					{
						(*dest)->rule[i].prod23[destProdIndx] = src->rule[i].prod23[p];
						destProdIndx++;
					}
					else if(src->rule[i].prod23[p].eventType == EVENT_SE_ALL ||
							 src->rule[i].prod23[p].eventType == EVENT_CH ||
							 src->rule[i].prod23[p].eventType == EVENT_ER ||
							 src->rule[i].prod23[p].eventType == EVENT_CM ||
							 src->rule[i].prod23[p].eventType == EVENT_PI)
					{
						(*dest)->rule[i].prod23[destProdIndx] = src->rule[i].prod23[p];
						(*dest)->rule[i].prod23[destProdIndx].nonTermID = (*dest)->count - 1;
						destProdIndx++;
					}

					if(p == src->rule[i].p2Count)
						(*dest)->rule[i].p2Count = destProdIndx;
				}

				(*dest)->rule[i].p3Count = destProdIndx - (*dest)->rule[i].p2Count;
			}

			(*dest)->rule[i].bits1 = getBitsNumber((*dest)->rule[i].p1Count - 1 + ((*dest)->rule[i].p2Count > 0));
		}

		/* The last rule is:
		 *
		 * 	NT-contentIndex-1 :
		 *						EE										0
		 *						SE (*) 				NT-contentIndex-1	1.0
		 *						CH [untyped value] 	NT-contentIndex-1	1.1
		 *						ER 					NT-contentIndex-1	1.2
		 *						CM 					NT-contentIndex-1	1.3.0
		 *						PI 					NT-contentIndex-1	1.3.1
		 *  */
		/* Part 1 */
		(*dest)->rule[(*dest)->count - 1].prod1 = static_prod_empty_part0;
		(*dest)->rule[(*dest)->count - 1].bits1 = 1;
		(*dest)->rule[(*dest)->count - 1].p1Count = 1;

		{ /* Part 2 and 3 */
			int part2count = 2;
			int part3count = 0;

			if(IS_PRESENT(strm->header.opts.preserve, PRESERVE_DTD))
				part2count++;

			if(IS_PRESENT(strm->header.opts.preserve, PRESERVE_COMMENTS))
				part3count++;

			if(IS_PRESENT(strm->header.opts.preserve, PRESERVE_PIS))
				part3count++;

			(*dest)->rule[(*dest)->count - 1].prod23 = memManagedAllocate(&strm->memList, sizeof(Production)*(part2count + part3count));
			if((*dest)->rule[(*dest)->count - 1].prod23 == NULL)
				return MEMORY_ALLOCATION_ERROR;

			(*dest)->rule[(*dest)->count - 1].prod23[part2count-1].eventType = EVENT_SE_ALL;
			(*dest)->rule[(*dest)->count - 1].prod23[part2count-1].nonTermID = (*dest)->count - 1;
			(*dest)->rule[(*dest)->count - 1].prod23[part2count-1].typeId = INDEX_MAX;

			(*dest)->rule[(*dest)->count - 1].prod23[part2count-2].eventType = EVENT_CH;
			(*dest)->rule[(*dest)->count - 1].prod23[part2count-2].nonTermID = (*dest)->count - 1;
			(*dest)->rule[(*dest)->count - 1].prod23[part2count-2].typeId = INDEX_MAX;

			if(IS_PRESENT(strm->header.opts.preserve, PRESERVE_DTD))
			{
				(*dest)->rule[(*dest)->count - 1].prod23[0].eventType = EVENT_ER;
				(*dest)->rule[(*dest)->count - 1].prod23[0].nonTermID = (*dest)->count - 1;
				(*dest)->rule[(*dest)->count - 1].prod23[0].typeId = INDEX_MAX;
			}

			(*dest)->rule[(*dest)->count - 1].p2Count = part2count;

			if(part3count > 0)
			{
				if(IS_PRESENT(strm->header.opts.preserve, PRESERVE_COMMENTS))
				{
					(*dest)->rule[(*dest)->count - 1].prod23[part2count+part3count - 1].eventType = EVENT_CM;
					(*dest)->rule[(*dest)->count - 1].prod23[part2count+part3count - 1].nonTermID = (*dest)->count - 1;
					(*dest)->rule[(*dest)->count - 1].prod23[part2count+part3count - 1].typeId = INDEX_MAX;
				}

				if(IS_PRESENT(strm->header.opts.preserve, PRESERVE_PIS))
				{
					(*dest)->rule[(*dest)->count - 1].prod23[part2count].eventType = EVENT_PI;
					(*dest)->rule[(*dest)->count - 1].prod23[part2count].nonTermID = (*dest)->count - 1;
					(*dest)->rule[(*dest)->count - 1].prod23[part2count].typeId = INDEX_MAX;
				}

				(*dest)->rule[(*dest)->count - 1].p3Count = part3count;
			}
			else
			{
				(*dest)->rule[(*dest)->count - 1].p3Count = 0;
			}
		}
	}

#if DEBUG_GRAMMAR == ON
	{
		unsigned int r;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Empty grammar:\n"));

		for(r = 0; r < (*dest)->count; r++)
		{
			if(printGrammarRule(r, &(*dest)->rule[r], strm->schema) != ERR_OK)
				DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Error printing grammar rule\n"));
		}
	}
#endif

	return ERR_OK;
}
