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

#include "EXISerializer.h"
#include "grammars.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "sTables.h"

static errorCode encodeEXIEvent(EXIStream* strm, EXIEvent event);
static errorCode encodeEXIComplexEvent(EXIStream* strm, QName qname, unsigned char isElemOrAttr);

errorCode initStream(EXIStream* strm, unsigned int initialBufSize)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	strm->memStack = NULL;
	strm->buffer = (char*) memManagedAllocate(strm, sizeof(char)*initialBufSize);
	if(strm->buffer == NULL)
		return MEMORY_ALLOCATION_ERROR;

	strm->opts = (struct EXIOptions*) memManagedAllocate(strm, sizeof(struct EXIOptions));
	if(strm->opts == NULL)
		return MEMORY_ALLOCATION_ERROR;
	strm->bitPointer = 0;
	strm->bufLen = initialBufSize;
	strm->bufferIndx = 0;
	strm->nonTermID = GR_DOCUMENT;
	strm->sContext.curr_uriID = 0;
	strm->sContext.curr_lnID = 0;
	strm->sContext.expectATData = 0;

	strm->gStack = (EXIGrammarStack*) memManagedAllocate(strm, sizeof(EXIGrammarStack));
	if(strm->gStack == NULL)
		return MEMORY_ALLOCATION_ERROR;
	tmp_err_code = getBuildInDocGrammar(strm->gStack, strm->opts, strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createElementGrammarPool(&(strm->gPool));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createInitialStringTables(strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode encodeEXIEvent(EXIStream* strm, EXIEvent event)
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
				if(eventsEqual(strm->gStack->ruleArray[i].prodArray[j].event, event))
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

static errorCode encodeEXIComplexEvent(EXIStream* strm, QName qname, unsigned char isElemOrAttr)
{
	unsigned char e_qname;
	unsigned char e_uri;
	unsigned char e_all;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int j = 0;
	int i = 0;

	if(isElemOrAttr == 1) // SE event, Element
	{
		e_qname = EVENT_SE_QNAME;
		e_uri = EVENT_SE_URI;
		e_all = EVENT_SE_ALL;
	}
	else // AT event, Attribute
	{
		e_qname = EVENT_AT_QNAME;
		e_uri = EVENT_AT_URI;
		e_all = EVENT_AT_ALL;
	}
	for(; i < strm->gStack->rulesDimension; i++)
	{
		if(strm->gStack->ruleArray[i].nonTermID == strm->nonTermID)
		{
			int prodHit = -1;
			int prodHitIndicator = 0; // What type of event (ALL, URI, QNAME) is hit so far; 0-nothing, 1-ALL, 2-URI, 3-QNAME
			j = 0;
			for(; j < strm->gStack->ruleArray[i].prodCount; j++)
			{
				if(strm->gStack->ruleArray[i].prodArray[j].event.eventType == e_all)
				{
					if(prodHitIndicator == 0)
					{
						prodHit = j;
						prodHitIndicator = 1;
					}
				}
				else if(strm->gStack->ruleArray[i].prodArray[j].event.eventType == e_uri)
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
				else if(strm->gStack->ruleArray[i].prodArray[j].event.eventType == e_qname)
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
				strm->sContext.curr_uriID = strm->gStack->ruleArray[i].prodArray[prodHit].uriRowID;
				strm->sContext.curr_lnID = strm->gStack->ruleArray[i].prodArray[prodHit].lnRowID;
				if(prodHitIndicator == 1)
				{
					tmp_err_code = encodeQName(strm, qname);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}

				if(prodHitIndicator == 1 && isElemOrAttr == 1) // SE(*) Event
				{
					unsigned char isDocGr = 0;
					struct EXIGrammar* res = NULL;
					unsigned char is_found = 0;

					tmp_err_code = isDocumentGrammar(strm->gStack, &isDocGr);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					if(!isDocGr)  // If the current grammar is Element grammar ...
					{
						tmp_err_code = insertZeroProduction(&(strm->gStack->ruleArray[i]), getEventDefType(EVENT_SE_QNAME), strm->nonTermID, strm->sContext.curr_lnID, strm->sContext.curr_uriID);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}

					// New element grammar is pushed on the stack
					tmp_err_code = checkElementGrammarInPool(strm->gPool, strm->sContext.curr_uriID, strm->sContext.curr_lnID, &is_found, &res);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					strm->gStack->lastNonTermID = strm->nonTermID;
					if(is_found)
					{
						strm->nonTermID = GR_START_TAG_CONTENT;
						tmp_err_code = pushGrammar(&(strm->gStack), res);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
					else
					{
						struct EXIGrammar* elementGrammar = (struct EXIGrammar*) memManagedAllocate(strm, sizeof(struct EXIGrammar));
						if(elementGrammar == NULL)
							return MEMORY_ALLOCATION_ERROR;
						tmp_err_code = createBuildInElementGrammar(elementGrammar, strm->opts, strm);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
						tmp_err_code = addElementGrammarInPool(strm->gPool, strm->sContext.curr_uriID, strm->sContext.curr_lnID, elementGrammar);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
						strm->nonTermID = GR_START_TAG_CONTENT;
						tmp_err_code = pushGrammar(&(strm->gStack), elementGrammar);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
				}
				else if(prodHitIndicator == 3 && isElemOrAttr == 1) // SE(QName) Event
				{
					// New element grammar is pushed on the stack
					struct EXIGrammar* res = NULL;
					unsigned char is_found = 0;
					tmp_err_code = checkElementGrammarInPool(strm->gPool, strm->sContext.curr_uriID, strm->sContext.curr_lnID, &is_found, &res);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
					strm->gStack->lastNonTermID = strm->nonTermID;
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
				else if(prodHitIndicator == 1 && isElemOrAttr != 1) // AT(*) Event
				{
					tmp_err_code = insertZeroProduction(&(strm->gStack->ruleArray[i]), getEventDefType(EVENT_AT_QNAME), strm->nonTermID, strm->sContext.curr_lnID, strm->sContext.curr_uriID);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}

				if(isElemOrAttr != 1) // AT event
					strm->sContext.expectATData = 1;
				return ERR_OK;
			}
		}
	}
	return EVENT_CODE_MISSING;
}

errorCode startDocumentSer(EXIStream* strm)
{
	if(strm->nonTermID != GR_DOCUMENT)
		return INCONSISTENT_PROC_STATE;

	return encodeEXIEvent(strm, getEventDefType(EVENT_SD));
}

errorCode endDocumentSer(EXIStream* strm)
{
	if(strm->nonTermID != GR_DOC_END)
		return INCONSISTENT_PROC_STATE;

	return encodeEXIEvent(strm, getEventDefType(EVENT_ED));
}

errorCode startElementSer(EXIStream* strm, QName qname)
{
	return encodeEXIComplexEvent(strm, qname, 1);
}

errorCode endElementSer(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	tmp_err_code = encodeEXIEvent(strm, getEventDefType(EVENT_EE));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(strm->nonTermID == GR_VOID_NON_TERMINAL)
	{
		struct EXIGrammar* grammar;
		tmp_err_code = popGrammar(&(strm->gStack), &grammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(strm->gStack != NULL) // There is more grammars in the stack
			strm->nonTermID = strm->gStack->lastNonTermID;
	}
	return ERR_OK;
}

errorCode attributeSer(EXIStream* strm, QName qname)
{
	return encodeEXIComplexEvent(strm, qname, 2);
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
	if(strm->sContext.expectATData) // Value for an attribute
	{
		strm->sContext.expectATData = 0;
		return encodeStringData(strm, str_val);
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		tmp_err_code = encodeEXIEvent(strm, getEventDefType(EVENT_CH));
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
