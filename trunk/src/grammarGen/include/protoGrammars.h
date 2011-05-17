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
 * @file protoGrammars.h
 * @brief Definitions and utility functions for EXI Proto-Grammars
 * @date May 11, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef PROTOGRAMMARS_H_
#define PROTOGRAMMARS_H_

#include "procTypes.h"

struct protoGrammar
{
	Production** prods;
	unsigned int rulesCount;
	unsigned int rulesDim;
	unsigned int* prodCount;
	unsigned int* prodDim;
	unsigned int contentIndex;
	struct reAllocPair rulesMemPair; // Used by the memoryManager when there is a reallocation for prods
	struct reAllocPair* prodMemPair; // Used by the memoryManager when there is a reallocation for prods[k]
};

typedef struct protoGrammar ProtoGrammar;

/**
 * @brief Creates and allocates memory for new proto grammar
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] rulesDim initial dimension of the rule
 * @param[in] prodDim initial dimension of the productions in the rules
 * @param[out] result an empty proto-grammar
 * @return Error handling code
 */
errorCode createProtoGrammar(AllocList* memlist, unsigned int rulesDim, unsigned int prodDim, ProtoGrammar** result);

/**
 * @brief Add an empty rule to a ProtoGrammar
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in, out] pg the proto-grammar
 * @return Error handling code
 */
errorCode addProtoRule(AllocList* memlist, ProtoGrammar* pg);

/**
 * @brief Add a production to a particular proto rule
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in, out] pg the proto-grammar
 * @param[in] ruleIndex the index of the rule in which the production is inserted
 * @param[in] event of the production
 * @param[in] uriRowID of the production
 * @param[in] lnRowID of the production
 * @param[in] nonTermID of the production
 * @return Error handling code
 */
errorCode addProductionToAProtoRule(AllocList* memlist, ProtoGrammar* pg, unsigned int ruleIndex, EXIEvent event, uint16_t uriRowID, size_t lnRowID, size_t nonTermID);

/**
 * @brief Create a new EXI grammar from existing proto grammar
 *
 * @param[in, out] memList A list storing the memory allocations for the new EXI grammar
 * @param[in] pg the source proto-grammar
 * @param[out] result a pointer to the newly created EXI grammar
 * @return Error handling code
 */
errorCode convertProtoGrammar(AllocList* memlist, ProtoGrammar* pg, EXIGrammar** result);

#endif /* PROTOGRAMMARS_H_ */
