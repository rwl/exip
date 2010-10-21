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

#include "../include/grammarRules.h"
#include "procTypes.h"

errorCode initGrammarRule(GrammarRule* rule)
{
	rule->prodArray = (Production*) EXIP_MALLOC(sizeof(Production)*DEFAULT_PROD_ARRAY_DIM);
	if(rule->prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;
	rule->prodCount = 0;
	rule->prodDimension = DEFAULT_PROD_ARRAY_DIM;
	rule->bits[0] = 0;
	rule->bits[1] = 0;
	rule->bits[2] = 0;
	rule->nonTermID = GR_VOID_NON_TERMINAL;
	return ERR_OK;
}

errorCode addProduction(GrammarRule* rule, EventCode eCode, EventType eType, unsigned int nonTermID)
{
	if(rule->prodCount == rule->prodDimension) // The dynamic array prodArray needs to be resized
	{
		void* new_ptr = EXIP_REALLOC(rule->prodArray, sizeof(Production)*(rule->prodCount + DEFAULT_PROD_ARRAY_DIM));
		if(new_ptr == NULL)
			return MEMORY_ALLOCATION_ERROR;
		rule->prodArray = new_ptr;
		rule->prodDimension = rule->prodDimension + DEFAULT_PROD_ARRAY_DIM;
	}
	rule->prodArray[rule->prodCount].code = eCode;
	rule->prodArray[rule->prodCount].eType = eType;
	rule->prodArray[rule->prodCount].nonTermID = nonTermID;
	rule->prodCount = rule->prodCount + 1;
	return ERR_OK;
}

errorCode insertZeroProduction(GrammarRule* rule, EventType eType, unsigned int nonTermID,
							   unsigned int lnRowID, unsigned int uriRowID)
{
	if(rule->prodCount == rule->prodDimension) // The dynamic array prodArray needs to be resized
	{
		void* new_ptr = EXIP_REALLOC(rule->prodArray, sizeof(Production)*(rule->prodCount + DEFAULT_PROD_ARRAY_DIM));
		if(new_ptr == NULL)
			return MEMORY_ALLOCATION_ERROR;
		rule->prodArray = new_ptr;
		rule->prodDimension = rule->prodDimension + DEFAULT_PROD_ARRAY_DIM;
	}
	int i = 0;
	unsigned int maxCodePart = 0;
	for(i = 0; i < rule->prodCount; i++)
	{
		rule->prodArray[i].code.code[0] += 1;
		if(rule->prodArray[i].code.code[0] > maxCodePart)
			maxCodePart = rule->prodArray[i].code.code[0];
	}
	rule->bits[0] = getBitsNumber(maxCodePart);
	EventCode eCode = getEventCode1(0);

	rule->prodArray[rule->prodCount].code = eCode;
	rule->prodArray[rule->prodCount].eType = eType;
	rule->prodArray[rule->prodCount].nonTermID = nonTermID;
	rule->prodArray[rule->prodCount].lnRowID = lnRowID;
	rule->prodArray[rule->prodCount].uriRowID = uriRowID;
	rule->prodCount = rule->prodCount + 1;
	return ERR_OK;
}

#ifdef EXIP_DEBUG // TODO: document this macro #DOCUMENT#

errorCode printGrammarRule(GrammarRule* rule)
{
	DEBUG_MSG(INFO,("\n>RULE\n"));
	switch(rule->nonTermID)
	{
		case GR_VOID_NON_TERMINAL:
			DEBUG_MSG(INFO,("VOID:"));
			break;
		case GR_DOCUMENT:
			DEBUG_MSG(INFO,("Document:"));
			break;
		case GR_DOC_CONTENT:
			DEBUG_MSG(INFO,("DocContent:"));
			break;
		case GR_DOC_END:
			DEBUG_MSG(INFO,("DocEnd:"));
			break;
		case GR_START_TAG_CONTENT:
			DEBUG_MSG(INFO,("StartTagContent:"));
			break;
		case GR_ELEMENT_CONTENT:
			DEBUG_MSG(INFO,("ElementContent:"));
			break;
		case GR_FRAGMENT:
			DEBUG_MSG(INFO,("Fragment:"));
			break;
		default:
			return UNEXPECTED_ERROR;
	}
	DEBUG_MSG(INFO,("\n"));
	int i = 0;
	for(i = 0; i < rule->prodCount; i++)
	{
		DEBUG_MSG(INFO,("\t"));
		switch(rule->prodArray[i].eType)
		{
			case EVENT_SD:
				DEBUG_MSG(INFO,("SD "));
				break;
			case EVENT_ED:
				DEBUG_MSG(INFO,("ED "));
				break;
			case EVENT_SE_QNAME:
				DEBUG_MSG(INFO,("SE (qname) "));
				break;
			case EVENT_SE_URI:
				DEBUG_MSG(INFO,("SE (uri) "));
				break;
			case EVENT_SE_ALL:
				DEBUG_MSG(INFO,("SE (*) "));
				break;
			case EVENT_EE:
				DEBUG_MSG(INFO,("EE "));
				break;
			case EVENT_AT_QNAME:
				DEBUG_MSG(INFO,("AT (qname) "));
				break;
			case EVENT_AT_URI:
				DEBUG_MSG(INFO,("AT (uri) "));
				break;
			case EVENT_AT_ALL:
				DEBUG_MSG(INFO,("AT (*) "));
				break;
			case EVENT_CH:
				DEBUG_MSG(INFO,("CH "));
				break;
			case EVENT_NS:
				DEBUG_MSG(INFO,("NS "));
				break;
			case EVENT_CM:
				DEBUG_MSG(INFO,("CM "));
				break;
			case EVENT_PI:
				DEBUG_MSG(INFO,("PI "));
				break;
			case EVENT_DT:
				DEBUG_MSG(INFO,("DT "));
				break;
			case EVENT_ER:
				DEBUG_MSG(INFO,("ER "));
				break;
			case EVENT_SC:
				DEBUG_MSG(INFO,("SC "));
				break;
			default:
				return UNEXPECTED_ERROR;
		}
		switch(rule->prodArray[i].nonTermID)
		{
			case GR_VOID_NON_TERMINAL:
				DEBUG_MSG(INFO,("VOID:"));
				break;
			case GR_DOCUMENT:
				DEBUG_MSG(INFO,("Document:"));
				break;
			case GR_DOC_CONTENT:
				DEBUG_MSG(INFO,("DocContent:"));
				break;
			case GR_DOC_END:
				DEBUG_MSG(INFO,("DocEnd:"));
				break;
			case GR_START_TAG_CONTENT:
				DEBUG_MSG(INFO,("StartTagContent:"));
				break;
			case GR_ELEMENT_CONTENT:
				DEBUG_MSG(INFO,("ElementContent:"));
				break;
			case GR_FRAGMENT:
				DEBUG_MSG(INFO,("Fragment:"));
				break;
			default:
				return UNEXPECTED_ERROR;

		}
		DEBUG_MSG(INFO,(" Event Code: "));
		int j = 0;
		for(j = 0; j < rule->prodArray[i].code.size; j++)
		{
			DEBUG_MSG(INFO,(".%d", rule->prodArray[i].code.code[j]));
		}
		DEBUG_MSG(INFO,("\n"));
	}
	return ERR_OK;
}

#endif // EXIP_DEBUG
