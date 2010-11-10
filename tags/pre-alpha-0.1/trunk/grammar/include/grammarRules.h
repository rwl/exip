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

#include "errorHandle.h"
#include "procTypes.h"

// Defines the initial dimension of the dynamic array - prodArray
#define DEFAULT_PROD_ARRAY_DIM 10

/**
 * @brief Initialize the dynamic array prodArray with the default size
 * @param[in, out] rule a Grammar Rule
 * @param[in, out] mStack Memory stack to which this allocation will be registered
 * @return Error handling code
 */
errorCode initGrammarRule(GrammarRule* rule, struct memAlloc** mStack);

/**
 * @brief Adds a Production to a Grammar Rule
 * Note! The number of bits used for the integers constituting the EventCode must be set
 * separately. This function does not set them.
 * @param[in, out] rule a Grammar Rule
 * @param[in] eCode event code
 * @param[in] eType event type
 * @param[in] nonTermID unique identifier of right-hand side Non-terminal
 * @return Error handling code
 */
errorCode addProduction(GrammarRule* rule, EventCode eCode, EventType eType, unsigned int nonTermID);

/**
 * @brief Inserts a Production to a Grammar Rule (with LeftHandSide) with an event code 0
 * Note! It increments the first part of the event code of each production
 * in the current grammar with the non-terminal LeftHandSide on the left-hand side
 * @param[in, out] rule a Grammar Rule
 * @param[in] eType event type
 * @param[in] nonTermID unique identifier of right-hand side Non-terminal
 * @param[in] lnRowID Local name part of the qname of the Event Type corresponding to the inserted production
 * @param[in] uriRowID URI part of the qname of the Event Type corresponding to the inserted production
 * @return Error handling code
 */
errorCode insertZeroProduction(GrammarRule* rule, EventType eType, unsigned int nonTermID,
								uint32_t lnRowID, uint32_t uriRowID);

#ifdef EXIP_DEBUG // TODO: document this macro #DOCUMENT#
/**
 * @brief Prints a grammar rule
 * Note! This is only for debugging purposes!
 * @param[in] rule a Grammar Rule to be printed
 * @return Error handling code
 */
errorCode printGrammarRule(GrammarRule* rule);

#endif // EXIP_DEBUG

#endif /* GRAMMARRULES_H_ */
