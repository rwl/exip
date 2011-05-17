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

errorCode insertZeroProduction(DynGrammarRule* rule, EXIEvent event, size_t nonTermID,
								size_t lnRowID, uint16_t uriRowID)
{
	if(rule->prodCounts[0] == rule->prod1Dimension) // The dynamic array rule->prodArrays[0] needs to be resized
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		tmp_err_code = memManagedReAllocate((void *) &rule->prodArrays[0], sizeof(Production)*(rule->prod1Dimension + DEFAULT_PROD_ARRAY_DIM), rule->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		rule->prod1Dimension += DEFAULT_PROD_ARRAY_DIM;
	}

	rule->prodArrays[0][rule->prodCounts[0]].event = event;
	rule->prodArrays[0][rule->prodCounts[0]].nonTermID = nonTermID;
	rule->prodArrays[0][rule->prodCounts[0]].lnRowID = lnRowID;
	rule->prodArrays[0][rule->prodCounts[0]].uriRowID = uriRowID;

	rule->prodCounts[0] += 1;
	rule->bits[0] = getBitsNumber(rule->prodCounts[0] - 1 + (rule->prodCounts[1] + rule->prodCounts[2] > 0));
	return ERR_OK;
}

errorCode copyGrammarRule(AllocList* memList, GrammarRule* src, GrammarRule* dest)
{
	unsigned char b;
	size_t j = 0;

	for(b = 0; b < 3; b++)
	{
		dest->bits[b] = src->bits[b];
		dest->prodCounts[b] = src->prodCounts[b];

		if(src->prodCounts[b] != 0)
		{
			dest->prodArrays[b] = (Production*) memManagedAllocate(memList, sizeof(Production)*dest->prodCounts[b]);
			if(dest->prodArrays[b] == NULL)
				return MEMORY_ALLOCATION_ERROR;

			for(j = 0;j < dest->prodCounts[b]; j++)
			{
				dest->prodArrays[b][j] = src->prodArrays[b][j];
				if(src->prodArrays[b][j].nonTermID != GR_VOID_NON_TERMINAL)
					dest->prodArrays[b][j].nonTermID = src->prodArrays[b][j].nonTermID;
			}
		}
		else
			dest->prodArrays[b] = NULL;
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
		for(j = 0; j < rule->prodCounts[b]; j++)
		{
			tmp_prod_indx = rule->prodCounts[b] - 1 - j;
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\t"));
			switch(rule->prodArrays[b][tmp_prod_indx].event.eventType)
			{
				case EVENT_SD:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SD "));
					break;
				case EVENT_ED:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("ED "));
					break;
				case EVENT_SE_QNAME:
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("SE (qname: %d:%d) ", rule->prodArrays[b][tmp_prod_indx].uriRowID, rule->prodArrays[b][tmp_prod_indx].lnRowID));
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
					DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("AT (qname %d:%d) ", rule->prodArrays[b][tmp_prod_indx].uriRowID, rule->prodArrays[b][tmp_prod_indx].lnRowID));
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
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("NT-%u", (unsigned int) rule->prodArrays[b][tmp_prod_indx].nonTermID));
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\t"));
			if(b > 0)
				DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("."));
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("%d", j));
			DEBUG_MSG(INFO, DEBUG_ALL_MODULES, ("\n"));
		}
	}
	return ERR_OK;
}

#endif // EXIP_DEBUG
