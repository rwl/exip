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
	rule->bits[0] = getBitsNumber(maxCodePart - 1);

	rule->prodArray[rule->prodCount].code = getEventCode1(0);
	rule->prodArray[rule->prodCount].eType = eType;
	rule->prodArray[rule->prodCount].nonTermID = nonTermID;
	rule->prodArray[rule->prodCount].lnRowID = lnRowID;
	rule->prodArray[rule->prodCount].uriRowID = uriRowID;
	rule->prodCount = rule->prodCount + 1;
	return ERR_OK;
}
