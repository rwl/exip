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
 * @file bodyEncode.c
 * @brief Implementation of data and events serialization
 *
 * @date Mar 23, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "bodyEncode.h"
#include "sTables.h"
#include "streamEncode.h"
#include "ioUtil.h"
#include "eventsEXI.h"
#include "grammarRules.h"
#include "stringManipulate.h"
#include "grammars.h"
#include "memManagement.h"

// What type of event (ALL, URI, QNAME) is hit so far; 0-nothing, 1-ALL, 2-URI, 3-QNAME
#define EVENT_CATEGORY_NONE  0
#define EVENT_CATEGORY_ALL   1
#define EVENT_CATEGORY_URI   2
#define EVENT_CATEGORY_QNAME 3

errorCode encodeStringData(EXIStream* strm, StringType strng)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char flag_StringLiteralsPartition = 0;
	uint16_t p_uriID = strm->sContext.curr_uriID;
	size_t p_lnID = strm->sContext.curr_lnID;
	uint16_t lvRowID = 0;
	flag_StringLiteralsPartition = lookupLV(strm->vTable, strm->uriTable->rows[p_uriID].lTable->rows[p_lnID].vCrossTable, strng, &lvRowID);
	if(flag_StringLiteralsPartition) //  "local" value partition table hit
	{
		unsigned char lvBits;

		tmp_err_code = encodeUnsignedInteger(strm, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		lvBits = getBitsNumber(strm->uriTable->rows[p_uriID].lTable->rows[p_lnID].vCrossTable->rowCount - 1);
		tmp_err_code = encodeNBitUnsignedInteger(strm, lvBits, lvRowID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else //  "local" value partition table miss
	{
		size_t gvRowID = 0;
		flag_StringLiteralsPartition = lookupVal(strm->vTable, strng, &gvRowID);
		if(flag_StringLiteralsPartition) // global value partition table hit
		{
			unsigned char gvBits;

			tmp_err_code = encodeUnsignedInteger(strm, 1);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			gvBits = getBitsNumber(strm->vTable->rowCount - 1);
			tmp_err_code = encodeNBitUnsignedInteger(strm, gvBits, gvRowID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else // "local" value partition and global value partition table miss
		{
			tmp_err_code = encodeUnsignedInteger(strm, strng.length + 2);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			tmp_err_code = encodeStringOnly(strm, &strng);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			//TODO: Take into account valuePartitionCapacity parameter for setting globalID variable

			tmp_err_code = addGVRow(strm->vTable, strng, &gvRowID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = addLVRow(&(strm->uriTable->rows[p_uriID].lTable->rows[p_lnID]), gvRowID, &strm->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

errorCode encodeSimpleEXIEvent(EXIStream* strm, EXIEvent event)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t j = 0;
	GrammarRule* ruleArr;
	int32_t prodWithShortestCode = -1;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Ser EXI simple event: %d\n", event.eventType));

	if(strm->nonTermID >=  strm->gStack->grammar->rulesDimension)
		return INCONSISTENT_PROC_STATE;

	ruleArr = strm->gStack->grammar->ruleArray;

#if DEBUG_CONTENT_IO == ON
	{
		tmp_err_code = printGrammarRule(strm->nonTermID, &ruleArr[strm->nonTermID]);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Error printing grammar rule\n"));
		}
	}
#endif

	for(j = 0; j < ruleArr[strm->nonTermID].prodCount; j++)
	{
		if(eventsEqual(ruleArr[strm->nonTermID].prodArray[j].event, event))
		{
			if(prodWithShortestCode == -1)
				prodWithShortestCode = j;
			else
			{
				if(ruleArr[strm->nonTermID].prodArray[j].code.size < ruleArr[strm->nonTermID].prodArray[prodWithShortestCode].code.size)
					prodWithShortestCode = j;
			}
		}
	}

	if(prodWithShortestCode == -1)
		return EVENT_CODE_MISSING;

	tmp_err_code = writeEventCode(strm, ruleArr[strm->nonTermID].prodArray[prodWithShortestCode].code, ruleArr[strm->nonTermID].bits);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM && ruleArr[strm->nonTermID].prodArray[prodWithShortestCode].code.size > 1
		&& (event.eventType == EVENT_CH || event.eventType == EVENT_EE))  // If the current grammar is build-in Element grammar and the event code size is bigger than 1 and the event is CH or EE...
	{
		// #1# COMMENT and #2# COMMENT
		tmp_err_code = insertZeroProduction(&(ruleArr[strm->nonTermID]),getEventDefType(event.eventType), ruleArr[strm->nonTermID].prodArray[prodWithShortestCode].nonTermID,
				ruleArr[strm->nonTermID].prodArray[prodWithShortestCode].lnRowID, ruleArr[strm->nonTermID].prodArray[prodWithShortestCode].uriRowID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	strm->nonTermID = ruleArr[strm->nonTermID].prodArray[prodWithShortestCode].nonTermID;
	return ERR_OK;
}

errorCode encodeComplexEXIEvent(EXIStream* strm, QName qname, EXIEvent event)
{
	unsigned char e_qname;
	unsigned char e_uri;
	unsigned char e_all;
	unsigned char isElement = TRUE;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t j = 0;
	int prodHit = -1;
	int prodHitIndicator = EVENT_CATEGORY_NONE; // What type of event (ALL, URI, QNAME) is hit so far; 0-nothing, 1-ALL, 2-URI, 3-QNAME
	GrammarRule* ruleArr;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Ser EXI complex event\n"));

	if(event.eventType == EVENT_SE_ALL) // SE event, Element
	{
		isElement = TRUE;
		e_qname = EVENT_SE_QNAME;
		e_uri = EVENT_SE_URI;
		e_all = EVENT_SE_ALL;
	}
	else if(event.eventType == EVENT_AT_ALL) // AT event, Attribute
	{
		isElement = FALSE;
		e_qname = EVENT_AT_QNAME;
		e_uri = EVENT_AT_URI;
		e_all = EVENT_AT_ALL;
	}
	else
		return INCONSISTENT_PROC_STATE;

	if(strm->nonTermID >=  strm->gStack->grammar->rulesDimension)
			return INCONSISTENT_PROC_STATE;

	ruleArr = strm->gStack->grammar->ruleArray;

#if DEBUG_CONTENT_IO == ON
	{
		tmp_err_code = printGrammarRule(strm->nonTermID, &ruleArr[strm->nonTermID]);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Error printing grammar rule\n"));
		}
	}
#endif

	for(j = 0; j < ruleArr[strm->nonTermID].prodCount; j++)
	{
		if(ruleArr[strm->nonTermID].prodArray[j].event.eventType == e_all)
		{
			if(prodHitIndicator == EVENT_CATEGORY_NONE)
			{
				prodHit = j;
				prodHitIndicator = EVENT_CATEGORY_ALL;
			}
		}
		else if(ruleArr[strm->nonTermID].prodArray[j].event.eventType == e_uri)
		{
			if(str_equal(strm->uriTable->rows[ruleArr[strm->nonTermID].prodArray[j].uriRowID].string_val, *(qname.uri)))
			{
				if(prodHitIndicator < EVENT_CATEGORY_QNAME)
				{
					prodHit = j;
					prodHitIndicator = EVENT_CATEGORY_URI;
				}
			}
		}
		else if(ruleArr[strm->nonTermID].prodArray[j].event.eventType == e_qname)
		{
			if(str_equal(strm->uriTable->rows[ruleArr[strm->nonTermID].prodArray[j].uriRowID].string_val, *(qname.uri)) &&
					str_equal(strm->uriTable->rows[ruleArr[strm->nonTermID].prodArray[j].uriRowID].lTable->rows[ruleArr[strm->nonTermID].prodArray[j].lnRowID].string_val, *(qname.localName)))
			{
				prodHit = j;
				prodHitIndicator = EVENT_CATEGORY_QNAME;
				break; // There is not a shorter code for that Event code
			}
		}
	}
	if(prodHit >= 0)
	{
		tmp_err_code = writeEventCode(strm, ruleArr[strm->nonTermID].prodArray[prodHit].code, ruleArr[strm->nonTermID].bits);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		strm->sContext.curr_uriID = ruleArr[strm->nonTermID].prodArray[prodHit].uriRowID;
		strm->sContext.curr_lnID = ruleArr[strm->nonTermID].prodArray[prodHit].lnRowID;
		if(prodHitIndicator == EVENT_CATEGORY_ALL)
		{
			tmp_err_code = encodeQName(strm, qname);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(isElement == TRUE) // SE(*) Event
			{
				struct EXIGrammar* res = NULL;
				unsigned char is_found = 0;

				DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Ser SE(*) Event \n"));

				if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)  // If the current grammar is build-in Element grammar ...
				{
					tmp_err_code = insertZeroProduction(&(ruleArr[strm->nonTermID]), getEventDefType(EVENT_SE_QNAME), ruleArr[strm->nonTermID].prodArray[prodHit].nonTermID, strm->sContext.curr_lnID, strm->sContext.curr_uriID);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}

				// New element grammar is pushed on the stack
				tmp_err_code = checkGrammarInPool(strm->ePool, strm->sContext.curr_uriID, strm->sContext.curr_lnID, &is_found, &res);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
				strm->gStack->lastNonTermID = ruleArr[strm->nonTermID].prodArray[prodHit].nonTermID;
				if(is_found)
				{
					DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Element grammar found in the pool \n"));
					strm->nonTermID = GR_START_TAG_CONTENT;
					tmp_err_code = pushGrammar(&(strm->gStack), res);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}
				else
				{
					DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Element grammar NOT found in the pool \n"));
					struct EXIGrammar* elementGrammar = (struct EXIGrammar*) memManagedAllocate(&strm->memList, sizeof(struct EXIGrammar));
					if(elementGrammar == NULL)
						return MEMORY_ALLOCATION_ERROR;
					tmp_err_code = createBuildInElementGrammar(elementGrammar, strm);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					tmp_err_code = addGrammarInPool(strm->ePool, strm->sContext.curr_uriID, strm->sContext.curr_lnID, elementGrammar);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					strm->nonTermID = GR_START_TAG_CONTENT;
					tmp_err_code = pushGrammar(&(strm->gStack), elementGrammar);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}
			}
			else if(isElement == FALSE) // AT(*) Event
			{
				DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Ser AT(*) Event \n"));
				tmp_err_code = insertZeroProduction(&(ruleArr[strm->nonTermID]), getEventDefType(EVENT_AT_QNAME), ruleArr[strm->nonTermID].prodArray[prodHit].nonTermID, strm->sContext.curr_lnID, strm->sContext.curr_uriID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				strm->nonTermID = ruleArr[strm->nonTermID].prodArray[prodHit].nonTermID;
			}
		}
		else if(prodHitIndicator == EVENT_CATEGORY_QNAME)
		{
			if(isElement == TRUE) // SE(QName) Event
			{
				// New element grammar is pushed on the stack
				struct EXIGrammar* res = NULL;
				unsigned char is_found = 0;

				DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Ser SE(QName) Event \n"));

				tmp_err_code = checkGrammarInPool(strm->ePool, strm->sContext.curr_uriID, strm->sContext.curr_lnID, &is_found, &res);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
				strm->gStack->lastNonTermID = ruleArr[strm->nonTermID].prodArray[prodHit].nonTermID;
				if(is_found)
				{
					strm->nonTermID = GR_START_TAG_CONTENT;
					tmp_err_code = pushGrammar(&(strm->gStack), res);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}
				else
				{
					return INCONSISTENT_PROC_STATE;  // The event require the presence of Element Grammar In the Pool
				}
			}
			else if(isElement == FALSE) // AT(QName) Event
			{
				DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Ser AT(QName) Event \n"));
				strm->nonTermID = ruleArr[strm->nonTermID].prodArray[prodHit].nonTermID;
			}
		}

		if(isElement == FALSE) // AT event
			strm->sContext.expectATData = TRUE;
		return ERR_OK;
	}
	return EVENT_CODE_MISSING;
}

errorCode encodeQName(EXIStream* strm, QName qname)
{
	//TODO: add the case when Preserve.prefixes is true
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t lnID = 0;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encoding QName\n"));

/******* Start: URI **********/
	uint16_t uriID = 0;
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

		tmp_err_code = addURIRow(strm->uriTable, *(qname.uri), &uriID, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	strm->sContext.curr_uriID = uriID;
/******* End: URI **********/

/******* Start: Local name **********/
	if(lookupLN(strm->uriTable->rows[uriID].lTable, *(qname.localName), &lnID)) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber(strm->uriTable->rows[uriID].lTable->rowCount - 1);
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
			tmp_err_code = createLocalNamesTable(&strm->uriTable->rows[uriID].lTable, &strm->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		tmp_err_code = addLNRow(strm->uriTable->rows[uriID].lTable, *(qname.localName), &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	strm->sContext.curr_lnID = lnID;

/******* End: Local name **********/
	return ERR_OK;
}
