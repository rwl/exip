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
 * @file grammars.h
 * @brief Types and functions describing EXI grammars
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef GRAMMARS_H_
#define GRAMMARS_H_

#include "errorHandle.h"
#include "grammarRules.h"
#include "eventsEXI.h"
#include "procTypes.h"
#include "contentHandler.h"

/**
 * @brief Process the next grammar production in the Current Grammar
 * Returns the terminal symbol of the production i.e. the EXI Event Type;
 * @param[in] strm EXI stream of bits
 * @param[out] evnt the terminal part of the production
 * @param[out] nonTermID_out unique identifier of right-hand side Non-terminal
 * @param[in] handler content handler callbacks
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode processNextProduction(EXIStream* strm, EXIEvent* evnt,
							    size_t* nonTermID_out, ContentHandler* handler, void* app_data);

/**
 * @brief Push a grammar on top of the Grammar Stack
 * @param[in, out] gStack the Grammar Stack
 * @param[in] grammar a grammar
 * @return Error handling code
 */
errorCode pushGrammar(EXIGrammarStack** gStack, EXIGrammar* grammar);

/**
 * @brief Pop a grammar off the top of the Grammar Stack
 * @param[in, out] gStack the Grammar stack
 * @param[out] grammar the popped out grammar
 */
void popGrammar(EXIGrammarStack** gStack, EXIGrammar** grammar);

/**
 * @brief Creates an instance of the EXI Built-in Document Grammar or Schema-Informed Document Grammar
 * If schema is NULL then it creates EXI Built-in Document Grammar, otherwise
 * it creates Schema-Informed Document Grammar
 *
 * @param[in, out] docGrammar empty grammar container to be filled with rules
 * @param[in, out] strm EXI stream for which the allocations are made; also the EXI options are read from here
 * @param[in] schema the schema describing the document if any; if Built-in Document Grammar is created then the schema is NULL
 * @return Error handling code
 */
errorCode createDocGrammar(EXIGrammar* docGrammar, EXIStream* strm, const EXIPSchema* schema);

/**
 * @brief Creates an instance of the EXI Built-in Fragment Grammar or Schema-Informed Fragment Grammar
 * If schema is NULL then it creates EXI Built-in Fragment Grammar, otherwise
 * it creates Schema-Informed Fragment Grammar
 *
 * @param[in, out] fragGrammar empty grammar container to be filled with rules
 * @param[in, out] strm EXI stream for which the allocations are made; also the EXI options are read from here
 * @param[in] schema the schema describing the document if any; if Built-in Fragment Grammar is created then the schema is NULL
 * @return Error handling code
 */
errorCode createFragmentGrammar(EXIGrammar* fragGrammar, EXIStream* strm, const EXIPSchema* schema);

/**
 * @brief Creates an instance of EXI Built-in Element Grammar
 * @param[in] elementGrammar empty grammar container
 * @param[in, out] strm EXI stream for which the allocation is made
 * @return Error handling code
 */
errorCode createBuildInElementGrammar(EXIGrammar* elementGrammar, EXIStream* strm);

#endif /* GRAMMARS_H_ */
