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

static URITable* comparison_ptr = NULL;

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
			if(eventsIdentical(left->prods[ruleIndex][prodIterL].event, rightRule[prodIterR].event) &&
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

errorCode createSimpleTypeGrammar(AllocList* tmpMemList, ValueType vType, ProtoGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createProtoGrammar(tmpMemList, 2, 3, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->contentIndex = 0;

	(*result)->prods[0][0].event.eventType = EVENT_CH;
	(*result)->prods[0][0].event.valueType = vType;
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

errorCode createComplexTypeGrammar(AllocList* memList, String* name, String* target_ns,
								   ProtoGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           String* wildcardArray, unsigned int wildcardArraySize,
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

errorCode createComplexEmptyTypeGrammar(AllocList* memList, String name, String target_ns,
									ProtoGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                            String* wildcardArray, unsigned int wildcardArraySize,
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

errorCode createAttributeUseGrammar(AllocList* tmpMemList, unsigned char required, String* name, String* target_ns,
										  QNameID simpleTypeID, QName scope, ProtoGrammar** result,  uint16_t uriRowID, size_t lnRowID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIEvent event1;

	tmp_err_code = createProtoGrammar(tmpMemList, 2, 4, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	(*result)->contentIndex = 0;

	event1.eventType = EVENT_AT_QNAME;
	tmp_err_code = getEXIDataTypeFromSimpleType(simpleTypeID, &event1.valueType);
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

errorCode createElementTermGrammar(AllocList* memList, String* name, String* target_ns,
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

errorCode createWildcardTermGrammar(AllocList* memList, String* wildcardArray, unsigned int wildcardArraySize, ProtoGrammar** result)
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

	(*result)->rulesCount = 2;

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

	popFromStack(&protoGrammars, (void**) &tmpGrammar);

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

		popFromStack(&protoGrammars, (void**) &tmpGrammar);

		if(tmpGrammar == NULL)
			return NULL_POINTER_REF;

		tmp_err_code = concatenateGrammars(memList, (*result), tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		while(protoGrammars != NULL)
		{
			popFromStack(&protoGrammars, (void**) &tmpGrammar);

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

errorCode getEXIDataTypeFromSimpleType(QNameID simpleXSDType, ValueType* vType)
{
	if(simpleXSDType.uriRowId != 3) // != http://www.w3.org/2001/XMLSchema
		return UNEXPECTED_ERROR;

	switch(simpleXSDType.lnRowId)
	{
		case 0: // "ENTITIES"
			vType->exiType = VALUE_TYPE_LIST;
			vType->simpleTypeID = SIMPLE_TYPE_ENTITIES;
		break;
		case 1: // "ENTITY"
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_ENTITY;
		break;
		case 2: // ID
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_ID;
		break;
		case 3: // IDREF
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_IDREF;
		break;
		case 4: // IDREFS
			vType->exiType = VALUE_TYPE_LIST;
			vType->simpleTypeID = SIMPLE_TYPE_IDREFS;
		break;
		case 5: // NCName
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_NCNAME;
		break;
		case 6: // NMTOKEN
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_NMTOKEN;
		break;
		case 7: // NMTOKENS
			vType->exiType = VALUE_TYPE_LIST;
			vType->simpleTypeID = SIMPLE_TYPE_NMTOKENS;
		break;
		case 8: // NOTATION
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_NOTATION;
		break;
		case 9: // Name
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_NAME;
		break;
		case 10: // QName
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_QNAME;
		break;
		case 11: // anySimpleType
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_ANY_SIMPLE_TYPE;
		break;
		case 12: // anyType
			vType->simpleTypeID = SIMPLE_TYPE_ANY_TYPE;
			// TODO: This is not a simple type!
			// It must be handled with creating a Complex Ur-Type Grammar: http://www.w3.org/TR/2011/REC-exi-20110310/#anyTypeGrammar
		break;
		case 13: // anyURI
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_ANY_URI;
		break;
		case 14: // base64Binary
			vType->exiType = VALUE_TYPE_BINARY;
			vType->simpleTypeID = SIMPLE_TYPE_BASE64_BINARY;
		break;
		case 15: // boolean
			vType->exiType = VALUE_TYPE_BOOLEAN;
			vType->simpleTypeID = SIMPLE_TYPE_BOOLEAN;
		break;
		case 16: // byte
			vType->exiType = VALUE_TYPE_SMALL_INTEGER;
			vType->simpleTypeID = SIMPLE_TYPE_BYTE;
		break;
		case 17: // date
			vType->exiType = VALUE_TYPE_DATE_TIME;
			vType->simpleTypeID = SIMPLE_TYPE_DATE;
		break;
		case 18: // dateTime
			vType->exiType = VALUE_TYPE_DATE_TIME;
			vType->simpleTypeID = SIMPLE_TYPE_DATE_TIME;
		break;
		case 19: // decimal
			vType->exiType = VALUE_TYPE_DECIMAL;
			vType->simpleTypeID = SIMPLE_TYPE_DECIMAL;
		break;
		case 20: // double
			vType->exiType = VALUE_TYPE_FLOAT;
			vType->simpleTypeID = SIMPLE_TYPE_DOUBLE;
		break;
		case 21: // duration
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_DURATION;
		break;
		case 22: // float
			vType->exiType = VALUE_TYPE_FLOAT;
			vType->simpleTypeID = SIMPLE_TYPE_FLOAT;
		break;
		case 23: // gDay
			vType->exiType = VALUE_TYPE_DATE_TIME;
			vType->simpleTypeID = SIMPLE_TYPE_GDAY;
		break;
		case 24: // gMonth
			vType->exiType = VALUE_TYPE_DATE_TIME;
			vType->simpleTypeID = SIMPLE_TYPE_GMONTH;
		break;
		case 25: // gMonthDay
			vType->exiType = VALUE_TYPE_DATE_TIME;
			vType->simpleTypeID = SIMPLE_TYPE_GMONTH_DAY;
		break;
		case 26: // gYear
			vType->exiType = VALUE_TYPE_DATE_TIME;
			vType->simpleTypeID = SIMPLE_TYPE_GYEAR;
		break;
		case 27: // gYearMonth
			vType->exiType = VALUE_TYPE_DATE_TIME;
			vType->simpleTypeID = SIMPLE_TYPE_GYEAR_MONTH;
		break;
		case 28: // hexBinary
			vType->exiType = VALUE_TYPE_BINARY;
			vType->simpleTypeID = SIMPLE_TYPE_HEX_BINARY;
		break;
		case 29: // int
			vType->exiType = VALUE_TYPE_INTEGER;
			vType->simpleTypeID = SIMPLE_TYPE_INT;
		break;
		case 30: // integer
			vType->exiType = VALUE_TYPE_INTEGER;
			vType->simpleTypeID = SIMPLE_TYPE_INTEGER;
		break;
		case 31: // language
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_LANGUAGE;
		break;
		case 32: // long
			vType->exiType = VALUE_TYPE_INTEGER;
			vType->simpleTypeID = SIMPLE_TYPE_LONG;
		break;
		case 33: // negativeInteger
			vType->exiType = VALUE_TYPE_INTEGER;
			vType->simpleTypeID = SIMPLE_TYPE_NEGATIVE_INTEGER;
		break;
		case 34: // nonNegativeInteger
			vType->exiType = VALUE_TYPE_NON_NEGATIVE_INT;
			vType->simpleTypeID = SIMPLE_TYPE_NON_NEGATIVE_INTEGER;
		break;
		case 35: // nonPositiveInteger
			vType->exiType = VALUE_TYPE_INTEGER;
			vType->simpleTypeID = SIMPLE_TYPE_NON_POSITIVE_INTEGER;
		break;
		case 36: // normalizedString
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_NORMALIZED_STRING;
		break;
		case 37: // positiveInteger
			vType->exiType = VALUE_TYPE_NON_NEGATIVE_INT;
			vType->simpleTypeID = SIMPLE_TYPE_POSITIVE_INTEGER;
		break;
		case 38: // short
			vType->exiType = VALUE_TYPE_INTEGER;
			vType->simpleTypeID = SIMPLE_TYPE_SHORT;
		break;
		case 39: // string
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_STRING;
		break;
		case 40: // time
			vType->exiType = VALUE_TYPE_DATE_TIME;
			vType->simpleTypeID = SIMPLE_TYPE_TIME;
		break;
		case 41: // token
			vType->exiType = VALUE_TYPE_STRING;
			vType->simpleTypeID = SIMPLE_TYPE_TOKEN;
		break;
		case 42: // unsignedByte
			vType->exiType = VALUE_TYPE_SMALL_INTEGER;
			vType->simpleTypeID = SIMPLE_TYPE_UNSIGNED_BYTE;
		break;
		case 43: // unsignedInt
			vType->exiType = VALUE_TYPE_NON_NEGATIVE_INT;
			vType->simpleTypeID = SIMPLE_TYPE_UNSIGNED_INT;
		break;
		case 44: // unsignedLong
			vType->exiType = VALUE_TYPE_NON_NEGATIVE_INT;
			vType->simpleTypeID = SIMPLE_TYPE_UNSIGNED_LONG;
		break;
		case 45: // unsignedShort
			vType->exiType = VALUE_TYPE_NON_NEGATIVE_INT;
			vType->simpleTypeID = SIMPLE_TYPE_UNSIGNED_SHORT;
		break;
	}

	return ERR_OK;
}

int qnamesCompare(const String* uri1, const String* ln1, const String* uri2, const String* ln2)
{
	int uri_cmp_res = stringCompare(*uri1, *uri2);
	if(uri_cmp_res == 0) // equal URIs
	{
		return stringCompare(*ln1, *ln2);
	}
	return uri_cmp_res;
}

errorCode assignCodes(ProtoGrammar* grammar, URITable* metaSTable)
{
	uint16_t i = 0;

	comparison_ptr = metaSTable;
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
			int i = 0;
			i = stringCompare(comparison_ptr->rows[p1->uriRowID].lTable->rows[p1->lnRowID].string_val, comparison_ptr->rows[p2->uriRowID].lTable->rows[p2->lnRowID].string_val);
			if(i == 0)
			{
				return -stringCompare(comparison_ptr->rows[p1->uriRowID].string_val, comparison_ptr->rows[p2->uriRowID].string_val);
			}
			else
				return -i;
		}
		else if(p1->event.eventType == EVENT_AT_URI)
		{
			return -stringCompare(comparison_ptr->rows[p1->uriRowID].string_val, comparison_ptr->rows[p2->uriRowID].string_val);
		}
		else if(p1->event.eventType == EVENT_SE_QNAME)
		{
			// Using the fact that the uri tables are not sorted yet.
			// when lnID_1 < lnID_2 then lnID_1 came first in the schema
			// NOTE however that this only works for single namespace (uriID) i.e. the target namespace
			// TODO: figure out how it should be done so it works between elements from
			//       different namespace

			if(p1->lnRowID < p2->lnRowID)
				return 1;
			else
				return -1;
		}
		else if(p1->event.eventType == EVENT_SE_URI)
		{
			// TODO: figure out how it should be done
		}
		return 0;
	}
}

errorCode createBuildInTypesDefinitions(DynArray* sTypeArr, AllocList* memList)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SimpleType sType;
	size_t elID;

	// TODO: complete the description of the types with all the facets needed

	// String
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// normalizedString
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// token
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// nmtoken
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// nmtokens
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// name
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// language
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// ncname
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// idref
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// idrefs
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// entity
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// entities
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// id
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// decimal
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// integer
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// NonPositiveInteger
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// negativeInteger
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.maxInclusive = -1;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// long
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Int
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// short
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 32767;
	sType.minInclusive = -32768;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// byte
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 127;
	sType.minInclusive = -128;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// NonNegativeInteger
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned Long
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned int
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned short
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 65535;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned byte
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.maxInclusive = 255;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Positive Integer
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 1;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// boolean
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// base64 binary
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// hex binary
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// float
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// double
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any uri
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// qname
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// notation
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// duration
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// date time
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// time
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// date
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gYearMonth
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gYear
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gMonthDay
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gDay
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gMonth
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any simple type
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any type
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynElement(sTypeArr, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}
