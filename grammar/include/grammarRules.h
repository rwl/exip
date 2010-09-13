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
 * @file grammarRules.h
 * @brief Types and functions describing EXI grammar rules
 * @date Sep 8, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef GRAMMARRULES_H_
#define GRAMMARRULES_H_

#include "eventsEXI.h"
#include "errorHandle.h"

// Defines the initial dimension of the dynamic array - prodArray
#define DEFAULT_PROD_ARRAY_DIM 10

// Define Built-in Document Grammar non-terminals
#define GR_VOID_NON_TERMINAL 0 // Used to indicate that the production does not have NON_TERMINAL
#define GR_DOCUMENT 1
#define GR_DOC_CONTENT 2
#define GR_DOC_END 3

struct Production
{
	EventCode code;
	EventType eType;
	unsigned int nonTermID; // unique identifier of right-hand side Non-terminal
};

typedef struct Production Production;

struct GrammarRule
{
	unsigned int nonTermID; // unique identifier of left-hand side Non-terminal
	Production* prodArray; // Array of grammar productions included in that rule
	unsigned int prodCount; // The number of productions in this Grammar Rule
	unsigned int prodDimension; // The size of the productions' array /allocated space for Productions/
};

typedef struct GrammarRule GrammarRule;

/**
 * @brief Initialize the dynamic array prodArray with the default size
 * @param[in, out] rule a Grammar Rule
 * @return Error handling code
 */
errorCode initGrammarRule(GrammarRule* rule);

/**
 * @brief Adds a Production to a Grammar Rule
 * @param[in, out] rule a Grammar Rule
 * @param[in] eCode event code
 * @param[in] eType event type
 * @param[in] nonTermID unique identifier of right-hand side Non-terminal
 * @return Error handling code
 */
errorCode addProduction(GrammarRule* rule, EventCode eCode, EventType eType, unsigned int nonTermID);

#endif /* GRAMMARRULES_H_ */
