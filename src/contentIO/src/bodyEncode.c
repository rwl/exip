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

errorCode encodeStringData(EXIStream* strm, StringType strng)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char flag_StringLiteralsPartition = 0;
	uint16_t p_uriID = strm->context.curr_uriID;
	size_t p_lnID = strm->context.curr_lnID;
	uint16_t lvRowID = 0;
	ValueLocalCrossTable* vlTable = strm->uriTable->rows[p_uriID].lTable->rows[p_lnID].vCrossTable;

	flag_StringLiteralsPartition = lookupLV(strm->vTable, vlTable, strng, &lvRowID);
	if(flag_StringLiteralsPartition && vlTable->valueRowIds[lvRowID] != SIZE_MAX) //  "local" value partition table hit; when SIZE_MAX -> compact identifier permanently unassigned
	{
		unsigned char lvBits;

		tmp_err_code = encodeUnsignedInteger(strm, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		lvBits = getBitsNumber(vlTable->rowCount - 1);
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
			gvBits = getBitsNumber((unsigned int)(strm->vTable->rowCount - 1));
			tmp_err_code = encodeNBitUnsignedInteger(strm, gvBits, (uint32_t)(gvRowID) );
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else // "local" value partition and global value partition table miss
		{
			tmp_err_code = encodeUnsignedInteger(strm, (uint32_t)(strng.length + 2));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			tmp_err_code = encodeStringOnly(strm, &strng);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(strng.length > 0 && strng.length <= strm->header.opts->valueMaxLength && strm->header.opts->valuePartitionCapacity > 0)
			{
				StringType copiedValue;
				tmp_err_code = cloneString(&strng, &copiedValue, &strm->memList);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = addValueRows(strm, &copiedValue);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
	}

	return ERR_OK;
}

errorCode encodeSimpleEXIEvent(EXIStream* strm, EXIEvent event)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char b = 0;
	size_t j = 0;
	GrammarRule* currentRule;
	Production* prodHit = NULL;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Ser EXI simple event: %d\n", event.eventType));

	if(strm->context.nonTermID >=  strm->gStack->grammar->rulesDimension)
		return INCONSISTENT_PROC_STATE;

	currentRule = &strm->gStack->grammar->ruleArray[strm->context.nonTermID];

#if DEBUG_CONTENT_IO == ON
	{
		tmp_err_code = printGrammarRule(strm->nonTermID, currentRule);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Error printing grammar rule\n"));
		}
	}
#endif
	for (b = 0; b < 3; b++)
	{
		for(j = 0; j < currentRule->prodCounts[b]; j++)
		{
			if(eventsEqual(currentRule->prodArrays[b][currentRule->prodCounts[b] - 1 - j].event, event))
			{
				prodHit = &currentRule->prodArrays[b][currentRule->prodCounts[b] - 1 - j];
				break;
			}
		}
		if(prodHit != NULL)
			break;
	}

	tmp_err_code = writeEventCode(strm, currentRule, b + 1, j);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM && b > 0
		&& (event.eventType == EVENT_CH || event.eventType == EVENT_EE))  // If the current grammar is build-in Element grammar and the event code size is bigger than 1 and the event is CH or EE...
	{
		// #1# COMMENT and #2# COMMENT
		tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, getEventDefType(event.eventType), prodHit->nonTermID,
				prodHit->lnRowID, prodHit->uriRowID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	strm->context.nonTermID = prodHit->nonTermID;
	return ERR_OK;
}

errorCode encodeComplexEXIEvent(EXIStream* strm, QName qname, unsigned char event_all, unsigned char event_uri, unsigned char event_qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char b = 0;
	size_t j = 0;
	size_t tmp_prod_indx = 0;
	GrammarRule* currentRule;
	Production* prodHit = NULL;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Ser SE event\n"));

	if(strm->context.nonTermID >=  strm->gStack->grammar->rulesDimension)
		return INCONSISTENT_PROC_STATE;

	currentRule = &strm->gStack->grammar->ruleArray[strm->context.nonTermID];

#if DEBUG_CONTENT_IO == ON
	{
		tmp_err_code = printGrammarRule(strm->nonTermID, currentRule);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Error printing grammar rule\n"));
		}
	}
#endif
	for (b = 0; b < 3; b++)
	{
		for(j = 0; j < currentRule->prodCounts[b]; j++)
		{
			tmp_prod_indx = currentRule->prodCounts[b] - 1 - j;
			if(currentRule->prodArrays[b][tmp_prod_indx].event.eventType == event_all ||   // (1)
		       (currentRule->prodArrays[b][tmp_prod_indx].event.eventType == event_uri &&    // (2)
			    str_equal(strm->uriTable->rows[currentRule->prodArrays[b][tmp_prod_indx].uriRowID].string_val, *(qname.uri))) ||
			    (currentRule->prodArrays[b][tmp_prod_indx].event.eventType == event_qname && // (3)
				 str_equal(strm->uriTable->rows[currentRule->prodArrays[b][tmp_prod_indx].uriRowID].string_val, *(qname.uri)) &&
				 str_equal(strm->uriTable->rows[currentRule->prodArrays[b][tmp_prod_indx].uriRowID].lTable->rows[currentRule->prodArrays[b][tmp_prod_indx].lnRowID].string_val, *(qname.localName)))
		       )
			{
				prodHit = &currentRule->prodArrays[b][tmp_prod_indx];
				break;
			}
		}
		if(prodHit != NULL)
			break;
	}

	tmp_err_code = writeEventCode(strm, currentRule, b + 1, j);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(prodHit->event.eventType == event_all)
	{
		tmp_err_code = encodeQName(strm, qname);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)  // If the current grammar is build-in Element grammar ...
		{
			tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, getEventDefType(EVENT_SE_QNAME), prodHit->nonTermID, strm->context.curr_lnID, strm->context.curr_uriID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else if(prodHit->event.eventType == event_uri)
	{
		return NOT_IMPLEMENTED_YET;
	}
	else if(prodHit->event.eventType == event_qname)
	{
		strm->context.curr_uriID = prodHit->uriRowID;
		strm->context.curr_lnID = prodHit->lnRowID;
	}

	strm->context.nonTermID = prodHit->nonTermID;
	return ERR_OK;
}

errorCode encodeQName(EXIStream* strm, QName qname)
{
	//TODO: add the case when Preserve.prefixes is true
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t lnID = 0;

/******* Start: URI **********/
	uint16_t uriID = 0;
	unsigned char uriBits = getBitsNumber(strm->uriTable->rowCount);

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encoding QName\n"));
	if(lookupURI(strm->uriTable, *(qname.uri), &uriID)) // uri hit
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, uriBits, uriID + 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else  // uri miss
	{
		StringType copiedURI;
		tmp_err_code = encodeNBitUnsignedInteger(strm, uriBits, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = encodeString(strm, qname.uri);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = cloneString(qname.uri, &copiedURI, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addURIRow(strm->uriTable, copiedURI, &uriID, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	strm->context.curr_uriID = uriID;
/******* End: URI **********/

/******* Start: Local name **********/
	if(lookupLN(strm->uriTable->rows[uriID].lTable, *(qname.localName), &lnID)) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->uriTable->rows[uriID].lTable->rowCount - 1));
		tmp_err_code = encodeUnsignedInteger(strm, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeNBitUnsignedInteger(strm, lnBits, (uint32_t)(lnID) );
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // local-name table miss
	{
		StringType copiedLN;
		tmp_err_code = encodeUnsignedInteger(strm, (uint32_t)(qname.localName->length + 1) );
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

		tmp_err_code = cloneString(qname.localName, &copiedLN, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLNRow(strm->uriTable->rows[uriID].lTable, copiedLN, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	strm->context.curr_lnID = lnID;

/******* End: Local name **********/
	return ERR_OK;
}
