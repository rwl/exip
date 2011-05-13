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
 * @file grammarAugment.h
 * @brief Utility functions for adding Undeclared Productions to a normalized grammar.
 * @date Feb 3, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef GRAMMARAUGMENT_H_
#define GRAMMARAUGMENT_H_

#include "procTypes.h"

/**
 * @brief Adds Undeclared Productions
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] strict strict option value; 0-false, true otherwise
 * @param[in] selfContained Enables self-contained elements: TRUE or FALSE
 * @param[in] preserve Specifies whether comments, pis, etc. are preserved - bit mask of booleans
 * Use IS_PRESERVED macro to retrieve the values different preserve options
 * @param[in, out] grammar the normalized grammar for assigning the event codes
 * @return Error handling code
 */
errorCode addUndeclaredProductions(AllocList* memList, unsigned char strict, unsigned char selfContained, unsigned char preserve, EXIGrammar* grammar);

/**
 * @brief Adds Undeclared Productions in all global grammars accessible through the stringTables
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] stringTables the string table containing links to the global grammars
 * @param[in] opts options from the EXI header
 * @return Error handling code
 */
errorCode addUndeclaredProductionsToAll(AllocList* memList, URITable* stringTables, EXIOptions* opts);


#endif /* GRAMMARAUGMENT_H_ */
