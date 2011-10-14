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
 * @file grammarAugment.c
 * @brief Implementation of Event Code Assignment and Undeclared Productions addition
 * @date Feb 3, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "grammarAugment.h"
#include "ioUtil.h"
#include "eventsEXI.h"
#include "grammarRules.h"
#include "memManagement.h"

#define ATTR_PROD_ARRAY_SIZE 30

errorCode addUndeclaredProductions(AllocList* memList, unsigned char strict, unsigned char selfContained, unsigned char preserve, EXIGrammar* grammar, SimpleType* simpleTypeArray, uint16_t sTypeArraySize)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t i = 0;
	size_t j = 0;
	uint16_t att = 0;
	unsigned int prod2number = 0;  // number of productions with event codes with length 2
	unsigned int prod3number = 0; // number of productions with event codes with length 3
	EXIEvent tmpEvent;

	if(strict == FALSE)
	{
		unsigned char prodEEFound = FALSE;
		Production* attrProdArray[ATTR_PROD_ARRAY_SIZE];
		size_t tmp_prod2_indx;
		size_t tmp_prod3_indx;

		// #DOCUMENT# IMPORTANT! It must be assured that the schema informed grammars have one empty slot for the rule:  Element i, content2
		grammar->rulesDimension += 1;
		tmp_err_code = copyGrammarRule(memList, &grammar->ruleArray[grammar->contentIndex], &grammar->ruleArray[grammar->rulesDimension-1]);
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

			for(j = 0; j < grammar->ruleArray[i].prodCounts[0]; j++)
			{
				if(prodEEFound == FALSE && grammar->ruleArray[i].prodArrays[0][j].nonTermID == GR_VOID_NON_TERMINAL && grammar->ruleArray[i].prodArrays[0][j].event.eventType == EVENT_EE)
				{
					prodEEFound = TRUE;
					prod2number -= 1;
				}

				if(grammar->ruleArray[i].prodArrays[0][j].event.eventType == EVENT_AT_QNAME)
				{
					if(att >= ATTR_PROD_ARRAY_SIZE)
						return INCONSISTENT_PROC_STATE;

					attrProdArray[att] = &(grammar->ruleArray[i].prodArrays[0][j]);
					att++;
				}
			}

			prod3number += att;

			grammar->ruleArray[i].bits[0] = getBitsNumber(grammar->ruleArray[i].prodCounts[0]);

			grammar->ruleArray[i].bits[1] = getBitsNumber(prod2number);
			grammar->ruleArray[i].prodCounts[1] = prod2number;
			grammar->ruleArray[i].prodArrays[1] = (Production*) memManagedAllocate(memList, sizeof(Production)*prod2number);
			if(grammar->ruleArray[i].prodArrays[1] == NULL)
				return MEMORY_ALLOCATION_ERROR;

			tmp_prod2_indx = prod2number - 1;

			if(prodEEFound == FALSE) //	There is no production Gi,0 : EE so add one
			{
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = getEventDefType(EVENT_EE);
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = GR_VOID_NON_TERMINAL;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
				tmp_prod2_indx --;
			}

			if(i == 0)  // AT(xsi:type) Element i, 0 and AT(xsi:nil) Element i, 0
			{
				tmpEvent.eventType = EVENT_AT_QNAME;
				tmpEvent.valueType.exiType = VALUE_TYPE_QNAME;
				tmpEvent.valueType.simpleTypeID = UINT16_MAX;

				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = 0;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = 2; // "http://www.w3.org/2001/XMLSchema-instance"
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = 1; // type
				tmp_prod2_indx --;

				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = 0;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = 2; // "http://www.w3.org/2001/XMLSchema-instance"
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = 0; // nil
				tmp_prod2_indx --;
			}

			// 	Element i, j : AT (*) Element i, j
			tmpEvent.eventType = EVENT_AT_ALL;
			tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
			tmpEvent.valueType.simpleTypeID = UINT16_MAX;

			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = i;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
			tmp_prod2_indx --;

			grammar->ruleArray[i].bits[2] = getBitsNumber(prod3number);
			grammar->ruleArray[i].prodCounts[2] = prod3number;
			grammar->ruleArray[i].prodArrays[2] = (Production*) memManagedAllocate(memList, sizeof(Production)*prod3number);
			if(grammar->ruleArray[i].prodArrays[2] == NULL)
				return MEMORY_ALLOCATION_ERROR;

			tmp_prod3_indx = prod3number - 1;

			for(j = 0; j < att; j++)
			{
				tmpEvent.eventType = EVENT_AT_QNAME;
				tmpEvent.valueType.exiType = VALUE_TYPE_UNTYPED;
				tmpEvent.valueType.simpleTypeID = UINT16_MAX;

				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].event = tmpEvent;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].nonTermID = attrProdArray[j]->nonTermID;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].uriRowID = attrProdArray[j]->uriRowID;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].lnRowID = attrProdArray[j]->lnRowID;
				tmp_prod3_indx --;
			}

			tmpEvent.eventType = EVENT_AT_ALL;
			tmpEvent.valueType.exiType = VALUE_TYPE_UNTYPED;
			tmpEvent.valueType.simpleTypeID = UINT16_MAX;

			grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].event = tmpEvent;
			grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].nonTermID = i;
			grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].uriRowID = UINT16_MAX;
			grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].lnRowID = SIZE_MAX;
			tmp_prod3_indx --;

			if(i == 0)
			{
				if(IS_PRESERVED(preserve, PRESERVE_PREFIXES)) // Element i, 0 : NS Element i, 0
				{
					tmpEvent.eventType = EVENT_NS;
					tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
					tmpEvent.valueType.simpleTypeID = UINT16_MAX;

					grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
					grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = 0;
					grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
					grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
					tmp_prod2_indx --;
				}

				if(selfContained == TRUE) // Element i, 0 : SC Fragment
				{
					tmpEvent.eventType = EVENT_SC;
					tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
					tmpEvent.valueType.simpleTypeID = UINT16_MAX;

					grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
					grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = GR_FRAGMENT;
					grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
					grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
					tmp_prod2_indx --;
				}
			}

			// Element i, j : SE (*) Element i, content2
			tmpEvent.eventType = EVENT_SE_ALL;
			tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
			tmpEvent.valueType.simpleTypeID = UINT16_MAX;

			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = grammar->rulesDimension - 1;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
			tmp_prod2_indx --;

			// Element i, j : CH [untyped value] Element i, content2
			tmpEvent.eventType = EVENT_CH;
			tmpEvent.valueType.exiType = VALUE_TYPE_UNTYPED;
			tmpEvent.valueType.simpleTypeID = UINT16_MAX;

			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = grammar->rulesDimension - 1;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
			tmp_prod2_indx --;

			if(IS_PRESERVED(preserve, PRESERVE_DTD)) // Element i, j : ER Element i, content2
			{
				tmpEvent.eventType = EVENT_ER;
				tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
				tmpEvent.valueType.simpleTypeID = UINT16_MAX;

				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = grammar->rulesDimension - 1;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
				tmp_prod2_indx --;
			}

			if(IS_PRESERVED(preserve, PRESERVE_COMMENTS)) // Element i, j : CM Element i, content2
			{
				tmpEvent.eventType = EVENT_CM;
				tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
				tmpEvent.valueType.simpleTypeID = UINT16_MAX;

				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].event = tmpEvent;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].nonTermID = grammar->rulesDimension - 1;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].uriRowID = UINT16_MAX;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].lnRowID = SIZE_MAX;
				tmp_prod3_indx --;
			}
			if(IS_PRESERVED(preserve, PRESERVE_PIS))  // Element i, j : PI Element i, content2
			{
				tmpEvent.eventType = EVENT_PI;
				tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
				tmpEvent.valueType.simpleTypeID = UINT16_MAX;

				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].event = tmpEvent;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].nonTermID = grammar->rulesDimension - 1;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].uriRowID = UINT16_MAX;
				grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].lnRowID = SIZE_MAX;
				tmp_prod3_indx --;
			}
		}

		for(i = grammar->contentIndex + 1; i < grammar->rulesDimension; i++)
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

			for(j = 0; j < grammar->ruleArray[i].prodCounts[0]; j++)
			{
				if(grammar->ruleArray[i].prodArrays[0][j].nonTermID == GR_VOID_NON_TERMINAL && grammar->ruleArray[i].prodArrays[0][j].event.eventType == EVENT_EE)
				{
					prodEEFound = TRUE;
					prod2number -= 1;
					break;
				}
			}

			grammar->ruleArray[i].bits[0] = getBitsNumber(grammar->ruleArray[i].prodCounts[0]);

			grammar->ruleArray[i].bits[1] = getBitsNumber(prod2number);
			grammar->ruleArray[i].prodCounts[1] = prod2number;
			grammar->ruleArray[i].prodArrays[1] = (Production*) memManagedAllocate(memList, sizeof(Production)*prod2number);
			if(grammar->ruleArray[i].prodArrays[1] == NULL)
				return MEMORY_ALLOCATION_ERROR;

			tmp_prod2_indx = prod2number - 1;

			if(prodEEFound == FALSE) //	There is no production Gi,0 : EE so add one
			{
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = getEventDefType(EVENT_EE);
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = GR_VOID_NON_TERMINAL;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
				tmp_prod2_indx --;
			}

			//  Element i, j : SE (*) Element i, j
			tmpEvent.eventType = EVENT_SE_ALL;
			tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
			tmpEvent.valueType.simpleTypeID = UINT16_MAX;

			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = i;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
			tmp_prod2_indx --;

			//  Element i, j : CH [untyped value] Element i, j
			tmpEvent.eventType = EVENT_CH;
			tmpEvent.valueType.exiType = VALUE_TYPE_UNTYPED;
			tmpEvent.valueType.simpleTypeID = UINT16_MAX;

			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = i;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
			grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
			tmp_prod2_indx --;

			if(IS_PRESERVED(preserve, PRESERVE_DTD))  // Element i, j : ER Element i, j
			{
				tmpEvent.eventType = EVENT_SE_ALL;
				tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
				tmpEvent.valueType.simpleTypeID = UINT16_MAX;

				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].event = tmpEvent;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].nonTermID = i;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].uriRowID = UINT16_MAX;
				grammar->ruleArray[i].prodArrays[1][tmp_prod2_indx].lnRowID = SIZE_MAX;
				tmp_prod2_indx --;
			}

			if(prod3number > 0)
			{
				grammar->ruleArray[i].bits[2] = getBitsNumber(prod3number);
				grammar->ruleArray[i].prodCounts[2] = prod3number;
				grammar->ruleArray[i].prodArrays[2] = (Production*) memManagedAllocate(memList, sizeof(Production)*prod3number);
				if(grammar->ruleArray[i].prodArrays[2] == NULL)
					return MEMORY_ALLOCATION_ERROR;

				tmp_prod3_indx = prod3number - 1;

				if(IS_PRESERVED(preserve, PRESERVE_COMMENTS)) // Element i, j : CM Element i, j
				{
					tmpEvent.eventType = EVENT_CM;
					tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
					tmpEvent.valueType.simpleTypeID = UINT16_MAX;

					grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].event = tmpEvent;
					grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].nonTermID = i;
					grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].uriRowID = UINT16_MAX;
					grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].lnRowID = SIZE_MAX;
					tmp_prod3_indx --;
				}

				if(IS_PRESERVED(preserve, PRESERVE_PIS)) // Element i, j : PI Element i, j
				{
					tmpEvent.eventType = EVENT_PI;
					tmpEvent.valueType.exiType = VALUE_TYPE_NONE;
					tmpEvent.valueType.simpleTypeID = UINT16_MAX;

					grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].event = tmpEvent;
					grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].nonTermID = i;
					grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].uriRowID = UINT16_MAX;
					grammar->ruleArray[i].prodArrays[2][tmp_prod3_indx].lnRowID = SIZE_MAX;
					tmp_prod3_indx --;
				}
			}
			else
			{
				grammar->ruleArray[i].bits[2] = 0;
				grammar->ruleArray[i].prodCounts[2] = 0;
				grammar->ruleArray[i].prodArrays[2] = NULL;
			}
		}
	}
	else // strict == TRUE
	{
		unsigned char isNillable;
		unsigned char subTypeFound = FALSE;

		isNillable = grammar->isNillable;

		// If Tk either has named sub-types
		// TODO: This only works for simple types now
		for(i = 0; i < grammar->rulesDimension; i++)
		{
			for(j = 0; j < grammar->ruleArray[i].prodCounts[0]; j++)
			{
				if(grammar->ruleArray[i].prodArrays[0][j].event.eventType == EVENT_CH)
				{
					if(grammar->ruleArray[i].prodArrays[0][j].event.valueType.simpleTypeID < sTypeArraySize &&
							(simpleTypeArray[grammar->ruleArray[i].prodArrays[0][j].event.valueType.simpleTypeID].facetPresenceMask & TYPE_FACET_NAMED_SUBTYPE) > 0)
					{
						grammar->ruleArray[0].prodArrays[1] = (Production*) memManagedAllocate(memList, sizeof(Production) * (1 + isNillable));
						if(grammar->ruleArray[0].prodArrays[1] == NULL)
							return MEMORY_ALLOCATION_ERROR;

						grammar->ruleArray[0].prodCounts[1] = 1 + isNillable;
						grammar->ruleArray[0].bits[1] = isNillable;
						grammar->ruleArray[0].bits[0] = getBitsNumber(grammar->ruleArray[0].prodCounts[0]);

						grammar->ruleArray[0].prodArrays[1][0].event.eventType = EVENT_AT_QNAME;
						grammar->ruleArray[0].prodArrays[1][0].event.valueType.exiType = VALUE_TYPE_NONE;
						grammar->ruleArray[0].prodArrays[1][0].event.valueType.simpleTypeID = UINT16_MAX;
						grammar->ruleArray[0].prodArrays[1][0].nonTermID = 0;

						// "http://www.w3.org/2001/XMLSchema-instance" = 2
						grammar->ruleArray[0].prodArrays[1][0].uriRowID = 2;
						// type = 1
						grammar->ruleArray[0].prodArrays[1][0].lnRowID = 1;
						subTypeFound = TRUE;
						break;
					}
				}
			}
			if(subTypeFound)
				break;
		}

		// TODO: or is a simple type definition of which {variety} is union,
		//       add the following production to Element_i)

		if(isNillable == TRUE)
		{
			unsigned char prodIndex = 0;
			if(subTypeFound)
			{
				prodIndex = 1;
			}
			else
			{
				grammar->ruleArray[0].prodArrays[1] = (Production*) memManagedAllocate(memList, sizeof(Production));
				if(grammar->ruleArray[0].prodArrays[1] == NULL)
					return MEMORY_ALLOCATION_ERROR;

				grammar->ruleArray[0].prodCounts[1] = 1;
				grammar->ruleArray[0].bits[1] = 0;
				grammar->ruleArray[0].bits[0] = getBitsNumber(grammar->ruleArray[0].prodCounts[0]);
			}

			grammar->ruleArray[0].prodArrays[1][prodIndex].event.eventType = EVENT_AT_QNAME;
			grammar->ruleArray[0].prodArrays[1][prodIndex].event.valueType.exiType = VALUE_TYPE_NONE;
			grammar->ruleArray[0].prodArrays[1][prodIndex].event.valueType.simpleTypeID = UINT16_MAX;
			grammar->ruleArray[0].prodArrays[1][prodIndex].nonTermID = 0;

			// "http://www.w3.org/2001/XMLSchema-instance" = 2
			grammar->ruleArray[0].prodArrays[1][prodIndex].uriRowID = 2;
			// nil = 0
			grammar->ruleArray[0].prodArrays[1][prodIndex].lnRowID = 0;
		}
	}
	return ERR_OK;
}

errorCode addUndeclaredProductionsToAll(AllocList* memList, URITable* stringTables, EXIOptions* opts, SimpleType* simpleTypeArray, uint16_t sTypeArraySize)
{
	unsigned int i = 0;
	size_t j = 0;
	EXIGrammar* tmpGrammar = NULL;
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	for (i = 0; i < stringTables->rowCount; i++)
	{
		for (j = 0; j < stringTables->rows[i].lTable->rowCount; j++)
		{
			tmpGrammar = stringTables->rows[i].lTable->rows[j].globalGrammar;
			if(tmpGrammar != NULL)
			{
				tmp_err_code = addUndeclaredProductions(memList, WITH_STRICT(opts->enumOpt), WITH_SELF_CONTAINED(opts->enumOpt), opts->preserve, tmpGrammar, simpleTypeArray, sTypeArraySize);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
	}

	return ERR_OK;
}
