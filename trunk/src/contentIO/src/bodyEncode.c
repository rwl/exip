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

errorCode encodeStringData(EXIStream* strm, String strng, uint16_t uriID, size_t lnID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char flag_StringLiteralsPartition = 0;
	uint16_t lvRowID = 0;
	ValueLocalCrossTable* vlTable = strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable;

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
			tmp_err_code = encodeNBitUnsignedInteger(strm, gvBits, (unsigned int)(gvRowID) );
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else // "local" value partition and global value partition table miss
		{
			tmp_err_code = encodeUnsignedInteger(strm, (UnsignedInteger)(strng.length + 2));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			tmp_err_code = encodeStringOnly(strm, &strng);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(strng.length > 0 && strng.length <= strm->header.opts.valueMaxLength && strm->header.opts.valuePartitionCapacity > 0)
			{
				String copiedValue;
				tmp_err_code = cloneString(&strng, &copiedValue, &strm->memList);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = addValueRows(strm, &copiedValue, uriID, lnID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
	}

	return ERR_OK;
}

errorCode lookupProduction(EXIStream* strm, EXIEvent event, QName* qname, unsigned char* codeLength, size_t* lastCodePart)
{
	unsigned char b = 0;
	size_t j = 0;
	GrammarRule* currentRule;
	Production* prodHit = NULL;
	size_t tmp_prod_indx = 0;

	if(strm->context.nonTermID >=  strm->gStack->grammar->rulesDimension)
		return INCONSISTENT_PROC_STATE;

	currentRule = &strm->gStack->grammar->ruleArray[strm->context.nonTermID];

	for (b = 0; b < 3; b++)
	{
		for(j = 0; j < currentRule->part[b].prodArraySize; j++)
		{
			tmp_prod_indx = currentRule->part[b].prodArraySize - 1 - j;
			if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM || valueTypeClassesEqual(currentRule->part[b].prodArray[tmp_prod_indx].event.valueType.exiType, event.valueType.exiType))
			{
				if(currentRule->part[b].prodArray[tmp_prod_indx].event.eventType == event.eventType || // (1)
						(qname != NULL &&
						(((currentRule->part[b].prodArray[tmp_prod_indx].event.eventType == EVENT_AT_URI || currentRule->part[b].prodArray[tmp_prod_indx].event.eventType == EVENT_SE_URI) &&    // (2)
						stringEqual(strm->uriTable->rows[currentRule->part[b].prodArray[tmp_prod_indx].qname.uriRowId].string_val, *(qname->uri))) ||
						((currentRule->part[b].prodArray[tmp_prod_indx].event.eventType == EVENT_AT_QNAME || currentRule->part[b].prodArray[tmp_prod_indx].event.eventType == EVENT_SE_QNAME) && // (3)
						stringEqual(strm->uriTable->rows[currentRule->part[b].prodArray[tmp_prod_indx].qname.uriRowId].string_val, *(qname->uri)) &&
						stringEqual(strm->uriTable->rows[currentRule->part[b].prodArray[tmp_prod_indx].qname.uriRowId].lTable->rows[currentRule->part[b].prodArray[tmp_prod_indx].qname.lnRowId].string_val, *(qname->localName))))
						)
				)
				{
					prodHit = &currentRule->part[b].prodArray[tmp_prod_indx];
					break;
				}
			}
		}
		if(prodHit != NULL)
			break;
	}

	if(prodHit == NULL)
		return INCONSISTENT_PROC_STATE;

	*codeLength = b + 1;
	*lastCodePart = j;

	return ERR_OK;
}

errorCode encodeQName(EXIStream* strm, QName qname, EventType eventT, uint16_t* uriID, size_t* lnID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encoding QName\n"));

/******* Start: URI **********/
	tmp_err_code = encodeURI(strm, (String*) qname.uri, uriID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
/******* End: URI **********/

/******* Start: Local name **********/
	tmp_err_code = encodeLocalName(strm, (String*) qname.localName, *uriID, lnID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
/******* End: Local name **********/

	return encodePrefixQName(strm, &qname, eventT, *uriID);
}

errorCode encodeURI(EXIStream* strm, String* uri, uint16_t* uriID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char uriBits = getBitsNumber(strm->uriTable->rowCount);

	if(lookupURI(strm->uriTable, *uri, uriID)) // uri hit
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, uriBits, *uriID + 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else  // uri miss
	{
		String copiedURI;
		tmp_err_code = encodeNBitUnsignedInteger(strm, uriBits, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = encodeString(strm, uri);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = cloneString(uri, &copiedURI, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addURIRow(strm->uriTable, copiedURI, uriID, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode encodeLocalName(EXIStream* strm, String* ln, uint16_t uriID, size_t* lnID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(lookupLN(strm->uriTable->rows[uriID].lTable, *ln, lnID)) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->uriTable->rows[uriID].lTable->rowCount - 1));
		tmp_err_code = encodeUnsignedInteger(strm, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeNBitUnsignedInteger(strm, lnBits, (unsigned int)(*lnID) );
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // local-name table miss
	{
		String copiedLN;
		tmp_err_code = encodeUnsignedInteger(strm, (UnsignedInteger)(ln->length + 1) );
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeStringOnly(strm,  ln);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(strm->uriTable->rows[uriID].lTable == NULL)
		{
			tmp_err_code = createLocalNamesTable(&strm->uriTable->rows[uriID].lTable, &strm->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		tmp_err_code = cloneString(ln, &copiedLN, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLNRow(strm->uriTable->rows[uriID].lTable, copiedLN, lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode encodePrefixQName(EXIStream* strm, QName* qname, EventType eventT, uint16_t uriID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char prefixBits = 0;
	unsigned int prefixID = 0;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) == FALSE)
		return ERR_OK;

	if(strm->uriTable->rows[uriID].pTable == NULL || strm->uriTable->rows[uriID].pTable->rowCount == 0)
		return ERR_OK;

	prefixBits = getBitsNumber(strm->uriTable->rows[uriID].pTable->rowCount - 1);

	if(prefixBits > 0)
	{
		if(qname == NULL)
			return NULL_POINTER_REF;

		if(lookupPrefix(strm->uriTable->rows[uriID].pTable, *qname->prefix, &prefixID) == TRUE)
		{
			tmp_err_code = encodeNBitUnsignedInteger(strm, prefixBits, prefixID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			if(eventT != EVENT_SE_ALL)
				return INCONSISTENT_PROC_STATE;

			tmp_err_code = encodeNBitUnsignedInteger(strm, prefixBits, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

errorCode encodePrefix(EXIStream* strm, uint16_t uriID, String* prefix)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int prfxID;
	unsigned char prfxBits = getBitsNumber(strm->uriTable->rows[uriID].pTable->rowCount);

	if(lookupPrefix(strm->uriTable->rows[uriID].pTable, *prefix, &prfxID)) // prefix hit
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, prfxBits, prfxID + 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else  // prefix miss
	{
		String copiedPrefix;
		tmp_err_code = encodeNBitUnsignedInteger(strm, prfxBits, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = encodeString(strm, prefix);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = cloneString(prefix, &copiedPrefix, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addPrefixRow(strm->uriTable->rows[uriID].pTable, copiedPrefix, &prfxID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode encodeIntData(EXIStream* strm, Integer int_val, ValueType intType)
{
	if(intType.exiType == VALUE_TYPE_SMALL_INTEGER)
	{
		return NOT_IMPLEMENTED_YET;
	}
	else if(intType.exiType == VALUE_TYPE_NON_NEGATIVE_INT)
	{
		return encodeUnsignedInteger(strm, (UnsignedInteger) int_val);
	}
	else if(intType.exiType == VALUE_TYPE_INTEGER)
	{
		return encodeIntegerValue(strm, int_val);
	}
	else
	{
		return INCONSISTENT_PROC_STATE;
	}
}
