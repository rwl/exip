/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammarRules.h
 * @brief Types and functions describing EXI grammar rules
 * @date Sep 8, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef GRAMMARRULES_H_
#define GRAMMARRULES_H_

#include "errorHandle.h"
#include "procTypes.h"

// Defines the initial dimension of the dynamic array - prod
#define DEFAULT_PROD_ARRAY_DIM 10

/**
 * @brief Inserts a Production to a Grammar Rule (with LeftHandSide) with an event code 0
 * Note! It increments the first part of the event code of each production
 * in the current grammar with the non-terminal LeftHandSide on the left-hand side
 * @param[in, out] rule a Grammar Rule
 * @param[in] evnt event type
 * @param[in] nonTermID unique identifier of right-hand side Non-terminal
 * @param[in] qname qname identifier of the Event Type corresponding to the inserted production
 * @return Error handling code
 */
errorCode insertZeroProduction(DynGrammarRule* rule, EventType evnt, SmallIndex nonTermID, QNameID* qname);

/**
 * @brief Copies the productions in one rule into another
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] src Source grammar rule
 * @param[out] dest Destination grammar rule
 * @return Error handling code
 */
errorCode copyGrammarRule(AllocList* memList, GrammarRule* src, GrammarRule* dest);


#if EXIP_DEBUG == ON
/**
 * @brief Prints a grammar rule
 * Note! This is only for debugging purposes!
 * @param[in] nonTermID The left hand side nonTerminal of the Rule
 * @param[in] rule a Grammar Rule to be printed
 * @param[in] schema for string tables
 * @return Error handling code
 */
errorCode printGrammarRule(SmallIndex nonTermID, GrammarRule* rule, EXIPSchema *schema);

#endif // EXIP_DEBUG

#endif /* GRAMMARRULES_H_ */
