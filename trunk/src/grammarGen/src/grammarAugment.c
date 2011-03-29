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

#define ATTR_PROD_ARRAY_SIZE 30

static int compareProductions(const void* prod1, const void* prod2);

errorCode assignCodes(struct EXIGrammar* grammar)
{
	uint16_t i = 0;
	uint16_t j = 0;

	for (i = 0; i < grammar->rulesDimension; i++)
	{
		qsort(grammar->ruleArray[i].prodArray, grammar->ruleArray[i].prodCount, sizeof(Production), compareProductions);
		grammar->ruleArray[i].bits[0] = getBitsNumber(grammar->ruleArray[i].prodCount - 1);
		for (j = 0; j < grammar->ruleArray[i].prodCount; j++)
		{
			grammar->ruleArray[i].prodArray[j].code = getEventCode1(j);
		}
	}
	return ERR_OK;
}

errorCode addUndeclaredProductions(AllocList* memList, unsigned char strict, struct EXIGrammar* grammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t i = 0;
	uint16_t j = 0;
	uint16_t a = 0;
	unsigned int maxFirstCodePart = 0;
	unsigned int maxSecondCodePart = 0;
	EXIEvent tmpEvent;

	if(strict == FALSE)
	{
		unsigned char maxCodeSizeInRule = 1;
		unsigned char prodEEFound = 0;
		Production* attrProdArray[ATTR_PROD_ARRAY_SIZE];

		for(i = 0; i <= grammar->contentIndex; i++)
		{
			maxFirstCodePart = 0;
			maxSecondCodePart = 0;
			maxCodeSizeInRule = 1;
			a = 0;
			prodEEFound = 0;
			for(j = 0; j < grammar->ruleArray[i].prodCount; j++)
			{
				if(grammar->ruleArray[i].prodArray[j].code.code[0] > maxFirstCodePart)
					maxFirstCodePart = grammar->ruleArray[i].prodArray[j].code.code[0];

				if(grammar->ruleArray[i].prodArray[j].code.size > 1 && grammar->ruleArray[i].prodArray[j].code.code[1] > maxSecondCodePart)
				{
					maxSecondCodePart = grammar->ruleArray[i].prodArray[j].code.code[1];
					maxCodeSizeInRule = 2;
				}

				if(grammar->ruleArray[i].prodArray[j].nonTermID == GR_VOID_NON_TERMINAL && grammar->ruleArray[i].prodArray[j].event.eventType == EVENT_EE)
				{
					prodEEFound = 1;
				}

				if(grammar->ruleArray[i].prodArray[j].event.eventType == EVENT_AT_QNAME)
				{
					if(a >= ATTR_PROD_ARRAY_SIZE)
						return INCONSISTENT_PROC_STATE;

					attrProdArray[a] = &grammar->ruleArray[i].prodArray[j];
					a++;
				}
			}

			if(maxCodeSizeInRule == 1)
			{
				maxFirstCodePart += 1;
				grammar->ruleArray[i].bits[0] = getBitsNumber(maxFirstCodePart);
			}

			if(!prodEEFound) //	There is no production Gi,0 : EE so add one
			{
				tmp_err_code = addProduction(&(grammar->ruleArray[i]), getEventCode2(maxFirstCodePart, maxSecondCodePart + 1), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				maxSecondCodePart += 1;
				grammar->ruleArray[i].bits[1] = getBitsNumber(maxSecondCodePart);
			}

			if(i == 0)  // AT(xsi:type) Element i, 0 and AT(xsi:nil) Element i, 0
			{
				tmpEvent.eventType = EVENT_AT_QNAME;
				tmpEvent.valueType = VALUE_TYPE_QNAME;

				tmp_err_code = addProduction(&grammar->ruleArray[0], getEventCode2(maxFirstCodePart, maxSecondCodePart + 1), tmpEvent, 0);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				grammar->ruleArray[0].prodArray[grammar->ruleArray[0].prodCount-1].uriRowID = 2; // "http://www.w3.org/2001/XMLSchema-instance"
				grammar->ruleArray[0].prodArray[grammar->ruleArray[0].prodCount-1].lnRowID = 1; // type

				tmp_err_code = addProduction(&grammar->ruleArray[0], getEventCode2(maxFirstCodePart, maxSecondCodePart + 2), tmpEvent, 0);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				grammar->ruleArray[0].prodArray[grammar->ruleArray[0].prodCount-1].uriRowID = 2; // "http://www.w3.org/2001/XMLSchema-instance"
				grammar->ruleArray[0].prodArray[grammar->ruleArray[0].prodCount-1].lnRowID = 0; // nil

				maxSecondCodePart += 2;
				grammar->ruleArray[0].bits[1] = getBitsNumber(maxSecondCodePart);
			}

			tmpEvent.eventType = EVENT_AT_ALL;
			tmpEvent.valueType = VALUE_TYPE_NONE;

			tmp_err_code = addProduction(&grammar->ruleArray[i], getEventCode2(maxFirstCodePart, maxSecondCodePart + 1), tmpEvent, i);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			maxSecondCodePart += 1;
			grammar->ruleArray[i].bits[1] = getBitsNumber(maxSecondCodePart);

			for(j = 0; j < a; j++)
			{

				tmpEvent.eventType = EVENT_AT_QNAME;
				tmpEvent.valueType = VALUE_TYPE_UNTYPED;

				tmp_err_code = addProduction(&grammar->ruleArray[i], getEventCode3(maxFirstCodePart, maxSecondCodePart, a), tmpEvent, attrProdArray[a]->nonTermID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				maxSecondCodePart += 1;


				grammar->ruleArray[i].prodArray[grammar->ruleArray[i].prodCount-1].uriRowID = attrProdArray[a]->uriRowID;
				grammar->ruleArray[i].prodArray[grammar->ruleArray[i].prodCount-1].lnRowID = attrProdArray[a]->lnRowID;

			}
			grammar->ruleArray[i].bits[2] = getBitsNumber(a);
			// TODO: not finished!
		}

		// TODO: not finished yet!
	}
	else // strict == TRUE
	{
		return NOT_IMPLEMENTED_YET;
	}
	return ERR_OK;
}

static int compareProductions(const void* prod1, const void* prod2)
{
	Production* p1 = (Production*) prod1;
	Production* p2 = (Production*) prod2;

	if(p1->event.eventType < p2->event.eventType)
		return -1;
	else if(p1->event.eventType > p2->event.eventType)
		return 1;
	else // the same event Type
	{
		if(p1->event.eventType == EVENT_AT_QNAME)
		{
			if(p1->lnRowID < p2->lnRowID)
				return -1;
			else if(p1->lnRowID > p2->lnRowID)
				return 1;
			else
			{
				if(p1->uriRowID < p2->uriRowID)
					return -1;
				else if(p1->uriRowID > p2->uriRowID)
					return 1;
				else
					return 0;
			}
		}
		else if(p1->event.eventType == EVENT_AT_URI)
		{
			if(p1->uriRowID < p2->uriRowID)
				return -1;
			else if(p1->uriRowID > p2->uriRowID)
				return 1;
			else
				return 0;
		}
		else if(p1->event.eventType == EVENT_SE_QNAME)
		{
			// TODO: figure out how it should be done
		}
		else if(p1->event.eventType == EVENT_SE_URI)
		{
			// TODO: figure out how it should be done
		}
		return 0;
	}
}
