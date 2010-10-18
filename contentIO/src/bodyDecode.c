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

#include "../include/bodyDecode.h"
#include "grammars.h"
#include "sTables.h"

// TODO: use macros for conditional debugging for error messages

void decodeBody(EXIStream* strm, ContentHandler* handler)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIGrammarStack docGr;
	EXIGrammarStack* gStack = &docGr;
	tmp_err_code = getBuildInDocGrammar(gStack, strm->opts);
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create BuildInDocGrammar");
		}
		return;
	}

	tmp_err_code = createInitialStringTables(strm);
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create InitialStringTables");
		}
		return;
	}

	struct ElementGrammarPool gPool;
	tmp_err_code = createElementGrammarPool(&gPool);
	if(tmp_err_code != ERR_OK)
	{
		if(handler->fatalError != NULL)
		{
			handler->fatalError(tmp_err_code, "Cannot create ElementGrammarPool");
		}
		return;
	}

	unsigned int currNonTermID = GR_DOCUMENT;
	unsigned int tmpNonTermID = GR_VOID_NON_TERMINAL;
	EventType eType;
	while(currNonTermID != GR_VOID_NON_TERMINAL)  // Process grammar productions until gets to the end of the stream
	{
		tmp_err_code = processNextProduction(strm, &gStack, currNonTermID, &eType, &tmpNonTermID, handler, &gPool);
		if(tmp_err_code != ERR_OK)
		{
			if(handler->fatalError != NULL)
			{
				handler->fatalError(tmp_err_code, "Error processing next production");
			}
			return;
		}
		if(tmpNonTermID == GR_VOID_NON_TERMINAL)
		{
			struct EXIGrammar* grammar; // TODO: check when the memory should be freed
			tmp_err_code = popGrammar(&gStack, grammar);
			if(tmp_err_code != ERR_OK)
			{
				if(handler->fatalError != NULL)
				{
					handler->fatalError(tmp_err_code, "popGrammar failed");
				}
				return;
			}
			if(gStack == NULL) // There is no more grammars in the stack
			{
				currNonTermID = GR_VOID_NON_TERMINAL; // The stream is parsed
			}
			else
			{
				currNonTermID = gStack->lastNonTermID;
			}
		}
		else
		{
			currNonTermID = tmpNonTermID;
		}
		tmpNonTermID = GR_VOID_NON_TERMINAL;
	}
}
