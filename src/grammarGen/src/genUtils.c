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
 * @file genUtils.c
 * @brief Implementation of utility functions for generating Schema-informed Grammar definitions
 * @date Nov 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "genUtils.h"
#include "memManagement.h"
#include "grammarRules.h"
#include "eventsEXI.h"
#include "stringManipulate.h"
#include "grammars.h"
#include "ioUtil.h"

#define MAX_COLLISIONS_NUMBER 50

struct collisionInfo
{
	size_t leftNonTerminal;
	size_t rightNonTerminal;
	size_t createdNonTerminal;
};

/** Collision aware addition */
static errorCode addProductionsToARule(AllocList* memList, ProtoGrammar* left, unsigned int ruleIndex, Production* rightRule,
		unsigned int rightProdCount, struct collisionInfo* collisions, unsigned int* collisionCount, unsigned int* currRuleIndex, unsigned int initialLeftRulesCount);

// Creates the new grammar rules based on the collision information
static errorCode resolveCollisionsInGrammar(AllocList* memList, struct collisionInfo* collisions,
											unsigned int* collisionCount, ProtoGrammar* left, unsigned int* currRuleIndex);

static errorCode recursiveGrammarConcat(AllocList* memList, GenericStack* protoGrammars, ProtoGrammar** result);

static int compareProductions(const void* prod1, const void* prod2);

errorCode concatenateGrammars(AllocList* memList, ProtoGrammar* left, ProtoGrammar* right)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int ruleIterL = 0;
	unsigned int ruleIterR = 0;
	unsigned int prodIterL = 0;
	unsigned int prodIterR = 0;
	struct collisionInfo collisions[MAX_COLLISIONS_NUMBER];
	unsigned int collisionCount = 0;
	unsigned int currRuleIndex;
	unsigned int initialLeftRulesCount = left->rulesCount;

	for(ruleIterR = 1; ruleIterR < right->rulesCount; ruleIterR++)
	{
		tmp_err_code = addProtoRule(memList, left);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for(prodIterR = 0; prodIterR < right->prodCount[ruleIterR]; prodIterR++)
		{
			tmp_err_code = addProductionToAProtoRule(memList, left, left->rulesCount - 1, right->prods[ruleIterR][prodIterR].event, right->prods[ruleIterR][prodIterR].uriRowID, right->prods[ruleIterR][prodIterR].lnRowID, right->prods[ruleIterR][prodIterR].nonTermID + ((right->prods[ruleIterR][prodIterR].event.eventType == EVENT_EE)?0:(initialLeftRulesCount-1)));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	currRuleIndex = left->rulesCount;

	for(ruleIterL = 0; ruleIterL < initialLeftRulesCount; ruleIterL++)
	{
		for(prodIterL = 0; prodIterL < left->prodCount[ruleIterL]; prodIterL++)
		{
			if(left->prods[ruleIterL][prodIterL].event.eventType == EVENT_EE)
			{
				// Remove this production
				if(prodIterL == left->prodCount[ruleIterL] - 1)
					left->prodCount[ruleIterL] -= 1;
				else
				{
					memcpy(left->prods[ruleIterL] + prodIterL, left->prods[ruleIterL] + prodIterL + 1, left->prodCount[ruleIterL] - prodIterL - 1);
					left->prodCount[ruleIterL] -= 1;
				}
				tmp_err_code = addProductionsToARule(memList, left, ruleIterL, right->prods[0], right->prodCount[0], collisions, &collisionCount, &currRuleIndex, initialLeftRulesCount - 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
				break;
			}
		}
	}

	// Create the new grammar rules based on the collision information
	return resolveCollisionsInGrammar(memList, collisions, &collisionCount, left, &currRuleIndex);
}

static errorCode addProductionsToARule(AllocList* memList, ProtoGrammar* left, unsigned int ruleIndex, Production* rightRule,
		unsigned int rightProdCount, struct collisionInfo* collisions, unsigned int* collisionCount, unsigned int* currRuleIndex, unsigned int initialLeftRulesCount)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int prodIterL = 0;
	unsigned int prodIterR = 0;
	unsigned char terminalCollision = FALSE;
	unsigned char collisionFound = FALSE;
	unsigned int collisIter = 0;

	for(prodIterR = 0; prodIterR < rightProdCount; prodIterR++)
	{
		terminalCollision = FALSE;
		for(prodIterL = 0; prodIterL < left->prodCount[ruleIndex]; prodIterL++)
		{
			// If the terminal symbol is identical
			if(eventsEqual(left->prods[ruleIndex][prodIterL].event, rightRule[prodIterR].event) &&
					left->prods[ruleIndex][prodIterL].uriRowID == rightRule[prodIterR].uriRowID &&
					left->prods[ruleIndex][prodIterL].lnRowID == rightRule[prodIterR].lnRowID)
			{
				// Collision
				collisionFound = FALSE;
				if(left->prods[ruleIndex][prodIterL].event.eventType == EVENT_EE ||
						(left->prods[ruleIndex][prodIterL].nonTermID == rightRule[prodIterR].nonTermID + initialLeftRulesCount))
				{
					// If the NonTerminals are the same
					// discard the addition of this production as they are identical
					collisionFound = TRUE;
					terminalCollision = TRUE;
					break;
				}

				for(collisIter = 0; collisIter < *collisionCount; collisIter++)
				{
					if(collisions[collisIter].leftNonTerminal == left->prods[ruleIndex][prodIterL].nonTermID
							&& collisions[collisIter].rightNonTerminal == rightRule[prodIterR].nonTermID + initialLeftRulesCount)
					{
						// Already collided nonTerminals
						collisionFound = TRUE;
						left->prods[ruleIndex][prodIterL].nonTermID = collisions[collisIter].createdNonTerminal;
						break;
					}

				}
				if(collisionFound == FALSE)
				{
					if(*collisionCount == MAX_COLLISIONS_NUMBER - 1)
						return OUT_OF_BOUND_BUFFER;
					collisions[*collisionCount].leftNonTerminal = left->prods[ruleIndex][prodIterL].nonTermID;
					collisions[*collisionCount].rightNonTerminal = rightRule[prodIterR].nonTermID + initialLeftRulesCount;
					collisions[*collisionCount].createdNonTerminal = *currRuleIndex;
					left->prods[ruleIndex][prodIterL].nonTermID = *currRuleIndex;

					*collisionCount += 1;
					*currRuleIndex += 1;
				}

				terminalCollision = TRUE;
				break;
			}
		}
		if(terminalCollision == FALSE)
		{
			// just add the production
			tmp_err_code = addProductionToAProtoRule(memList, left, ruleIndex, rightRule[prodIterR].event, rightRule[prodIterR].uriRowID, rightRule[prodIterR].lnRowID, rightRule[prodIterR].nonTermID + ((rightRule[prodIterR].event.eventType == EVENT_EE)?0:initialLeftRulesCount));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	return ERR_OK;
}

static errorCode resolveCollisionsInGrammar(AllocList* memList, struct collisionInfo* collisions,
											unsigned int* collisionCount, ProtoGrammar* left, unsigned int* currRuleIndex)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int collisIter = 0;
	unsigned int prodIterL = 0;
	Production* tmpProduction;

	for(collisIter = 0; collisIter < *collisionCount; collisIter++)
	{
		tmp_err_code = addProtoRule(memList, left);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for(prodIterL = 0; prodIterL < left->prodCount[collisions[collisIter].leftNonTerminal]; prodIterL++)
		{
			tmpProduction = &(left->prods[collisions[collisIter].leftNonTerminal][prodIterL]);
			tmp_err_code = addProductionToAProtoRule(memList, left, left->rulesCount - 1, tmpProduction->event, tmpProduction->uriRowID, tmpProduction->lnRowID, tmpProduction->nonTermID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		tmp_err_code = addProductionsToARule(memList, left, left->rulesCount-1, left->prods[collisions[collisIter].rightNonTerminal], left->prodCount[collisions[collisIter].rightNonTerminal], collisions, collisionCount, currRuleIndex, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode createSimpleTypeGrammar(AllocList* memList, QName simpleType, ProtoGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIEvent event;

	tmp_err_code = createProtoGrammar(memList, 2, 3, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->contentIndex = 0;

	event.eventType = EVENT_CH;
	tmp_err_code = getEXIDataType(simpleType, &(event.valueType));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->prods[0][0].event = event;
	(*result)->prods[0][0].nonTermID = 1;
	(*result)->prods[0][0].uriRowID = UINT16_MAX;
	(*result)->prods[0][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[0] = 1;

	(*result)->prods[1][0].event = getEventDefType(EVENT_EE);
	(*result)->prods[1][0].nonTermID = GR_VOID_NON_TERMINAL;
	(*result)->prods[1][0].uriRowID = UINT16_MAX;
	(*result)->prods[1][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[1] = 1;

	(*result)->rulesCount = 2;

	return ERR_OK;
}

errorCode createSimpleEmptyTypeGrammar(AllocList* memList, ProtoGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createProtoGrammar(memList, 1, 3, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->contentIndex = 0;

	(*result)->prods[0][0].event = getEventDefType(EVENT_EE);
	(*result)->prods[0][0].nonTermID = GR_VOID_NON_TERMINAL;
	(*result)->prods[0][0].uriRowID = UINT16_MAX;
	(*result)->prods[0][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[0] = 1;

	(*result)->rulesCount = 1;

	return ERR_OK;
}

errorCode createComplexTypeGrammar(AllocList* memList, StringType* name, StringType* target_ns,
								   ProtoGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType* wildcardArray, unsigned int wildcardArraySize,
		                           ProtoGrammar* contentTypeGrammar,
		                           ProtoGrammar** result)
{
	//TODO: Implement the case when there are wildcards i.e. wildcardArray is not empty

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;

	if(attrUsesArraySize > 0)
	{
		tmp_err_code = createProtoGrammar(memList, 10, 10, result);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		(*result)->prods[0][0].event = getEventDefType(EVENT_EE);
		(*result)->prods[0][0].nonTermID = GR_VOID_NON_TERMINAL;
		(*result)->prods[0][0].uriRowID = UINT16_MAX;
		(*result)->prods[0][0].lnRowID = SIZE_MAX;
		(*result)->prodCount[0] = 1;

		(*result)->rulesCount = 1;

		for(i = 0; i < attrUsesArraySize; i++)
		{
			tmp_err_code = concatenateGrammars(memList, *result, &(attrUsesArray[i]));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		(*result)->contentIndex = (*result)->rulesCount - 1;

		tmp_err_code = concatenateGrammars(memList, *result, contentTypeGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		(*result) = contentTypeGrammar;
		(*result)->contentIndex = 0;
	}

	return ERR_OK;
}

errorCode createComplexEmptyTypeGrammar(AllocList* memList, StringType name, StringType target_ns,
									ProtoGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                            StringType* wildcardArray, unsigned int wildcardArraySize,
		                            ProtoGrammar** result)
{
	//TODO: Implement the case when there are wildcards i.e. wildcardArray is not empty
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;
	ProtoGrammar* emptyContent;

	tmp_err_code = createProtoGrammar(memList, 10, 10, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->prods[0][0].event = getEventDefType(EVENT_EE);
	(*result)->prods[0][0].nonTermID = GR_VOID_NON_TERMINAL;
	(*result)->prods[0][0].uriRowID = UINT16_MAX;
	(*result)->prods[0][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[0] = 1;

	(*result)->rulesCount = 1;

	for(i = 0; i < attrUsesArraySize; i++)
	{
		tmp_err_code = concatenateGrammars(memList, *result, &(attrUsesArray[i]));
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = createSimpleEmptyTypeGrammar(memList, &emptyContent);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = concatenateGrammars(memList, *result, emptyContent);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->contentIndex = (*result)->rulesCount - 1; // The content index of grammar TypeEmpty i  is the index of its last non-terminal symbol.

	return ERR_OK;
}

errorCode createComplexUrTypeGrammar(AllocList* memList, ProtoGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexUrEmptyTypeGrammar(AllocList* memList, ProtoGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createAttributeUseGrammar(AllocList* memList, unsigned char required, StringType* name, StringType* target_ns,
										  QName simpleType, QName scope, ProtoGrammar** result,  uint16_t uriRowID, size_t lnRowID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIEvent event1;

	tmp_err_code = createProtoGrammar(memList, 2, 4, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	(*result)->contentIndex = 0;

	event1.eventType = EVENT_AT_QNAME;
	tmp_err_code = getEXIDataType(simpleType, &event1.valueType);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->prods[0][0].event = event1;
	(*result)->prods[0][0].nonTermID = 1;
	(*result)->prods[0][0].uriRowID = uriRowID;
	(*result)->prods[0][0].lnRowID = lnRowID;
	(*result)->prodCount[0] = 1;

	if(!required)
	{
		(*result)->prods[0][1].event = getEventDefType(EVENT_EE);
		(*result)->prods[0][1].nonTermID = GR_VOID_NON_TERMINAL;
		(*result)->prods[0][1].uriRowID = UINT16_MAX;
		(*result)->prods[0][1].lnRowID = SIZE_MAX;
		(*result)->prodCount[0] = 2;
	}

	(*result)->prods[1][0].event = getEventDefType(EVENT_EE);
	(*result)->prods[1][0].nonTermID = GR_VOID_NON_TERMINAL;
	(*result)->prods[1][0].uriRowID = UINT16_MAX;
	(*result)->prods[1][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[1] = 1;

	(*result)->rulesCount = 2;

	return ERR_OK;
}

errorCode createParticleGrammar(AllocList* memList, unsigned int minOccurs, int32_t maxOccurs,
								ProtoGrammar* termGrammar, ProtoGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i = 0;

	tmp_err_code = createProtoGrammar(memList, minOccurs + 10, 5, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	(*result)->contentIndex = 0;

	(*result)->prods[0][0].event = getEventDefType(EVENT_EE);
	(*result)->prods[0][0].nonTermID = GR_VOID_NON_TERMINAL;
	(*result)->prods[0][0].uriRowID = UINT16_MAX;
	(*result)->prods[0][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[0] = 1;

	(*result)->rulesCount = 1;

	for(i = 0; i < minOccurs; i++)
	{
		tmp_err_code = concatenateGrammars(memList, *result, termGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	if(maxOccurs - minOccurs > 0 || maxOccurs < 0) // Only if maxOccurs is unbounded or maxOccurs > minOccurs
	{
		unsigned char prodEEFound = FALSE;
		for(i = 0; i < termGrammar->prodCount[0]; i++)
		{
			if(termGrammar->prods[0][i].event.eventType == EVENT_EE)
			{
				prodEEFound = TRUE;
				break;
			}
		}
		if(prodEEFound == FALSE) //	There is no production Gi,0 : EE so add one
		{
			tmp_err_code = addProductionToAProtoRule(memList, termGrammar, 0, getEventDefType(EVENT_EE), UINT16_MAX, SIZE_MAX, GR_VOID_NON_TERMINAL);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(maxOccurs >= 0) // {max occurs} is not unbounded
		{
			for(i = 0; i < maxOccurs - (int)minOccurs; i++)
			{
				tmp_err_code = concatenateGrammars(memList, *result, termGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		else // {max occurs} is unbounded
		{
			uint16_t j = 0;
			struct collisionInfo collisions[MAX_COLLISIONS_NUMBER];
			unsigned int collisionCount = 0;
			unsigned int currRuleIndex = termGrammar->rulesCount;

			// Excluding the first rule
			for(i = 1; i < termGrammar->rulesCount; i++)
			{
				for(j = 0; j < termGrammar->prodCount[i]; j++)
				{
					if(termGrammar->prods[i][j].event.eventType == EVENT_EE)
					{
						// Remove this production
						if(j == termGrammar->prodCount[i] - 1)
							termGrammar->prodCount[i] -= 1;
						else
						{
							memcpy(termGrammar->prods[i] + j, termGrammar->prods[i] + j + 1, termGrammar->prodCount[i] - j - 1);
							termGrammar->prodCount[i] -= 1;
						}
						tmp_err_code = addProductionsToARule(memList, termGrammar, i, termGrammar->prods[0], termGrammar->prodCount[0], collisions, &collisionCount, &currRuleIndex, 0);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
						break;
					}
				}
			}

			// Create the new grammar rules based on the collision information
			tmp_err_code = resolveCollisionsInGrammar(memList, collisions, &collisionCount, termGrammar, &currRuleIndex);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = concatenateGrammars(memList, *result, termGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

errorCode createElementTermGrammar(AllocList* memList, StringType* name, StringType* target_ns,
								   ProtoGrammar** result, uint16_t uriRowID, size_t lnRowID)
{
	//TODO: enable support for {substitution group affiliation} property of the elements

	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createProtoGrammar(memList, 2, 3, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	(*result)->contentIndex = 0;

	(*result)->prods[0][0].event =  getEventDefType(EVENT_SE_QNAME);
	(*result)->prods[0][0].nonTermID = 1;
	(*result)->prods[0][0].uriRowID = uriRowID;
	(*result)->prods[0][0].lnRowID = lnRowID;
	(*result)->prodCount[0] = 1;

	(*result)->prods[1][0].event = getEventDefType(EVENT_EE);
	(*result)->prods[1][0].nonTermID = GR_VOID_NON_TERMINAL;
	(*result)->prods[1][0].uriRowID = UINT16_MAX;
	(*result)->prods[1][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[1] = 1;

	(*result)->rulesCount = 2;

	return ERR_OK;
}

errorCode createWildcardTermGrammar(AllocList* memList, StringType* wildcardArray, unsigned int wildcardArraySize, ProtoGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createProtoGrammar(memList, 2, wildcardArraySize + 1, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	(*result)->contentIndex = 0;

	if(stringEqualToAscii(wildcardArray[0], "##any") || stringEqualToAscii(wildcardArray[0], "##other"))
	{
		(*result)->prods[0][0].event =  getEventDefType(EVENT_SE_ALL);
		(*result)->prods[0][0].nonTermID = 1;
		(*result)->prods[0][0].uriRowID = UINT16_MAX;
		(*result)->prods[0][0].lnRowID = SIZE_MAX;
		(*result)->prodCount[0] = 1;
	}
	else
	{
		return NOT_IMPLEMENTED_YET;
	}

	(*result)->prods[1][0].event = getEventDefType(EVENT_EE);
	(*result)->prods[1][0].nonTermID = GR_VOID_NON_TERMINAL;
	(*result)->prods[1][0].uriRowID = UINT16_MAX;
	(*result)->prods[1][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[1] = 1;

	return ERR_OK;
}

errorCode createSequenceModelGroupsGrammar(AllocList* memList, GenericStack* protoGrammars, ProtoGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(protoGrammars == NULL)
	{
		tmp_err_code = createSimpleEmptyTypeGrammar(memList, result);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		tmp_err_code = createProtoGrammar(memList, 10, 5, result);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		(*result)->contentIndex = 0;

		(*result)->prods[0][0].event = getEventDefType(EVENT_EE);
		(*result)->prods[0][0].nonTermID = GR_VOID_NON_TERMINAL;
		(*result)->prods[0][0].uriRowID = UINT16_MAX;
		(*result)->prods[0][0].lnRowID = SIZE_MAX;
		(*result)->prodCount[0] = 1;
		(*result)->rulesCount = 1;

		tmp_err_code = recursiveGrammarConcat(memList, protoGrammars, result);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

static errorCode recursiveGrammarConcat(AllocList* memList, GenericStack* protoGrammars, ProtoGrammar** result)
{
	ProtoGrammar* tmpGrammar;
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = popFromStack(&protoGrammars, (void**) &tmpGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(protoGrammars == NULL)
	{
		tmp_err_code = concatenateGrammars(memList, *result, tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		return ERR_OK;
	}
	else
	{
		tmp_err_code = recursiveGrammarConcat(memList, protoGrammars, result);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = concatenateGrammars(memList, *result, tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode createChoiceModelGroupsGrammar(AllocList* memList, GenericStack* protoGrammars, ProtoGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createProtoGrammar(memList, 10, 5, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->contentIndex = 0;

	(*result)->prods[0][0].event = getEventDefType(EVENT_EE);
	(*result)->prods[0][0].nonTermID = GR_VOID_NON_TERMINAL;
	(*result)->prods[0][0].uriRowID = UINT16_MAX;
	(*result)->prods[0][0].lnRowID = SIZE_MAX;
	(*result)->prodCount[0] = 1;
	(*result)->rulesCount = 1;

	if(protoGrammars != NULL)
	{
		unsigned int ruleIterTerm = 0;
		unsigned int prodIterTerm = 0;
		struct collisionInfo collisions[MAX_COLLISIONS_NUMBER];
		unsigned int collisionCount = 0;
		unsigned int currRuleIndex;
		unsigned int initialResultRulesCount;
		ProtoGrammar* tmpGrammar;

		tmp_err_code = popFromStack(&protoGrammars, (void**) &tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(tmpGrammar == NULL)
			return NULL_POINTER_REF;

		tmp_err_code = concatenateGrammars(memList, (*result), tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		while(protoGrammars != NULL)
		{
			tmp_err_code = popFromStack(&protoGrammars, (void**) &tmpGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(tmpGrammar == NULL)
				return NULL_POINTER_REF;

			initialResultRulesCount = (*result)->rulesCount;

			for(ruleIterTerm = 1; ruleIterTerm < tmpGrammar->rulesCount; ruleIterTerm++)
			{
				tmp_err_code = addProtoRule(memList, (*result));
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				for(prodIterTerm = 0; prodIterTerm < tmpGrammar->prodCount[ruleIterTerm]; prodIterTerm++)
				{
					tmp_err_code = addProductionToAProtoRule(memList, (*result), (*result)->rulesCount - 1, tmpGrammar->prods[ruleIterTerm][prodIterTerm].event, tmpGrammar->prods[ruleIterTerm][prodIterTerm].uriRowID, tmpGrammar->prods[ruleIterTerm][prodIterTerm].lnRowID, tmpGrammar->prods[ruleIterTerm][prodIterTerm].nonTermID + ((tmpGrammar->prods[ruleIterTerm][prodIterTerm].event.eventType == EVENT_EE)?0:(initialResultRulesCount-1)));
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}
			}

			currRuleIndex = (*result)->rulesCount;

			tmp_err_code = addProductionsToARule(memList, (*result), 0, tmpGrammar->prods[0], tmpGrammar->prodCount[0], collisions, &collisionCount, &currRuleIndex, initialResultRulesCount - 1);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			// Create the new grammar rules based on the collision information
			tmp_err_code = resolveCollisionsInGrammar(memList, collisions, &collisionCount, (*result), &currRuleIndex);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

errorCode createAllModelGroupsGrammar(AllocList* memList, ProtoGrammar* pTermArray, unsigned int pTermArraySize, ProtoGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode getEXIDataType(QName simpleXSDType, ValueType* exiType)
{
	if(stringEqualToAscii(*simpleXSDType.localName, "string") ||
	   stringEqualToAscii(*simpleXSDType.localName, "duration") ||
	   stringEqualToAscii(*simpleXSDType.localName, "anyURI") ||
	   stringEqualToAscii(*simpleXSDType.localName, "normalizedString") ||
	   stringEqualToAscii(*simpleXSDType.localName, "token") ||
	   stringEqualToAscii(*simpleXSDType.localName, "Name") ||
	   stringEqualToAscii(*simpleXSDType.localName, "NMTOKEN") ||
	   stringEqualToAscii(*simpleXSDType.localName, "NCName") ||
	   stringEqualToAscii(*simpleXSDType.localName, "ID") ||
	   stringEqualToAscii(*simpleXSDType.localName, "IDREF") ||
	   stringEqualToAscii(*simpleXSDType.localName, "ENTITY"))
	{
		*exiType = VALUE_TYPE_STRING;
		return ERR_OK;
	}
	else if(stringEqualToAscii(*simpleXSDType.localName, "boolean"))
	{
		*exiType = VALUE_TYPE_BOOLEAN;
		return ERR_OK;
	}
	else if(stringEqualToAscii(*simpleXSDType.localName, "integer") ||
			stringEqualToAscii(*simpleXSDType.localName, "nonPositiveInteger") ||
			stringEqualToAscii(*simpleXSDType.localName, "long") ||
			stringEqualToAscii(*simpleXSDType.localName, "int") ||
			stringEqualToAscii(*simpleXSDType.localName, "short") ||
			stringEqualToAscii(*simpleXSDType.localName, "byte") ||
			stringEqualToAscii(*simpleXSDType.localName, "negativeInteger"))
	{
		*exiType = VALUE_TYPE_INTEGER;
		return ERR_OK;
	}
	else if(stringEqualToAscii(*simpleXSDType.localName, "nonNegativeInteger") ||
			stringEqualToAscii(*simpleXSDType.localName, "positiveInteger") ||
			stringEqualToAscii(*simpleXSDType.localName, "unsignedLong") ||
			stringEqualToAscii(*simpleXSDType.localName, "unsignedInt") ||
			stringEqualToAscii(*simpleXSDType.localName, "unsignedShort") ||
			stringEqualToAscii(*simpleXSDType.localName, "unsignedByte"))
	{
		*exiType = VALUE_TYPE_NON_NEGATIVE_INT;
		return ERR_OK;
	}
	else if(stringEqualToAscii(*simpleXSDType.localName, "float") ||
				stringEqualToAscii(*simpleXSDType.localName, "double"))
	{
		*exiType = VALUE_TYPE_FLOAT;
		return ERR_OK;
	}
	else if(stringEqualToAscii(*simpleXSDType.localName, "decimal"))
	{
		*exiType = VALUE_TYPE_DECIMAL;
		return ERR_OK;
	}
	else if(stringEqualToAscii(*simpleXSDType.localName, "hexBinary") ||
				stringEqualToAscii(*simpleXSDType.localName, "base64Binary"))
	{
		*exiType = VALUE_TYPE_BINARY;
		return ERR_OK;
	}
	else if(stringEqualToAscii(*simpleXSDType.localName, "dateTime") ||
			stringEqualToAscii(*simpleXSDType.localName, "time") ||
			stringEqualToAscii(*simpleXSDType.localName, "date") ||
			stringEqualToAscii(*simpleXSDType.localName, "gYearMonth") ||
			stringEqualToAscii(*simpleXSDType.localName, "gYear") ||
			stringEqualToAscii(*simpleXSDType.localName, "gMonthDay") ||
			stringEqualToAscii(*simpleXSDType.localName, "gDay") ||
			stringEqualToAscii(*simpleXSDType.localName, "gMonth"))
	{
		*exiType = VALUE_TYPE_DATE_TIME;
		return ERR_OK;
	}
	else if(stringEqualToAscii(*simpleXSDType.localName, "NMTOKENS") ||
			stringEqualToAscii(*simpleXSDType.localName, "IDREFS") ||
			stringEqualToAscii(*simpleXSDType.localName, "ENTITIES"))
	{
		*exiType = VALUE_TYPE_LIST;
		return ERR_OK;
	}

	return INCONSISTENT_PROC_STATE;
}

int qnamesCompare(const StringType* uri1, const StringType* ln1, const StringType* uri2, const StringType* ln2)
{
	int uri_cmp_res = stringCompare(*uri1, *uri2);
	if(uri_cmp_res == 0) // equal URIs
	{
		return stringCompare(*ln1, *ln2);
	}
	return uri_cmp_res;
}

errorCode assignCodes(ProtoGrammar* grammar)
{
	uint16_t i = 0;

	for (i = 0; i < grammar->rulesCount; i++)
	{
		qsort(grammar->prods[i], grammar->prodCount[i], sizeof(Production), compareProductions);
	}
	return ERR_OK;
}

static int compareProductions(const void* prod1, const void* prod2)
{
	Production* p1 = (Production*) prod1;
	Production* p2 = (Production*) prod2;

	if(p1->event.eventType < p2->event.eventType)
		return 1;
	else if(p1->event.eventType > p2->event.eventType)
		return -1;
	else // the same event Type
	{
		if(p1->event.eventType == EVENT_AT_QNAME)
		{
			if(p1->lnRowID < p2->lnRowID)
				return 1;
			else if(p1->lnRowID > p2->lnRowID)
				return -1;
			else
			{
				if(p1->uriRowID < p2->uriRowID)
					return 1;
				else if(p1->uriRowID > p2->uriRowID)
					return -1;
				else
					return 0;
			}
		}
		else if(p1->event.eventType == EVENT_AT_URI)
		{
			if(p1->uriRowID < p2->uriRowID)
				return 1;
			else if(p1->uriRowID > p2->uriRowID)
				return -1;
			else
				return 0;
		}
		else if(p1->event.eventType == EVENT_SE_QNAME)
		{
			// TODO: figure out how it should be done
		}
		else if(p1->event.eventType == EVENT_SE_URI)
		{
			// TODO: figure out how it should be done
		}
		return 0;
	}
}

