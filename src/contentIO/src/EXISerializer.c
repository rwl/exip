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
 * @file EXISerializer.c
 * @brief Implementation of the serializer of EXI streams
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "EXISerializer.h"
#include "grammars.h"
#include "memManagement.h"
#include "sTables.h"
#include "headerEncode.h"
#include "bodyEncode.h"
#include "grammarAugment.h"
#include "hashtable.h"
#include "stringManipulate.h"
#include "streamEncode.h"
#include "buildInGrammars.h"

/**
 * The handler to be used by the applications to serialize EXI streams
 */
const EXISerializer serialize ={startDocument,
								endDocument,
								startElement,
								endElement,
								attribute,
								intData,
								booleanData,
								stringData,
								floatData,
								binaryData,
								dateTimeData,
								decimalData,
								processingInstruction,
								namespaceDeclaration,
								encodeHeader,
								selfContained,
								initHeader,
								initStream,
								closeEXIStream};

void initHeader(EXIStream* strm)
{
	strm->header.has_cookie = FALSE;
	strm->header.has_options = FALSE;
	strm->header.is_preview_version = FALSE;
	strm->header.version_number = 1;
	makeDefaultOpts(&strm->header.opts);
}

errorCode initStream(EXIStream* strm, char* buf, size_t bufSize, IOStream* ioStrm, EXIPSchema* schema, unsigned char schemaIdMode, String* schemaID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIGrammar* docGr;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">EXI stream initialization \n"));

	tmp_err_code = initAllocList(&(strm->memList));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	strm->buffer = buf;
	strm->context.bitPointer = 0;
	strm->bufLen = bufSize;
	strm->bufContent = 0;
	strm->context.bufferIndx = 0;
	strm->context.nonTermID = GR_DOCUMENT;
	strm->context.curr_uriID = 0;
	strm->context.curr_lnID = 0;
	strm->context.curr_attr_uriID = 0;
	strm->context.curr_attr_lnID = 0;
	strm->context.expectATData = 0;
	strm->gStack = NULL;
	strm->uriTable = NULL;
	strm->vTable = NULL;
	strm->schema = schema;
	if(ioStrm == NULL)
	{
		strm->ioStrm.readWriteToStream = NULL;
		strm->ioStrm.stream = NULL;
	}
	else
	{
		strm->ioStrm.readWriteToStream = ioStrm->readWriteToStream;
		strm->ioStrm.stream = ioStrm->stream;
	}

	docGr = (EXIGrammar*) memManagedAllocate(&(strm->memList), sizeof(EXIGrammar));
	if(docGr == NULL)
		return MEMORY_ALLOCATION_ERROR;

	if(schema != NULL) // schema enabled encoding
	{
		if(schemaIdMode == SCHEMA_ID_NIL || schemaIdMode == SCHEMA_ID_EMPTY)
			return INVALID_EXIP_CONFIGURATION;

		strm->uriTable = schema->initialStringTables;
		tmp_err_code = createValueTable(&(strm->vTable), &(strm->memList));
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addUndeclaredProductionsToAll(&strm->memList, strm->uriTable, &strm->header.opts, schema->simpleTypeArray, schema->sTypeArraySize);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(schemaIdMode != SCHEMA_ID_EMPTY)
	{
		tmp_err_code = createInitialStringTables(strm);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	if(WITH_FRAGMENT(strm->header.opts.enumOpt))
	{
		tmp_err_code = createFragmentGrammar(docGr, strm, schema);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		tmp_err_code = createDocGrammar(docGr, strm, schema);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = pushGrammar(&strm->gStack, docGr);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// EXI schemaID handling

	if(schemaIdMode == SCHEMA_ID_ABSENT)
	{
		if(schemaID != NULL)
			return INVALID_EXIP_CONFIGURATION;
	}
	else if(schemaIdMode == SCHEMA_ID_SET)
	{
		if(schemaID == NULL)
			return INVALID_EXIP_CONFIGURATION;
		tmp_err_code = cloneString(schemaID, &strm->header.opts.schemaID, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(schemaIdMode == SCHEMA_ID_NIL)
	{
		strm->header.opts.schemaID.length = SCHEMA_ID_NIL;
	}
	else if(schemaIdMode == SCHEMA_ID_EMPTY)
	{
		strm->header.opts.schemaID.length = SCHEMA_ID_EMPTY;

		strm->schema = memManagedAllocate(&strm->memList, sizeof(EXIPSchema));
		if(strm->schema == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_err_code = generateSchemaBuildInGrammars(strm->schema);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		strm->uriTable = strm->schema->initialStringTables;
		tmp_err_code = createValueTable(&(strm->vTable), &(strm->memList));
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addUndeclaredProductionsToAll(&strm->memList, strm->uriTable, &strm->header.opts, strm->schema->simpleTypeArray, strm->schema->sTypeArraySize);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	// #DOCUMENT#
	// Hashtable for fast look-up of global values in the table.
	// Only used when:
	// serializing &&
	// valuePartitionCapacity > 50  &&   //for small table full-scan will work better
	// valueMaxLength > 0 && // this is essentially equal to valuePartitionCapacity == 0
	// HASH_TABLE_USE == ON // build configuration parameter
	if(strm->header.opts.valuePartitionCapacity > DEFAULT_VALUE_ROWS_NUMBER &&
			strm->header.opts.valueMaxLength > 0 &&
			HASH_TABLE_USE == ON)
	{
		strm->vTable->hashTbl = create_hashtable(53, djbHash, stringEqual);
		if(strm->vTable->hashTbl == NULL)
			return HASH_TABLE_ERROR;
	}

	return ERR_OK;
}

errorCode startDocument(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	size_t lastCodePart;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start doc serialization\n"));

	if(strm->context.nonTermID != GR_DOCUMENT)
		return INCONSISTENT_PROC_STATE;

	tmp_err_code = lookupProduction(strm, getEventDefType(EVENT_SD), NULL, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return serializeEvent(strm, codeLength, lastCodePart, NULL);
}

errorCode endDocument(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	size_t lastCodePart;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">End doc serialization\n"));

	tmp_err_code = lookupProduction(strm, getEventDefType(EVENT_ED), NULL, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return serializeEvent(strm, codeLength, lastCodePart, NULL);
}

errorCode startElement(EXIStream* strm, QName qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	size_t lastCodePart;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start element serialization\n"));

	tmp_err_code = lookupProduction(strm, getEventDefType(EVENT_SE_ALL), &qname, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return serializeEvent(strm, codeLength, lastCodePart, &qname);
}

errorCode endElement(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	size_t lastCodePart;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">End element serialization\n"));

	tmp_err_code = lookupProduction(strm, getEventDefType(EVENT_EE), NULL, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return serializeEvent(strm, codeLength, lastCodePart, NULL);
}

errorCode attribute(EXIStream* strm, QName qname, EXIType exiType)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	size_t lastCodePart;
	EXIEvent atEvent;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start attribute serialization\n"));

	atEvent.eventType = EVENT_AT_ALL;
	atEvent.valueType.exiType = exiType;
	atEvent.valueType.simpleTypeID = UINT16_MAX;

	tmp_err_code = lookupProduction(strm, atEvent, &qname, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return serializeEvent(strm, codeLength, lastCodePart, &qname);
}

errorCode intData(EXIStream* strm, Integer int_val)
{
	ValueType intType;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start integer data serialization\n"));

	intType.exiType = VALUE_TYPE_INTEGER;
	intType.simpleTypeID = UINT16_MAX;

	if(strm->context.expectATData) // Value for an attribute
	{
		intType.exiType = strm->context.expectATData;
		strm->context.expectATData = FALSE;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		EXIEvent event = {EVENT_CH, {VALUE_TYPE_INTEGER, UINT16_MAX}};
		unsigned char codeLength;
		size_t lastCodePart;
		size_t currentRule = strm->context.nonTermID;

		tmp_err_code = lookupProduction(strm, event, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = serializeEvent(strm, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		intType.exiType = strm->gStack->grammar->ruleArray[currentRule].part[codeLength - 1].prodArray[lastCodePart].event.valueType.exiType;
	}

	return encodeIntData(strm, int_val, intType);
}

errorCode booleanData(EXIStream* strm, unsigned char bool_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char isXsiNilAttr = FALSE;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start boolean data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
		if(strm->context.curr_attr_uriID == 2 && strm->context.curr_attr_lnID == 0)
		{
			// xsi:nill
			isXsiNilAttr = TRUE;
		}
	}
	else
	{
		EXIEvent event = {EVENT_CH, {VALUE_TYPE_BOOLEAN, SIMPLE_TYPE_BOOLEAN}};
		unsigned char codeLength;
		size_t lastCodePart;

		tmp_err_code = lookupProduction(strm, event, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = serializeEvent(strm, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = encodeBoolean(strm, bool_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->gStack->grammar->grammarType >= GR_TYPE_SCHEMA_DOC && isXsiNilAttr && bool_val)
	{
		// In a schema-informed grammar && xsi:nil == TRUE
		EXIGrammar* tmpGrammar;
		popGrammar(&(strm->gStack), &tmpGrammar);

		tmpGrammar = strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].typeEmptyGrammar;
		if(tmpGrammar == NULL)
			return INCONSISTENT_PROC_STATE;

		tmp_err_code = pushGrammar(&(strm->gStack), tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		strm->context.nonTermID = GR_START_TAG_CONTENT;
	}

	return ERR_OK;
}

errorCode stringData(EXIStream* strm, const String str_val)
{
	uint16_t uriID;
	size_t lnID;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start string data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
		uriID = strm->context.curr_attr_uriID;
		lnID = strm->context.curr_attr_lnID;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		EXIEvent event = {EVENT_CH, {VALUE_TYPE_STRING, UINT16_MAX}};
		unsigned char codeLength;
		size_t lastCodePart;

		tmp_err_code = lookupProduction(strm, event, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = serializeEvent(strm, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		uriID = strm->context.curr_uriID;
		lnID = strm->context.curr_lnID;
	}

	return encodeStringData(strm, str_val, uriID, lnID);
}

errorCode floatData(EXIStream* strm, Float float_val)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start float data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		EXIEvent event = {EVENT_CH, {VALUE_TYPE_FLOAT, UINT16_MAX}};
		unsigned char codeLength;
		size_t lastCodePart;

		tmp_err_code = lookupProduction(strm, event, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = serializeEvent(strm, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return encodeFloatValue(strm, float_val);
}

errorCode binaryData(EXIStream* strm, const char* binary_val, size_t nbytes)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode dateTimeData(EXIStream* strm, struct tm dt_val, uint16_t presenceMask)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decimalData(EXIStream* strm, Decimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode processingInstruction(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode namespaceDeclaration(EXIStream* strm, const String namespace, const String prefix, unsigned char isLocalElementNS)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	size_t lastCodePart;
	uint16_t uriID;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start namespace declaration\n"));

	tmp_err_code = lookupProduction(strm, getEventDefType(EVENT_NS), NULL, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = serializeEvent(strm, codeLength, lastCodePart, NULL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = encodeURI(strm, (String*) &namespace, &uriID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->uriTable->rows[uriID].pTable == NULL)
	{
		tmp_err_code = createPrefixTable(&strm->uriTable->rows[uriID].pTable, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = encodePrefix(strm, uriID, (String*) &prefix);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeBoolean(strm, isLocalElementNS);
}

errorCode selfContained(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode closeEXIStream(EXIStream* strm)
{
	errorCode tmp_err_code = ERR_OK;
	EXIGrammar* tmp;

	while(strm->gStack != NULL)
	{
		popGrammar(&strm->gStack, &tmp);
	}

	// Flush the buffer first if there is output Stream
	if(strm->ioStrm.readWriteToStream != NULL)
	{
		if(strm->ioStrm.readWriteToStream(strm->buffer, strm->context.bufferIndx + 1, strm->ioStrm.stream) < strm->context.bufferIndx + 1)
			tmp_err_code = BUFFER_END_REACHED;
	}
	if(strm->schema != NULL)
	{
		if(strm->schema->isStatic == TRUE)
		{
			// Reseting the value cross table links to NULL
			uint16_t i;
			size_t j;
			for(i = 0; i < strm->schema->initialStringTables->rowCount; i++)
			{
				for(j = 0; j < strm->schema->initialStringTables->rows[i].lTable->rowCount; j++)
				{
					strm->schema->initialStringTables->rows[i].lTable->rows[j].vCrossTable = NULL;
				}
			}
		}
		else
			freeAllocList(&strm->schema->memList);
	}
	freeAllMem(strm);
	return tmp_err_code;
}

errorCode serializeEvent(EXIStream* strm, unsigned char codeLength, size_t lastCodePart, QName* qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	GrammarRule* currentRule;
	Production* prodHit = NULL;

	if(strm->context.nonTermID >=  strm->gStack->grammar->rulesDimension)
		return INCONSISTENT_PROC_STATE;

	currentRule = &strm->gStack->grammar->ruleArray[strm->context.nonTermID];

#if DEBUG_CONTENT_IO == ON
	{
		tmp_err_code = printGrammarRule(strm->context.nonTermID, currentRule);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Error printing grammar rule\n"));
		}
	}
#endif

	if(codeLength - 1 > 2)
		return INCONSISTENT_PROC_STATE;

	if(lastCodePart >= currentRule->part[codeLength - 1].prodArraySize)
		return INCONSISTENT_PROC_STATE;

	prodHit = &currentRule->part[codeLength - 1].prodArray[currentRule->part[codeLength - 1].prodArraySize - 1 - lastCodePart];

	tmp_err_code = writeEventCode(strm, currentRule, codeLength, lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM && codeLength > 1
		&& (prodHit->event.eventType == EVENT_CH || prodHit->event.eventType == EVENT_EE))  // If the current grammar is build-in Element grammar and the event code size is bigger than 1 and the event is CH or EE...
	{
		// #1# COMMENT and #2# COMMENT
		tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, prodHit->event, prodHit->nonTermID,
				prodHit->qname.lnRowId, prodHit->qname.uriRowId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	switch(prodHit->event.eventType)
	{
		case EVENT_AT_ALL:
		{
			if(qname == NULL)
				return NULL_POINTER_REF;

			tmp_err_code = encodeQName(strm, *qname, prodHit->event.eventType, &strm->context.curr_attr_uriID, &strm->context.curr_attr_lnID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)  // If the current grammar is build-in Element grammar ...
			{
				EXIEvent newEvent;

				/** prodHit->event - 2 is equivalent to EVENT_AT_QNAME or EVENT_SE_QNAME*/
				newEvent.eventType = prodHit->event.eventType - 2;
				newEvent.valueType.exiType = prodHit->event.valueType.exiType;
				newEvent.valueType.simpleTypeID = prodHit->event.valueType.simpleTypeID;

				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, newEvent, prodHit->nonTermID, strm->context.curr_attr_lnID, strm->context.curr_attr_uriID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		break;
		case EVENT_SE_ALL:
		{
			if(qname == NULL)
				return NULL_POINTER_REF;

			tmp_err_code = encodeQName(strm, *qname, prodHit->event.eventType, &strm->context.curr_uriID, &strm->context.curr_lnID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(strm->gStack->grammar->grammarType == GR_TYPE_BUILD_IN_ELEM)  // If the current grammar is build-in Element grammar ...
			{
				EXIEvent newEvent;

				/** prodHit->event - 2 is equivalent to EVENT_AT_QNAME or EVENT_SE_QNAME*/
				newEvent.eventType = prodHit->event.eventType - 2;
				newEvent.valueType.exiType = prodHit->event.valueType.exiType;
				newEvent.valueType.simpleTypeID = prodHit->event.valueType.simpleTypeID;

				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, newEvent, prodHit->nonTermID, strm->context.curr_lnID, strm->context.curr_uriID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		break;
		case EVENT_AT_URI:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		case EVENT_SE_URI:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		case EVENT_AT_QNAME:
		{
			strm->context.curr_attr_uriID = prodHit->qname.uriRowId;
			strm->context.curr_attr_lnID = prodHit->qname.lnRowId;

			tmp_err_code = encodePrefixQName(strm, qname, prodHit->event.eventType, prodHit->qname.uriRowId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		break;
		case EVENT_SE_QNAME:
		{
			strm->context.curr_uriID = prodHit->qname.uriRowId;
			strm->context.curr_lnID = prodHit->qname.lnRowId;

			tmp_err_code = encodePrefixQName(strm, qname, prodHit->event.eventType, prodHit->qname.uriRowId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		break;
	}

	strm->context.nonTermID = prodHit->nonTermID;

	if(prodHit->event.eventType == EVENT_SE_ALL ||
			prodHit->event.eventType == EVENT_SE_QNAME ||
			prodHit->event.eventType == EVENT_SE_URI)
	{
		EXIGrammar* elemGrammar = NULL;

		// New element grammar is pushed on the stack
		elemGrammar = strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].typeGrammar;
		strm->gStack->lastNonTermID = strm->context.nonTermID;
		if(elemGrammar != NULL) // The grammar is found
		{
			strm->context.nonTermID = GR_START_TAG_CONTENT;
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

			strm->context.nonTermID = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), newElementGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else if(prodHit->event.eventType == EVENT_EE)
	{
		if(strm->context.nonTermID == GR_VOID_NON_TERMINAL)
		{
			EXIGrammar* grammar;
			popGrammar(&(strm->gStack), &grammar);
			if(strm->gStack != NULL) // There is more grammars in the stack
				strm->context.nonTermID = strm->gStack->lastNonTermID;
		}
	}
	else if(prodHit->event.eventType == EVENT_AT_ALL ||
			prodHit->event.eventType == EVENT_AT_QNAME ||
			prodHit->event.eventType == EVENT_AT_URI)
	{
		strm->context.expectATData = prodHit->event.valueType.exiType;
	}

	return ERR_OK;
}
