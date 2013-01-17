/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammarAugment.h
 * @brief Utility functions for adding Undeclared Productions to a normalized grammar.
 * @date Feb 3, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef GRAMMARAUGMENT_H_
#define GRAMMARAUGMENT_H_

#include "procTypes.h"

/**
 * @brief Augment document grammar
 * Adds the missing DT, CM, PI if needed
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] preserve Specifies whether comments, pis, etc. are preserved - bit mask of booleans
 * Use IS_PRESERVED macro to retrieve the values different preserve options
 * @param[in, out] docGrammar document grammar to be augmented
 * @return Error handling code
 */
errorCode augmentDocGrammar(AllocList* memList, unsigned char preserve, EXIGrammar* docGrammar);

/**
 * @brief Augment fragment grammar
 * Adds the missing CM and PI if needed
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] preserve Specifies whether comments, pis, etc. are preserved - bit mask of booleans
 * Use IS_PRESERVED macro to retrieve the values different preserve options
 * @param[in, out] fragGrammar fragment grammar to be augmented
 * @return Error handling code
 */
errorCode augmentFragGrammar(AllocList* memList, unsigned char preserve, EXIGrammar* fragGrammar);

/**
 * @brief Adds Undeclared Productions to a grammar
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] strict strict option value; 0-false, true otherwise
 * @param[in] selfContained Enables self-contained elements: TRUE or FALSE
 * @param[in] preserve Specifies whether comments, pis, etc. are preserved - bit mask of booleans
 * Use IS_PRESERVED macro to retrieve the values different preserve options
 * @param[in, out] grammar the normalized grammar for assigning the event codes
 * @param[in] simpleTypeTable array of schema simple type definitions
 * @return Error handling code
 */
errorCode addUndeclaredProductions(AllocList* memList, unsigned char strict, unsigned char selfContained, unsigned char preserve, EXIGrammar* grammar, SimpleTypeTable* simpleTypeTable);

/**
 * @brief Adds Undeclared Productions in all grammars in a schema
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] schema the schema containing the grammars to be augmented
 * @param[in] opts options from the EXI header
 * @return Error handling code
 */
errorCode addUndeclaredProductionsToAll(AllocList* memList, EXIPSchema* schema, EXIOptions* opts);


#endif /* GRAMMARAUGMENT_H_ */
