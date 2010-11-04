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

#include "../include/grammars.h"
#include "procTypes.h"

#define DEF_GRAMMAR_RULE_NUMBER 3
#define DEF_ELEMENT_GRAMMAR_RULE_NUMBER 2
#define GRAMMAR_POOL_DIMENSION 5

errorCode getBuildInDocGrammar(struct EXIGrammar* buildInGrammar, struct EXIOptions* opts)
{
	//TODO: depends on the EXI fidelity options! Take this into account
	// For now only the default fidelity_opts pruning is supported - all preserve opts are false
	char is_default_fidelity = 0;

	if(opts->preserve == 0) //all preserve opts are false
		is_default_fidelity = 1;

	buildInGrammar->lastNonTermID = GR_VOID_NON_TERMINAL;
	buildInGrammar->nextInStack = NULL;
	buildInGrammar->rulesDimension = DEF_GRAMMAR_RULE_NUMBER;
	buildInGrammar->ruleArray = (GrammarRule*) memManagedAllocate(sizeof(GrammarRule)*DEF_GRAMMAR_RULE_NUMBER);
	if(buildInGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	errorCode tmp_err_code = UNEXPECTED_ERROR;

	/* Document : SD DocContent	0 */
	tmp_err_code = initGrammarRule(&(buildInGrammar->ruleArray[0]));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	buildInGrammar->ruleArray[0].nonTermID = GR_DOCUMENT;
	buildInGrammar->ruleArray[0].bits[0] = 0;
	tmp_err_code = addProduction(&(buildInGrammar->ruleArray[0]), getEventCode1(0), EVENT_SD, GR_DOC_CONTENT);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/*
	   DocContent :
					SE (*) DocEnd	0
					DT DocContent	1.0
					CM DocContent	1.1.0
					PI DocContent	1.1.1
	 */
	tmp_err_code = initGrammarRule(&(buildInGrammar->ruleArray[1]));
	if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	buildInGrammar->ruleArray[1].nonTermID = GR_DOC_CONTENT;
	if(is_default_fidelity)
	{
		buildInGrammar->ruleArray[1].bits[0] = 0;
	}
	else
	{
		buildInGrammar->ruleArray[1].bits[0] = 1;
		buildInGrammar->ruleArray[1].bits[1] = 1;
		buildInGrammar->ruleArray[1].bits[2] = 1;
	}

	/* SE (*) DocEnd	0 */
	tmp_err_code = addProduction(&(buildInGrammar->ruleArray[1]), getEventCode1(0), EVENT_SE_ALL, GR_DOC_END);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(is_default_fidelity == 0)
	{
		/* DT DocContent	1.0 */
		tmp_err_code = addProduction(&(buildInGrammar->ruleArray[1]), getEventCode2(1, 0), EVENT_DT, GR_DOC_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CM DocContent	1.1.0 */
		tmp_err_code = addProduction(&(buildInGrammar->ruleArray[1]), getEventCode3(1, 1, 0), EVENT_CM, GR_DOC_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* PI DocContent	1.1.1 */
		tmp_err_code = addProduction(&(buildInGrammar->ruleArray[1]), getEventCode3(1, 1, 1), EVENT_PI, GR_DOC_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}


	/* DocEnd :
				ED	        0
				CM DocEnd	1.0
				PI DocEnd	1.1 */
	tmp_err_code = initGrammarRule(&(buildInGrammar->ruleArray[2]));
	if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	buildInGrammar->ruleArray[2].nonTermID = GR_DOC_END;
	if(is_default_fidelity == 1)
	{
		buildInGrammar->ruleArray[2].bits[0] = 0;
	}
	else
	{
		buildInGrammar->ruleArray[2].bits[0] = 1;
		buildInGrammar->ruleArray[2].bits[1] = 1;
	}

	/* ED	0 */
	tmp_err_code = addProduction(&(buildInGrammar->ruleArray[2]), getEventCode1(0), EVENT_ED, GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(is_default_fidelity == 0)
	{
		/* CM DocEnd	1.0  */
		tmp_err_code = addProduction(&(buildInGrammar->ruleArray[2]), getEventCode2(1, 0), EVENT_CM, GR_DOC_END);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* PI DocEnd	1.1 */
		tmp_err_code = addProduction(&(buildInGrammar->ruleArray[2]), getEventCode2(1, 1), EVENT_PI, GR_DOC_END);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode createBuildInElementGrammar(struct EXIGrammar* elementGrammar, struct EXIOptions* opts)
{
	//TODO: depends on the EXI fidelity options! Take this into account
	// For now only the default fidelity_opts pruning is supported - all preserve opts are false
	// and selfContained is also false
	char is_default_fidelity = 0;

	if(opts->preserve == 0 && opts->selfContained == 0) //all preserve opts are false and selfContained is also false
		is_default_fidelity = 1;

	elementGrammar->lastNonTermID = GR_VOID_NON_TERMINAL;
	elementGrammar->nextInStack = NULL;
	elementGrammar->rulesDimension = DEF_ELEMENT_GRAMMAR_RULE_NUMBER;
	elementGrammar->ruleArray = (GrammarRule*) memManagedAllocate(sizeof(GrammarRule)*DEF_ELEMENT_GRAMMAR_RULE_NUMBER);
	if(elementGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	errorCode tmp_err_code = UNEXPECTED_ERROR;

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
	tmp_err_code = initGrammarRule(&(elementGrammar->ruleArray[0]));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	elementGrammar->ruleArray[0].nonTermID = GR_START_TAG_CONTENT;
	if(is_default_fidelity == 1)
	{
		elementGrammar->ruleArray[0].bits[0] = 0;
		elementGrammar->ruleArray[0].bits[1] = 2;
	}
	else
	{
		elementGrammar->ruleArray[0].bits[0] = 0;
		elementGrammar->ruleArray[0].bits[1] = 4;
		elementGrammar->ruleArray[0].bits[2] = 1;
	}

	/* EE	                    0.0 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,0), EVENT_EE, GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/* AT (*) StartTagContent	0.1 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,1), EVENT_AT_ALL, GR_START_TAG_CONTENT);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(is_default_fidelity == 1)
	{
		/* SE (*) ElementContent	0.2 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,2), EVENT_SE_ALL, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CH ElementContent	    0.3 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,3), EVENT_CH, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

	}
	else
	{
		/* NS StartTagContent	    0.2 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,2), EVENT_NS, GR_START_TAG_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* SC Fragment	            0.3 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,3), EVENT_SC, GR_FRAGMENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* SE (*) ElementContent	0.4 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,4), EVENT_SE_ALL, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CH ElementContent	    0.5 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,5), EVENT_CH, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* ER ElementContent	    0.6 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode2(0,6), EVENT_ER, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CM ElementContent	    0.7.0 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode3(0,7,0), EVENT_CM, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* PI ElementContent	    0.7.1 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[0]), getEventCode3(0,7,1), EVENT_PI, GR_ELEMENT_CONTENT);
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
	tmp_err_code = initGrammarRule(&(elementGrammar->ruleArray[1]));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	elementGrammar->ruleArray[1].nonTermID = GR_ELEMENT_CONTENT;
	if(is_default_fidelity == 1)
	{
		elementGrammar->ruleArray[1].bits[0] = 1;
		elementGrammar->ruleArray[1].bits[1] = 1;
	}
	else
	{
		elementGrammar->ruleArray[1].bits[0] = 1;
		elementGrammar->ruleArray[1].bits[1] = 2;
		elementGrammar->ruleArray[1].bits[2] = 1;
	}

	/* EE	                  0 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[1]), getEventCode1(0), EVENT_EE, GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/* SE (*) ElementContent	1.0 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[1]), getEventCode2(1,0), EVENT_SE_ALL, GR_ELEMENT_CONTENT);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/* CH ElementContent	    1.1 */
	tmp_err_code = addProduction(&(elementGrammar->ruleArray[1]), getEventCode2(1,1), EVENT_CH, GR_ELEMENT_CONTENT);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(is_default_fidelity == 0)
	{
		/* ER ElementContent	    1.2 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[1]), getEventCode2(1,2), EVENT_ER, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* CM ElementContent	    1.3.0 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[1]), getEventCode3(1,3,0), EVENT_CM, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* PI ElementContent	    1.3.1 */
		tmp_err_code = addProduction(&(elementGrammar->ruleArray[1]), getEventCode3(1,3,1), EVENT_PI, GR_ELEMENT_CONTENT);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode pushGrammar(EXIGrammarStack** gStack, struct EXIGrammar* grammar)
{
	grammar->nextInStack = *gStack;
	*gStack = grammar;
	return ERR_OK;
}

errorCode popGrammar(EXIGrammarStack** gStack, struct EXIGrammar** grammar)
{
	*grammar = *gStack;
	*gStack = (*gStack)->nextInStack;
	(*grammar)->nextInStack = NULL;
	return ERR_OK;
}

static errorCode decodeQName(EXIStream* strm, QName* qname, uint32_t* p_uriID, uint32_t* p_lnID);

static errorCode decodeQName(EXIStream* strm, QName* qname, uint32_t* p_uriID, uint32_t* p_lnID)
{
	//TODO: add the case when Preserve.prefixes is true
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t tmp_val_buf = 0;
	unsigned char uriBits = getBitsNumber(strm->uriTable->rowCount);
	tmp_err_code = decodeNBitUnsignedInteger(strm, uriBits, &tmp_val_buf);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	uint32_t uriID = 0; // The URI id in the URI string table
	if(tmp_val_buf == 0) // uri miss
	{
		StringType str;
		tmp_err_code = decodeString(strm, &str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addURIRow(strm->uriTable, str, &uriID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		qname->uri = &(strm->uriTable->rows[uriID].string_val);
	}
	else // uri hit
	{
		qname->uri = &(strm->uriTable->rows[tmp_val_buf-1].string_val);
		uriID = tmp_val_buf-1;
	}

	uint32_t flag_StringLiteralsPartition = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &flag_StringLiteralsPartition);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	uint32_t lnID = 0;
	if(flag_StringLiteralsPartition == 0) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber(strm->uriTable->rows[uriID].lTable->rowCount);
		tmp_err_code = decodeNBitUnsignedInteger(strm, lnBits, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		qname->localName = &(strm->uriTable->rows[uriID].lTable->rows[lnID].string_val);
	}
	else // local-name table miss
	{
		StringType lnStr;
		tmp_err_code = decodeStringOnly(strm, flag_StringLiteralsPartition - 1, &lnStr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(strm->uriTable->rows[uriID].lTable == NULL)
		{
			tmp_err_code = createLocalNamesTable(&strm->uriTable->rows[uriID].lTable);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		tmp_err_code = addLNRow(strm->uriTable->rows[uriID].lTable, lnStr, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		qname->localName = &(strm->uriTable->rows[uriID].lTable->rows[lnID].string_val);
	}
	*p_uriID = uriID;
	*p_lnID = lnID;
	return ERR_OK;
}

static errorCode decodeStringValue(EXIStream* strm, StringType** value, uint32_t uriID, uint32_t lnID);

static errorCode decodeStringValue(EXIStream* strm, StringType** value, uint32_t uriID, uint32_t lnID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t flag_StringLiteralsPartition = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &flag_StringLiteralsPartition);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(flag_StringLiteralsPartition == 0) // "local" value partition table hit
	{
		uint32_t lvID = 0;
		unsigned char lvBits = getBitsNumber(strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable->rowCount);
		tmp_err_code = decodeNBitUnsignedInteger(strm, lvBits, &lvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		uint32_t value_table_rowID = strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable->valueRowIds[lvID];
		(*value) = &(strm->vTable->rows[value_table_rowID].string_val);
	}
	else if(flag_StringLiteralsPartition == 1)// global value partition table hit
	{
		uint32_t gvID = 0;
		unsigned char gvBits = getBitsNumber(strm->vTable->rowCount);
		tmp_err_code = decodeNBitUnsignedInteger(strm, gvBits, &gvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		(*value) = &(strm->vTable->rows[gvID].string_val);
	}
	else  // "local" value partition and global value partition table miss
	{
		StringType gvStr;
		uint32_t gvID = 0;
		tmp_err_code = decodeStringOnly(strm, flag_StringLiteralsPartition - 2, &gvStr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addGVRow(strm->vTable, gvStr, &gvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLVRow(&(strm->uriTable->rows[uriID].lTable->rows[lnID]), gvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		(*value) = &(strm->vTable->rows[gvID].string_val);
	}
	return ERR_OK;
}

static errorCode decodeEventContent(EXIStream* strm, EventType eType, ContentHandler* handler,
									ValueType vType, uint32_t uriRowID, uint32_t lnRowID,
									unsigned int* nonTermID_out, GrammarRule* currRule);

static errorCode decodeEventContent(EXIStream* strm, EventType eType, ContentHandler* handler,
									ValueType vType, uint32_t uriRowID, uint32_t lnRowID,
									unsigned int* nonTermID_out, GrammarRule* currRule)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	// TODO: implement all cases
	uint32_t uriID = 0;
	uint32_t lnID = 0;
	QName qname;
	if(eType == EVENT_SE_ALL)
	{
		DEBUG_MSG(INFO,(">SE(*) event\n"));
		// The content of SE event is the element qname
		tmp_err_code = decodeQName(strm, &qname, &uriID, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(handler->startElement != NULL)  // Invoke handler method passing the element qname
			handler->startElement(qname);

		unsigned char isDocGr = 0;
		tmp_err_code = isDocumentGrammar(strm->gStack, &isDocGr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(!isDocGr)  // If the current grammar is Element grammar ...
		{
			tmp_err_code = insertZeroProduction(currRule, EVENT_SE_QNAME, *nonTermID_out, lnID, uriID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		// New element grammar is pushed on the stack
		struct EXIGrammar* res = NULL;
		char is_found = 0;
		tmp_err_code = checkElementGrammarInPool(strm->gPool, uriID, lnID, &is_found, &res);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		strm->gStack->lastNonTermID = *nonTermID_out;
		if(is_found)
		{
			*nonTermID_out = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), res);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			struct EXIGrammar* elementGrammar = (struct EXIGrammar*) memManagedAllocate(sizeof(struct EXIGrammar));
			if(elementGrammar == NULL)
				return MEMORY_ALLOCATION_ERROR;
			tmp_err_code = createBuildInElementGrammar(elementGrammar, strm->opts);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			tmp_err_code = addElementGrammarInPool(strm->gPool, uriID, lnID, elementGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			*nonTermID_out = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), elementGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

	}
	else if(eType == EVENT_AT_ALL)
	{
		DEBUG_MSG(INFO,(">AT(*) event\n"));
		// The content of SE event is the element qname
		tmp_err_code = decodeQName(strm, &qname, &uriID, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(handler->attribute != NULL)  // Invoke handler method
			handler->attribute(qname);
		if(vType == VALUE_TYPE_STRING)
		{
			StringType* value;
			tmp_err_code = decodeStringValue(strm, &value, uriID, lnID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->stringData != NULL)  // Invoke handler method
				handler->stringData(*value);
		}
		tmp_err_code = insertZeroProduction(currRule, EVENT_AT_QNAME, *nonTermID_out, lnID, uriID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(eType == EVENT_SE_QNAME)
	{
		DEBUG_MSG(INFO,(">SE(qname) event\n"));
		qname.uri = &(strm->uriTable->rows[uriRowID].string_val);
		qname.localName = &(strm->uriTable->rows[uriRowID].lTable->rows[lnRowID].string_val);
		if(handler->startElement != NULL)  // Invoke handler method passing the element qname
			handler->startElement(qname);

		// New element grammar is pushed on the stack
		struct EXIGrammar* res = NULL;
		char is_found = 0;
		tmp_err_code = checkElementGrammarInPool(strm->gPool, uriRowID, lnRowID, &is_found, &res);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		strm->gStack->lastNonTermID = *nonTermID_out;
		if(is_found)
		{
			*nonTermID_out = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), res);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			return INCONSISTENT_PROC_STATE;  // The event require the presence of Element Grammar In the Pool
		}
	}
	else if(eType == EVENT_AT_QNAME)
	{
		DEBUG_MSG(INFO,(">AT(qname) event\n"));
		qname.uri = &(strm->uriTable->rows[uriRowID].string_val);
		qname.localName = &(strm->uriTable->rows[uriRowID].lTable->rows[lnRowID].string_val);
		if(handler->attribute != NULL)  // Invoke handler method
			handler->attribute(qname);
		if(vType == VALUE_TYPE_STRING)
		{
			StringType* value;
			tmp_err_code = decodeStringValue(strm, &value, uriRowID, lnRowID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->stringData != NULL)  // Invoke handler method
				handler->stringData(*value);
		}
	}
	else if(eType == EVENT_CH)
	{
		DEBUG_MSG(INFO,(">CH event\n"));
		StringType* value;
		tmp_err_code = decodeStringValue(strm, &value, uriRowID, lnRowID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(handler->stringData != NULL)  // Invoke handler method
			handler->stringData(*value);
	}
	return ERR_OK;
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

errorCode processNextProduction(EXIStream* strm, EventType* eType,
							    unsigned int* nonTermID_out, ContentHandler* handler)
{
	DEBUG_MSG(INFO,(">Next production non-term-id: %d\n", strm->nonTermID));

	// TODO: it is not finished - only simple productions are handled!
	ValueType vType = VALUE_TYPE_STRING; //TODO: This sets the value content type to String. This is only valid for schema-less decoding

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_bits_val = 0;
	unsigned int currProduction = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int b = 0;
	for(i = 0; i < strm->gStack->rulesDimension; i++)
	{
		if(strm->nonTermID == strm->gStack->ruleArray[i].nonTermID)
		{
			for(b = 0; b < 3; b++)
			{
				if(strm->gStack->ruleArray[i].bits[b] == 0 &&
						strm->gStack->ruleArray[i].prodCount > b) // zero bits encoded part of event code with more parts available
				{
					continue;
				}
				else if(strm->gStack->ruleArray[i].bits[b] == 0) // encoded with zero bits
				{
					*eType = strm->gStack->ruleArray[i].prodArray[currProduction].eType;
					*nonTermID_out = strm->gStack->ruleArray[i].prodArray[currProduction].nonTermID;
					if(*eType == EVENT_SD)
					{
						if(handler->startDocument != NULL)
							handler->startDocument();
					}
					else if(*eType == EVENT_ED)
					{
						if(handler->endDocument != NULL)
							handler->endDocument();
					}
					else if(*eType == EVENT_EE)
					{
						if(handler->endElement != NULL)
							handler->endElement();

					}
					else if(*eType == EVENT_SC)
					{
						if(handler->selfContained != NULL)
							handler->selfContained();
					}
					else // The event has content!
					{
						tmp_err_code = decodeEventContent(strm, *eType, handler, vType,
										strm->gStack->ruleArray[i].prodArray[currProduction].uriRowID,
										strm->gStack->ruleArray[i].prodArray[currProduction].lnRowID,
								        nonTermID_out, &(strm->gStack->ruleArray[i]));
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
					return ERR_OK;
				}
				else
				{
					tmp_err_code = decodeNBitUnsignedInteger(strm, strm->gStack->ruleArray[i].bits[b], &tmp_bits_val);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					for(j = 0; j < strm->gStack->ruleArray[i].prodCount; j++)
					{
						if(strm->gStack->ruleArray[i].prodArray[j].code.size < b + 1)
							continue;
						if(strm->gStack->ruleArray[i].prodArray[j].code.code[b] == tmp_bits_val)
						{
							if(strm->gStack->ruleArray[i].prodArray[j].code.size == b + 1)
							{
								*eType = strm->gStack->ruleArray[i].prodArray[j].eType;
								*nonTermID_out = strm->gStack->ruleArray[i].prodArray[j].nonTermID;
								if(*eType == EVENT_SD)
								{
									if(handler->startDocument != NULL)
										handler->startDocument();
								}
								else if(*eType == EVENT_ED)
								{
									if(handler->endDocument != NULL)
										handler->endDocument();
								}
								else if(*eType == EVENT_EE)
								{
									if(handler->endElement != NULL)
										handler->endElement();
									unsigned char isDocGr = 0;
									tmp_err_code = isDocumentGrammar(strm->gStack, &isDocGr);
									if(tmp_err_code != ERR_OK)
										return tmp_err_code;

									if(!isDocGr)  // If the current grammar is Element grammar ...
									{
										if(b > 0)   // #1# COMMENT
										{
											tmp_err_code = insertZeroProduction(&(strm->gStack->ruleArray[i]),EVENT_EE, GR_VOID_NON_TERMINAL,
																				strm->gStack->ruleArray[i].prodArray[j].lnRowID,
																				strm->gStack->ruleArray[i].prodArray[j].uriRowID);
											if(tmp_err_code != ERR_OK)
												return tmp_err_code;
										}
									}
								}
								else if(*eType == EVENT_SC)
								{
									if(handler->selfContained != NULL)
										handler->selfContained();
								}
								else // The event has content!
								{
									if(*eType == EVENT_CH)
									{
										unsigned char isDocGr = 0;
										tmp_err_code = isDocumentGrammar(strm->gStack, &isDocGr);
										if(tmp_err_code != ERR_OK)
											return tmp_err_code;

										if(!isDocGr)  // If the current grammar is Element grammar ...
										{
											if(b > 0)   // #2# COMMENT
											{
												tmp_err_code = insertZeroProduction(&(strm->gStack->ruleArray[i]),EVENT_CH, *nonTermID_out,
																					strm->gStack->ruleArray[i].prodArray[j].lnRowID,
																					strm->gStack->ruleArray[i].prodArray[j].uriRowID);
												if(tmp_err_code != ERR_OK)
													return tmp_err_code;
											}
										}
									}
									tmp_err_code = decodeEventContent(strm, *eType, handler, vType,
													strm->gStack->ruleArray[i].prodArray[j].uriRowID,
													strm->gStack->ruleArray[i].prodArray[j].lnRowID,
											        nonTermID_out, &(strm->gStack->ruleArray[i]));
									if(tmp_err_code != ERR_OK)
										return tmp_err_code;
								}
								return ERR_OK;
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
			break;
		}
	}
	return tmp_err_code;
}

errorCode createElementGrammarPool(struct ElementGrammarPool* pool)
{
	pool->refsDimension = GRAMMAR_POOL_DIMENSION;
	pool->refsCount = 0;
	pool->refs = (struct ElementGrammarLabel*) memManagedAllocatePtr(sizeof(struct ElementGrammarLabel)*GRAMMAR_POOL_DIMENSION, &pool->memNode);
	if(pool->refs == NULL)
		return MEMORY_ALLOCATION_ERROR;

	return ERR_OK;
}

//TODO: Smarter algorithm must be employed here
errorCode checkElementGrammarInPool(struct ElementGrammarPool* pool, uint32_t uriRowID,
									uint32_t lnRowID, unsigned char* is_found, struct EXIGrammar** result)
{
	unsigned int i = 0;
	for(i = 0; i < pool->refsCount; i++)
	{
		if(pool->refs[i].uriRowID == uriRowID && pool->refs[i].lnRowID == lnRowID)
		{
			*is_found = 1;
			*result = pool->refs[i].elementGrammar;
			return ERR_OK;
		}
	}
	*is_found = 0;
	*result = NULL;
	return ERR_OK;
}

errorCode addElementGrammarInPool(struct ElementGrammarPool* pool, uint32_t uriRowID,
								uint32_t lnRowID, struct EXIGrammar* newGr)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(pool->refsCount == pool->refsDimension) // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate(&pool->refs, sizeof(struct ElementGrammarLabel)*(pool->refsCount + GRAMMAR_POOL_DIMENSION), pool->memNode);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		pool->refsDimension += GRAMMAR_POOL_DIMENSION;
	}

	pool->refs[pool->refsCount].lnRowID = lnRowID;
	pool->refs[pool->refsCount].uriRowID = uriRowID;
	pool->refs[pool->refsCount].elementGrammar = newGr;

	pool->refsCount += 1;
	return ERR_OK;
}

errorCode isDocumentGrammar(struct EXIGrammar* grammar, unsigned char* bool_result)
{
	if(grammar == NULL || grammar->ruleArray == NULL)
		return NULL_POINTER_REF;
	else
	{
		if(grammar->ruleArray[0].nonTermID == GR_DOCUMENT)
			*bool_result = 1;
		else
			*bool_result = 0;
	}
	return ERR_OK;
}

errorCode encodeQName(EXIStream* strm, QName qname, uint32_t* p_uriID, uint32_t* p_lnID)
{
	//TODO: add the case when Preserve.prefixes is true
	errorCode tmp_err_code = UNEXPECTED_ERROR;

/******* Start: URI **********/
	uint32_t uriID = 0;
	unsigned char uriBits = getBitsNumber(strm->uriTable->rowCount);
	if(lookupURI(strm->uriTable, *(qname.uri), &uriID)) // uri hit
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, uriBits, uriID + 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else  // uri miss
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, uriBits, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = encodeString(strm, qname.uri);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addURIRow(strm->uriTable, *(qname.uri), &uriID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	*p_uriID = uriID;
/******* End: URI **********/

/******* Start: Local name **********/
	uint32_t lnID = 0;
	if(lookupLN(strm->uriTable->rows[uriID].lTable, *(qname.localName), &lnID)) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber(strm->uriTable->rows[uriID].lTable->rowCount);
		tmp_err_code = encodeUnsignedInteger(strm, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeNBitUnsignedInteger(strm, lnBits, lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // local-name table miss
	{
		tmp_err_code = encodeUnsignedInteger(strm, qname.localName->length + 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeStringOnly(strm,  qname.localName);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(strm->uriTable->rows[uriID].lTable == NULL)
		{
			tmp_err_code = createLocalNamesTable(&strm->uriTable->rows[uriID].lTable);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		tmp_err_code = addLNRow(strm->uriTable->rows[uriID].lTable, *(qname.localName), &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	*p_lnID = lnID;

/******* End: Local name **********/
	return ERR_OK;
}

#endif /* BUILTINDOCGRAMMAR_H_ */
