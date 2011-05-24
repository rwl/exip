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

/**
 * The handler to be used by the applications to serialize EXI streams
 */
const EXISerializer serEXI  =  {startDocumentSer,
								endDocumentSer,
								startElementSer,
								endElementSer,
								attributeSer,
								intDataSer,
								bigIntDataSer,
								booleanDataSer,
								stringDataSer,
								floatDataSer,
								bigFloatDataSer,
								binaryDataSer,
								dateTimeDataSer,
								decimalDataSer,
								bigDecimalDataSer,
								processingInstructionSer,
								encodeHeader,
								selfContainedSer,
								initStream,
								closeEXIStream};

errorCode initStream(EXIStream* strm, char* buf, size_t bufSize, IOStream* ioStrm, EXIOptions* opts, ExipSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIGrammar* docGr;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">EXI stream initialization \n"));

	initAllocList(&(strm->memList));
	strm->buffer = buf;
	strm->header.opts = opts;
	strm->context.bitPointer = 0;
	strm->bufLen = bufSize;
	strm->bufContent = 0;
	strm->context.bufferIndx = 0;
	strm->context.nonTermID = GR_DOCUMENT;
	strm->context.curr_uriID = 0;
	strm->context.curr_lnID = 0;
	strm->context.expectATData = 0;
	strm->ioStrm = ioStrm;
	strm->gStack = NULL;

	docGr = (EXIGrammar*) memManagedAllocate(&(strm->memList), sizeof(EXIGrammar));
	if(docGr == NULL)
		return MEMORY_ALLOCATION_ERROR;

	if(schema != NULL)
	{
		strm->uriTable = schema->initialStringTables;
		tmp_err_code = createValueTable(&(strm->vTable), &(strm->memList));
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addUndeclaredProductionsToAll(&strm->memList, strm->uriTable, strm->header.opts);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		tmp_err_code = createInitialStringTables(strm);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = createDocGrammar(docGr, strm, schema);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = pushGrammar(&strm->gStack, docGr);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// #DOCUMENT#
	// Hashtable for fast look-up of global values in the table.
	// Only used when:
	// serializing &&
	// valuePartitionCapacity > 50  &&   //for small table full-scan will work better
	// valueMaxLength > 0 // this is essentially equal to valuePartitionCapacity == 0
	if(opts->valuePartitionCapacity > DEFAULT_VALUE_ROWS_NUMBER && opts->valueMaxLength > 0)
	{
		strm->vTable->hashTbl = create_hashtable(53, djbHash, str_equal);
		if(strm->vTable->hashTbl == NULL)
			return HASH_TABLE_ERROR;
	}

	return ERR_OK;
}

errorCode startDocumentSer(EXIStream* strm)
{
	if(strm->context.nonTermID != GR_DOCUMENT)
		return INCONSISTENT_PROC_STATE;

	return encodeSimpleEXIEvent(strm, getEventDefType(EVENT_SD));
}

errorCode endDocumentSer(EXIStream* strm)
{
	if(strm->context.nonTermID != GR_DOC_END)
		return INCONSISTENT_PROC_STATE;

	return encodeSimpleEXIEvent(strm, getEventDefType(EVENT_ED));
}

errorCode startElementSer(EXIStream* strm, QName qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIGrammar* elemGrammar = NULL;

	tmp_err_code = encodeComplexEXIEvent(strm, qname, EVENT_SE_ALL, EVENT_SE_URI, EVENT_SE_QNAME);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// New element grammar is pushed on the stack
	elemGrammar = strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].globalGrammar;
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

		strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID].globalGrammar = newElementGrammar;

		strm->context.nonTermID = GR_START_TAG_CONTENT;
		tmp_err_code = pushGrammar(&(strm->gStack), newElementGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode endElementSer(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	tmp_err_code = encodeSimpleEXIEvent(strm, getEventDefType(EVENT_EE));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(strm->context.nonTermID == GR_VOID_NON_TERMINAL)
	{
		EXIGrammar* grammar;
		tmp_err_code = popGrammar(&(strm->gStack), &grammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(strm->gStack != NULL) // There is more grammars in the stack
			strm->context.nonTermID = strm->gStack->lastNonTermID;
	}
	return ERR_OK;
}

errorCode attributeSer(EXIStream* strm, QName qname)
{
	strm->context.expectATData = TRUE;
	return encodeComplexEXIEvent(strm, qname, EVENT_AT_ALL, EVENT_AT_URI, EVENT_AT_QNAME);
}

errorCode intDataSer(EXIStream* strm, int32_t int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode bigIntDataSer(EXIStream* strm, const BigSignedInt int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode booleanDataSer(EXIStream* strm, unsigned char bool_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode stringDataSer(EXIStream* strm, const StringType str_val)
{
	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
		return encodeStringData(strm, str_val);
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		tmp_err_code = encodeSimpleEXIEvent(strm, getEventDefType(EVENT_CH));
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		return encodeStringData(strm, str_val);
	}
}

errorCode floatDataSer(EXIStream* strm, double float_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode bigFloatDataSer(EXIStream* strm, BigFloat float_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode binaryDataSer(EXIStream* strm, const char* binary_val, size_t nbytes)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode dateTimeDataSer(EXIStream* strm, struct tm dt_val, uint16_t presenceMask)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decimalDataSer(EXIStream* strm, decimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode bigDecimalDataSer(EXIStream* strm, bigDecimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode processingInstructionSer(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode selfContainedSer(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode closeEXIStream(EXIStream* strm)
{
	// Flush the buffer first if there is output Stream
	if(strm->ioStrm != NULL && strm->ioStrm->readWriteToStream != NULL)
	{
		if(strm->ioStrm->readWriteToStream(strm->buffer, strm->context.bufferIndx + 1, strm->ioStrm->stream) < strm->context.bufferIndx + 1)
			return BUFFER_END_REACHED;
	}
	freeAllMem(strm);
	return ERR_OK;
}
