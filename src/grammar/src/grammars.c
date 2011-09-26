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
#include "sTables.h"
#include "memManagement.h"
#include "ioUtil.h"
#include "streamDecode.h"
#include "bodyDecode.h"

#define DEF_DOC_GRAMMAR_RULE_NUMBER 3
#define DEF_ELEMENT_GRAMMAR_RULE_NUMBER 2

static errorCode handleProduction(EXIStream* strm, GrammarRule* currentRule, Production* prodHit,
				EXIEvent* event, size_t* nonTermID_out, ContentHandler* handler, void* app_data, unsigned int codeLength);

errorCode createDocGrammar(EXIGrammar* docGrammar, EXIStream* strm, const EXIPSchema* schema)
{
	GrammarRule* tmp_rule;
	unsigned int tmp_code1 = 0; // the number of productions with event codes with length 1
	unsigned int tmp_code2 = 0; // the number of productions with event codes with length 2
	unsigned int tmp_code3 = 0; // the number of productions with event codes with length 3

	docGrammar->rulesDimension = DEF_DOC_GRAMMAR_RULE_NUMBER;
	docGrammar->grammarType = GR_TYPE_BUILD_IN_DOC;
	docGrammar->contentIndex = 0;
	docGrammar->isNillable = FALSE;
	docGrammar->ruleArray = (GrammarRule*) memManagedAllocate(&strm->memList, sizeof(GrammarRule)*DEF_DOC_GRAMMAR_RULE_NUMBER);
	if(docGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* Document : SD DocContent	0 */
	tmp_rule = &docGrammar->ruleArray[GR_DOCUMENT];
	tmp_rule->prodArrays[0] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
	if(tmp_rule->prodArrays[0] == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->prodArrays[0]->event = getEventDefType(EVENT_SD);
	tmp_rule->prodArrays[0]->nonTermID = GR_DOC_CONTENT;
	tmp_rule->prodCounts[0] = 1;
	tmp_rule->bits[0] = 0;
	tmp_rule->bits[1] = 0;
	tmp_rule->bits[2] = 0;
	tmp_rule->prodArrays[1] = NULL;
	tmp_rule->prodCounts[1] = 0;
	tmp_rule->prodArrays[2] = NULL;
	tmp_rule->prodCounts[2] = 0;


	tmp_rule = &docGrammar->ruleArray[GR_DOC_CONTENT];
	tmp_rule->prodArrays[1] = NULL;
	tmp_rule->prodCounts[1] = 0;
	tmp_rule->prodArrays[2] = NULL;
	tmp_rule->prodCounts[2] = 0;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		tmp_code3 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		tmp_code3 += 1;

	if(schema != NULL)   // Creates Schema Informed Grammar
	{
		unsigned int e = 0;

		docGrammar->grammarType = GR_TYPE_SCHEMA_DOC;
		tmp_code1 = schema->globalElemGrammarsCount + 1;

		tmp_rule->prodArrays[0] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code1);
		if(tmp_rule->prodArrays[0] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->prodCounts[0] = tmp_code1;
		tmp_rule->bits[0] = getBitsNumber(schema->globalElemGrammarsCount + ((tmp_code2 + tmp_code3) > 0));

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

		for(e = 0; e < schema->globalElemGrammarsCount; e++)
		{
			tmp_rule->prodArrays[0][schema->globalElemGrammarsCount - e].event = getEventDefType(EVENT_SE_QNAME);
			tmp_rule->prodArrays[0][schema->globalElemGrammarsCount - e].nonTermID = GR_DOC_END;
			tmp_rule->prodArrays[0][schema->globalElemGrammarsCount - e].lnRowID = schema->globalElemGrammars[e].lnRowId;
			tmp_rule->prodArrays[0][schema->globalElemGrammarsCount - e].uriRowID = schema->globalElemGrammars[e].uriRowId;
		}
	}
	else
	{
		tmp_rule->prodArrays[0] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
		if(tmp_rule->prodArrays[0] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_code1 = 1;

		tmp_rule->prodCounts[0] = 1;
		tmp_rule->bits[0] = (tmp_code2 + tmp_code3) > 0;
	}

	/*
	   DocContent :
					SE (*) DocEnd	0
					DT DocContent	1.0
					CM DocContent	1.1.0
					PI DocContent	1.1.1
	 */

	tmp_rule->bits[1] = (tmp_code2 && (tmp_code3 > 0));
	tmp_rule->bits[2] = tmp_code3 > 1;

	tmp_rule->prodArrays[0][0].event = getEventDefType(EVENT_SE_ALL);
	tmp_rule->prodArrays[0][0].nonTermID = GR_DOC_END;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
	{
		tmp_rule->prodArrays[1] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
		if(tmp_rule->prodArrays[1] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->prodCounts[1] = 1;
		tmp_rule->prodArrays[1]->event = getEventDefType(EVENT_DT);
		tmp_rule->prodArrays[1]->nonTermID = GR_DOC_CONTENT;
	}
	if(tmp_code3 > 0)
	{
		tmp_rule->prodArrays[2] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code3);
		if(tmp_rule->prodArrays[2] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->prodCounts[2] = tmp_code3;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			tmp_rule->prodArrays[2][tmp_code3 - 1].event = getEventDefType(EVENT_CM);
			tmp_rule->prodArrays[2][tmp_code3 - 1].nonTermID = GR_DOC_CONTENT;
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			tmp_rule->prodArrays[2][0].event = getEventDefType(EVENT_PI);
			tmp_rule->prodArrays[2][0].nonTermID = GR_DOC_CONTENT;
		}
	}


	/* DocEnd :
				ED	        0
				CM DocEnd	1.0
				PI DocEnd	1.1 */

	tmp_rule = &docGrammar->ruleArray[GR_DOC_END];
	tmp_code1 = 1;
	tmp_code2 = 0;
	tmp_code3 = 0;
	tmp_rule->prodArrays[1] = NULL;
	tmp_rule->prodCounts[1] = 0;
	tmp_rule->prodArrays[2] = NULL;
	tmp_rule->prodCounts[2] = 0;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		tmp_code2 += 1;

	tmp_rule->bits[0] = tmp_code2 > 0;
	tmp_rule->bits[1] = tmp_code2 > 1;
	tmp_rule->bits[2] = 0;

	tmp_rule->prodArrays[0] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
	if(tmp_rule->prodArrays[0] == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->prodCounts[0] = 1;
	tmp_rule->prodArrays[0]->event = getEventDefType(EVENT_ED);
	tmp_rule->prodArrays[0]->nonTermID = GR_VOID_NON_TERMINAL;

	if(tmp_code2 > 0)
	{
		tmp_rule->prodArrays[1] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
		if(tmp_rule->prodArrays[1] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->prodCounts[1] = tmp_code2;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			tmp_rule->prodArrays[1][tmp_code2 - 1].event = getEventDefType(EVENT_CM);
			tmp_rule->prodArrays[1][tmp_code2 - 1].nonTermID = GR_DOC_END;
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			tmp_rule->prodArrays[1][0].event = getEventDefType(EVENT_PI);
			tmp_rule->prodArrays[1][0].nonTermID = GR_DOC_END;
		}
	}

	return ERR_OK;
}

errorCode createBuildInElementGrammar(EXIGrammar* elementGrammar, EXIStream* strm)
{
	unsigned int tmp_code1 = 0; // the number of productions with event codes with length 1
	unsigned int tmp_code2 = 0; // the number of productions with event codes with length 2
	unsigned int tmp_code3 = 0; // the number of productions with event codes with length 3
	DynGrammarRule* tmp_rule;
	unsigned int p = 1;

	elementGrammar->rulesDimension = DEF_ELEMENT_GRAMMAR_RULE_NUMBER;
	elementGrammar->grammarType = GR_TYPE_BUILD_IN_ELEM;
	elementGrammar->contentIndex = 0;
	elementGrammar->isNillable = FALSE;
	elementGrammar->ruleArray = (GrammarRule*) memManagedAllocate(&strm->memList, sizeof(DynGrammarRule)*DEF_ELEMENT_GRAMMAR_RULE_NUMBER);
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

	tmp_rule = (DynGrammarRule*) &elementGrammar->ruleArray[GR_START_TAG_CONTENT];
	tmp_rule->prodArrays[0] = (Production*) memManagedAllocatePtr(&strm->memList, sizeof(Production)*DEFAULT_PROD_ARRAY_DIM, &tmp_rule->memPair);
	if(tmp_rule->prodArrays[0] == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->prodCounts[0] = 0;
	tmp_rule->prod1Dimension = DEFAULT_PROD_ARRAY_DIM;
	tmp_rule->prodArrays[1] = NULL;
	tmp_rule->prodCounts[1] = 0;
	tmp_rule->prodArrays[2] = NULL;
	tmp_rule->prodCounts[2] = 0;

	tmp_code1 = 0;
	tmp_code2 = 4;
	tmp_code3 = 0;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
		tmp_code2 += 1;
	if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		tmp_code3 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		tmp_code3 += 1;

	tmp_rule->bits[0] = 0;
	tmp_rule->bits[1] = getBitsNumber(tmp_code2 - 1 + (tmp_code3 > 0));
	tmp_rule->bits[2] = tmp_code3 > 1;


	tmp_rule->prodArrays[1] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
	if(tmp_rule->prodArrays[1] == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->prodCounts[1] = tmp_code2;

	/* EE	                    0.0 */
	tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_EE);
	tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_VOID_NON_TERMINAL;
	p += 1;

	/* AT (*) StartTagContent	0.1 */
	tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_AT_ALL);
	tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_START_TAG_CONTENT;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
	{
		/* NS StartTagContent	    0.2 */
		tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_NS);
		tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_START_TAG_CONTENT;
		p += 1;
	}

	if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
	{
		/* SC Fragment	            0.3 */
		tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_SC);
		tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_FRAGMENT;
		p += 1;
	}

	/* SE (*) ElementContent	0.2 */
	tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_SE_ALL);
	tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	p += 1;

	/* CH ElementContent	    0.3 */
	tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_CH);
	tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
	{
		/* ER ElementContent	    0.6 */
		tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_ER);
		tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
		p += 1;
	}

	if(tmp_code3 > 0)
	{
		p = 1;

		tmp_rule->prodArrays[2] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code3);
		if(tmp_rule->prodArrays[2] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->prodCounts[2] = tmp_code3;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			/* CM ElementContent	    0.7.0 */
			tmp_rule->prodArrays[2][tmp_code3-p].event = getEventDefType(EVENT_CM);
			tmp_rule->prodArrays[2][tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
			p += 1;
		}
		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			/* PI ElementContent	    0.7.1 */
			tmp_rule->prodArrays[2][tmp_code3-p].event = getEventDefType(EVENT_PI);
			tmp_rule->prodArrays[2][tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
		}
	}

	p = 1;

	/* ElementContent :
							EE	                    0
							SE (*) ElementContent	1.0
							CH ElementContent	    1.1
							ER ElementContent	    1.2
							CM ElementContent	    1.3.0
							PI ElementContent	    1.3.1 */

	tmp_rule = (DynGrammarRule*) &elementGrammar->ruleArray[GR_ELEMENT_CONTENT];
	tmp_rule->prodArrays[0] = (Production*) memManagedAllocatePtr(&strm->memList, sizeof(Production)*DEFAULT_PROD_ARRAY_DIM, &tmp_rule->memPair);
	if(tmp_rule->prodArrays[0] == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* EE	                  0 */
	tmp_rule->prodArrays[0][0].event = getEventDefType(EVENT_EE);
	tmp_rule->prodArrays[0][0].nonTermID = GR_VOID_NON_TERMINAL;
	tmp_rule->prodCounts[0] = 1;
	tmp_rule->prod1Dimension = DEFAULT_PROD_ARRAY_DIM;
	tmp_rule->prodArrays[1] = NULL;
	tmp_rule->prodCounts[1] = 0;
	tmp_rule->prodArrays[2] = NULL;
	tmp_rule->prodCounts[2] = 0;

	tmp_code1 = 1;
	tmp_code2 = 2;
	tmp_code3 = 0;


	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		tmp_code3 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		tmp_code3 += 1;

	tmp_rule->bits[0] = 1;
	tmp_rule->bits[1] = 1 + ((tmp_code2 - 2 + tmp_code3) > 0);
	tmp_rule->bits[2] = tmp_code3 > 1;


	tmp_rule->prodArrays[1] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
	if(tmp_rule->prodArrays[1] == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->prodCounts[1] = tmp_code2;

	/* SE (*) ElementContent	1.0 */
	tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_SE_ALL);
	tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	p += 1;

	/* CH ElementContent	    1.1 */
	tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_CH);
	tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
	{
		/* ER ElementContent	    1.2 */
		tmp_rule->prodArrays[1][tmp_code2-p].event = getEventDefType(EVENT_ER);
		tmp_rule->prodArrays[1][tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	}

	if(tmp_code3 > 0)
	{
		p = 1;

		tmp_rule->prodArrays[2] = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code3);
		if(tmp_rule->prodArrays[2] == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->prodCounts[2] = tmp_code3;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			/* CM ElementContent	    1.3.0 */
			tmp_rule->prodArrays[2][tmp_code3-p].event = getEventDefType(EVENT_CM);
			tmp_rule->prodArrays[2][tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
			p += 1;
		}
		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			/* PI ElementContent	    1.3.1 */
			tmp_rule->prodArrays[2][tmp_code3-p].event = getEventDefType(EVENT_PI);
			tmp_rule->prodArrays[2][tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
		}
	}

	return ERR_OK;
}

errorCode pushGrammar(EXIGrammarStack** gStack, EXIGrammar* grammar)
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

void popGrammar(EXIGrammarStack** gStack, EXIGrammar** grammar)
{
	struct GrammarStackNode* node = *gStack;
	*gStack = (*gStack)->nextInStack;

	(*grammar) = node->grammar;
	EXIP_MFREE(node);
}

errorCode processNextProduction(EXIStream* strm, EXIEvent* event,
							    size_t* nonTermID_out, ContentHandler* handler, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_bits_val = 0;
	unsigned char b = 0;
	GrammarRule* currentRule;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Next production non-term-id: %u\n", (unsigned int) strm->context.nonTermID));

	if(strm->context.nonTermID >=  strm->gStack->grammar->rulesDimension)
		return INCONSISTENT_PROC_STATE;

	currentRule = &strm->gStack->grammar->ruleArray[strm->context.nonTermID];

#if DEBUG_GRAMMAR == ON
	{
		tmp_err_code = printGrammarRule(strm->context.nonTermID, currentRule);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Error printing grammar rule\n"));
		}
	}
#endif

	for(b = 0; b < 3; b++)
	{
		if(currentRule->prodCounts[b] == 0) // No productions available with this length code
			continue;
		if(currentRule->bits[b] == 0) // encoded with zero bits
			return handleProduction(strm, currentRule, &currentRule->prodArrays[b][0], event, nonTermID_out, handler, app_data, b + 1);

		tmp_err_code = decodeNBitUnsignedInteger(strm, currentRule->bits[b], &tmp_bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(tmp_bits_val == currentRule->prodCounts[b]) // The code has more parts
			continue;

		return handleProduction(strm, currentRule, &currentRule->prodArrays[b][currentRule->prodCounts[b] - 1 - tmp_bits_val], event, nonTermID_out, handler, app_data, b + 1);
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

static errorCode handleProduction(EXIStream* strm, GrammarRule* currentRule, Production* prodHit,
				EXIEvent* event, size_t* nonTermID_out, ContentHandler* handler, void* app_data, unsigned int codeLength)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	*event = prodHit->event;
	*nonTermID_out = prodHit->nonTermID;
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

		if(codeLength > 1 && strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)   // #1# COMMENT
		{
			strm->context.curr_uriID = prodHit->uriRowID;
			strm->context.curr_lnID = prodHit->lnRowID;
			tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL,
												strm->context.curr_lnID, strm->context.curr_uriID);
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
			if(codeLength > 1 && strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)   // #2# COMMENT
			{
				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, getEventDefType(EVENT_CH), *nonTermID_out,
													strm->context.curr_lnID, strm->context.curr_uriID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		else // event->eventType != EVENT_CH; CH events do not have QName in their content
		{
			strm->context.curr_uriID = prodHit->uriRowID;
			strm->context.curr_lnID = prodHit->lnRowID;
		}
		tmp_err_code = decodeEventContent(strm, *event, handler, nonTermID_out, currentRule, app_data);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

#endif /* BUILTINDOCGRAMMAR_H_ */
