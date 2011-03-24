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
 * @file grammarRules.c
 * @brief Defines grammar rules related functions
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "grammarRules.h"
#include "eventsEXI.h"
#include "memManagement.h"
#include "ioUtil.h"

errorCode initGrammarRule(GrammarRule* rule, AllocList* memList)
{
	rule->prodArray = (Production*) memManagedAllocatePtr(memList, sizeof(Production)*DEFAULT_PROD_ARRAY_DIM, &rule->memPair);
	if(rule->prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;
	rule->prodCount = 0;
	rule->prodDimension = DEFAULT_PROD_ARRAY_DIM;
	rule->bits[0] = 0;
	rule->bits[1] = 0;
	rule->bits[2] = 0;
	return ERR_OK;
}

errorCode addProduction(GrammarRule* rule, EventCode eCode, EXIEvent event, size_t nonTermID)
{
	if(rule->prodCount == rule->prodDimension) // The dynamic array prodArray needs to be resized
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		tmp_err_code = memManagedReAllocate((void *) &rule->prodArray, sizeof(Production)*(rule->prodCount + DEFAULT_PROD_ARRAY_DIM), rule->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		rule->prodDimension = rule->prodDimension + DEFAULT_PROD_ARRAY_DIM;
	}
	rule->prodArray[rule->prodCount].code = eCode;
	rule->prodArray[rule->prodCount].event = event;
	rule->prodArray[rule->prodCount].nonTermID = nonTermID;
	rule->prodCount = rule->prodCount + 1;
	return ERR_OK;
}

errorCode insertZeroProduction(GrammarRule* rule, EXIEvent event, size_t nonTermID,
								size_t lnRowID, uint16_t uriRowID)
{
	uint16_t i = 0;
	unsigned int maxCodePart = 0;

	if(rule->prodCount == rule->prodDimension) // The dynamic array prodArray needs to be resized
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		tmp_err_code = memManagedReAllocate((void *) &rule->prodArray, sizeof(Production)*(rule->prodCount + DEFAULT_PROD_ARRAY_DIM), rule->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		rule->prodDimension = rule->prodDimension + DEFAULT_PROD_ARRAY_DIM;
	}

	for(i = 0; i < rule->prodCount; i++)
	{
		rule->prodArray[i].code.code[0] += 1;
		if(rule->prodArray[i].code.code[0] > maxCodePart)
			maxCodePart = rule->prodArray[i].code.code[0];
	}
	rule->bits[0] = getBitsNumber(maxCodePart);

	rule->prodArray[rule->prodCount].code = getEventCode1(0);
	rule->prodArray[rule->prodCount].event = event;
	rule->prodArray[rule->prodCount].nonTermID = nonTermID;
	rule->prodArray[rule->prodCount].lnRowID = lnRowID;
	rule->prodArray[rule->prodCount].uriRowID = uriRowID;
	rule->prodCount = rule->prodCount + 1;
	return ERR_OK;
}

errorCode copyGrammarRule(AllocList* memList, GrammarRule* src, GrammarRule* dest, unsigned int nonTermIdShift)
{
	dest->bits[0] = src->bits[0];
	dest->bits[1] = src->bits[1];
	dest->bits[2] = src->bits[2];

	dest->memPair.memBlock = NULL;
	dest->memPair.allocIndx = 0;
	dest->prodCount = src->prodCount;
	dest->prodDimension = src->prodDimension;

	dest->prodArray = (Production*) memManagedAllocate(memList, sizeof(Production)*dest->prodDimension);
	if(dest->prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	{
		uint16_t i = 0;
		for(i = 0;i < dest->prodCount; i++)
		{
			dest->prodArray[i] = src->prodArray[i];
			if(src->prodArray[i].nonTermID != GR_VOID_NON_TERMINAL)
				dest->prodArray[i].nonTermID = src->prodArray[i].nonTermID + nonTermIdShift;
		}
	}

	return ERR_OK;
}

#ifdef EXIP_DEBUG // TODO: document this macro #DOCUMENT#

errorCode printGrammarRule(size_t nonTermID, GrammarRule* rule)
{
	uint16_t i = 0;

	DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\n>RULE\n"));
	DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("NT-%d:", nonTermID));

	DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\n"));
	for(i = 0; i < rule->prodCount; i++)
	{
		unsigned char j = 0;

		DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\t"));
		switch(rule->prodArray[i].event.eventType)
		{
			case EVENT_SD:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SD "));
				break;
			case EVENT_ED:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("ED "));
				break;
			case EVENT_SE_QNAME:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SE (qname) "));
				break;
			case EVENT_SE_URI:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SE (uri) "));
				break;
			case EVENT_SE_ALL:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SE (*) "));
				break;
			case EVENT_EE:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("EE "));
				break;
			case EVENT_AT_QNAME:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("AT (qname) "));
				break;
			case EVENT_AT_URI:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("AT (uri) "));
				break;
			case EVENT_AT_ALL:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("AT (*) "));
				break;
			case EVENT_CH:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("CH "));
				break;
			case EVENT_NS:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("NS "));
				break;
			case EVENT_CM:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("CM "));
				break;
			case EVENT_PI:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("PI "));
				break;
			case EVENT_DT:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("DT "));
				break;
			case EVENT_ER:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("ER "));
				break;
			case EVENT_SC:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SC "));
				break;
			case EVENT_VOID:
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, (" "));
				break;
			default:
				return UNEXPECTED_ERROR;
		}
		DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\t"));
		DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("NT-%d",rule->prodArray[i].nonTermID));

		DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\t\t"));
		for(j = 0; j < rule->prodArray[i].code.size; j++)
		{
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, (".%d", rule->prodArray[i].code.code[j]));
		}
		DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\n"));
	}
	return ERR_OK;
}

#endif // EXIP_DEBUG
