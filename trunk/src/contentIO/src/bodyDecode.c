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

// TODO: use macros for conditional debugging for error messages

void decodeBody(EXIStream* strm, ContentHandler* handler)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIGrammarStack docGr;
	unsigned int tmpNonTermID = GR_VOID_NON_TERMINAL;
	EXIEvent event;

	strm->gStack = &docGr;
	tmp_err_code = createDocGrammar(strm->gStack, strm->opts, strm, NULL);
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create BuildInDocGrammar");
		}
		freeAllMem(strm);
		return;
	}

	tmp_err_code = createInitialStringTables(strm, FALSE);
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create InitialStringTables");
		}
		freeAllMem(strm);
		return;
	}

	tmp_err_code = createGrammarPool(&(strm->ePool));
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create ElementGrammarPool");
		}
		freeAllMem(strm);
		return;
	}

	while(strm->nonTermID != GR_VOID_NON_TERMINAL)  // Process grammar productions until gets to the end of the stream
	{
		tmp_err_code = processNextProduction(strm, &event, &tmpNonTermID, handler);
		if(tmp_err_code != ERR_OK)
		{
			if(handler->fatalError != NULL)
			{
				handler->fatalError(tmp_err_code, "Error processing next production");
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
					handler->fatalError(tmp_err_code, "popGrammar failed");
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
