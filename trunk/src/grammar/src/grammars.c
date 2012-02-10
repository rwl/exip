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
#define DEF_FRAG_GRAMMAR_RULE_NUMBER 2
#define DEF_ELEMENT_GRAMMAR_RULE_NUMBER 2

static errorCode handleProduction(EXIStream* strm, GrammarRule* currentRule, Production* prodHit,
				EXIEvent* evnt, size_t* nonTermID_out, ContentHandler* handler, void* app_data, unsigned int codeLength);

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
	docGrammar->isAugmented = TRUE;
	docGrammar->ruleArray = (GrammarRule*) memManagedAllocate(&strm->memList, sizeof(GrammarRule)*DEF_DOC_GRAMMAR_RULE_NUMBER);
	if(docGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* Document : SD DocContent	0 */
	tmp_rule = &docGrammar->ruleArray[GR_DOCUMENT];
	tmp_rule->part[0].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
	if(tmp_rule->part[0].prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->part[0].prodArray->evnt = getEventDefType(EVENT_SD);
	tmp_rule->part[0].prodArray->nonTermID = GR_DOC_CONTENT;
	tmp_rule->part[0].prodArraySize = 1;
	tmp_rule->part[0].bits = 0;
	tmp_rule->part[1].bits = 0;
	tmp_rule->part[2].bits = 0;
	tmp_rule->part[1].prodArray = NULL;
	tmp_rule->part[1].prodArraySize = 0;
	tmp_rule->part[2].prodArray = NULL;
	tmp_rule->part[2].prodArraySize = 0;


	tmp_rule = &docGrammar->ruleArray[GR_DOC_CONTENT];
	tmp_rule->part[1].prodArray = NULL;
	tmp_rule->part[1].prodArraySize = 0;
	tmp_rule->part[2].prodArray = NULL;
	tmp_rule->part[2].prodArraySize = 0;
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

		tmp_rule->part[0].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code1);
		if(tmp_rule->part[0].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[0].prodArraySize = tmp_code1;
		tmp_rule->part[0].bits = getBitsNumber(schema->globalElemGrammarsCount + ((tmp_code2 + tmp_code3) > 0));

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
			tmp_rule->part[0].prodArray[schema->globalElemGrammarsCount - e].evnt = getEventDefType(EVENT_SE_QNAME);
			tmp_rule->part[0].prodArray[schema->globalElemGrammarsCount - e].nonTermID = GR_DOC_END;
			tmp_rule->part[0].prodArray[schema->globalElemGrammarsCount - e].qname.lnRowId = schema->globalElemGrammars[e].lnRowId;
			tmp_rule->part[0].prodArray[schema->globalElemGrammarsCount - e].qname.uriRowId = schema->globalElemGrammars[e].uriRowId;
		}
	}
	else
	{
		tmp_rule->part[0].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
		if(tmp_rule->part[0].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_code1 = 1;

		tmp_rule->part[0].prodArraySize = 1;
		tmp_rule->part[0].bits = (tmp_code2 + tmp_code3) > 0;
	}

	/*
	   DocContent :
					SE (*) DocEnd	0
					DT DocContent	1.0
					CM DocContent	1.1.0
					PI DocContent	1.1.1
	 */

	tmp_rule->part[1].bits = (tmp_code2 && (tmp_code3 > 0));
	tmp_rule->part[2].bits = tmp_code3 > 1;

	tmp_rule->part[0].prodArray[0].evnt = getEventDefType(EVENT_SE_ALL);
	tmp_rule->part[0].prodArray[0].nonTermID = GR_DOC_END;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
	{
		tmp_rule->part[1].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
		if(tmp_rule->part[1].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[1].prodArraySize = 1;
		tmp_rule->part[1].prodArray->evnt = getEventDefType(EVENT_DT);
		tmp_rule->part[1].prodArray->nonTermID = GR_DOC_CONTENT;
	}
	if(tmp_code3 > 0)
	{
		tmp_rule->part[2].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code3);
		if(tmp_rule->part[2].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[2].prodArraySize = tmp_code3;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			tmp_rule->part[2].prodArray[tmp_code3 - 1].evnt = getEventDefType(EVENT_CM);
			tmp_rule->part[2].prodArray[tmp_code3 - 1].nonTermID = GR_DOC_CONTENT;
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			tmp_rule->part[2].prodArray[0].evnt = getEventDefType(EVENT_PI);
			tmp_rule->part[2].prodArray[0].nonTermID = GR_DOC_CONTENT;
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
	tmp_rule->part[1].prodArray = NULL;
	tmp_rule->part[1].prodArraySize = 0;
	tmp_rule->part[2].prodArray = NULL;
	tmp_rule->part[2].prodArraySize = 0;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		tmp_code2 += 1;

	tmp_rule->part[0].bits = tmp_code2 > 0;
	tmp_rule->part[1].bits = tmp_code2 > 1;
	tmp_rule->part[2].bits = 0;

	tmp_rule->part[0].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
	if(tmp_rule->part[0].prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->part[0].prodArraySize = 1;
	tmp_rule->part[0].prodArray->evnt = getEventDefType(EVENT_ED);
	tmp_rule->part[0].prodArray->nonTermID = GR_VOID_NON_TERMINAL;

	if(tmp_code2 > 0)
	{
		tmp_rule->part[1].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
		if(tmp_rule->part[1].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[1].prodArraySize = tmp_code2;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			tmp_rule->part[1].prodArray[tmp_code2 - 1].evnt = getEventDefType(EVENT_CM);
			tmp_rule->part[1].prodArray[tmp_code2 - 1].nonTermID = GR_DOC_END;
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			tmp_rule->part[1].prodArray[0].evnt = getEventDefType(EVENT_PI);
			tmp_rule->part[1].prodArray[0].nonTermID = GR_DOC_END;
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
	elementGrammar->isAugmented = TRUE;
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
	tmp_rule->part[0].prodArray = (Production*) memManagedAllocatePtr(&strm->memList, sizeof(Production)*DEFAULT_PROD_ARRAY_DIM, &tmp_rule->memPair);
	if(tmp_rule->part[0].prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->part[0].prodArraySize = 0;
	tmp_rule->part0Dimension = DEFAULT_PROD_ARRAY_DIM;
	tmp_rule->part[1].prodArray = NULL;
	tmp_rule->part[1].prodArraySize = 0;
	tmp_rule->part[2].prodArray = NULL;
	tmp_rule->part[2].prodArraySize = 0;

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

	tmp_rule->part[0].bits = 0;
	tmp_rule->part[1].bits = getBitsNumber(tmp_code2 - 1 + (tmp_code3 > 0));
	tmp_rule->part[2].bits = tmp_code3 > 1;


	tmp_rule->part[1].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
	if(tmp_rule->part[1].prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->part[1].prodArraySize = tmp_code2;

	/* EE	                    0.0 */
	tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_EE);
	tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_VOID_NON_TERMINAL;
	p += 1;

	/* AT (*) StartTagContent	0.1 */
	tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_AT_ALL);
	tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_START_TAG_CONTENT;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
	{
		/* NS StartTagContent	    0.2 */
		tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_NS);
		tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_START_TAG_CONTENT;
		p += 1;
	}

	if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
	{
		/* SC Fragment	            0.3 */
		tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_SC);
		tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_FRAGMENT;
		p += 1;
	}

	/* SE (*) ElementContent	0.2 */
	tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_SE_ALL);
	tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	p += 1;

	/* CH ElementContent	    0.3 */
	tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_CH);
	tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
	{
		/* ER ElementContent	    0.6 */
		tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_ER);
		tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
		p += 1;
	}

	if(tmp_code3 > 0)
	{
		p = 1;

		tmp_rule->part[2].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code3);
		if(tmp_rule->part[2].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[2].prodArraySize = tmp_code3;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			/* CM ElementContent	    0.7.0 */
			tmp_rule->part[2].prodArray[tmp_code3-p].evnt = getEventDefType(EVENT_CM);
			tmp_rule->part[2].prodArray[tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
			p += 1;
		}
		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			/* PI ElementContent	    0.7.1 */
			tmp_rule->part[2].prodArray[tmp_code3-p].evnt = getEventDefType(EVENT_PI);
			tmp_rule->part[2].prodArray[tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
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
	tmp_rule->part[0].prodArray = (Production*) memManagedAllocatePtr(&strm->memList, sizeof(Production)*DEFAULT_PROD_ARRAY_DIM, &tmp_rule->memPair);
	if(tmp_rule->part[0].prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* EE	                  0 */
	tmp_rule->part[0].prodArray[0].evnt = getEventDefType(EVENT_EE);
	tmp_rule->part[0].prodArray[0].nonTermID = GR_VOID_NON_TERMINAL;
	tmp_rule->part[0].prodArraySize = 1;
	tmp_rule->part0Dimension = DEFAULT_PROD_ARRAY_DIM;
	tmp_rule->part[1].prodArray = NULL;
	tmp_rule->part[1].prodArraySize = 0;
	tmp_rule->part[2].prodArray = NULL;
	tmp_rule->part[2].prodArraySize = 0;

	tmp_code1 = 1;
	tmp_code2 = 2;
	tmp_code3 = 0;


	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		tmp_code3 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		tmp_code3 += 1;

	tmp_rule->part[0].bits = 1;
	tmp_rule->part[1].bits = 1 + ((tmp_code2 - 2 + tmp_code3) > 0);
	tmp_rule->part[2].bits = tmp_code3 > 1;


	tmp_rule->part[1].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
	if(tmp_rule->part[1].prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->part[1].prodArraySize = tmp_code2;

	/* SE (*) ElementContent	1.0 */
	tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_SE_ALL);
	tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	p += 1;

	/* CH ElementContent	    1.1 */
	tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_CH);
	tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
	{
		/* ER ElementContent	    1.2 */
		tmp_rule->part[1].prodArray[tmp_code2-p].evnt = getEventDefType(EVENT_ER);
		tmp_rule->part[1].prodArray[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	}

	if(tmp_code3 > 0)
	{
		p = 1;

		tmp_rule->part[2].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code3);
		if(tmp_rule->part[2].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[2].prodArraySize = tmp_code3;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			/* CM ElementContent	    1.3.0 */
			tmp_rule->part[2].prodArray[tmp_code3-p].evnt = getEventDefType(EVENT_CM);
			tmp_rule->part[2].prodArray[tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
			p += 1;
		}
		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			/* PI ElementContent	    1.3.1 */
			tmp_rule->part[2].prodArray[tmp_code3-p].evnt = getEventDefType(EVENT_PI);
			tmp_rule->part[2].prodArray[tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
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

errorCode processNextProduction(EXIStream* strm, EXIEvent* evnt,
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
		if(currentRule->part[b].prodArraySize == 0) // No productions available with this length code
			continue;
		if(currentRule->part[b].bits == 0) // encoded with zero bits
			return handleProduction(strm, currentRule, &currentRule->part[b].prodArray[0], evnt, nonTermID_out, handler, app_data, b + 1);

		tmp_err_code = decodeNBitUnsignedInteger(strm, currentRule->part[b].bits, &tmp_bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(tmp_bits_val == currentRule->part[b].prodArraySize) // The code has more parts
			continue;

		return handleProduction(strm, currentRule, &currentRule->part[b].prodArray[currentRule->part[b].prodArraySize - 1 - tmp_bits_val], evnt, nonTermID_out, handler, app_data, b + 1);
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
				EXIEvent* evnt, size_t* nonTermID_out, ContentHandler* handler, void* app_data, unsigned int codeLength)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	*evnt = prodHit->evnt;
	*nonTermID_out = prodHit->nonTermID;

	// TODO: consider using switch{} statement instead of if

	if(evnt->eventType == EVENT_SD)
	{
		if(handler->startDocument != NULL)
		{
			if(handler->startDocument(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}
	}
	else if(evnt->eventType == EVENT_ED)
	{
		if(handler->endDocument != NULL)
		{
			if(handler->endDocument(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}
	}
	else if(evnt->eventType == EVENT_EE)
	{
		if(handler->endElement != NULL)
		{
			if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}

		if(codeLength > 1 && strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)   // #1# COMMENT
		{
			tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL,
												SIZE_MAX, UINT16_MAX);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else if(evnt->eventType == EVENT_SC)
	{
		if(handler->selfContained != NULL)
		{
			if(handler->selfContained(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}
	}
	else // The event has content!
	{
		if(evnt->eventType == EVENT_CH)
		{
			if(codeLength > 1 && strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)   // #2# COMMENT
			{
				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, getEventDefType(EVENT_CH), *nonTermID_out, SIZE_MAX, UINT16_MAX);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}

		tmp_err_code = decodeEventContent(strm, *evnt, handler, nonTermID_out, currentRule, app_data, prodHit->qname.uriRowId, prodHit->qname.lnRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode createFragmentGrammar(EXIGrammar* fragGrammar, EXIStream* strm, const EXIPSchema* schema)
{
	GrammarRule* tmp_rule;
	unsigned int tmp_code1 = 0; // the number of productions with event codes with length 1
	unsigned int tmp_code2 = 0; // the number of productions with event codes with length 2

	fragGrammar->rulesDimension = DEF_FRAG_GRAMMAR_RULE_NUMBER;
	fragGrammar->grammarType = GR_TYPE_BUILD_IN_FRAG;
	fragGrammar->contentIndex = 0;
	fragGrammar->isNillable = FALSE;
	fragGrammar->isAugmented = TRUE;
	fragGrammar->ruleArray = (GrammarRule*) memManagedAllocate(&strm->memList, sizeof(GrammarRule)*DEF_FRAG_GRAMMAR_RULE_NUMBER);
	if(fragGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* Fragment : SD FragmentContent	0 */
	tmp_rule = &fragGrammar->ruleArray[GR_FRAGMENT];
	tmp_rule->part[0].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production));
	if(tmp_rule->part[0].prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_rule->part[0].prodArray->evnt = getEventDefType(EVENT_SD);
	tmp_rule->part[0].prodArray->nonTermID = GR_FRAGMENT_CONTENT;
	tmp_rule->part[0].prodArraySize = 1;
	tmp_rule->part[0].bits = 0;
	tmp_rule->part[1].bits = 0;
	tmp_rule->part[2].bits = 0;
	tmp_rule->part[1].prodArray = NULL;
	tmp_rule->part[1].prodArraySize = 0;
	tmp_rule->part[2].prodArray = NULL;
	tmp_rule->part[2].prodArraySize = 0;


	tmp_rule = &fragGrammar->ruleArray[GR_FRAGMENT_CONTENT];
	tmp_rule->part[1].prodArray = NULL;
	tmp_rule->part[1].prodArraySize = 0;
	tmp_rule->part[2].prodArray = NULL;
	tmp_rule->part[2].prodArraySize = 0;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		tmp_code2 += 1;

	if(schema != NULL)   // Creates Schema Informed Grammar
	{
		unsigned int e = 0;

		fragGrammar->grammarType = GR_TYPE_SCHEMA_FRAG;
		tmp_code1 = schema->globalElemGrammarsCount + 2;

		tmp_rule->part[0].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code1);
		if(tmp_rule->part[0].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[0].prodArraySize = tmp_code1;
		tmp_rule->part[0].bits = getBitsNumber(schema->globalElemGrammarsCount + (tmp_code2 > 0));

		/*
		   FragmentContent :
					   	SE (F-0)   FragmentContent	0
						SE (F-1)   FragmentContent	1
						⋮	⋮      ⋮
						SE (F-n−1) FragmentContent  n-1
					//	SE (*)     FragmentContent	n		//  This is created as part of the Build-In grammar down
 					//	ED		   					n+1		//  This is created as part of the Build-In grammar down
					//	CM FragmentContent			n+2.0	//  This is created as part of the Build-In grammar down
					//	PI FragmentContent			n+2.1	//  This is created as part of the Build-In grammar down
		 */

		for(e = 0; e < schema->globalElemGrammarsCount; e++)
		{
			tmp_rule->part[0].prodArray[schema->globalElemGrammarsCount - e].evnt = getEventDefType(EVENT_SE_QNAME);
			tmp_rule->part[0].prodArray[schema->globalElemGrammarsCount - e].nonTermID = GR_FRAGMENT_CONTENT;
			tmp_rule->part[0].prodArray[schema->globalElemGrammarsCount - e].qname.lnRowId = schema->globalElemGrammars[e].lnRowId;
			tmp_rule->part[0].prodArray[schema->globalElemGrammarsCount - e].qname.uriRowId = schema->globalElemGrammars[e].uriRowId;
		}
	}
	else
	{
		tmp_code1 = 2;
		tmp_rule->part[0].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code1);
		if(tmp_rule->part[0].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[0].prodArraySize = 2;
		tmp_rule->part[0].bits = 1 + (tmp_code2 > 0);
	}

	/*
	   FragmentContent :
					SE (*) FragmentContent	0
					ED						1
					CM FragmentContent		2.0
					PI FragmentContent		2.1
	 */

	tmp_rule->part[1].bits = (tmp_code2 > 1);
	tmp_rule->part[2].bits = 0;

	tmp_rule->part[0].prodArray[0].evnt = getEventDefType(EVENT_ED);
	tmp_rule->part[0].prodArray[0].nonTermID = GR_VOID_NON_TERMINAL;

	tmp_rule->part[0].prodArray[1].evnt = getEventDefType(EVENT_SE_ALL);
	tmp_rule->part[0].prodArray[1].nonTermID = GR_FRAGMENT_CONTENT;

	if(tmp_code2 > 0)
	{
		tmp_rule->part[1].prodArray = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
		if(tmp_rule->part[1].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[1].prodArraySize = tmp_code2;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			tmp_rule->part[1].prodArray[tmp_code2 - 1].evnt = getEventDefType(EVENT_CM);
			tmp_rule->part[1].prodArray[tmp_code2 - 1].nonTermID = GR_FRAGMENT_CONTENT;
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			tmp_rule->part[1].prodArray[0].evnt = getEventDefType(EVENT_PI);
			tmp_rule->part[1].prodArray[0].nonTermID = GR_FRAGMENT_CONTENT;
		}
	}

	return ERR_OK;
}

#endif /* BUILTINDOCGRAMMAR_H_ */
