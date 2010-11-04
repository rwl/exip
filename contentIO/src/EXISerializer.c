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
 * @brief Implementation of serializer of EXI streams
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "../include/EXISerializer.h"
#include "grammars.h"

errorCode initStream(EXIStream* strm, unsigned int initialBufSize)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	strm->buffer = (char*) memManagedAllocate(sizeof(char)*initialBufSize);
	if(strm->buffer == NULL)
		return MEMORY_ALLOCATION_ERROR;

	strm->opts = (struct EXIOptions*) memManagedAllocate(sizeof(struct EXIOptions));
	if(strm->opts == NULL)
		return MEMORY_ALLOCATION_ERROR;
	strm->bitPointer = 0;
	strm->bufLen = initialBufSize;
	strm->bufferIndx = 0;
	strm->nonTermID = GR_DOCUMENT;

	strm->gStack = (EXIGrammarStack*) memManagedAllocate(sizeof(EXIGrammarStack));
	if(strm->gStack == NULL)
		return MEMORY_ALLOCATION_ERROR;
	tmp_err_code = getBuildInDocGrammar(strm->gStack, strm->opts);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	strm->gPool = (struct ElementGrammarPool*) memManagedAllocate(sizeof(struct ElementGrammarPool));
	if(strm->gPool == NULL)
		return MEMORY_ALLOCATION_ERROR;
	tmp_err_code = createElementGrammarPool(strm->gPool);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createInitialStringTables(strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

//TODO: IMPORTANT: the content of the serializer functions below must be refactored. There is a lot of repeating code

errorCode startDocumentSer(EXIStream* strm)
{
	if(strm->nonTermID != GR_DOCUMENT)
		return INCONSISTENT_PROC_STATE;

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int j = 0;
	int i = 0;
	for(; i < strm->gStack->rulesDimension; i++)
	{
		if(strm->gStack->ruleArray[i].nonTermID == strm->nonTermID)
		{
			j = 0;
			for(; j < strm->gStack->ruleArray[i].prodCount; j++)
			{
				if(strm->gStack->ruleArray[i].prodArray[j].eType == EVENT_SD)
				{
					tmp_err_code = writeEventCode(strm, strm->gStack->ruleArray[i].prodArray[j].code, strm->gStack->ruleArray[i].bits);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					strm->nonTermID = strm->gStack->ruleArray[i].prodArray[j].nonTermID;
					return ERR_OK;
				}
			}
		}
	}
	return EVENT_CODE_MISSING;
}

errorCode endDocumentSer(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int j = 0;
	int i = 0;
	for(; i < strm->gStack->rulesDimension; i++)
	{
		if(strm->gStack->ruleArray[i].nonTermID == strm->nonTermID)
		{
			j = 0;
			for(; j < strm->gStack->ruleArray[i].prodCount; j++)
			{
				if(strm->gStack->ruleArray[i].prodArray[j].eType == EVENT_ED)
				{
					tmp_err_code = writeEventCode(strm, strm->gStack->ruleArray[i].prodArray[j].code, strm->gStack->ruleArray[i].bits);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					strm->nonTermID = strm->gStack->ruleArray[i].prodArray[j].nonTermID;
					return ERR_OK;
				}
			}
		}
	}
	return EVENT_CODE_MISSING;
}

errorCode startElementSer(EXIStream* strm, QName qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int j = 0;
	int i = 0;
	for(; i < strm->gStack->rulesDimension; i++)
	{
		if(strm->gStack->ruleArray[i].nonTermID == strm->nonTermID)
		{
			int prodHit = -1;
			int prodHitIndicator = 0; // What type of event (ALL, URI, QNAME) is hit so far; 0-nothing, 1-ALL, 2-URI, 3-QNAME
			j = 0;
			for(; j < strm->gStack->ruleArray[i].prodCount; j++)
			{
				if(strm->gStack->ruleArray[i].prodArray[j].eType == EVENT_SE_ALL)
				{
					if(prodHitIndicator == 0)
					{
						prodHit = j;
						prodHitIndicator = 1;
					}
				}
				else if(strm->gStack->ruleArray[i].prodArray[j].eType == EVENT_SE_URI)
				{
					if(str_equal(strm->uriTable->rows[strm->gStack->ruleArray[i].prodArray[j].uriRowID].string_val, *(qname.uri)))
					{
						if(prodHitIndicator < 3)
						{
							prodHit = j;
							prodHitIndicator = 2;
						}
					}
				}
				else if(strm->gStack->ruleArray[i].prodArray[j].eType == EVENT_SE_QNAME)
				{
					if(str_equal(strm->uriTable->rows[strm->gStack->ruleArray[i].prodArray[j].uriRowID].string_val, *(qname.uri)) &&
							str_equal(strm->uriTable->rows[strm->gStack->ruleArray[i].prodArray[j].uriRowID].lTable->rows[strm->gStack->ruleArray[i].prodArray[j].lnRowID].string_val, *(qname.localName)))
					{
						prodHit = j;
						prodHitIndicator = 3;
						break; // There is not a shorter code for that Event code
					}
				}
			}
			if(prodHit >= 0)
			{
				tmp_err_code = writeEventCode(strm, strm->gStack->ruleArray[i].prodArray[prodHit].code, strm->gStack->ruleArray[i].bits);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
				strm->nonTermID = strm->gStack->ruleArray[i].prodArray[prodHit].nonTermID;
				uint32_t uriID;
				uint32_t lnID;
				tmp_err_code = encodeQName(strm, qname, &uriID, &lnID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
				return ERR_OK;
			}
		}
	}
	return EVENT_CODE_MISSING;
}

errorCode endElementSer(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int j = 0;
	int i = 0;
	for(; i < strm->gStack->rulesDimension; i++)
	{
		if(strm->gStack->ruleArray[i].nonTermID == strm->nonTermID)
		{
			j = 0;
			for(; j < strm->gStack->ruleArray[i].prodCount; j++)
			{
				if(strm->gStack->ruleArray[i].prodArray[j].eType == EVENT_EE)
				{
					tmp_err_code = writeEventCode(strm, strm->gStack->ruleArray[i].prodArray[j].code, strm->gStack->ruleArray[i].bits);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					strm->nonTermID = strm->gStack->ruleArray[i].prodArray[j].nonTermID;
					return ERR_OK;
				}
			}
		}
	}
	return EVENT_CODE_MISSING;
}

errorCode attributeSer(EXIStream* strm, QName qname)
{
	return NOT_IMPLEMENTED_YET;
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
	return NOT_IMPLEMENTED_YET;
}

errorCode floatDataSer(EXIStream* strm, double float_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode bigFloatDataSer(EXIStream* strm, BigFloat float_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode binaryDataSer(EXIStream* strm, const char* binary_val, uint32_t nbytes)
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
