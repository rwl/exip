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
 * @file bodyDecode.c
 * @brief Implementing an API for decoding EXI stream body
 * @date Oct 1, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "bodyDecode.h"
#include "sTables.h"
#include "memManagement.h"
#include "ioUtil.h"
#include "streamDecode.h"
#include "grammars.h"

errorCode decodeQName(EXIStream* strm, QName* qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t uriID;
	size_t lnID;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Decoding QName\n"));

	tmp_err_code = decodeURI(strm, &uriID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	qname->uri = &(strm->uriTable->rows[uriID].string_val);

	tmp_err_code = decodeLocalName(strm, uriID, &lnID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	qname->localName = &(strm->uriTable->rows[uriID].lTable->rows[lnID].string_val);

	strm->context.curr_uriID = uriID;
	strm->context.curr_lnID = lnID;

	return decodePrefixQname(strm, qname);
}

errorCode decodeURI(EXIStream* strm, uint16_t* uriID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_val_buf = 0;
	unsigned char uriBits = getBitsNumber(strm->uriTable->rowCount);

	tmp_err_code = decodeNBitUnsignedInteger(strm, uriBits, &tmp_val_buf);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(tmp_val_buf == 0) // uri miss
	{
		String str;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">URI miss\n"));
		tmp_err_code = decodeString(strm, &str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addURIRow(strm->uriTable, str, uriID, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // uri hit
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">URI hit\n"));
		*uriID = tmp_val_buf-1;
		if(*uriID >= strm->uriTable->rowCount)
			return INVALID_EXI_INPUT;
	}

	return ERR_OK;
}

errorCode decodeLocalName(EXIStream* strm, uint16_t uriID, size_t* lnID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	UnsignedInteger tmpVar = 0;

	tmp_err_code = decodeUnsignedInteger(strm, &tmpVar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmpVar == 0) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->uriTable->rows[uriID].lTable->rowCount - 1));
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">local-name table hit\n"));
		tmp_err_code = decodeNBitUnsignedInteger(strm, lnBits, lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(*lnID >= strm->uriTable->rows[uriID].lTable->rowCount)
			return INVALID_EXI_INPUT;
	}
	else // local-name table miss
	{
		String lnStr;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">local-name table miss\n"));
		tmp_err_code = decodeStringOnly(strm, tmpVar - 1, &lnStr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(strm->uriTable->rows[uriID].lTable == NULL)
		{
			tmp_err_code = createLocalNamesTable(&strm->uriTable->rows[uriID].lTable, &strm->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		tmp_err_code = addLNRow(strm->uriTable->rows[uriID].lTable, lnStr, lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode decodePrefixQname(EXIStream* strm, QName* qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char prefixBits = 0;
	unsigned int prefixID = 0;

	qname->prefix = NULL;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) == FALSE)
		return ERR_OK;

	if(strm->uriTable->rows[strm->context.curr_uriID].pTable->rowCount == 0)
		return ERR_OK;

	prefixBits = getBitsNumber(strm->uriTable->rows[strm->context.curr_uriID].pTable->rowCount - 1);

	if(prefixBits > 0)
	{
		tmp_err_code = decodeNBitUnsignedInteger(strm, prefixBits, &prefixID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(prefixID >= strm->uriTable->rows[strm->context.curr_uriID].pTable->rowCount)
			return INVALID_EXI_INPUT;
	}

	qname->prefix = &strm->uriTable->rows[strm->context.curr_uriID].pTable->string_val[prefixID];

	return ERR_OK;
}

errorCode decodePrefix(EXIStream* strm, uint16_t uriID, unsigned int* prfxID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_val_buf = 0;
	unsigned char prfxBits = getBitsNumber(strm->uriTable->rows[uriID].pTable->rowCount);

	tmp_err_code = decodeNBitUnsignedInteger(strm, prfxBits, &tmp_val_buf);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmp_val_buf == 0) // prefix miss
	{
		String str;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Prefix miss\n"));
		tmp_err_code = decodeString(strm, &str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addPrefixRow(strm->uriTable->rows[uriID].pTable, str, prfxID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // prefix hit
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Prefix hit\n"));
		*prfxID = tmp_val_buf-1;
		if(*prfxID >= strm->uriTable->rows[uriID].pTable->rowCount)
			return INVALID_EXI_INPUT;
	}

	return ERR_OK;
}

errorCode decodeStringValue(EXIStream* strm, String* value, unsigned char* freeable)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t uriID = strm->context.curr_uriID;
	size_t lnID = strm->context.curr_lnID;
	UnsignedInteger tmpVar = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &tmpVar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*freeable = FALSE;

	if(tmpVar == 0) // "local" value partition table hit
	{
		unsigned int lvID = 0;
		size_t value_table_rowID;

		unsigned char lvBits = getBitsNumber(strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable->rowCount - 1);
		tmp_err_code = decodeNBitUnsignedInteger(strm, lvBits, &lvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		value_table_rowID = strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable->valueRowIds[lvID];
		*value = strm->vTable->rows[value_table_rowID].string_val;
	}
	else if(tmpVar == 1)// global value partition table hit
	{
		size_t gvID = 0;
		unsigned char gvBits = getBitsNumber((unsigned int)(strm->vTable->rowCount - 1));
		tmp_err_code = decodeNBitUnsignedInteger(strm, gvBits, &gvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		*value = strm->vTable->rows[gvID].string_val;
	}
	else  // "local" value partition and global value partition table miss
	{
		tmp_err_code = decodeStringOnly(strm, tmpVar - 2, value);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(value->length > 0 && value->length <= strm->header.opts.valueMaxLength && strm->header.opts.valuePartitionCapacity > 0)
		{
			tmp_err_code = addValueRows(strm, value);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
			*freeable = TRUE;
	}
	return ERR_OK;
}

errorCode decodeEventContent(EXIStream* strm, EXIEvent event, ContentHandler* handler,
									size_t* nonTermID_out, GrammarRule* currRule,  void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	// TODO: implement all cases of events
	QName qname;

	if(event.eventType == EVENT_SE_ALL)
	{
		EXIGrammar* elemGrammar = NULL;

		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">SE(*) event\n"));
		// The content of SE event is the element qname
		tmp_err_code = decodeQName(strm, &qname);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(handler->startElement != NULL)  // Invoke handler method passing the element qname
		{
			if(handler->startElement(qname, app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}

		if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)  // If the current grammar is build-in Element grammar ...
		{
			tmp_err_code = insertZeroProduction((DynGrammarRule*) currRule, getEventDefType(EVENT_SE_QNAME), *nonTermID_out, strm->context.curr_lnID, strm->context.curr_uriID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		// New element grammar is pushed on the stack
		elemGrammar = strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].typeGrammar;

		strm->gStack->lastNonTermID = *nonTermID_out;
		if(elemGrammar != NULL) // The grammar is found
		{
			*nonTermID_out = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), elemGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			EXIGrammar* newElementGrammar = (EXIGrammar*) memManagedAllocate(&strm->memList, sizeof(EXIGrammar));
			if(newElementGrammar == NULL)
				return MEMORY_ALLOCATION_ERROR;
			tmp_err_code = createBuildInElementGrammar(newElementGrammar, strm);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].typeGrammar = newElementGrammar;
			*nonTermID_out = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), newElementGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

	}
	else if(event.eventType == EVENT_AT_ALL)
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">AT(*) event\n"));
		tmp_err_code = decodeQName(strm, &qname);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(handler->attribute != NULL)  // Invoke handler method
		{
			if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}
		if(event.valueType.exiType == VALUE_TYPE_STRING || event.valueType.exiType == VALUE_TYPE_NONE)
		{
			String value;
			unsigned char freeable = FALSE;
			tmp_err_code = decodeStringValue(strm, &value, &freeable);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->stringData != NULL)  // Invoke handler method
			{
				if(handler->stringData(value, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
			if(freeable)
				freeLastManagedAlloc(&strm->memList);
		}
		tmp_err_code = insertZeroProduction((DynGrammarRule*) currRule, getEventDefType(EVENT_AT_QNAME), *nonTermID_out, strm->context.curr_lnID, strm->context.curr_uriID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(event.eventType == EVENT_SE_QNAME)
	{
		EXIGrammar* elemGrammar = NULL;

		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">SE(qname) event\n"));
		qname.uri = &(strm->uriTable->rows[strm->context.curr_uriID].string_val);
		qname.localName = &(strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].string_val);
		tmp_err_code = decodePrefixQname(strm, &qname);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(handler->startElement != NULL)  // Invoke handler method passing the element qname
		{
			if(handler->startElement(qname, app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}

		// New element grammar is pushed on the stack
		elemGrammar = strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].typeGrammar;
		strm->gStack->lastNonTermID = *nonTermID_out;
		if(elemGrammar != NULL) // The grammar is found
		{
			*nonTermID_out = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), elemGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			return INCONSISTENT_PROC_STATE;  // The event require the presence of Element Grammar previously created
		}
	}
	else if(event.eventType == EVENT_AT_QNAME || event.eventType == EVENT_CH)
	{
		if(event.eventType == EVENT_AT_QNAME)
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">AT(qname) event\n"));
			qname.uri = &(strm->uriTable->rows[strm->context.curr_uriID].string_val);
			qname.localName = &(strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].string_val);
			tmp_err_code = decodePrefixQname(strm, &qname);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->attribute != NULL)  // Invoke handler method
			{
				if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			// handle xsi:nil attribute
			if(strm->gStack->grammar->grammarType >= GR_TYPE_SCHEMA_DOC && strm->context.curr_uriID == 2 && strm->context.curr_lnID == 0) // Schema-enabled grammar and http://www.w3.org/2001/XMLSchema-instance:nil
			{
				strm->context.expectATData = VALUE_TYPE_BOOLEAN;
			}
		}
		else
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));
		}

		if(event.valueType.exiType == VALUE_TYPE_STRING || event.valueType.exiType == VALUE_TYPE_NONE || event.valueType.exiType == VALUE_TYPE_UNTYPED)
		{
			String value;
			unsigned char freeable = FALSE;

			//  If nil is not a valid Boolean, represent the xsi:nil attribute event using the AT (*) [untyped value] terminal
			if(event.valueType.exiType == VALUE_TYPE_UNTYPED)
				strm->context.expectATData = FALSE;

			tmp_err_code = decodeStringValue(strm, &value, &freeable);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->stringData != NULL)  // Invoke handler method
			{
				if(handler->stringData(value, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
			if(freeable)
				freeLastManagedAlloc(&strm->memList);
		}
		else if(event.valueType.exiType == VALUE_TYPE_BOOLEAN)
		{
			unsigned char bool_val;
			tmp_err_code = decodeBoolean(strm, &bool_val);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->booleanData != NULL)  // Invoke handler method
			{
				if(handler->booleanData(bool_val, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			if(strm->gStack->grammar->grammarType >= GR_TYPE_SCHEMA_DOC && strm->context.expectATData == VALUE_TYPE_BOOLEAN)
			{
				if(bool_val == TRUE)
				{
					// TODO: If the value of nil is true, evaluate the element contents using the grammar TypeEmpty k defined above rather than RightHandSide.
					return NOT_IMPLEMENTED_YET;
				}
				strm->context.expectATData = FALSE;
			}
		}
		else if(event.valueType.exiType == VALUE_TYPE_BINARY)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else if(event.valueType.exiType == VALUE_TYPE_DATE_TIME)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else if(event.valueType.exiType == VALUE_TYPE_DECIMAL)
		{
			Decimal decVal;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Decimal value\n"));
			tmp_err_code = decodeDecimalValue(strm, &decVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->decimalData != NULL)  // Invoke handler method
			{
				if(handler->decimalData(decVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType.exiType == VALUE_TYPE_FLOAT)
		{
			Float flVal;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Float value\n"));
			tmp_err_code = decodeFloatValue(strm, &flVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->floatData != NULL)  // Invoke handler method
			{
				if(handler->floatData(flVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType.exiType == VALUE_TYPE_INTEGER)
		{
			Integer sintVal;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Integer value\n"));
			tmp_err_code = decodeIntegerValue(strm, &sintVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->intData != NULL)  // Invoke handler method
			{
				if(handler->intData(sintVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType.exiType == VALUE_TYPE_NON_NEGATIVE_INT)
		{
			UnsignedInteger uintVal;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Unsigned integer value\n"));
			tmp_err_code = decodeUnsignedInteger(strm, &uintVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->intData != NULL)  // Invoke handler method
			{
				// TODO: the cast to signed int can introduce errors. Check first!
				if(handler->intData((Integer) uintVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType.exiType == VALUE_TYPE_SMALL_INTEGER)
		{
			unsigned int uintVal;
			int base;
			int upLimit;

			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Small integer value\n"));

			if(event.valueType.simpleTypeID >= strm->schema->sTypeArraySize)
				return INVALID_EXI_INPUT;

			if((strm->schema->simpleTypeArray[event.valueType.simpleTypeID].facetPresenceMask & TYPE_FACET_MIN_INCLUSIVE) == 0
					&& (strm->schema->simpleTypeArray[event.valueType.simpleTypeID].facetPresenceMask & TYPE_FACET_MIN_EXCLUSIVE) == 0)
				return INVALID_EXI_INPUT;
			if((strm->schema->simpleTypeArray[event.valueType.simpleTypeID].facetPresenceMask & TYPE_FACET_MAX_INCLUSIVE) == 0
					&& (strm->schema->simpleTypeArray[event.valueType.simpleTypeID].facetPresenceMask & TYPE_FACET_MAX_EXCLUSIVE) == 0)
				return INVALID_EXI_INPUT;

			if((strm->schema->simpleTypeArray[event.valueType.simpleTypeID].facetPresenceMask & TYPE_FACET_MIN_INCLUSIVE) != 0)
				base = strm->schema->simpleTypeArray[event.valueType.simpleTypeID].minInclusive;
			else
				return NOT_IMPLEMENTED_YET;

			if((strm->schema->simpleTypeArray[event.valueType.simpleTypeID].facetPresenceMask & TYPE_FACET_MAX_INCLUSIVE) != 0)
				upLimit = strm->schema->simpleTypeArray[event.valueType.simpleTypeID].maxInclusive;
			else
				return NOT_IMPLEMENTED_YET;

			tmp_err_code = decodeNBitUnsignedInteger(strm, getBitsNumber(upLimit - base), &uintVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->intData != NULL)  // Invoke handler method
			{
				if(handler->intData((Integer) (base + uintVal), app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType.exiType == VALUE_TYPE_LIST)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else if(event.valueType.exiType == VALUE_TYPE_QNAME)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else
			return INCONSISTENT_PROC_STATE;
	}
	else if(event.eventType == EVENT_NS)
	{
		uint16_t uriID;
		unsigned int prfxID;
		unsigned char boolean = FALSE;

		tmp_err_code = decodeURI(strm, &uriID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = decodePrefix(strm, uriID, &prfxID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = decodeBoolean(strm, &boolean);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		// TODO: finish this
	}
	else
	{
		return NOT_IMPLEMENTED_YET;
	}

	return ERR_OK;
}
