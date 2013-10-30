/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file protoGrammars.h
 * @brief Definitions and utility functions for EXI Proto-Grammars
 * @date May 11, 2011
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id$
 */

#ifndef PROTOGRAMMARS_H_
#define PROTOGRAMMARS_H_

#include "procTypes.h"

/**
 * Productions in a rule with the same number of parts in their event codes
 */
struct ProtoRuleEntry
{
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	Production* prod;
	Index count;
};

typedef struct ProtoRuleEntry ProtoRuleEntry;

struct ProtoGrammar
{
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	/**
	 * Each proto rule has a set of productions
	 */
	ProtoRuleEntry* rule;
	SmallIndex count;
	unsigned int contentIndex;
};

typedef struct ProtoGrammar ProtoGrammar;

struct ProtoGrammarArray
{
#if DYN_ARRAY_USE == ON
	DynArray dynArray;
#endif
	ProtoGrammar** pg;
	uint16_t count;
};

typedef struct ProtoGrammarArray ProtoGrammarArray;

/**
 * @brief Creates and allocates memory for new proto grammar
 *
 * @param[in] rulesDim initial number of the rules in the protogrammar
 * @param[out] pg an empty proto-grammar
 * @return Error handling code
 */
errorCode createProtoGrammar(Index rulesDim, ProtoGrammar* pg);

/**
 * @brief Frees the allocated memory for a proto grammar
 *
 * @param[out] pg the proto-grammar to be removed
 */
void destroyProtoGrammar(ProtoGrammar* pg);

/**
 * @brief Add an empty rule to a proto grammar
 *
 * @param[in, out] pg the proto grammar
 * @param[in] prodDim initial number of productions in the rule
 * @param[in] ruleEntry pointer to the rule entry
 * @return Error handling code
 */
errorCode addProtoRule(ProtoGrammar* pg, Index prodDim, ProtoRuleEntry** ruleEntry);

/**
 * @brief Add a production to a particular proto rule
 *
 * @param[in, out] ruleEntry the rule to which the production is added
 * @param[in] eventType event type of the production
 * @param[in] typeId index of the type of the production in the simple type table
 * @param[in] qnameID qnameId of the production
 * @param[in] nonTermID of the production
 * @return Error handling code
 */
errorCode addProduction(ProtoRuleEntry* ruleEntry, EventType eventType, Index typeId, QNameID qnameID, SmallIndex nonTermID);

/**
 * @brief Create a new EXI grammar from existing proto grammar
 *
 * @param[in, out] memlist A list storing the memory allocations for the new EXI grammar
 * @param[in] pg the source proto-grammar
 * @param[out] exiGrammar a pointer to the newly created EXI grammar
 * @return Error handling code
 */
errorCode convertProtoGrammar(AllocList* memlist, ProtoGrammar* pg, EXIGrammar* exiGrammar);

/**
 * @brief Clones a proto grammar instance
 *
 * @param[in] src the source proto-grammar
 * @param[out] dest a pointer to the newly created proto grammar
 * @return Error handling code
 */
errorCode cloneProtoGrammar(ProtoGrammar* src, ProtoGrammar* dest);

#if EXIP_DEBUG == ON && DEBUG_GRAMMAR_GEN == ON

errorCode printProtoGrammarRule(SmallIndex nonTermID, ProtoRuleEntry* rule);

errorCode printProtoGrammar(ProtoGrammar* pgr);

#endif

#endif /* PROTOGRAMMARS_H_ */
