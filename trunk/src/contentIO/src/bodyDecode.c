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

void decodeBody(EXIStream* strm, ContentHandler* handler, ExipSchema* schema, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	struct EXIGrammar docGr;
	unsigned int tmpNonTermID = GR_VOID_NON_TERMINAL;
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

	tmp_err_code = createGrammarPool(&(strm->ePool));
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create ElementGrammarPool", app_data);
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
			addGrammarInPool(strm->ePool, schema->globalElemGrammars.elems[i].uriRowId, schema->globalElemGrammars.elems[i].lnRowId, &schema->globalElemGrammars.elems[i].grammar);
		}
		for (i = 0; i < schema->subElementGrammars.count; i++)
		{
			addGrammarInPool(strm->ePool, schema->subElementGrammars.elems[i].uriRowId, schema->subElementGrammars.elems[i].lnRowId, &schema->subElementGrammars.elems[i].grammar);
		}

		// TODO: the same for the type grammar pool
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

	while(strm->nonTermID != GR_VOID_NON_TERMINAL)  // Process grammar productions until gets to the end of the stream
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
			struct EXIGrammar* grammar;
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
				strm->nonTermID = GR_VOID_NON_TERMINAL; // The stream is parsed
			}
			else
			{
				strm->nonTermID = strm->gStack->lastNonTermID;
			}
		}
		else
		{
			strm->nonTermID = tmpNonTermID;
		}
		tmpNonTermID = GR_VOID_NON_TERMINAL;
	}
	freeAllMem(strm);
}
