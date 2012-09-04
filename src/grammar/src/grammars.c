/*==================================================================*\
|                EXIP - Embeddable EXI Processor in   C              |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammars.c
 * @brief Defines grammar related functions
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.4
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

extern Production static_prod_start_doc_part0[1];
extern Production static_prod_doc_end_part0[1];

static errorCode handleProduction(EXIStream* strm, GrammarRule* currentRule, Production* prodHit,
				SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data, unsigned int codeLength);

errorCode createDocGrammar(EXIPSchema* schema, QNameID* elQnameArr, Index qnameCount)
{
	GrammarRule* tmp_rule;

	schema->docGrammar.count = DEF_DOC_GRAMMAR_RULE_NUMBER;
	schema->docGrammar.props = 0;
	schema->docGrammar.contentIndex = 0;
	schema->docGrammar.rule = (GrammarRule*) memManagedAllocate(&schema->memList, sizeof(GrammarRule)*DEF_DOC_GRAMMAR_RULE_NUMBER);
	if(schema->docGrammar.rule == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* Rule for Document */
	/*
	 * Document :
	 *			  SD DocContent	0
	 */
	tmp_rule = &schema->docGrammar.rule[GR_DOCUMENT];

	/* Part 1 */
	tmp_rule->part[0].prod = static_prod_start_doc_part0;
	tmp_rule->part[0].count = 1;
	tmp_rule->part[0].bits = 0;

	/* Initialize Part 2 */
	tmp_rule->part[1].prod = NULL;
	tmp_rule->part[1].count = 0;
	tmp_rule->part[1].bits = 0;

	/* Initialize Part 3 */
	tmp_rule->part[2].prod = NULL;
	tmp_rule->part[2].count = 0;
	tmp_rule->part[2].bits = 0;

	/* Rule for document content */
	tmp_rule = &schema->docGrammar.rule[GR_DOC_CONTENT];

	/* Initialize Part 2 */
	tmp_rule->part[1].prod = NULL;
	tmp_rule->part[1].count = 0;
	tmp_rule->part[1].bits = 0;

	/* Initialize Part 3 */
	tmp_rule->part[2].prod = NULL;
	tmp_rule->part[2].count = 0;
	tmp_rule->part[2].bits = 0;

	/* Part 1 */
	if(elQnameArr != NULL)   // Creates Schema Informed Grammar
	{
		unsigned int e = 0;
		Index tmp_code1;

		SET_SCHEMA(schema->docGrammar.props);
		tmp_code1 = qnameCount + 1;

		tmp_rule->part[0].prod = (Production*) memManagedAllocate(&schema->memList, sizeof(Production)*tmp_code1);
		if(tmp_rule->part[0].prod == NULL)
			return MEMORY_ALLOCATION_ERROR;

		/*
		 * DocContent :
		 *			   	SE (G-0)   DocEnd	0
		 *				SE (G-1)   DocEnd	1
		 *				⋮	⋮      ⋮
		 *				SE (G-n−1) DocEnd n-1
		 *			//	SE (*)     DocEnd	n		//  This is created as part of the Built-In grammar further on
		 */

		for(e = 0; e < qnameCount; e++)
		{
			tmp_rule->part[0].prod[qnameCount - e].eventType = EVENT_SE_QNAME;
			tmp_rule->part[0].prod[qnameCount - e].typeId = GET_LN_URI_QNAME(schema->uriTable, elQnameArr[e]).elemGrammar;
			tmp_rule->part[0].prod[qnameCount - e].nonTermID = GR_DOC_END;
			tmp_rule->part[0].prod[qnameCount - e].qnameId = elQnameArr[e];
		}
		tmp_rule->part[0].count = tmp_code1;
		tmp_rule->part[0].bits = getBitsNumber(qnameCount);
	}
	else
	{
		tmp_rule->part[0].prod = (Production*) memManagedAllocate(&schema->memList, sizeof(Production));
		if(tmp_rule->part[0].prod == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_rule->part[0].count = 1;
		tmp_rule->part[0].bits = 0;
	}

	/*
	 * DocContent :
	 *				SE (*) DocEnd	0
	 */
	tmp_rule->part[0].prod[0].eventType = EVENT_SE_ALL;
	tmp_rule->part[0].prod[0].typeId = INDEX_MAX;
	tmp_rule->part[0].prod[0].nonTermID = GR_DOC_END;
	tmp_rule->part[0].prod[0].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[0].prod[0].qnameId.lnId = INDEX_MAX;

	/* Rule for Document end */
	/* 
	 * DocEnd :
	 *			ED	        0
	 */
	tmp_rule = &schema->docGrammar.rule[GR_DOC_END];

	/* Part 1 */
	tmp_rule->part[0].prod = static_prod_doc_end_part0;
	tmp_rule->part[0].count = 1;
	tmp_rule->part[0].bits = 0;

	/* Initialize Part 2 */
	tmp_rule->part[1].prod = NULL;
	tmp_rule->part[1].count = 0;
	tmp_rule->part[1].bits = 0;

	/* Initialize Part 3 */
	tmp_rule->part[2].prod = NULL;
	tmp_rule->part[2].count = 0;
	tmp_rule->part[2].bits = 0;

	return ERR_OK;
}

errorCode createBuiltInElementGrammar(EXIGrammar* elementGrammar, EXIStream* strm)
{
	unsigned int tmp_code1 = 0; // the number of productions with event codes with length 1
	unsigned int tmp_code2 = 0; // the number of productions with event codes with length 2
	unsigned int tmp_code3 = 0; // the number of productions with event codes with length 3
	DynGrammarRule* tmp_rule;
	unsigned int p = 1;

	elementGrammar->count = DEF_ELEMENT_GRAMMAR_RULE_NUMBER;
	elementGrammar->props = 0;
	SET_BUILT_IN_ELEM(elementGrammar->props);
	SET_AUGMENTED(elementGrammar->props);
	elementGrammar->contentIndex = 0;
	elementGrammar->rule = (GrammarRule*) memManagedAllocate(&strm->memList, sizeof(DynGrammarRule)*DEF_ELEMENT_GRAMMAR_RULE_NUMBER);
	if(elementGrammar->rule == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* Rule for StartTagContent */
	/* StartTagContent :
	 *						EE	                    0.0
	 *						AT (*) StartTagContent	0.1
	 *						NS StartTagContent	    0.2
	 *						SC Fragment	            0.3
	 *						SE (*) ElementContent	0.4
	 *						CH ElementContent	    0.5
	 *						ER ElementContent	    0.6
	 *						CM ElementContent	    0.7.0
	 *						PI ElementContent	    0.7.1
	 */

	tmp_rule = &((DynGrammarRule*) elementGrammar->rule)[GR_START_TAG_CONTENT];

	/* Initialize Part 2 */
	tmp_rule->part[1].prod = NULL;
	tmp_rule->part[1].count = 0;
	tmp_rule->part[1].bits = 0;

	/* Initialize Part 3 */
	tmp_rule->part[2].prod = NULL;
	tmp_rule->part[2].count = 0;
	tmp_rule->part[2].bits = 0;

	tmp_code1 = 0;
	tmp_code2 = 4;
	tmp_code3 = 0;

	/* Part 1 */
	tmp_rule->part[0].prod = (Production*) memManagedAllocatePtr(&strm->memList, sizeof(Production)*DEFAULT_PROD_ARRAY_DIM, &tmp_rule->memPair);
	if(tmp_rule->part[0].prod == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* The part 1 productions get added later... */
	tmp_rule->part[0].count = 0;
	tmp_rule->part[0].bits = 0;
	tmp_rule->part0Dimension = DEFAULT_PROD_ARRAY_DIM;

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

	/* Part 2 */
	tmp_rule->part[1].prod = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
	if(tmp_rule->part[1].prod == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* EE	                    0.0 */
	tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_EE;
	tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_VOID_NON_TERMINAL;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
	p += 1;

	/* AT (*) StartTagContent	0.1 */
	tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_AT_ALL;
	tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_START_TAG_CONTENT;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
	{
		/* NS StartTagContent	    0.2 */
		tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_NS;
		tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
		tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_START_TAG_CONTENT;
		tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
		tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
		p += 1;
	}

	if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
	{
		/* SC Fragment	            0.3 */
		tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_SC;
		tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
		tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_FRAGMENT;
		tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
		tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
		p += 1;
	}

	/* SE (*) ElementContent	0.2 */
	tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_SE_ALL;
	tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
	p += 1;

	/* CH ElementContent	    0.3 */
	tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_CH;
	tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
	{
		/* ER ElementContent	    0.6 */
		tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_ER;
		tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
		tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
		tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
		tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
		p += 1;
	}

	tmp_rule->part[1].count = tmp_code2;
	tmp_rule->part[1].bits = getBitsNumber(tmp_code2 - 1 + (tmp_code3 > 0));

	/* Part 3 */
	if(tmp_code3 > 0)
	{
		p = 1;

		tmp_rule->part[2].prod = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code3);
		if(tmp_rule->part[2].prod == NULL)
			return MEMORY_ALLOCATION_ERROR;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			/* CM ElementContent	    0.7.0 */
			tmp_rule->part[2].prod[tmp_code3-p].eventType = EVENT_CM;
			tmp_rule->part[2].prod[tmp_code3-p].typeId = INDEX_MAX;
			tmp_rule->part[2].prod[tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
			tmp_rule->part[2].prod[tmp_code3-p].qnameId.uriId = SMALL_INDEX_MAX;
			tmp_rule->part[2].prod[tmp_code3-p].qnameId.lnId = INDEX_MAX;
			p += 1;
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			/* PI ElementContent	    0.7.1 */
			tmp_rule->part[2].prod[tmp_code3-p].eventType = EVENT_PI;
			tmp_rule->part[2].prod[tmp_code3-p].typeId = INDEX_MAX;
			tmp_rule->part[2].prod[tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
			tmp_rule->part[2].prod[tmp_code3-p].qnameId.uriId = SMALL_INDEX_MAX;
			tmp_rule->part[2].prod[tmp_code3-p].qnameId.lnId = INDEX_MAX;
		}

		tmp_rule->part[2].count = tmp_code3;
		tmp_rule->part[2].bits = tmp_code3 > 1;
	}

	p = 1;

	/* Rule for ElementContent */
	/* ElementContent :
	 *						EE	                    0
	 *						SE (*) ElementContent	1.0
	 *						CH ElementContent	    1.1
	 *						ER ElementContent	    1.2
	 *						CM ElementContent	    1.3.0
	 *						PI ElementContent	    1.3.1
	 */
	tmp_rule = &((DynGrammarRule*) elementGrammar->rule)[GR_ELEMENT_CONTENT];

	/* Initialize Part 2 */
	tmp_rule->part[1].prod = NULL;
	tmp_rule->part[1].count = 0;
	tmp_rule->part[1].bits = 0;

	/* Initialize Part 3 */
	tmp_rule->part[2].prod = NULL;
	tmp_rule->part[2].count = 0;
	tmp_rule->part[2].bits = 0;

	tmp_code1 = 1;
	tmp_code2 = 2;
	tmp_code3 = 0;

	/* Part 1 */
	tmp_rule->part[0].prod = (Production*) memManagedAllocatePtr(&strm->memList, sizeof(Production)*DEFAULT_PROD_ARRAY_DIM, &tmp_rule->memPair);
	if(tmp_rule->part[0].prod == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* EE	                  0 */
	tmp_rule->part[0].prod[0].eventType = EVENT_EE;
	tmp_rule->part[0].prod[0].typeId = INDEX_MAX;
	tmp_rule->part[0].prod[0].nonTermID = GR_VOID_NON_TERMINAL;
	tmp_rule->part[0].prod[0].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[0].prod[0].qnameId.lnId = INDEX_MAX;
	tmp_rule->part[0].count = 1;
	tmp_rule->part[0].bits = 1;
	tmp_rule->part0Dimension = DEFAULT_PROD_ARRAY_DIM;
	/* More part 1 productions get added later... */

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
		tmp_code2 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		tmp_code3 += 1;
	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		tmp_code3 += 1;

	/* Part 2 */
	tmp_rule->part[1].prod = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code2);
	if(tmp_rule->part[1].prod == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* SE (*) ElementContent	1.0 */
	tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_SE_ALL;
	tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
	p += 1;

	/* CH ElementContent	    1.1 */
	tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_CH;
	tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
	p += 1;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
	{
		/* ER ElementContent	    1.2 */
		tmp_rule->part[1].prod[tmp_code2-p].eventType = EVENT_ER;
		tmp_rule->part[1].prod[tmp_code2-p].typeId = INDEX_MAX;
		tmp_rule->part[1].prod[tmp_code2-p].nonTermID = GR_ELEMENT_CONTENT;
		tmp_rule->part[1].prod[tmp_code2-p].qnameId.uriId = SMALL_INDEX_MAX;
		tmp_rule->part[1].prod[tmp_code2-p].qnameId.lnId = INDEX_MAX;
	}

	tmp_rule->part[1].count = tmp_code2;
	tmp_rule->part[1].bits = 1 + ((tmp_code2 - 2 + tmp_code3) > 0);

	/* Part 3 */
	if(tmp_code3 > 0)
	{
		p = 1;

		tmp_rule->part[2].prod = (Production*) memManagedAllocate(&strm->memList, sizeof(Production)*tmp_code3);
		if(tmp_rule->part[2].prod == NULL)
			return MEMORY_ALLOCATION_ERROR;

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
		{
			/* CM ElementContent	    1.3.0 */
			tmp_rule->part[2].prod[tmp_code3-p].eventType = EVENT_CM;
			tmp_rule->part[2].prod[tmp_code3-p].typeId = INDEX_MAX;
			tmp_rule->part[2].prod[tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
			tmp_rule->part[2].prod[tmp_code3-p].qnameId.uriId = SMALL_INDEX_MAX;
			tmp_rule->part[2].prod[tmp_code3-p].qnameId.lnId = INDEX_MAX;
			p += 1;
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			/* PI ElementContent	    1.3.1 */
			tmp_rule->part[2].prod[tmp_code3-p].eventType = EVENT_PI;
			tmp_rule->part[2].prod[tmp_code3-p].typeId = INDEX_MAX;
			tmp_rule->part[2].prod[tmp_code3-p].nonTermID = GR_ELEMENT_CONTENT;
			tmp_rule->part[2].prod[tmp_code3-p].qnameId.uriId = SMALL_INDEX_MAX;
			tmp_rule->part[2].prod[tmp_code3-p].qnameId.lnId = INDEX_MAX;
		}

		tmp_rule->part[2].count = tmp_code3;
		tmp_rule->part[2].bits = tmp_code3 > 1;
	}

	return ERR_OK;
}

errorCode pushGrammar(EXIGrammarStack** gStack, EXIGrammar* grammar)
{
	struct GrammarStackNode* node = (struct GrammarStackNode*)EXIP_MALLOC(sizeof(struct GrammarStackNode));
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
	if((*gStack) == NULL)
	{
		(*grammar) = NULL;
	}
	else
	{
		node = *gStack;
		*gStack = (*gStack)->nextInStack;

		(*grammar) = node->grammar;
		EXIP_MFREE(node);
	}
}

errorCode processNextProduction(EXIStream* strm, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_bits_val = 0;
	unsigned char b = 0;
	GrammarRule* currentRule;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Next production non-term-id: %u\n", (unsigned int) strm->context.currNonTermID));

	if(strm->context.currNonTermID >=  strm->gStack->grammar->count)
		return INCONSISTENT_PROC_STATE;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[strm->context.currNonTermID];
	else
		currentRule = &strm->gStack->grammar->rule[strm->context.currNonTermID];

#if DEBUG_GRAMMAR == ON
	{
		tmp_err_code = printGrammarRule(strm->context.currNonTermID, currentRule, strm->schema);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Error printing grammar rule\n"));
		}
	}
#endif

	for(b = 0; b < 3; b++)
	{
		if(currentRule->part[b].count == 0) // No productions available with this length code
			continue;
		if(currentRule->part[b].bits == 0) // encoded with zero bits
			return handleProduction(strm, currentRule, &currentRule->part[b].prod[0], nonTermID_out, handler, app_data, b + 1);

		tmp_err_code = decodeNBitUnsignedInteger(strm, currentRule->part[b].bits, &tmp_bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(tmp_bits_val == currentRule->part[b].count) // The code has more parts
			continue;

		return handleProduction(strm, currentRule, &currentRule->part[b].prod[currentRule->part[b].count - 1 - tmp_bits_val], nonTermID_out, handler, app_data, b + 1);
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
				SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data, unsigned int codeLength)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QNameID qnameID = {URI_MAX, LN_MAX};

	*nonTermID_out = prodHit->nonTermID;

	switch(prodHit->eventType)
	{
		case EVENT_SD:
			if(handler->startDocument != NULL)
			{
				if(handler->startDocument(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		break;
		case EVENT_ED:
			if(handler->endDocument != NULL)
			{
				if(handler->endDocument(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		break;
		case EVENT_EE:
			if(handler->endElement != NULL)
			{
				if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			if(codeLength > 1 && IS_BUILT_IN_ELEM(strm->gStack->grammar->props))   // #1# COMMENT
			{
				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_EE, GR_VOID_NON_TERMINAL, &qnameID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		break;
		case EVENT_SC:
			if(handler->selfContained != NULL)
			{
				if(handler->selfContained(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		break;
		default: // The event has content!
			if(prodHit->eventType == EVENT_CH)
			{
				if(codeLength > 1 && IS_BUILT_IN_ELEM(strm->gStack->grammar->props))   // #2# COMMENT
				{
					tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, *nonTermID_out, &qnameID);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}
			}

			tmp_err_code = decodeEventContent(strm, prodHit, handler, nonTermID_out, currentRule, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		break;
	}

	return ERR_OK;
}

errorCode createFragmentGrammar(EXIPSchema* schema, QNameID* elQnameArr, Index qnameCount)
{
	GrammarRule* tmp_rule;

	schema->docGrammar.count = DEF_FRAG_GRAMMAR_RULE_NUMBER;
	schema->docGrammar.props = 0;
	schema->docGrammar.contentIndex = 0;
	schema->docGrammar.rule = (GrammarRule*) memManagedAllocate(&schema->memList, sizeof(GrammarRule)*DEF_FRAG_GRAMMAR_RULE_NUMBER);
	if(schema->docGrammar.rule == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* Rule for Fragment */
	/* Fragment : SD FragmentContent	0 */
	tmp_rule = &schema->docGrammar.rule[GR_FRAGMENT];

	/* Part 1 */
	tmp_rule->part[0].prod = static_prod_start_doc_part0;
	tmp_rule->part[0].count = 1;
	tmp_rule->part[0].bits = 0;

	/* Initialize Part 2 */
	tmp_rule->part[1].prod = NULL;
	tmp_rule->part[1].count = 0;
	tmp_rule->part[1].bits = 0;

	/* Initialize Part 3 */
	tmp_rule->part[2].prod = NULL;
	tmp_rule->part[2].count = 0;
	tmp_rule->part[2].bits = 0;

	/* Rule for Fragment content */
	tmp_rule = &schema->docGrammar.rule[GR_FRAGMENT_CONTENT];

	/* Initialize Part 2 */
	tmp_rule->part[1].prod = NULL;
	tmp_rule->part[1].count = 0;
	tmp_rule->part[1].bits = 0;

	/* Initialize Part 3 */
	tmp_rule->part[2].prod = NULL;
	tmp_rule->part[2].count = 0;
	tmp_rule->part[2].bits = 0;

	/* Part 1 */
	if(elQnameArr != NULL)   // Creates Schema Informed Grammar
	{
		unsigned int e = 0;
		Index tmp_code1;

		SET_SCHEMA(schema->docGrammar.props);
		tmp_code1 = qnameCount + 2;

		tmp_rule->part[0].prod = (Production*) memManagedAllocate(&schema->memList, sizeof(Production)*tmp_code1);
		if(tmp_rule->part[0].prod == NULL)
			return MEMORY_ALLOCATION_ERROR;

		/*
		 * FragmentContent :
		 *			   	SE (F-0)   FragmentContent	0
		 *				SE (F-1)   FragmentContent	1
		 *				⋮	⋮      ⋮
		 *				SE (F-n−1) FragmentContent  n-1
		 *			//	SE (*)     FragmentContent	n		//  This is created as part of the Build-In grammar further on
 		 *			//	ED		   					n+1		//  This is created as part of the Build-In grammar further on
		 */

		for(e = 0; e < qnameCount; e++)
		{
			tmp_rule->part[0].prod[qnameCount - e].eventType = EVENT_SE_QNAME;
			tmp_rule->part[0].prod[qnameCount - e].typeId = GET_LN_URI_QNAME(schema->uriTable, elQnameArr[e]).elemGrammar;
			tmp_rule->part[0].prod[qnameCount - e].nonTermID = GR_FRAGMENT_CONTENT;
			tmp_rule->part[0].prod[qnameCount - e].qnameId = elQnameArr[e];
		}
		tmp_rule->part[0].count = tmp_code1;
		tmp_rule->part[0].bits = getBitsNumber(tmp_code1 - 1);
	}
	else
	{
		tmp_rule->part[0].prod = (Production*) memManagedAllocate(&schema->memList, sizeof(Production)*2);
		if(tmp_rule->part[0].prod == NULL)
			return MEMORY_ALLOCATION_ERROR;

		/* Productions further on... */
		tmp_rule->part[0].count = 2;
		tmp_rule->part[0].bits = 1;
	}

	/*
	 * FragmentContent :
	 *				SE (*) FragmentContent	0
	 *				ED						1
	 */

	tmp_rule->part[0].prod[0].eventType = EVENT_ED;
	tmp_rule->part[0].prod[0].typeId = INDEX_MAX;
	tmp_rule->part[0].prod[0].nonTermID = GR_VOID_NON_TERMINAL;
	tmp_rule->part[0].prod[0].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[0].prod[0].qnameId.lnId = INDEX_MAX;

	tmp_rule->part[0].prod[1].eventType = EVENT_SE_ALL;
	tmp_rule->part[0].prod[1].typeId = INDEX_MAX;
	tmp_rule->part[0].prod[1].nonTermID = GR_FRAGMENT_CONTENT;
	tmp_rule->part[0].prod[1].qnameId.uriId = SMALL_INDEX_MAX;
	tmp_rule->part[0].prod[1].qnameId.lnId = INDEX_MAX;

	return ERR_OK;
}

#endif /* BUILTINDOCGRAMMAR_H_ */
