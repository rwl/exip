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
 * @file EXIParser.c
 * @brief Implementation of a parser of EXI streams
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "EXIParser.h"
#include "procTypes.h"
#include "errorHandle.h"
#include "headerDecode.h"
#include "memManagement.h"
#include "grammars.h"
#include "sTables.h"
#include "grammarAugment.h"

errorCode initParser(Parser* parser, char* binaryBuf, size_t bufLen, size_t bufContent, IOStream* ioStrm, EXIPSchema* schema, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	tmp_err_code = initAllocList(&parser->strm.memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	parser->strm.buffer = binaryBuf;
	parser->strm.context.bitPointer = 0;
	parser->strm.context.bufferIndx = 0;
	parser->strm.bufLen = bufLen;
	parser->strm.context.nonTermID = GR_DOCUMENT;
	parser->strm.context.curr_lnID = 0;
	parser->strm.context.curr_uriID = 0;
	parser->strm.context.expectATData = 0;
	parser->strm.bufContent = bufContent;
	parser->strm.schema = schema;
	if(ioStrm == NULL)
	{
		parser->strm.ioStrm.readWriteToStream = NULL;
		parser->strm.ioStrm.stream = NULL;
	}
	else
	{
		parser->strm.ioStrm.readWriteToStream = ioStrm->readWriteToStream;
		parser->strm.ioStrm.stream = ioStrm->stream;
	}

	parser->app_data = app_data;
	initContentHandler(&parser->handler);

	return ERR_OK;
}

errorCode parseHeader(Parser* parser)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = decodeHeader(&parser->strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	parser->strm.gStack = NULL;
	tmp_err_code = createDocGrammar(&parser->documentGrammar, &parser->strm, parser->strm.schema);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = pushGrammar(&parser->strm.gStack, &parser->documentGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(parser->strm.schema != NULL)
	{
		parser->strm.uriTable = parser->strm.schema->initialStringTables;
		tmp_err_code = createValueTable(&(parser->strm.vTable), &(parser->strm.memList));
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(parser->strm.schema->isAugmented == FALSE)
		{
			tmp_err_code = addUndeclaredProductionsToAll(&parser->strm.memList, parser->strm.uriTable, &parser->strm.header.opts, parser->strm.schema->simpleTypeArray, parser->strm.schema->sTypeArraySize);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else
	{
		tmp_err_code = createInitialStringTables(&parser->strm);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode parseNext(Parser* parser)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t tmpNonTermID = GR_VOID_NON_TERMINAL;
	EXIEvent event;

	tmp_err_code = processNextProduction(&parser->strm, &event, &tmpNonTermID, &parser->handler, parser->app_data);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmpNonTermID == GR_VOID_NON_TERMINAL)
	{
		EXIGrammar* grammar;
		popGrammar(&(parser->strm.gStack), &grammar);
		if(parser->strm.gStack == NULL) // There is no more grammars in the stack
		{
			parser->strm.context.nonTermID = GR_VOID_NON_TERMINAL; // The stream is parsed
		}
		else
		{
			parser->strm.context.nonTermID = parser->strm.gStack->lastNonTermID;
		}
	}
	else
	{
		parser->strm.context.nonTermID = tmpNonTermID;
	}

	if(parser->strm.context.nonTermID == GR_VOID_NON_TERMINAL)
		return PARSING_COMPLETE;

	return ERR_OK;
}

void destroyParser(Parser* parser)
{
	if(parser->strm.schema != NULL)
	{
		if(parser->strm.schema->isStatic == TRUE)
		{
			// Reseting the value cross table links to NULL
			uint16_t i;
			size_t j;
			for(i = 0; i < parser->strm.schema->initialStringTables->rowCount; i++)
			{
				for(j = 0; j < parser->strm.schema->initialStringTables->rows[i].lTable->rowCount; j++)
				{
					parser->strm.schema->initialStringTables->rows[i].lTable->rows[j].vCrossTable = NULL;
				}
			}
		}
		else
			freeAllocList(&parser->strm.schema->memList);
	}
	freeAllMem(&parser->strm);
}
