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
#include "eventsEXI.h"
#include "grammarRules.h"
#include "procTypes.h"

struct EXIGrammar
{
	GrammarRule* ruleArray; // Array of grammar rules which constitute that grammar
	unsigned int rulesDimension; // The size of the array
	struct EXIGrammar* nextInStack;
};

typedef struct EXIGrammar EXIGrammarStack; // Used to differentiate between single grammar (nextInStack == NULL) and stack of grammars

/**
 * @brief Process the next grammar production in the Current Grammar
 * Returns the terminal symbol of the production i.e. the EXI Event Type;
 * @param[in] strm EXI stream of bits
 * @param[in, out] grStack Current Grammar stack
 * @param[in] nonTermID_in unique identifier of left-hand side Non-terminal
 * @param[out] eType the terminal part of the production
 * @param[out] nonTermID_out unique identifier of right-hand side Non-terminal
 * @return Error handling code
 */
errorCode processNextProduction(EXIStream* strm, EXIGrammarStack* grStack, unsigned int nonTermID_in, EventType* eType, unsigned int* nonTermID_out);

/**
 * @brief Push a grammar on top of the Grammar Stack
 * @param[in, out] gStack the Grammar Stack
 * @param[in] grammar a grammar
 * @return Error handling code
 */
errorCode pushGrammar(EXIGrammarStack* gStack, struct EXIGrammar* grammar);

/**
 * @brief Pop a grammar off the top of the Grammar Stack
 * @param[in, out] grStack the Grammar stack
 * @param[out] grammar the terminal part of the production
 * @return Error handling code
 */
errorCode popGrammar(EXIGrammarStack* gStack, struct EXIGrammar* grammar);


//TODO: depends on the EXI fidelity options! Take this into account
/**
 * @brief Creates an instance of the EXI Built-in Document Grammar
 * @param[in] buildInGrammar empty grammar container
 * @return Error handling code
 */
errorCode getBuildInDocGrammar(struct EXIGrammar* buildInGrammar);

#endif /* GRAMMARS_H_ */
