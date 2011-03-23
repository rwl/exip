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
#include "schema.h"

/**
 * @brief Process the next grammar production in the Current Grammar
 * Returns the terminal symbol of the production i.e. the EXI Event Type;
 * @param[in] strm EXI stream of bits
 * @param[out] eType the terminal part of the production
 * @param[out] nonTermID_out unique identifier of right-hand side Non-terminal
 * @param[in] handler content handler callbacks
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode processNextProduction(EXIStream* strm, EXIEvent* event,
							    unsigned int* nonTermID_out, ContentHandler* handler, void* app_data);

/**
 * @brief Push a grammar on top of the Grammar Stack
 * @param[in, out] gStack the Grammar Stack
 * @param[in] grammar a grammar
 * @return Error handling code
 */
errorCode pushGrammar(EXIGrammarStack** gStack, struct EXIGrammar* grammar);

/**
 * @brief Pop a grammar off the top of the Grammar Stack
 * @param[in, out] grStack the Grammar stack
 * @param[out] grammar the popped out grammar
 * @return Error handling code
 */
errorCode popGrammar(EXIGrammarStack** gStack, struct EXIGrammar** grammar);

/**
 * @brief Creates an instance of the EXI Built-in Document Grammar or Schema-Informed Document Grammar
 * If glElems is NULL -> then it creates EXI Built-in Document Grammar, otherwise
 * it creates Schema-Informed Document Grammar
 *
 * @param[in, out] docGrammar empty grammar container to be filled with rules
 * @param[in, out] strm EXI stream for which the allocations are made; also the EXI options are read from here
 * @param[in] schema the schema describing the document if any; if Built-in Document Grammar is created then the schema is NULL
 * @return Error handling code
 */
errorCode createDocGrammar(struct EXIGrammar* docGrammar, EXIStream* strm, ExipSchema* schema);

/**
 * @brief Creates an instance of EXI Built-in Element Grammar
 * @param[in] elementGrammar empty grammar container
 * @param[in, out] strm EXI stream for which the allocation is made
 * @return Error handling code
 */
errorCode createBuildInElementGrammar(struct EXIGrammar* elementGrammar, EXIStream* strm);

/**
 * @brief Creates a deep copy of a grammar with allocation fresh memory for the copy
 *
 * @param[in] memList A list storing the memory allocations
 * @param[in] src source EXI grammar
 * @param[out] dest destination EXI grammar clone of the src
 * @return Error handling code
 */
errorCode copyGrammar(AllocList* memList, struct EXIGrammar* src, struct EXIGrammar** dest);

/**
 * @brief Creates empty Element or Type Grammar pool
 * @param[in, out] pool empty pool container
 * @return Error handling code
 */
errorCode createGrammarPool(GrammarPool** pool);

/**
 * @brief Checks if a specific element or type grammar is already in the Element/Type Grammar pool
 * @param[in] pool Element/Type Grammar pool
 * @param[in] uriRowID Row id in the URI string table
 * @param[in] lnRowID Row id in the Local names string table
 * @param[out] is_found 0 is not found; 1 otherwise
 * @param[out] result if found - a pointer to the searched grammar
 * @return Error handling code
 */
errorCode checkGrammarInPool(GrammarPool* pool, uint16_t uriRowID,
									size_t lnRowID, unsigned char* is_found, struct EXIGrammar** result);


/**
 * @brief Adds a specific element or type grammar in the Element/Type Grammar pool
 * @param[in, out] pool Element/Type Grammar pool
 * @param[in] uriRowID Row id in the URI string table
 * @param[in] lnRowID Row id in the Local names string table
 * @param[in] newGr the grammar to be added
 * @return Error handling code
 */
errorCode addGrammarInPool(GrammarPool* pool, uint16_t uriRowID,
									size_t lnRowID, struct EXIGrammar* newGr);

#endif /* GRAMMARS_H_ */
