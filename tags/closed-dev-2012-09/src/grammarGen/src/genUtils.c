/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file genUtils.c
 * @brief Implementation of utility functions for generating Schema-informed Grammar definitions
 * @date Nov 23, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "genUtils.h"
#include "memManagement.h"
#include "grammarRules.h"
#include "eventsEXI.h"
#include "stringManipulate.h"
#include "grammars.h"
#include "ioUtil.h"
#include "initSchemaInstance.h"

#define MAX_COLLISIONS_NUMBER 50

struct collisionInfo
{
	SmallIndex leftNonTerminal;
	SmallIndex rightNonTerminal;
	SmallIndex createdNonTerminal;
};

/** Collision aware addition */
static errorCode addProductionsToARule(AllocList* memList, ProtoRuleEntry* leftRule, ProtoRuleEntry* rightRule,
									   struct collisionInfo* collisions, unsigned int* collisionCount, unsigned int* currRuleIndex, unsigned int initialLeftRulesCount);

// Creates the new grammar rules based on the collision information
static errorCode resolveCollisionsInGrammar(AllocList* memList, struct collisionInfo* collisions,
											unsigned int* collisionCount, ProtoGrammar* left, unsigned int* currRuleIndex);

static errorCode recursiveGrammarConcat(AllocList* memList, GenericStack** protoGrammars, ProtoGrammar* seqGrammar);

/** Descending order comparison.
 * The productions are ordered with the largest event code first. */
static int compareProductions(const void* prod1, const void* prod2);

static errorCode addEEProduction(AllocList* memList, ProtoRuleEntry* rule);

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
	unsigned int initialLeftRulesCount = left->count;
	ProtoRuleEntry* ruleEntry;
	Index ruleId;

	if(left == NULL)
		return NULL_POINTER_REF;
	else if(right == NULL)
		return ERR_OK;

	/* 
	 * Concatentation works as follows:
	 * Add each rule apart from the first one on the RHS to the LHS.
	 * Example:
	 * LHS has two rules already:
	 *     left->rule[2]
	 * RHS has four rules to concatentate:
	 *     right->rule[4]
	 * Append all RHS rule productions excluding the first rule directly to LHS
	 *     left->rule[2] = right->rule[1]
	 *     left->rule[3] = right->rule[2]
	 *     left->rule[4] = right->rule[3]
	 * Merge all LHS rule productions with the first RHS rule productions ("+=" means merge)
	 *     left->rule[0] += right->rule[0]
	 *     left->rule[1] += right->rule[0]
	 * Merging occurs after the EE production; this is replaced with the first production
	 * to be merged
	 * Resolve any collisions
	 */

	/* Make a note of how many rules are currently in LHS for the new non terminal IDs */
	initialLeftRulesCount = left->count;

	/* Add in rules from the RHS */
	for(ruleIterR = 1; ruleIterR < right->count; ruleIterR++)
	{
		/* Create new rule entry in LHS proto grammar */
		tmp_err_code = addEmptyDynEntry(&left->dynArray, (void**)&ruleEntry, &ruleId, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* Create production array for the new rule entry, size of number of RHS productions to copy over */
		tmp_err_code = createDynArray(&ruleEntry->dynArray, sizeof(Production), right->rule[ruleIterR].count, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* Copy the RHS productions into the new rule entry, adjusting the non terminal ID */
		for(prodIterR = 0; prodIterR < right->rule[ruleIterR].count; prodIterR++)
		{
			tmp_err_code = addProduction(memList,
				                         ruleEntry,
										 right->rule[ruleIterR].prod[prodIterR].eventType,
										 right->rule[ruleIterR].prod[prodIterR].typeId,
										 right->rule[ruleIterR].prod[prodIterR].qnameId,
										 right->rule[ruleIterR].prod[prodIterR].nonTermID + ((right->rule[ruleIterR].prod[prodIterR].eventType == EVENT_EE)?0:(initialLeftRulesCount-1)));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	currRuleIndex = left->count;

	for(ruleIterL = 0; ruleIterL < initialLeftRulesCount; ruleIterL++)
	{
		for(prodIterL = 0; prodIterL < left->rule[ruleIterL].count; prodIterL++)
		{
			if(left->rule[ruleIterL].prod[prodIterL].eventType == EVENT_EE)
			{
				/* Remove the EE production */
				delDynEntry(&left->rule[ruleIterL].dynArray, prodIterL);
	
				/* Merge productions from RHS rule 0 into each left rule */
				tmp_err_code = addProductionsToARule(memList,
													 &left->rule[ruleIterL],
													 &right->rule[0],
													 collisions,
													 &collisionCount,
													 &currRuleIndex,
													 initialLeftRulesCount - 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
				break;
			}
		}
	}

	// Create the new grammar rules based on the collision information
	return resolveCollisionsInGrammar(memList, collisions, &collisionCount, left, &currRuleIndex);
}

static errorCode addProductionsToARule(AllocList* memList, ProtoRuleEntry* leftRule, ProtoRuleEntry* rightRule, 
									   struct collisionInfo* collisions, unsigned int* collisionCount, unsigned int* currRuleIndex, unsigned int initialLeftRulesCount)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int prodIterL = 0;
	unsigned int prodIterR = 0;
	unsigned char terminalCollision = FALSE;
	unsigned char collisionFound = FALSE;
	unsigned int collisIter = 0;

	for(prodIterR = 0; prodIterR < rightRule->count; prodIterR++)
	{
		/* Check for terminal collisions with existing production. These must be merged */
		terminalCollision = FALSE;
		for(prodIterL = 0; prodIterL < leftRule->count; prodIterL++)
		{
			/* Check for the same terminal symbol e.g. SE(qname) */
			if(leftRule->prod[prodIterL].eventType == rightRule->prod[prodIterR].eventType &&
					leftRule->prod[prodIterL].typeId == rightRule->prod[prodIterR].typeId &&
					leftRule->prod[prodIterL].qnameId.uriId == rightRule->prod[prodIterR].qnameId.uriId &&
					leftRule->prod[prodIterL].qnameId.lnId == rightRule->prod[prodIterR].qnameId.lnId)
			{
				/* Now check the non-terminal ID (noting that EE's don't have a non-terminal ID) */
				collisionFound = FALSE;
				if((leftRule->prod[prodIterL].eventType == EVENT_EE) ||
				   (leftRule->prod[prodIterL].nonTermID == rightRule->prod[prodIterR].nonTermID + initialLeftRulesCount))
				{
					/*
					 * If the NonTerminals are the same as well, no need to add
					 * the production as it's already there
					 */
					collisionFound = TRUE;
					terminalCollision = TRUE;
					/* Check the next production in LHS... */
					break;
				}

				for(collisIter = 0; collisIter < *collisionCount; collisIter++)
				{
					if((collisions[collisIter].leftNonTerminal == leftRule->prod[prodIterL].nonTermID) && 
					   (collisions[collisIter].rightNonTerminal == rightRule->prod[prodIterR].nonTermID + initialLeftRulesCount))
					{
						/* Already collided nonTerminals. Modify the existing LHS non-terminal ID */
						collisionFound = TRUE;
						leftRule->prod[prodIterL].nonTermID = collisions[collisIter].createdNonTerminal;
						break;
					}

				}

				if(collisionFound == FALSE)
				{
					/* We have the same terminal but we haven't resolved the non-terminal ID yet */
					if(*collisionCount == MAX_COLLISIONS_NUMBER - 1)
						return OUT_OF_BOUND_BUFFER;

					/* Store the LHS and RHS non-terminal IDs and the current non-terminal ID for later checking */
					collisions[*collisionCount].leftNonTerminal = leftRule->prod[prodIterL].nonTermID;
					collisions[*collisionCount].rightNonTerminal = rightRule->prod[prodIterR].nonTermID + initialLeftRulesCount;
					collisions[*collisionCount].createdNonTerminal = *currRuleIndex;

					/* Modify the existing LHS non-terminal ID */
					leftRule->prod[prodIterL].nonTermID = *currRuleIndex;

					/* Increment collision array index and non-terminal ID */
					*collisionCount += 1;
					*currRuleIndex += 1;
				}

				terminalCollision = TRUE;
				break;
			}
		}

		if(terminalCollision == FALSE)
		{
			/*
			 * We have been through all LHS productions and there were no clashes
			 * so just add the production
			 */
			tmp_err_code = addProduction(memList,
										 leftRule,
										 rightRule->prod[prodIterR].eventType,
										 rightRule->prod[prodIterR].typeId,
										 rightRule->prod[prodIterR].qnameId,
										 rightRule->prod[prodIterR].nonTermID + ((rightRule->prod[prodIterR].eventType == EVENT_EE)?0:initialLeftRulesCount));
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
	ProtoRuleEntry* ruleEntry;
	Index ruleId;

	for(collisIter = 0; collisIter < *collisionCount; collisIter++)
	{
		tmp_err_code = addEmptyDynEntry(&left->dynArray, (void**)&ruleEntry, &ruleId, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = createDynArray(&ruleEntry->dynArray, sizeof(Production), left->rule[collisions[collisIter].leftNonTerminal].count, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for(prodIterL = 0; prodIterL < left->rule[collisions[collisIter].leftNonTerminal].count; prodIterL++)
		{
			tmpProduction = &left->rule[collisions[collisIter].leftNonTerminal].prod[prodIterL];
			tmp_err_code = addProduction(memList,
										 ruleEntry,
										 tmpProduction->eventType,
										 tmpProduction->typeId,
										 tmpProduction->qnameId,
										 tmpProduction->nonTermID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		tmp_err_code = addProductionsToARule(memList,
											 ruleEntry,
											 &left->rule[collisions[collisIter].rightNonTerminal],
											 collisions,
											 collisionCount,
											 currRuleIndex,
											 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode createSimpleTypeGrammar(AllocList* memList, Index typeId, ProtoGrammar* simpleGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QNameID qnameID = {URI_MAX, LN_MAX};

	tmp_err_code = createProtoGrammar(memList, 2, 3, simpleGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProduction(memList, &simpleGrammar->rule[0], EVENT_CH, typeId, qnameID, 1);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(memList, &simpleGrammar->rule[1]);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Set the rule array size
	simpleGrammar->count = 2;
	return ERR_OK;
}

errorCode createComplexTypeGrammar(AllocList* memList, ProtoGrammarArray* attrUseArray,
		                           String* wildcardArray, unsigned int wildcardArraySize,
		                           ProtoGrammar* contentTypeGrammar,
		                           ProtoGrammar* complexGrammar)
{
	//TODO: Implement the case when there are wildcards i.e. wildcardArray is not empty
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;

	if(attrUseArray->count > 0)
	{
		tmp_err_code = createProtoGrammar(memList, 10, 10, complexGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addEEProduction(memList, &complexGrammar->rule[0]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		// Set the rule array size
		complexGrammar->count = 1;

		for(i = 0; i < attrUseArray->count; i++)
		{
			tmp_err_code = concatenateGrammars(memList, complexGrammar, &attrUseArray->pg[i]);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		complexGrammar->contentIndex = complexGrammar->count - 1;

		if(contentTypeGrammar != NULL)
		{
			/* Concatentate in any existing passed-in grammar */
			tmp_err_code = concatenateGrammars(memList, complexGrammar, contentTypeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else
	{
		if(contentTypeGrammar != NULL)
		{
			tmp_err_code = cloneProtoGrammar(memList, contentTypeGrammar, complexGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		complexGrammar->contentIndex = 0;
	}

	return ERR_OK;
}

errorCode createComplexUrTypeGrammar(AllocList* memList, ProtoGrammar* result)
{
	return NOT_IMPLEMENTED_YET;
}

// TODO: do we really need the QName scope parameter?
errorCode createAttributeUseGrammar(AllocList* memList, unsigned char required, Index typeId, QName scope,
									ProtoGrammar* attrGrammar, QNameID qnameID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createProtoGrammar(memList, 2, 4, attrGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProduction(memList, &attrGrammar->rule[0], EVENT_AT_QNAME, typeId, qnameID, 1);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(!required)
	{
		tmp_err_code = addEEProduction(memList, &attrGrammar->rule[0]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = addEEProduction(memList, &attrGrammar->rule[1]);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	attrGrammar->count = 2;
	return ERR_OK;
}

errorCode createParticleGrammar(AllocList* memList, int minOccurs, int maxOccurs,
								ProtoGrammar* termGrammar, ProtoGrammar* particleGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	int i;

	tmp_err_code = createProtoGrammar(memList, minOccurs + 10, 5, particleGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(memList, &particleGrammar->rule[0]);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	particleGrammar->count = 1;

	for(i = 0; i < minOccurs; i++)
	{
		tmp_err_code = concatenateGrammars(memList, particleGrammar, termGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	if(maxOccurs - minOccurs > 0 || maxOccurs < 0) // Only if maxOccurs is unbounded or maxOccurs > minOccurs
	{
		unsigned char prodEEFound = FALSE;
		for(i = 0; i < (int)termGrammar->rule[0].count; i++)
		{
			if(termGrammar->rule[0].prod[i].eventType == EVENT_EE)
			{
				prodEEFound = TRUE;
				break;
			}
		}
		if(prodEEFound == FALSE) //	There is no production Gi,0 : EE so add one
		{
			tmp_err_code = addEEProduction(memList, &termGrammar->rule[0]);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(maxOccurs >= 0) // {max occurs} is not unbounded
		{
			for(i = 0; i < maxOccurs - minOccurs; i++)
			{
				tmp_err_code = concatenateGrammars(memList, particleGrammar, termGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		else // {max occurs} is unbounded
		{
			Index j = 0;
			struct collisionInfo collisions[MAX_COLLISIONS_NUMBER];
			unsigned int collisionCount = 0;
			unsigned int currRuleIndex = termGrammar->count;

			// Excluding the first rule
			for(i = 1; i < (int)termGrammar->count; i++)
			{
				for(j = 0; j < termGrammar->rule[i].count; j++)
				{
					if(termGrammar->rule[i].prod[j].eventType == EVENT_EE)
					{
						// Remove this production
						delDynEntry(&termGrammar->rule[i].dynArray, j);

						tmp_err_code = addProductionsToARule(memList, &termGrammar->rule[i], &termGrammar->rule[0], collisions, &collisionCount, &currRuleIndex, 0);
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

			tmp_err_code = concatenateGrammars(memList, particleGrammar, termGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

errorCode createElementTermGrammar(AllocList* memList, ProtoGrammar* elemGrammar, QNameID qnameID, Index grIndex)
{
	//TODO: enable support for {substitution group affiliation} property of the elements

	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createProtoGrammar(memList, 2, 3, elemGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProduction(memList, &elemGrammar->rule[0], EVENT_SE_QNAME, grIndex, qnameID, 1);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(memList, &elemGrammar->rule[1]);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	elemGrammar->count = 2;
	return ERR_OK;
}

errorCode createWildcardTermGrammar(AllocList* memList, String* wildcardArray, Index wildcardArraySize, ProtoGrammar* wildcardGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createProtoGrammar(memList, 2, wildcardArraySize + 1, wildcardGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(stringEqualToAscii(wildcardArray[0], "##any") || stringEqualToAscii(wildcardArray[0], "##other"))
	{
		QNameID qnameID = {URI_MAX, LN_MAX};
		tmp_err_code = addProduction(memList, &wildcardGrammar->rule[0], EVENT_SE_ALL, INDEX_MAX, qnameID, 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		return NOT_IMPLEMENTED_YET;
	}

	tmp_err_code = addEEProduction(memList, &wildcardGrammar->rule[1]);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	wildcardGrammar->count = 2;
	return ERR_OK;
}

errorCode createSequenceModelGroupsGrammar(AllocList* memList, GenericStack** protoGrammars, ProtoGrammar* seqGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	//TODO: use array for the protoGrammars and not a stack - it is easier and clearer

	if(*protoGrammars == NULL)
	{
		tmp_err_code = createProtoGrammar(memList, 1, 3, seqGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addEEProduction(memList, &seqGrammar->rule[0]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		// Set the rule array size
		seqGrammar->count = 1;
	}
	else
	{
		tmp_err_code = createProtoGrammar(memList, 10, 5, seqGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addEEProduction(memList, &seqGrammar->rule[0]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		seqGrammar->count = 1;

		tmp_err_code = recursiveGrammarConcat(memList, protoGrammars, seqGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

static errorCode recursiveGrammarConcat(AllocList* memList, GenericStack** protoGrammars, ProtoGrammar* seqGrammar)
{
	ProtoGrammar* tmpGrammar;
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	popFromStack(protoGrammars, (void**) &tmpGrammar);

	if(*protoGrammars == NULL)
	{
		return concatenateGrammars(memList, seqGrammar, tmpGrammar);
	}
	else
	{
		tmp_err_code = recursiveGrammarConcat(memList, protoGrammars, seqGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = concatenateGrammars(memList, seqGrammar, tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode createChoiceModelGroupsGrammar(AllocList* memList, ProtoGrammarArray* pgArray, ProtoGrammar* modGrpGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Index i;
	unsigned int ruleIterTerm = 0;
	unsigned int prodIterTerm = 0;
	struct collisionInfo collisions[MAX_COLLISIONS_NUMBER];
	unsigned int collisionCount = 0;
	unsigned int currRuleIndex;
	unsigned int initialResultRulesCount;
	ProtoGrammar* tmpGrammar;
	ProtoRuleEntry* ruleEntry;
	Index ruleId;

	tmp_err_code = createProtoGrammar(memList, 10, 5, modGrpGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(memList, &modGrpGrammar->rule[0]);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	modGrpGrammar->count = 1;

	tmpGrammar = &pgArray->pg[0];
	if(tmpGrammar == NULL)
		return NULL_POINTER_REF;

	tmp_err_code = concatenateGrammars(memList, modGrpGrammar, tmpGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 1; i < pgArray->count; i++)
	{

		tmpGrammar = &pgArray->pg[i];

		if(tmpGrammar == NULL)
			return NULL_POINTER_REF;

		initialResultRulesCount = modGrpGrammar->count;

		for(ruleIterTerm = 1; ruleIterTerm < tmpGrammar->count; ruleIterTerm++)
		{
			tmp_err_code = addEmptyDynEntry(&modGrpGrammar->dynArray, (void**)&ruleEntry, &ruleId, memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = createDynArray(&ruleEntry->dynArray, sizeof(Production), tmpGrammar->rule[ruleIterTerm].count, memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			for(prodIterTerm = 0; prodIterTerm < tmpGrammar->rule[ruleIterTerm].count; prodIterTerm++)
			{
				tmp_err_code = addProduction(memList,
											 ruleEntry,
											 tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].eventType,
											 tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].typeId,
											 tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].qnameId,
											 tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].nonTermID + ((tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].eventType == EVENT_EE)?0:(initialResultRulesCount-1)));
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}

		currRuleIndex = modGrpGrammar->count;

		tmp_err_code = addProductionsToARule(memList,
											 &modGrpGrammar->rule[0],
											 &tmpGrammar->rule[0],
											 collisions,
											 &collisionCount,
											 &currRuleIndex,
											 initialResultRulesCount - 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		// Create the new grammar rules based on the collision information
		tmp_err_code = resolveCollisionsInGrammar(memList, collisions, &collisionCount, modGrpGrammar, &currRuleIndex);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode createAllModelGroupsGrammar(AllocList* memList, ProtoGrammar* pTermArray, unsigned int pTermArraySize, ProtoGrammar* modGrpGrammar)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode assignCodes(ProtoGrammar* grammar)
{
	Index i = 0;

	for (i = 0; i < grammar->count; i++)
	{
		qsort(grammar->rule[i].prod, grammar->rule[i].count, sizeof(Production), compareProductions);
	}
	return ERR_OK;
}

static int compareProductions(const void* prod1, const void* prod2)
{
	Production* p1 = (Production*) prod1;
	Production* p2 = (Production*) prod2;

	if(p1->eventType < p2->eventType)
		return 1;
	else if(p1->eventType > p2->eventType)
		return -1;
	else // the same event Type
	{
		if(p1->eventType == EVENT_AT_QNAME)
		{
			return -compareQNameID(&(p1->qnameId), &(p2->qnameId));
		}
		else if(p1->eventType == EVENT_AT_URI)
		{
			if(p1->qnameId.uriId < p2->qnameId.uriId)
			{
				return 1;
			}
			else if(p1->qnameId.uriId > p2->qnameId.uriId)
			{
				return -1;
			}
			else
				return 0;
		}
		else if(p1->eventType == EVENT_SE_QNAME)
		{
			// TODO: figure out how it works??? if this really works for all cases. Seems very unpossible that it does!
			if(p1->nonTermID < p2->nonTermID)
				return 1;
			else
				return -1;
		}
		else if(p1->eventType == EVENT_SE_URI)
		{
			// TODO: figure out how it should be done
		}
		return 0;
	}
}

static errorCode addEEProduction(AllocList* memList, ProtoRuleEntry* rule)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production *prod;
	Index prodId;

	tmp_err_code = addEmptyDynEntry(&rule->dynArray, (void**)&prod, &prodId, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	prod->eventType = EVENT_EE;
	prod->typeId = INDEX_MAX;
	prod->nonTermID = GR_VOID_NON_TERMINAL;
	prod->qnameId.uriId = URI_MAX;
	prod->qnameId.lnId = LN_MAX;

	return ERR_OK;
}

int compareQNameID(const void* qnameID1, const void* qnameID2)
{
	/**
	 *  The strings in the sting tables are sorted beforehand so simple comparison
	 *  of the indexes is enough.
	 */
	QNameID* qId1 = (QNameID*) qnameID1;
	QNameID* qId2 = (QNameID*) qnameID2;
	if(qId1->lnId < qId2->lnId)
	{
		return -1;
	}
	else if(qId1->lnId > qId2->lnId)
	{
		return 1;
	}
	else
	{
		if(qId1->uriId < qId2->uriId)
		{
			return -1;
		}
		else if(qId1->uriId > qId2->uriId)
		{
			return 1;
		}
	}

	return 0;
}
