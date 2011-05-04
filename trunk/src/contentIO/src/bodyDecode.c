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
#include "grammars.h"
#include "sTables.h"
#include "memManagement.h"
#include "ioUtil.h"
#include "streamDecode.h"
#include "grammarAugment.h"

void decodeBody(EXIStream* strm, ContentHandler* handler, ExipSchema* schema, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIGrammar docGr;
	size_t tmpNonTermID = GR_VOID_NON_TERMINAL;
	EXIEvent event;

	strm->gStack = NULL;
	tmp_err_code = createDocGrammar(&docGr, strm, schema);
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create BuildInDocGrammar", app_data);
		}
		freeAllMem(strm);
		return;
	}

	tmp_err_code = pushGrammar(&strm->gStack, &docGr);
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create grammar stack", app_data);
		}
		freeAllMem(strm);
		return;
	}

	if(schema != NULL)
	{
		unsigned int i = 0;
		strm->uriTable = schema->initialStringTables;
		tmp_err_code = createValueTable(&(strm->vTable), &(strm->memList));
		if(tmp_err_code != ERR_OK)
		{
			if(handler->fatalError != NULL)
			{
				handler->fatalError(tmp_err_code, "Cannot create InitialStringTables", app_data);
			}
			freeAllMem(strm);
			return;
		}

		for (i = 0; i < schema->globalElemGrammars.count; i++)
		{
			tmp_err_code = addUndeclaredProductions(&strm->memList, strm->header.opts->strict, strm->header.opts->selfContained, strm->header.opts->preserve, schema->globalElemGrammars.elems[i].grammar);
			if(tmp_err_code != ERR_OK)
			{
				if(handler->fatalError != NULL)
				{
					handler->fatalError(tmp_err_code, "Cannot add Undeclared Productions", app_data);
				}
				freeAllMem(strm);
				return;
			}
		}
		for (i = 0; i < schema->subElementGrammars.count; i++)
		{
			tmp_err_code = addUndeclaredProductions(&strm->memList, strm->header.opts->strict, strm->header.opts->selfContained, strm->header.opts->preserve, schema->subElementGrammars.elems[i].grammar);
			if(tmp_err_code != ERR_OK)
			{
				if(handler->fatalError != NULL)
				{
					handler->fatalError(tmp_err_code, "Cannot add Undeclared Productions", app_data);
				}
				freeAllMem(strm);
				return;
			}
		}

		// TODO: the same for the type grammars
	}
	else
	{
		tmp_err_code = createInitialStringTables(strm);
		if(tmp_err_code != ERR_OK)
		{
			if(handler->fatalError != NULL)
			{
				handler->fatalError(tmp_err_code, "Cannot create InitialStringTables", app_data);
			}
			freeAllMem(strm);
			return;
		}
	}

	while(strm->context.nonTermID != GR_VOID_NON_TERMINAL)  // Process grammar productions until gets to the end of the stream
	{
		tmp_err_code = processNextProduction(strm, &event, &tmpNonTermID, handler, app_data);
		if(tmp_err_code != ERR_OK)
		{
			if(handler->fatalError != NULL)
			{
				handler->fatalError(tmp_err_code, "Error processing next production", app_data);
			}
			freeAllMem(strm);
			return;
		}
		if(tmpNonTermID == GR_VOID_NON_TERMINAL)
		{
			EXIGrammar* grammar;
			tmp_err_code = popGrammar(&(strm->gStack), &grammar);
			if(tmp_err_code != ERR_OK)
			{
				if(handler->fatalError != NULL)
				{
					handler->fatalError(tmp_err_code, "popGrammar failed", app_data);
				}
				freeAllMem(strm);
				return;
			}
			if(strm->gStack == NULL) // There is no more grammars in the stack
			{
				strm->context.nonTermID = GR_VOID_NON_TERMINAL; // The stream is parsed
			}
			else
			{
				strm->context.nonTermID = strm->gStack->lastNonTermID;
			}
		}
		else
		{
			strm->context.nonTermID = tmpNonTermID;
		}
		tmpNonTermID = GR_VOID_NON_TERMINAL;
	}
	freeAllMem(strm);
}

errorCode decodeQName(EXIStream* strm, QName* qname)
{
	//TODO: add the case when Preserve.prefixes is true
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t tmp_val_buf = 0;
	unsigned char uriBits = getBitsNumber(strm->uriTable->rowCount);
	uint16_t uriID = 0; // The URI id in the URI string table
	uint32_t tmpVar = 0;
	size_t lnID = 0;


	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Decoding QName\n"));
	tmp_err_code = decodeNBitUnsignedInteger(strm, uriBits, &tmp_val_buf);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(tmp_val_buf == 0) // uri miss
	{
		StringType str;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">URI miss\n"));
		tmp_err_code = decodeString(strm, &str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addURIRow(strm->uriTable, str, &uriID, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		qname->uri = &(strm->uriTable->rows[uriID].string_val);
	}
	else // uri hit
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">URI hit\n"));
		qname->uri = &(strm->uriTable->rows[tmp_val_buf-1].string_val);
		uriID = tmp_val_buf-1;
	}

	tmp_err_code = decodeUnsignedInteger(strm, &tmpVar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmpVar == 0) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->uriTable->rows[uriID].lTable->rowCount - 1));
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">local-name table hit\n"));
		tmp_err_code = decodeNBitUnsignedInteger(strm, lnBits, &tmpVar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		lnID = (size_t) tmpVar;
		qname->localName = &(strm->uriTable->rows[uriID].lTable->rows[lnID].string_val);
	}
	else // local-name table miss
	{
		StringType lnStr;
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
		tmp_err_code = addLNRow(strm->uriTable->rows[uriID].lTable, lnStr, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		qname->localName = &(strm->uriTable->rows[uriID].lTable->rows[lnID].string_val);
	}
	strm->context.curr_uriID = uriID;
	strm->context.curr_lnID = lnID;
	return ERR_OK;
}

errorCode decodeStringValue(EXIStream* strm, StringType** value)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t uriID = strm->context.curr_uriID;
	size_t lnID = strm->context.curr_lnID;
	uint32_t tmpVar = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &tmpVar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmpVar == 0) // "local" value partition table hit
	{
		uint16_t lvID = 0;
		size_t value_table_rowID;

		unsigned char lvBits = getBitsNumber(strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable->rowCount - 1);
		tmp_err_code = decodeNBitUnsignedInteger(strm, lvBits, &tmpVar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		lvID = (uint16_t) tmpVar;
		value_table_rowID = strm->uriTable->rows[uriID].lTable->rows[lnID].vCrossTable->valueRowIds[lvID];
		(*value) = &(strm->vTable->rows[value_table_rowID].string_val);
	}
	else if(tmpVar == 1)// global value partition table hit
	{
		size_t gvID = 0;
		unsigned char gvBits = getBitsNumber((unsigned int)(strm->vTable->rowCount - 1));
		tmp_err_code = decodeNBitUnsignedInteger(strm, gvBits, &tmpVar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		gvID = (size_t) tmpVar;
		(*value) = &(strm->vTable->rows[gvID].string_val);
	}
	else  // "local" value partition and global value partition table miss
	{
		StringType gvStr;
		size_t gvID = 0;
		tmp_err_code = decodeStringOnly(strm, tmpVar - 2, &gvStr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		//TODO: Take into account valuePartitionCapacity parameter for setting globalID variable

		tmp_err_code = addGVRow(strm->vTable, gvStr, &gvID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLVRow(&(strm->uriTable->rows[uriID].lTable->rows[lnID]), gvID, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		(*value) = &(strm->vTable->rows[gvID].string_val);
	}
	return ERR_OK;
}

errorCode decodeEventContent(EXIStream* strm, EXIEvent event, ContentHandler* handler,
									size_t* nonTermID_out, GrammarRule* currRule,  void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	// TODO: implement all cases
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
		elemGrammar = strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].globalGrammar;

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

			strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].globalGrammar = newElementGrammar;
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
		if(event.valueType == VALUE_TYPE_STRING || event.valueType == VALUE_TYPE_NONE)
		{
			StringType* value;
			tmp_err_code = decodeStringValue(strm, &value);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->stringData != NULL)  // Invoke handler method
			{
				if(handler->stringData(*value, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
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
		if(handler->startElement != NULL)  // Invoke handler method passing the element qname
		{
			if(handler->startElement(qname, app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}

		// New element grammar is pushed on the stack
		elemGrammar = strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].globalGrammar;
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
			if(handler->attribute != NULL)  // Invoke handler method
			{
				if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));
		}

		if(event.valueType == VALUE_TYPE_STRING || event.valueType == VALUE_TYPE_NONE || event.valueType == VALUE_TYPE_UNTYPED)
		{
			StringType* value;
			tmp_err_code = decodeStringValue(strm, &value);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->stringData != NULL)  // Invoke handler method
			{
				if(handler->stringData(*value, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType == VALUE_TYPE_BOOLEAN)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else if(event.valueType == VALUE_TYPE_BINARY)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else if(event.valueType == VALUE_TYPE_DATE_TIME)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else if(event.valueType == VALUE_TYPE_DECIMAL)
		{
			decimal decVal;
			tmp_err_code = decodeDecimalValue(strm, &decVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->decimalData != NULL)  // Invoke handler method
			{
				if(handler->decimalData(decVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType == VALUE_TYPE_FLOAT)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else if(event.valueType == VALUE_TYPE_INTEGER)
		{
			int32_t sintVal;
			tmp_err_code = decodeIntegerValue(strm, &sintVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->intData != NULL)  // Invoke handler method
			{
				if(handler->intData(sintVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType == VALUE_TYPE_NON_NEGATIVE_INT)
		{
			uint32_t uintVal;
			tmp_err_code = decodeUnsignedInteger(strm, &uintVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->intData != NULL)  // Invoke handler method
			{
				// TODO: the cast to signed int can introduce errors. Check first!
				if(handler->intData((int32_t) uintVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		else if(event.valueType == VALUE_TYPE_LIST)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else if(event.valueType == VALUE_TYPE_QNAME)
		{
			return NOT_IMPLEMENTED_YET;
		}
		else
			return INCONSISTENT_PROC_STATE;
	}
	else
	{
		return NOT_IMPLEMENTED_YET;
	}

	return ERR_OK;
}
