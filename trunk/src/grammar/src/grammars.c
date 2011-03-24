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
 * @file grammars.c
 * @brief Defines grammar related functions
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef BUILTINDOCGRAMMAR_H_
#define BUILTINDOCGRAMMAR_H_

#include "grammars.h"
#include "hashtable.h"
#include "hashUtils.h"
#include "sTables.h"
#include "memManagement.h"
#include "ioUtil.h"
#include "streamDecode.h"
#include "bodyDecode.h"

#define DEF_DOC_GRAMMAR_RULE_NUMBER 3
#define DEF_ELEMENT_GRAMMAR_RULE_NUMBER 2
#define GRAMMAR_POOL_DIMENSION 16

static errorCode handleProduction(EXIStream* strm, unsigned int ruleIndx, unsigned int prodIndx,
				EXIEvent* event, size_t* nonTermID_out, ContentHandler* handler, void* app_data, unsigned int codeIndx);

errorCode createDocGrammar(struct EXIGrammar* docGrammar, EXIStream* strm, ExipSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int n = 0; // first part of the event codes in the second rule

	//TODO: depends on the EXI fidelity options! Take this into account
	// For now only the default fidelity_opts pruning is supported - all preserve opts are false
	char is_default_fidelity = 0;

	if(strm->header.opts->preserve == 0) //all preserve opts are false
		is_default_fidelity = 1;

	docGrammar->rulesDimension = DEF_DOC_GRAMMAR_RULE_NUMBER;
	docGrammar->grammarType = GR_TYPE_BUILD_IN_DOC;
	docGrammar->contentIndex = 0;
	docGrammar->ruleArray = (GrammarRule*) memManagedAllocate(&strm->memList, sizeof(GrammarRule)*DEF_DOC_GRAMMAR_RULE_NUMBER);
	if(docGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* Document : SD DocContent	0 */
	tmp_err_code = initGrammarRule(&(docGrammar->ruleArray[GR_DOCUMENT]), &strm->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	docGrammar->ruleArray[GR_DOCUMENT].bits[0] = 0;
	tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOCUMENT]), getEventCode1(0), getEventDefType(EVENT_SD), GR_DOC_CONTENT);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = initGrammarRule(&(docGrammar->ruleArray[GR_DOC_CONTENT]), &strm->memList);
	if(tmp_err_code != ERR_OK)
			return tmp_err_code;

	if(schema != NULL)   // Creates Schema Informed Grammar
	{
		unsigned int e = 0;

		docGrammar->grammarType = GR_TYPE_SCHEMA_DOC;
		/*
		   DocContent :
					   	SE (G-0)   DocEnd	0
						SE (G-1)   DocEnd	1
						⋮	⋮      ⋮
						SE (G-n−1) DocEnd n-1
					//	SE (*)     DocEnd	n		//  This is created as part of the Build-In grammar down
					//	DT DocContent   	n+1.0	//  This is created as part of the Build-In grammar down
					//	CM DocContent		n+1.1.0	//  This is created as part of the Build-In grammar down
					//	PI DocContent		n+1.1.1	//  This is created as part of the Build-In grammar down
		 */

		for(e = 0; e < schema->globalElemGrammars.count; e++)
		{
			tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOC_CONTENT]), getEventCode1(e), getEventDefType(EVENT_SE_QNAME), GR_DOC_END);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			docGrammar->ruleArray[GR_DOC_CONTENT].prodArray[docGrammar->ruleArray[GR_DOC_CONTENT].prodCount - 1].lnRowID = schema->globalElemGrammars.elems[e].lnRowId;
			docGrammar->ruleArray[GR_DOC_CONTENT].prodArray[docGrammar->ruleArray[GR_DOC_CONTENT].prodCount - 1].uriRowID = schema->globalElemGrammars.elems[e].uriRowId;
		}
		n = schema->globalElemGrammars.count;
	}

	/*
	   DocContent :
					SE (*) DocEnd	0
					DT DocContent	1.0
					CM DocContent	1.1.0
					PI DocContent	1.1.1
	 */


	if(is_default_fidelity)
	{
		docGrammar->ruleArray[1].bits[0] = getBitsNumber(n);
	}
	else
	{
		docGrammar->ruleArray[1].bits[0] = getBitsNumber(n + 1);
		docGrammar->ruleArray[1].bits[1] = 1;
		docGrammar->ruleArray[1].bits[2] = 1;
	}

	/* SE (*) DocEnd	0 */
	tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOC_CONTENT]), getEventCode1(n), getEventDefType(EVENT_SE_ALL), GR_DOC_END);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(is_default_fidelity == 0)
	{
		/* DT DocContent	1.0 */
		tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOC_CONTENT]), getEventCode2(n + 1, 0), getEventDefType(EVENT_DT), GR_DOC_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CM DocContent	1.1.0 */
		tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOC_CONTENT]), getEventCode3(n + 1, 1, 0), getEventDefType(EVENT_CM), GR_DOC_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* PI DocContent	1.1.1 */
		tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOC_CONTENT]), getEventCode3(n + 1, 1, 1), getEventDefType(EVENT_PI), GR_DOC_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}


	/* DocEnd :
				ED	        0
				CM DocEnd	1.0
				PI DocEnd	1.1 */
	tmp_err_code = initGrammarRule(&(docGrammar->ruleArray[GR_DOC_END]), &strm->memList);
	if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	if(is_default_fidelity == 1)
	{
		docGrammar->ruleArray[GR_DOC_END].bits[0] = 0;
	}
	else
	{
		docGrammar->ruleArray[GR_DOC_END].bits[0] = 1;
		docGrammar->ruleArray[GR_DOC_END].bits[1] = 1;
	}

	/* ED	0 */
	tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOC_END]), getEventCode1(0), getEventDefType(EVENT_ED), GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(is_default_fidelity == 0)
	{
		/* CM DocEnd	1.0  */
		tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOC_END]), getEventCode2(1, 0), getEventDefType(EVENT_CM), GR_DOC_END);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* PI DocEnd	1.1 */
		tmp_err_code = addProduction(&(docGrammar->ruleArray[GR_DOC_END]), getEventCode2(1, 1), getEventDefType(EVENT_PI), GR_DOC_END);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode createBuildInElementGrammar(struct EXIGrammar* elementGrammar, EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	//TODO: depends on the EXI fidelity options! Take this into account
	// For now only the default fidelity_opts pruning is supported - all preserve opts are false
	// and selfContained is also false
	char is_default_fidelity = 0;

	if(strm->header.opts->preserve == 0 && strm->header.opts->selfContained == 0) //all preserve opts are false and selfContained is also false
		is_default_fidelity = 1;

	elementGrammar->rulesDimension = DEF_ELEMENT_GRAMMAR_RULE_NUMBER;
	elementGrammar->grammarType = GR_TYPE_BUILD_IN_ELEM;
	elementGrammar->contentIndex = 0;
	elementGrammar->ruleArray = (GrammarRule*) memManagedAllocate(&strm->memList, sizeof(GrammarRule)*DEF_ELEMENT_GRAMMAR_RULE_NUMBER);
	if(elementGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* StartTagContent :
							EE	                    0.0
							AT (*) StartTagContent	0.1
							NS StartTagContent	    0.2
							SC Fragment	            0.3
							SE (*) ElementContent	0.4
							CH ElementContent	    0.5
							ER ElementContent	    0.6
							CM ElementContent	    0.7.0
							PI ElementContent	    0.7.1 */
	tmp_err_code = initGrammarRule(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), &strm->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(is_default_fidelity == 1)
	{
		elementGrammar->ruleArray[GR_START_TAG_CONTENT].bits[0] = 0;
		elementGrammar->ruleArray[GR_START_TAG_CONTENT].bits[1] = 2;
	}
	else
	{
		elementGrammar->ruleArray[GR_START_TAG_CONTENT].bits[0] = 0;
		elementGrammar->ruleArray[GR_START_TAG_CONTENT].bits[1] = 4;
		elementGrammar->ruleArray[GR_START_TAG_CONTENT].bits[2] = 1;
	}

	/* EE	                    0.0 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/* AT (*) StartTagContent	0.1 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,1), getEventDefType(EVENT_AT_ALL), GR_START_TAG_CONTENT);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(is_default_fidelity == 1)
	{
		/* SE (*) ElementContent	0.2 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,2), getEventDefType(EVENT_SE_ALL), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CH ElementContent	    0.3 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,3), getEventDefType(EVENT_CH), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

	}
	else
	{
		/* NS StartTagContent	    0.2 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,2), getEventDefType(EVENT_NS), GR_START_TAG_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* SC Fragment	            0.3 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,3), getEventDefType(EVENT_SC), GR_FRAGMENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* SE (*) ElementContent	0.4 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,4), getEventDefType(EVENT_SE_ALL), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CH ElementContent	    0.5 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,5), getEventDefType(EVENT_CH), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* ER ElementContent	    0.6 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode2(0,6), getEventDefType(EVENT_ER), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CM ElementContent	    0.7.0 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode3(0,7,0), getEventDefType(EVENT_CM), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* PI ElementContent	    0.7.1 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_START_TAG_CONTENT]), getEventCode3(0,7,1), getEventDefType(EVENT_PI), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	/* ElementContent :
							EE	                    0
							SE (*) ElementContent	1.0
							CH ElementContent	    1.1
							ER ElementContent	    1.2
							CM ElementContent	    1.3.0
							PI ElementContent	    1.3.1 */
	tmp_err_code = initGrammarRule(&(elementGrammar->ruleArray[GR_ELEMENT_CONTENT]), &strm->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(is_default_fidelity == 1)
	{
		elementGrammar->ruleArray[GR_ELEMENT_CONTENT].bits[0] = 1;
		elementGrammar->ruleArray[GR_ELEMENT_CONTENT].bits[1] = 1;
	}
	else
	{
		elementGrammar->ruleArray[GR_ELEMENT_CONTENT].bits[0] = 1;
		elementGrammar->ruleArray[GR_ELEMENT_CONTENT].bits[1] = 2;
		elementGrammar->ruleArray[GR_ELEMENT_CONTENT].bits[2] = 1;
	}

	/* EE	                  0 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_ELEMENT_CONTENT]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/* SE (*) ElementContent	1.0 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_ELEMENT_CONTENT]), getEventCode2(1,0), getEventDefType(EVENT_SE_ALL), GR_ELEMENT_CONTENT);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/* CH ElementContent	    1.1 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_ELEMENT_CONTENT]), getEventCode2(1,1), getEventDefType(EVENT_CH), GR_ELEMENT_CONTENT);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(is_default_fidelity == 0)
	{
		/* ER ElementContent	    1.2 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_ELEMENT_CONTENT]), getEventCode2(1,2), getEventDefType(EVENT_ER), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CM ElementContent	    1.3.0 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_ELEMENT_CONTENT]), getEventCode3(1,3,0), getEventDefType(EVENT_CM), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* PI ElementContent	    1.3.1 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[GR_ELEMENT_CONTENT]), getEventCode3(1,3,1), getEventDefType(EVENT_PI), GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode copyGrammar(AllocList* memList, struct EXIGrammar* src, struct EXIGrammar** dest)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t i = 0;

	*dest = memManagedAllocate(memList, sizeof(struct EXIGrammar));
	if(*dest == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*dest)->rulesDimension = src->rulesDimension;
	(*dest)->grammarType = src->grammarType;
	(*dest)->contentIndex = src->contentIndex;

	(*dest)->ruleArray = memManagedAllocate(memList, sizeof(GrammarRule) * (*dest)->rulesDimension);
	if((*dest)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(i = 0; i < (*dest)->rulesDimension; i++)
	{
		tmp_err_code = copyGrammarRule(memList, &src->ruleArray[i], &(*dest)->ruleArray[i], 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode pushGrammar(EXIGrammarStack** gStack, struct EXIGrammar* grammar)
{
	struct GrammarStackNode* node = EXIP_MALLOC(sizeof(struct GrammarStackNode));
	if(node == NULL)
		return MEMORY_ALLOCATION_ERROR;

	node->grammar = grammar;
	node->lastNonTermID = GR_VOID_NON_TERMINAL;
	node->nextInStack = *gStack;
	*gStack = node;
	return ERR_OK;
}

errorCode popGrammar(EXIGrammarStack** gStack, struct EXIGrammar** grammar)
{
	struct GrammarStackNode* node = *gStack;
	*gStack = (*gStack)->nextInStack;

	(*grammar) = node->grammar;
	EXIP_MFREE(node);
	return ERR_OK;
}

errorCode processNextProduction(EXIStream* strm, EXIEvent* event,
							    size_t* nonTermID_out, ContentHandler* handler, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t tmp_bits_val = 0;
	uint16_t currProduction = 0;
	uint16_t j = 0;
	unsigned int b = 0;
	GrammarRule* ruleArr;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Next production non-term-id: %d\n", strm->nonTermID));

	if(strm->nonTermID >=  strm->gStack->grammar->rulesDimension)
		return INCONSISTENT_PROC_STATE;

	ruleArr = strm->gStack->grammar->ruleArray;

#if DEBUG_GRAMMAR == ON
	{
		tmp_err_code = printGrammarRule(strm->nonTermID, &ruleArr[strm->nonTermID]);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Error printing grammar rule\n"));
		}
	}
#endif

	for(b = 0; b < 3; b++)
	{
		if(ruleArr[strm->nonTermID].bits[b] == 0 &&
				ruleArr[strm->nonTermID].prodCount > b) // zero bits encoded part of event code with more parts available
		{
			continue;
		}
		else if(ruleArr[strm->nonTermID].bits[b] == 0) // encoded with zero bits
		{
			return handleProduction(strm, strm->nonTermID, currProduction, event, nonTermID_out, handler, app_data, b);
		}
		else
		{
			tmp_err_code = decodeNBitUnsignedInteger(strm, ruleArr[strm->nonTermID].bits[b], &tmp_bits_val);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			for(j = 0; j < ruleArr[strm->nonTermID].prodCount; j++)
			{
				if(ruleArr[strm->nonTermID].prodArray[j].code.size < b + 1)
					continue;
				if(ruleArr[strm->nonTermID].prodArray[j].code.code[b] == tmp_bits_val)
				{
					if(ruleArr[strm->nonTermID].prodArray[j].code.size == b + 1)
					{
						return handleProduction(strm, strm->nonTermID, j, event, nonTermID_out, handler, app_data, b);
					}
					else
					{
						currProduction = j;
					}
					break;
				}
			}
		}
	}
	return tmp_err_code;
}

/*
 * #1#:
 * All productions in the built-in element grammar of the form LeftHandSide : EE are evaluated as follows:
 * - If a production of the form, LeftHandSide : EE with an event code of length 1 does not exist in
 *   the current element grammar, create one with event code 0 and increment the first part of the
 *   event code of each production in the current grammar with the non-terminal LeftHandSide on the left-hand side.
 * - Add the production created in step 1 to the grammar
 *
 * #2#
 * All productions in the built-in element grammar of the form LeftHandSide : CH RightHandSide are evaluated as follows:
 * - If a production of the form, LeftHandSide : CH RightHandSide with an event code of length 1 does not exist in
 *   the current element grammar, create one with event code 0 and increment the first part of the event code of
 *   each production in the current grammar with the non-terminal LeftHandSide on the left-hand side.
 * - Add the production created in step 1 to the grammar
 * - Evaluate the remainder of event sequence using RightHandSide.
 * */

static errorCode handleProduction(EXIStream* strm, unsigned int ruleIndx, unsigned int prodIndx,
				EXIEvent* event, size_t* nonTermID_out, ContentHandler* handler, void* app_data, unsigned int codeIndx)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	GrammarRule* ruleArr = strm->gStack->grammar->ruleArray;

	*event = ruleArr[ruleIndx].prodArray[prodIndx].event;
	*nonTermID_out = ruleArr[ruleIndx].prodArray[prodIndx].nonTermID;
	if(event->eventType == EVENT_SD)
	{
		if(handler->startDocument != NULL)
		{
			if(handler->startDocument(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}
	}
	else if(event->eventType == EVENT_ED)
	{
		if(handler->endDocument != NULL)
		{
			if(handler->endDocument(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}
	}
	else if(event->eventType == EVENT_EE)
	{
		if(handler->endElement != NULL)
		{
			if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}

		if(codeIndx > 0 && strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)   // #1# COMMENT
		{
			strm->sContext.curr_uriID = ruleArr[ruleIndx].prodArray[prodIndx].uriRowID;
			strm->sContext.curr_lnID = ruleArr[ruleIndx].prodArray[prodIndx].lnRowID;
			tmp_err_code = insertZeroProduction(&(ruleArr[ruleIndx]), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL,
												strm->sContext.curr_lnID, strm->sContext.curr_uriID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else if(event->eventType == EVENT_SC)
	{
		if(handler->selfContained != NULL)
		{
			if(handler->selfContained(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}
	}
	else // The event has content!
	{
		if(event->eventType == EVENT_CH)
		{
			if(codeIndx > 0 && strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)   // #2# COMMENT
			{
				tmp_err_code = insertZeroProduction(&(ruleArr[ruleIndx]),getEventDefType(EVENT_CH), *nonTermID_out,
													strm->sContext.curr_lnID, strm->sContext.curr_uriID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		else // event->eventType != EVENT_CH; CH events do not have QName in their content
		{
			strm->sContext.curr_uriID = ruleArr[ruleIndx].prodArray[prodIndx].uriRowID;
			strm->sContext.curr_lnID = ruleArr[ruleIndx].prodArray[prodIndx].lnRowID;
		}
		tmp_err_code = decodeEventContent(strm, *event, handler, nonTermID_out, &(ruleArr[ruleIndx]), app_data);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode createGrammarPool(GrammarPool** pool)
{
	*pool = (GrammarPool*) create_hashtable(GRAMMAR_POOL_DIMENSION, djbHash, keyEqual);
	if(*pool == NULL)
		return MEMORY_ALLOCATION_ERROR;

	return ERR_OK;
}

errorCode checkGrammarInPool(GrammarPool* pool, uint16_t uriRowID,
									size_t lnRowID, unsigned char* is_found, struct EXIGrammar** result)
{
	char key[8];
	createKey64bits(uriRowID, lnRowID, key);

	*result = hashtable_search(pool, key, 8);
	if(*result == NULL)
		*is_found = 0;
	else
		*is_found = 1;

	return ERR_OK;
}

errorCode addGrammarInPool(GrammarPool* pool, uint16_t uriRowID,
								size_t lnRowID, struct EXIGrammar* newGr)
{
	char* key = (char*) EXIP_MALLOC(8); // Keys are freed from the hash table
	if(key == NULL)
		return MEMORY_ALLOCATION_ERROR;
	createKey64bits(uriRowID, lnRowID, key);

	if (! hashtable_insert(pool, key, 8, newGr) )
		return HASH_TABLE_ERROR;

	return ERR_OK;
}

#endif /* BUILTINDOCGRAMMAR_H_ */
