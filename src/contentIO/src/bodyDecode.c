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

errorCode decodeQName(EXIStream* strm, QName* qname, uint16_t* uriID, size_t* lnID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Decoding QName\n"));

	tmp_err_code = decodeURI(strm, uriID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	qname->uri = &(strm->uriTable->rows[*uriID].string_val);

	tmp_err_code = decodeLocalName(strm, *uriID, lnID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	qname->localName = &(strm->uriTable->rows[*uriID].lTable->rows[*lnID].string_val);

	return decodePrefixQname(strm, qname, *uriID);
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
		unsigned int l_lnID;
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->uriTable->rows[uriID].lTable->rowCount - 1));
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">local-name table hit\n"));
		tmp_err_code = decodeNBitUnsignedInteger(strm, lnBits, &l_lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(l_lnID >= strm->uriTable->rows[uriID].lTable->rowCount)
			return INVALID_EXI_INPUT;
		*lnID = l_lnID;
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

errorCode decodePrefixQname(EXIStream* strm, QName* qname, uint16_t uriID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char prefixBits = 0;
	unsigned int prefixID = 0;

	qname->prefix = NULL;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) == FALSE)
		return ERR_OK;

	if(strm->uriTable->rows[uriID].pTable == NULL || strm->uriTable->rows[uriID].pTable->rowCount == 0)
		return ERR_OK;

	prefixBits = getBitsNumber(strm->uriTable->rows[uriID].pTable->rowCount - 1);

	if(prefixBits > 0)
	{
		tmp_err_code = decodeNBitUnsignedInteger(strm, prefixBits, &prefixID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(prefixID >= strm->uriTable->rows[uriID].pTable->rowCount)
			return INVALID_EXI_INPUT;
	}

	qname->prefix = &strm->uriTable->rows[uriID].pTable->string_val[prefixID];

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

errorCode decodeStringValue(EXIStream* strm, uint16_t uriID, size_t lnID, String* value, unsigned char* freeable)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
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
		unsigned int gvID = 0;
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
			tmp_err_code = addValueRows(strm, value, uriID, lnID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
			*freeable = TRUE;
	}
	return ERR_OK;
}

errorCode decodeEventContent(EXIStream* strm, EXIEvent evnt, ContentHandler* handler,
							size_t* nonTermID_out, GrammarRule* currRule,  void* app_data, uint16_t prod_uriID, size_t prod_lnID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QName qname;
	uint16_t uriID;
	size_t lnID;
	// TODO: implement all cases of events such as PI, CM etc.

	switch(evnt.eventType)
	{
		case EVENT_SE_ALL:
		{
			EXIGrammar* elemGrammar = NULL;

			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">SE(*) event\n"));
			// The content of SE event is the element qname
			tmp_err_code = decodeQName(strm, &qname, &uriID, &lnID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->startElement != NULL)  // Invoke handler method passing the element qname
			{
				if(handler->startElement(qname, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			if(IS_BUILD_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
			{
				tmp_err_code = insertZeroProduction((DynGrammarRule*) currRule, getEventDefType(EVENT_SE_QNAME), *nonTermID_out, lnID, uriID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}

			// New element grammar is pushed on the stack
			elemGrammar = strm->uriTable->rows[uriID].lTable->rows[lnID].typeGrammar;

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

				strm->uriTable->rows[uriID].lTable->rows[lnID].typeGrammar = newElementGrammar;
				*nonTermID_out = GR_START_TAG_CONTENT;
				tmp_err_code = pushGrammar(&(strm->gStack), newElementGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}

			strm->context.currElem.uriRowId = uriID;
			strm->context.currElem.lnRowId = lnID;
		}
		break;
		case EVENT_AT_ALL:
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">AT(*) event\n"));
			tmp_err_code = decodeQName(strm, &qname, &uriID, &lnID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->attribute != NULL)  // Invoke handler method
			{
				if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			tmp_err_code = decodeValueItem(strm, evnt.valueType, handler, nonTermID_out, uriID, lnID, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = insertZeroProduction((DynGrammarRule*) currRule, getEventDefType(EVENT_AT_QNAME), *nonTermID_out, lnID, uriID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			strm->context.currAttr.uriRowId = uriID;
			strm->context.currAttr.lnRowId = lnID;
		}
		break;
		case EVENT_SE_QNAME:
		{
			EXIGrammar* elemGrammar = NULL;

			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">SE(qname) event\n"));
			strm->context.currElem.uriRowId = prod_uriID;
			strm->context.currElem.lnRowId = prod_lnID;
			qname.uri = &(strm->uriTable->rows[prod_uriID].string_val);
			qname.localName = &(strm->uriTable->rows[prod_uriID].lTable->rows[strm->context.currElem.lnRowId].string_val);
			tmp_err_code = decodePrefixQname(strm, &qname, prod_uriID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->startElement != NULL)  // Invoke handler method passing the element qname
			{
				if(handler->startElement(qname, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			// New element grammar is pushed on the stack
			elemGrammar = strm->uriTable->rows[prod_uriID].lTable->rows[strm->context.currElem.lnRowId].typeGrammar;
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
		break;
		case EVENT_AT_QNAME:
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">AT(qname) event\n"));
			strm->context.currAttr.uriRowId = prod_uriID;
			strm->context.currAttr.lnRowId = prod_lnID;
			qname.uri = &(strm->uriTable->rows[strm->context.currAttr.uriRowId].string_val);
			qname.localName = &(strm->uriTable->rows[strm->context.currAttr.uriRowId].lTable->rows[prod_lnID].string_val);
			tmp_err_code = decodePrefixQname(strm, &qname, strm->context.currAttr.uriRowId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->attribute != NULL)  // Invoke handler method
			{
				if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			tmp_err_code = decodeValueItem(strm, evnt.valueType, handler, nonTermID_out, prod_uriID, prod_lnID, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		break;
		case EVENT_CH:
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));

			tmp_err_code = decodeValueItem(strm, evnt.valueType, handler, nonTermID_out, strm->context.currElem.uriRowId, strm->context.currElem.lnRowId, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		break;
		case EVENT_NS:
		{
			uint16_t ns_uriID;
			unsigned int prfxID;
			unsigned char boolean = FALSE;

			tmp_err_code = decodeURI(strm, &ns_uriID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(strm->uriTable->rows[ns_uriID].pTable == NULL)
			{
				tmp_err_code = createPrefixTable(&strm->uriTable->rows[ns_uriID].pTable, &strm->memList);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}

			tmp_err_code = decodePrefix(strm, ns_uriID, &prfxID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = decodeBoolean(strm, &boolean);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->namespaceDeclaration != NULL)  // Invoke handler method
			{
				if(handler->namespaceDeclaration(strm->uriTable->rows[ns_uriID].string_val, strm->uriTable->rows[ns_uriID].pTable->string_val[prfxID], boolean, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		break;
		default:
			return NOT_IMPLEMENTED_YET;
	}

	return ERR_OK;
}

errorCode decodeValueItem(EXIStream* strm, ValueType type, ContentHandler* handler, size_t* nonTermID_out, uint16_t local_uriID, size_t local_lnID, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	switch(type.exiType)
	{
		case VALUE_TYPE_NON_NEGATIVE_INT:
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
		break;
		case VALUE_TYPE_INTEGER:
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
		break;
		case VALUE_TYPE_SMALL_INTEGER:
		{
			unsigned int uintVal;
			int base;
			int upLimit;

			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Small integer value\n"));

			if(type.simpleTypeID >= strm->schema->sTypeArraySize)
				return INVALID_EXI_INPUT;

			if((strm->schema->simpleTypeArray[type.simpleTypeID].facetPresenceMask & TYPE_FACET_MIN_INCLUSIVE) == 0
					&& (strm->schema->simpleTypeArray[type.simpleTypeID].facetPresenceMask & TYPE_FACET_MIN_EXCLUSIVE) == 0)
				return INVALID_EXI_INPUT;
			if((strm->schema->simpleTypeArray[type.simpleTypeID].facetPresenceMask & TYPE_FACET_MAX_INCLUSIVE) == 0
					&& (strm->schema->simpleTypeArray[type.simpleTypeID].facetPresenceMask & TYPE_FACET_MAX_EXCLUSIVE) == 0)
				return INVALID_EXI_INPUT;

			if((strm->schema->simpleTypeArray[type.simpleTypeID].facetPresenceMask & TYPE_FACET_MIN_INCLUSIVE) != 0)
				base = strm->schema->simpleTypeArray[type.simpleTypeID].minInclusive;
			else
				return NOT_IMPLEMENTED_YET;

			if((strm->schema->simpleTypeArray[type.simpleTypeID].facetPresenceMask & TYPE_FACET_MAX_INCLUSIVE) != 0)
				upLimit = strm->schema->simpleTypeArray[type.simpleTypeID].maxInclusive;
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
		break;
		case VALUE_TYPE_FLOAT:
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
		break;
		case VALUE_TYPE_BOOLEAN:
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

			// handle xsi:nil attribute
			if(IS_SCHEMA(strm->gStack->grammar->props) && local_uriID == 2 && local_lnID == 0) // Schema-enabled grammar and http://www.w3.org/2001/XMLSchema-instance:nil
			{
				if(bool_val == TRUE)
				{
					// xsi:nil attribute equals to true & schema mode
					EXIGrammar* tmpGrammar;
					popGrammar(&(strm->gStack), &tmpGrammar);

					tmpGrammar = strm->uriTable->rows[strm->context.currElem.uriRowId].lTable->rows[strm->context.currElem.lnRowId].typeEmptyGrammar;
					if(tmpGrammar == NULL)
						return INCONSISTENT_PROC_STATE;

					tmp_err_code = pushGrammar(&(strm->gStack), tmpGrammar);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					*nonTermID_out = GR_START_TAG_CONTENT;
				}
			}
		}
		break;
		case VALUE_TYPE_BINARY:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		case VALUE_TYPE_DECIMAL:
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
		break;
		case VALUE_TYPE_DATE_TIME:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		case VALUE_TYPE_LIST:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		case VALUE_TYPE_QNAME:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		default: // VALUE_TYPE_STRING || VALUE_TYPE_NONE || VALUE_TYPE_UNTYPED
		{
			String value;
			unsigned char freeable = FALSE;

			tmp_err_code = decodeStringValue(strm, local_uriID, local_lnID, &value, &freeable);
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
	}

	return ERR_OK;
}
