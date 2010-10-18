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
	buildInGrammar->ruleArray = (GrammarRule*) EXIP_MALLOC(sizeof(GrammarRule)*DEF_GRAMMAR_RULE_NUMBER);
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
	elementGrammar->ruleArray = (GrammarRule*) EXIP_MALLOC(sizeof(GrammarRule)*DEF_ELEMENT_GRAMMAR_RULE_NUMBER);
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

errorCode popGrammar(EXIGrammarStack** gStack, struct EXIGrammar* grammar)
{
	grammar = *gStack;
	*gStack = (*gStack)->nextInStack;
	grammar->nextInStack = NULL;
}

static errorCode decodeQName(EXIStream* strm, QName* qname, unsigned int* p_uriID, unsigned int* p_lnID);

static errorCode decodeQName(EXIStream* strm, QName* qname, unsigned int* p_uriID, unsigned int* p_lnID)
{
	//TODO: add the case when Preserve.prefixes is true
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_val_buf = 0;
	DEBUG_MSG(INFO,(">decodeQName uriTable row numbers: %d\n", strm->uriTable->rowCount));
	unsigned char uriBits = getBitsNumber(strm->uriTable->rowCount - 1);
	DEBUG_MSG(INFO,(">decodeQName uriBits: %d\n", uriBits));
	tmp_err_code = decodeNBitUnsignedInteger(strm, uriBits, &tmp_val_buf);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	DEBUG_MSG(INFO,(">decodeQName NBit Value: %d\n", tmp_val_buf));
	unsigned int uriID = 0; // The URI id in the URI string table
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
		DEBUG_MSG(INFO,(">decodeQName uri hit\n"));
		qname->uri = &(strm->uriTable->rows[tmp_val_buf-1].string_val);
		uriID = tmp_val_buf-1;
	}

	DEBUG_MSG(INFO,(">decodeQName URI decoded\n"));

	unsigned int flag_StringLiteralsPartition = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &flag_StringLiteralsPartition);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	DEBUG_MSG(INFO,(">decodeQName flag_StringLiteralsPartition: %d\n", flag_StringLiteralsPartition));
	unsigned int lnID = 0;
	if(flag_StringLiteralsPartition == 0) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber(strm->uriTable->rows[uriID].lTable->rowCount - 1);
		tmp_err_code = decodeNBitUnsignedInteger(strm, lnBits, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		qname->localName = &(strm->uriTable->rows[uriID].lTable->rows[lnID].string_val);
	}
	else // local-name table miss
	{
		DEBUG_MSG(INFO,(">decodeQName local-name table miss\n"));
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

static errorCode decodeStringValue(EXIStream* strm, StringType** value, unsigned int uriID, unsigned int lnID);

static errorCode decodeStringValue(EXIStream* strm, StringType** value, unsigned int uriID, unsigned int lnID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int flag_StringLiteralsPartition = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &flag_StringLiteralsPartition);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(flag_StringLiteralsPartition == 0) // "local" value partition table hit
	{
		unsigned int lvID = 0;
		unsigned char lvBits = getBitsNumber(strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable->rowCount - 1);
		tmp_err_code = decodeNBitUnsignedInteger(strm, lvBits, &lvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		unsigned int value_table_rowID = strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable->valueRowIds[lvID];
		(*value) = &(strm->vTable->rows[value_table_rowID].string_val);
	}
	else if(flag_StringLiteralsPartition == 1)// global value partition table hit
	{
		unsigned int gvID = 0;
		unsigned char gvBits = getBitsNumber(strm->vTable->rowCount - 1);
		tmp_err_code = decodeNBitUnsignedInteger(strm, gvBits, &gvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		(*value) = &(strm->vTable->rows[gvID].string_val);
	}
	else  // "local" value partition and global value partition table miss
	{
		StringType gvStr;
		unsigned int gvID = 0;
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
									ValueType vType, unsigned int uriRowID, unsigned int lnRowID,
									struct ElementGrammarPool* gPool, unsigned int* nonTermID_out,
									EXIGrammarStack** grStack, GrammarRule* currRule);

static errorCode decodeEventContent(EXIStream* strm, EventType eType, ContentHandler* handler,
									ValueType vType, unsigned int uriRowID, unsigned int lnRowID,
									struct ElementGrammarPool* gPool, unsigned int* nonTermID_out,
									EXIGrammarStack** grStack, GrammarRule* currRule)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	// TODO: implement all cases
	unsigned int uriID = 0;
	unsigned int lnID = 0;
	QName qname;
	if(eType == EVENT_SE_ALL)
	{
		DEBUG_MSG(INFO,(">SE(*) event\n"));
		// The content of SE event is the element qname
		tmp_err_code = decodeQName(strm, &qname, &uriID, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		DEBUG_MSG(INFO,(">Qname decoded\n"));
		if(handler->startElement != NULL)  // Invoke handler method passing the element qname
			handler->startElement(qname);

		// New element grammar is pushed on the stack
		struct EXIGrammar* res = NULL;
		char is_found = 0;
		tmp_err_code = checkElementGrammarInPool(gPool, uriID, lnID, &is_found, &res);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		(*grStack)->lastNonTermID = *nonTermID_out;
		if(is_found)
		{
			DEBUG_MSG(INFO,(">Element grammar for that element found\n"));
			*nonTermID_out = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(grStack, res);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			DEBUG_MSG(INFO,(">No element grammar found\n"));
			struct EXIGrammar* elementGrammar = EXIP_MALLOC(sizeof(struct EXIGrammar));
			if(elementGrammar == NULL)
				return MEMORY_ALLOCATION_ERROR;
			tmp_err_code = createBuildInElementGrammar(elementGrammar, strm->opts);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			DEBUG_MSG(INFO,(">New element grammar created\n"));
			tmp_err_code = addElementGrammarInPool(gPool, uriID, lnID, elementGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			*nonTermID_out = GR_START_TAG_CONTENT;
			DEBUG_MSG(INFO,(">Added to the pool\n"));
			tmp_err_code = pushGrammar(grStack, elementGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			DEBUG_MSG(INFO,(">Push on the stack\n"));
		}

	}
	else if(eType == EVENT_AT_ALL)
	{
		DEBUG_MSG(INFO,(">AT(*) event\n"));
		// The content of SE event is the element qname
		tmp_err_code = decodeQName(strm, &qname, &uriID, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(vType == VALUE_TYPE_STRING)
		{
			StringType* value;
			tmp_err_code = decodeStringValue(strm, &value, uriID, lnID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->attributeString != NULL)  // Invoke handler method
				handler->attributeString(qname, *value);
		}
		tmp_err_code = insertZeroProduction(currRule, EVENT_AT_QNAME, *nonTermID_out, lnID, uriID);

	}
	else if(eType == EVENT_SE_QNAME)
	{
		qname.uri = &(strm->uriTable->rows[uriRowID].string_val);
		qname.localName = &(strm->uriTable->rows[uriRowID].lTable->rows[lnRowID].string_val);
		if(handler->startElement != NULL)  // Invoke handler method passing the element qname
			handler->startElement(qname);
	}
	else if(eType == EVENT_AT_QNAME)
	{
		qname.uri = &(strm->uriTable->rows[uriRowID].string_val);
		qname.localName = &(strm->uriTable->rows[uriRowID].lTable->rows[lnRowID].string_val);
		if(vType == VALUE_TYPE_STRING)
		{
			StringType* value;
			tmp_err_code = decodeStringValue(strm, &value, uriID, lnID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->attributeString != NULL)  // Invoke handler method
				handler->attributeString(qname, *value);
		}
	}
	return ERR_OK;
}

errorCode processNextProduction(EXIStream* strm, EXIGrammarStack** grStack, unsigned int nonTermID_in,
								EventType* eType, unsigned int* nonTermID_out, ContentHandler* handler,
								struct ElementGrammarPool* gPool)
{
	DEBUG_MSG(INFO,(">Next production non-term-id: %d\n", nonTermID_in));

	// TODO: it is not finished - only simple productions are handled!
	ValueType vType = VALUE_TYPE_STRING; //TODO: This sets the value content type to String. This is only valid for schema-less decoding

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_bits_val = 0;
	int currProduction = 0;
	int i = 0;
	int j = 0;
	int b = 0;
	for(i = 0; i < (*grStack)->rulesDimension; i++)
	{
		DEBUG_MSG(INFO,(">Checking the grammar rules: %d\n",i));
		if(nonTermID_in == (*grStack)->ruleArray[i].nonTermID)
		{
			DEBUG_MSG(INFO,(">Found the correct rule: %d\n",i));
			for(b = 0; b < 3; b++)
			{
				DEBUG_MSG(INFO,(">Looking at the bits number: %d\n",b));
				DEBUG_MSG(INFO,(">Looking at the bits number: %d\n",(*grStack)->ruleArray[i].bits[b]));
				if((*grStack)->ruleArray[i].bits[b] == 0 &&
					(*grStack)->ruleArray[i].prodCount > b) // zero bits encoded part of event code with more parts available
				{
					DEBUG_MSG(INFO,(">Zero bits event code part\n"));
					continue;
				}
				else if((*grStack)->ruleArray[i].bits[b] == 0) // encoded with zero bits
				{
					*eType = (*grStack)->ruleArray[i].prodArray[currProduction].eType;
					*nonTermID_out = (*grStack)->ruleArray[i].prodArray[currProduction].nonTermID;
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
						DEBUG_MSG(INFO,(">Found element with content: %d\n", *eType));
						tmp_err_code = decodeEventContent(strm, *eType, handler, vType,
									   (*grStack)->ruleArray[i].prodArray[currProduction].uriRowID,
									   (*grStack)->ruleArray[i].prodArray[currProduction].lnRowID,
								       gPool, nonTermID_out, grStack, &((*grStack)->ruleArray[i]));
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
					return ERR_OK;
				}
				else
				{
					DEBUG_MSG(INFO,(">Not encoded with 0 bits\n"));
					tmp_err_code = decodeNBitUnsignedInteger(strm, (*grStack)->ruleArray[i].bits[b], &tmp_bits_val);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					for(j = 0; j < (*grStack)->ruleArray[i].prodCount && (*grStack)->ruleArray[i].prodArray[j].code.size >= b + 1; j++)
					{
						if((*grStack)->ruleArray[i].prodArray[j].code.code[b] == tmp_bits_val)
						{
							if((*grStack)->ruleArray[i].prodArray[j].code.size == b + 1)
							{
								*eType = (*grStack)->ruleArray[i].prodArray[j].eType;
								*nonTermID_out = (*grStack)->ruleArray[i].prodArray[j].nonTermID;
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
												   (*grStack)->ruleArray[i].prodArray[j].uriRowID,
												   (*grStack)->ruleArray[i].prodArray[j].lnRowID,
											       gPool, nonTermID_out, grStack, &((*grStack)->ruleArray[i]));
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
	pool->refs = EXIP_MALLOC(sizeof(struct ElementGrammarLabel)*GRAMMAR_POOL_DIMENSION);
	if(pool->refs == NULL)
		return MEMORY_ALLOCATION_ERROR;

	return ERR_OK;
}

//TODO: Smarter algorithm must be employed here
errorCode checkElementGrammarInPool(struct ElementGrammarPool* pool, unsigned int uriRowID,
									unsigned int lnRowID, unsigned char* is_found, struct EXIGrammar** result)
{
	int i = 0;
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

errorCode addElementGrammarInPool(struct ElementGrammarPool* pool, unsigned int uriRowID,
									unsigned int lnRowID, struct EXIGrammar* newGr)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(pool->refsCount == pool->refsDimension) // The dynamic array must be extended first
	{
		void* new_ptr = EXIP_REALLOC(pool->refs, sizeof(struct ElementGrammarLabel)*(pool->refsCount + GRAMMAR_POOL_DIMENSION));
		if(new_ptr == NULL)
			return MEMORY_ALLOCATION_ERROR;
		pool->refs = new_ptr;
		pool->refsDimension += GRAMMAR_POOL_DIMENSION;
	}

	pool->refs[pool->refsCount].lnRowID = lnRowID;
	pool->refs[pool->refsCount].uriRowID = uriRowID;
	pool->refs[pool->refsCount].elementGrammar = newGr;

	pool->refsCount += 1;
	return ERR_OK;
}

#endif /* BUILTINDOCGRAMMAR_H_ */
