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

errorCode insertZeroProduction(DynGrammarRule* rule, EXIEvent evnt, size_t nonTermID,
								size_t lnRowID, uint16_t uriRowID)
{
	if(rule->part[0].prodArraySize == rule->part0Dimension) // The dynamic array rule->prodArrays[0] needs to be resized
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		tmp_err_code = memManagedReAllocate((void **) &rule->part[0].prodArray, sizeof(Production)*(rule->part0Dimension + DEFAULT_PROD_ARRAY_DIM), rule->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		rule->part0Dimension += DEFAULT_PROD_ARRAY_DIM;
	}

	rule->part[0].prodArray[rule->part[0].prodArraySize].evnt = evnt;
	rule->part[0].prodArray[rule->part[0].prodArraySize].nonTermID = nonTermID;
	rule->part[0].prodArray[rule->part[0].prodArraySize].qname.lnRowId = lnRowID;
	rule->part[0].prodArray[rule->part[0].prodArraySize].qname.uriRowId = uriRowID;

	rule->part[0].prodArraySize += 1;
	rule->part[0].bits = getBitsNumber(rule->part[0].prodArraySize - 1 + (rule->part[1].prodArraySize + rule->part[2].prodArraySize > 0));
	return ERR_OK;
}

errorCode copyGrammarRule(AllocList* memList, GrammarRule* src, GrammarRule* dest)
{
	unsigned char b;
	size_t j = 0;

	for(b = 0; b < 3; b++)
	{
		dest->part[b].bits = src->part[b].bits;
		dest->part[b].prodArraySize = src->part[b].prodArraySize;

		if(src->part[b].prodArraySize != 0)
		{
			dest->part[b].prodArray = (Production*) memManagedAllocate(memList, sizeof(Production)*dest->part[b].prodArraySize);
			if(dest->part[b].prodArray == NULL)
				return MEMORY_ALLOCATION_ERROR;

			for(j = 0;j < dest->part[b].prodArraySize; j++)
			{
				dest->part[b].prodArray[j] = src->part[b].prodArray[j];
				if(src->part[b].prodArray[j].nonTermID != GR_VOID_NON_TERMINAL)
					dest->part[b].prodArray[j].nonTermID = src->part[b].prodArray[j].nonTermID;
			}
		}
		else
			dest->part[b].prodArray = NULL;
	}

	return ERR_OK;
}

#ifdef EXIP_DEBUG // TODO: document this macro #DOCUMENT#

errorCode printGrammarRule(size_t nonTermID, GrammarRule* rule)
{
	size_t j = 0;
	unsigned char b = 0;
	size_t tmp_prod_indx = 0;

	DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\n>RULE\n"));
	DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("NT-%u:", (unsigned int) nonTermID));

	DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\n"));

	for(b = 0; b < 3; b++)
	{
		for(j = 0; j < rule->part[b].prodArraySize; j++)
		{
			tmp_prod_indx = rule->part[b].prodArraySize - 1 - j;
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\t"));
			switch(rule->part[b].prodArray[tmp_prod_indx].evnt.eventType)
			{
				case EVENT_SD:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SD "));
					break;
				case EVENT_ED:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("ED "));
					break;
				case EVENT_SE_QNAME:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SE (qname: %d:%d)", rule->part[b].prodArray[tmp_prod_indx].qname.uriRowId, rule->part[b].prodArray[tmp_prod_indx].qname.lnRowId));
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
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("AT (qname %d:%d) [%d]", rule->part[b].prodArray[tmp_prod_indx].qname.uriRowId, rule->part[b].prodArray[tmp_prod_indx].qname.lnRowId, rule->part[b].prodArray[tmp_prod_indx].evnt.valueType.exiType));
					break;
				case EVENT_AT_URI:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("AT (uri) "));
					break;
				case EVENT_AT_ALL:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("AT (*) [%d]", rule->part[b].prodArray[tmp_prod_indx].evnt.valueType.exiType));
					break;
				case EVENT_CH:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("CH [%d]", rule->part[b].prodArray[tmp_prod_indx].evnt.valueType.exiType));
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
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("NT-%u", (unsigned int) rule->part[b].prodArray[tmp_prod_indx].nonTermID));
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\t"));
			if(b > 0)
			{
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("%d", rule->part[0].prodArraySize));
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("."));
				if(b > 1)
				{
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("%d", rule->part[1].prodArraySize));
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("."));
				}
			}
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("%d", j));
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\n"));
		}
	}
	return ERR_OK;
}

#endif // EXIP_DEBUG
