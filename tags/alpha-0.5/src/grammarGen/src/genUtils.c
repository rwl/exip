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
#include "stringManipulate.h"
#include "grammars.h"
#include "ioUtil.h"
#include "initSchemaInstance.h"

/** This function @returns TRUE is the two grammar rules represent the same state. Otherwise false */
static char rulesEqual(ProtoGrammar* g1, Index ruleIndx1, ProtoGrammar* g2, Index ruleIndx2);

/** Collision aware addition of all the productions from grammar rule right[ruleIndxR]
 * to the grammar rule left[ruleIndxL] */
static errorCode addProductionsToARule(ProtoGrammar* left, Index ruleIndxL, ProtoGrammar* right, Index ruleIndxR,
									   unsigned int* currRuleIndex, unsigned int initialLeftRulesCount);

errorCode concatenateGrammars(ProtoGrammar* left, ProtoGrammar* right)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int ruleIterL = 0;
	unsigned int ruleIterR = 0;
	unsigned int prodIterL = 0;
	unsigned int prodIterR = 0;
	unsigned int currRuleIndex;
	unsigned int initialLeftRulesCount;
	ProtoRuleEntry* pRuleEntry;

	assert(left);

	if(right == NULL)
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
		tmp_err_code = addProtoRule(left, right->rule[ruleIterR].count, &pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* Copy the RHS productions into the new rule entry, adjusting the non terminal ID */
		for(prodIterR = 0; prodIterR < right->rule[ruleIterR].count; prodIterR++)
		{
			tmp_err_code = addProduction(pRuleEntry, GET_PROD_EXI_EVENT(right->rule[ruleIterR].prod[prodIterR].content),
										 right->rule[ruleIterR].prod[prodIterR].typeId,
										 right->rule[ruleIterR].prod[prodIterR].qnameId,
										 GET_PROD_NON_TERM(right->rule[ruleIterR].prod[prodIterR].content) + ((GET_PROD_EXI_EVENT(right->rule[ruleIterR].prod[prodIterR].content) == EVENT_EE)?0:(initialLeftRulesCount-1)));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	currRuleIndex = left->count;

	for(ruleIterL = 0; ruleIterL < initialLeftRulesCount; ruleIterL++)
	{
		for(prodIterL = 0; prodIterL < left->rule[ruleIterL].count; prodIterL++)
		{
			if(GET_PROD_EXI_EVENT(left->rule[ruleIterL].prod[prodIterL].content) == EVENT_EE)
			{
				if(!rulesEqual(left, ruleIterL, right, 0))
				{
					/* Remove the EE production */
					delDynEntry(&left->rule[ruleIterL].dynArray, prodIterL);

					if(left->rule[ruleIterL].count == 0)
					{
						// Just copy all the production...
						for(prodIterR = 0; prodIterR < right->rule[0].count; prodIterR++)
						{
							tmp_err_code = addProduction(&left->rule[ruleIterL],
														 GET_PROD_EXI_EVENT(right->rule[0].prod[prodIterR].content),
														 right->rule[0].prod[prodIterR].typeId,
														 right->rule[0].prod[prodIterR].qnameId,
														 GET_PROD_NON_TERM(right->rule[0].prod[prodIterR].content) + ((GET_PROD_EXI_EVENT(right->rule[0].prod[prodIterR].content) == EVENT_EE)?0:(initialLeftRulesCount-1)));
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;
						}
					}
					else
					{
						/* Merge productions from RHS rule 0 into each left rule */
						tmp_err_code = addProductionsToARule(left,
															 ruleIterL,
															 right,
															 0,
															 &currRuleIndex,
															 initialLeftRulesCount - 1);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
				}
				break;
			}
		}
	}

	return ERR_OK;
}

static errorCode addProductionsToARule(ProtoGrammar* left, Index ruleIndxL, ProtoGrammar* right, Index ruleIndxR,
									   unsigned int* currRuleIndex, unsigned int initialLeftRulesCount)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int prodIterL = 0;
	unsigned int prodIterR = 0;
	boolean rProdFoundInLeft = FALSE;
	Index nonTermRight;
	unsigned int prodCountR = right->rule[ruleIndxR].count; // Needed as left might be the same grammar as right

	for(prodIterR = 0; prodIterR < prodCountR; prodIterR++)
	{
		/* Check for equivalent productions */
		rProdFoundInLeft = FALSE;

		/* The use case when the Non-Terminal RHS symbol of the production is pointing to the same grammar rule
		 * can cause problems when the rules are merged. Few possible outcomes:
		 * - If all the productions from the right rule are contained in the left rule then the Non-Terminal RHS symbol
		 * can again point to the same rule safely
		 * - If that is not the case, try to find a rule in the left grammar that is equivalent to the
		 * right rule and make the Non-Terminal RHS symbol point to that grammar instead
		 * - If non of these is true then we need to create a new grammar rule on the left that is
		 * equivalent to the right rule and point the Non-Terminal RHS symbol to it */
		if(GET_PROD_NON_TERM(right->rule[ruleIndxR].prod[prodIterR].content) == 0)
		{
			ProtoRuleEntry* pRuleEntry;
			unsigned int tmpIterL, tmpIterR;
			boolean equalRRuleFound = FALSE;
			boolean allRProdIn = TRUE;
			boolean rProdFound = FALSE;

			// It is only an issue if we have productions in (right, ruleIndxR) that are not in (left, ruleIndxL); EE excluded
			for(tmpIterR = 0; tmpIterR < right->rule[ruleIndxR].count; tmpIterR++)
			{
				if(GET_PROD_EXI_EVENT(right->rule[ruleIndxR].prod[tmpIterR].content) != EVENT_EE)
				{
					rProdFound = FALSE;
					for(tmpIterL = 0; tmpIterL < left->rule[ruleIndxL].count; tmpIterL++)
					{
						if(GET_PROD_EXI_EVENT(right->rule[ruleIndxR].prod[tmpIterR].content) == GET_PROD_EXI_EVENT(left->rule[ruleIndxL].prod[tmpIterL].content))
						{
							rProdFound = TRUE;
							break;
						}
					}

					if(rProdFound == FALSE)
					{
						allRProdIn = FALSE;
						break;
					}
				}
			}

			if(allRProdIn)
			{
				// All productions in (right, ruleIndxR) are available in (left, ruleIndxL)
				nonTermRight = ruleIndxL;
			}
			else
			{
				// Try to find a rule in left, different from ruleIndxL that is equal to (right,ruleIndxR)
				for(tmpIterL = 0; tmpIterL < left->count; tmpIterL++)
				{
					if(tmpIterL != ruleIndxL && rulesEqual(left, tmpIterL, right, ruleIndxR))
					{
						// We found a rule that is equal, then make the Non-TermS on the RHS pointing to it
						nonTermRight = tmpIterL;
						equalRRuleFound = TRUE;
						break;
					}
				}

				if(equalRRuleFound == FALSE)
				{
					// Creating a new rule ...

					nonTermRight = *currRuleIndex;

					/* Create new rule entry in LHS proto grammar */
					tmp_err_code = addProtoRule(left, right->rule[ruleIndxR].count, &pRuleEntry);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					/* Copy the RHS productions into the new rule entry, adjusting the non terminal ID */
					for(tmpIterR = 0; tmpIterR < right->rule[ruleIndxR].count; tmpIterR++)
					{
						tmp_err_code = addProduction(pRuleEntry, GET_PROD_EXI_EVENT(right->rule[ruleIndxR].prod[tmpIterR].content),
								right->rule[ruleIndxR].prod[tmpIterR].typeId, right->rule[ruleIndxR].prod[tmpIterR].qnameId,
													 GET_PROD_NON_TERM(right->rule[ruleIndxR].prod[tmpIterR].content) + ((GET_PROD_EXI_EVENT(right->rule[ruleIndxR].prod[tmpIterR].content) == EVENT_EE)?0:(initialLeftRulesCount-1)));
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}

					*currRuleIndex += 1;
				}
			}
		}
		else
			nonTermRight = GET_PROD_NON_TERM(right->rule[ruleIndxR].prod[prodIterR].content) + ((GET_PROD_EXI_EVENT(right->rule[ruleIndxR].prod[prodIterR].content) == EVENT_EE)?0:initialLeftRulesCount);

		for(prodIterL = 0; prodIterL < left->rule[ruleIndxL].count; prodIterL++)
		{
			/* Check for the same terminal symbol e.g. SE(qname) */
			if(GET_PROD_EXI_EVENT(left->rule[ruleIndxL].prod[prodIterL].content) == GET_PROD_EXI_EVENT(right->rule[ruleIndxR].prod[prodIterR].content) &&
					left->rule[ruleIndxL].prod[prodIterL].typeId == right->rule[ruleIndxR].prod[prodIterR].typeId &&
					left->rule[ruleIndxL].prod[prodIterL].qnameId.uriId == right->rule[ruleIndxR].prod[prodIterR].qnameId.uriId &&
					left->rule[ruleIndxL].prod[prodIterL].qnameId.lnId == right->rule[ruleIndxR].prod[prodIterR].qnameId.lnId)
			{
				/* Now check the non-terminal ID (noting that EE's don't have a non-terminal ID) */
				if(GET_PROD_EXI_EVENT(left->rule[ruleIndxL].prod[prodIterL].content) == EVENT_EE ||
				   GET_PROD_NON_TERM(left->rule[ruleIndxL].prod[prodIterL].content) == nonTermRight)
				{
					/*
					 * If the NonTerminals are the same as well, no need to add
					 * the production as it's already there
					 */
					rProdFoundInLeft = TRUE;
					/* Check the next production in LHS... */
					break;
				}
				else
				{
					// The LHS non-terminals are different for the two productions with the same terminal
					// Check if the rules that are indicated by the terminals are equal
					if(rulesEqual(left, GET_PROD_NON_TERM(left->rule[ruleIndxL].prod[prodIterL].content), left, nonTermRight))
					{
						// NonTerminals are different indexes but are otherwise equal
						// no need to add the production as it's already there
						rProdFoundInLeft = TRUE;
						/* Check the next production in LHS... */
						break;
					}
					else
					{
						// Collision: equal terminals and non-equal non-terminals
						// We have a collision detected - must be traced why it happens
						assert(FALSE);
					}
				}
			}
		}

		if(rProdFoundInLeft == FALSE)
		{
			/*
			 * We have been through all LHS productions and there were no clashes
			 * so just add the production
			 */
			tmp_err_code = addProduction(&left->rule[ruleIndxL],
										 GET_PROD_EXI_EVENT(right->rule[ruleIndxR].prod[prodIterR].content),
										 right->rule[ruleIndxR].prod[prodIterR].typeId,
										 right->rule[ruleIndxR].prod[prodIterR].qnameId,
										 nonTermRight);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	return ERR_OK;
}

errorCode createSimpleTypeGrammar(Index typeId, ProtoGrammar* simpleGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QNameID qnameID = {URI_MAX, LN_MAX};
	ProtoRuleEntry* pRuleEntry;

	tmp_err_code = createProtoGrammar(2, simpleGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProtoRule(simpleGrammar, 3, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProduction(pRuleEntry, EVENT_CH, typeId, qnameID, 1);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProtoRule(simpleGrammar, 2, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode createComplexTypeGrammar(ProtoGrammarArray* attrUseArray, ProtoGrammar* contentTypeGrammar,
							boolean isMixedContent, ProtoGrammar* complexGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;

	if(isMixedContent && contentTypeGrammar != NULL)
	{
		/* If {content type} is a content model particle with mixed content, add a production for each non-terminal
		 * Content-i,j  in Content-i  as follows:
		 *  	Content-i,j  : CH [untyped value] Content-i,j
		 */
		QNameID dummyQId = {URI_MAX, LN_MAX};
		for(i = 0; i < contentTypeGrammar->count; i++)
		{
			tmp_err_code = addProduction(&contentTypeGrammar->rule[i], EVENT_CH, INDEX_MAX, dummyQId, i);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	if(attrUseArray->count > 0)
	{
		ProtoRuleEntry* pRuleEntry;

		tmp_err_code = createProtoGrammar(10, complexGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addProtoRule(complexGrammar, 10, &pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addEEProduction(pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for(i = 0; i < attrUseArray->count; i++)
		{
			tmp_err_code = concatenateGrammars(complexGrammar, attrUseArray->pg[i]);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		complexGrammar->contentIndex = complexGrammar->count - 1;

		if(contentTypeGrammar != NULL)
		{
			tmp_err_code = concatenateGrammars(complexGrammar, contentTypeGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else
	{
		if(contentTypeGrammar != NULL)
		{
			tmp_err_code = cloneProtoGrammar(contentTypeGrammar, complexGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		complexGrammar->contentIndex = 0;
	}

	return ERR_OK;
}

errorCode createComplexUrTypeGrammar(ProtoGrammar* result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createAttributeUseGrammar(boolean required, Index typeId,
									ProtoGrammar* attrGrammar, QNameID qnameID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoRuleEntry* pRuleEntry;

	tmp_err_code = createProtoGrammar(2, attrGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProtoRule(attrGrammar, 4, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProduction(pRuleEntry, EVENT_AT_QNAME, typeId, qnameID, 1);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(!required)
	{
		tmp_err_code = addEEProduction(pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = addProtoRule(attrGrammar, 4, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode createParticleGrammar(int minOccurs, int maxOccurs,
								ProtoGrammar* termGrammar, ProtoGrammar* particleGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoRuleEntry* pRuleEntry;
	int i;

	tmp_err_code = createProtoGrammar(minOccurs + 10, particleGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProtoRule(particleGrammar, 5, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 0; i < minOccurs; i++)
	{
		tmp_err_code = concatenateGrammars(particleGrammar, termGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	if(maxOccurs - minOccurs > 0 || maxOccurs < 0) // Only if maxOccurs is unbounded or maxOccurs > minOccurs
	{
		boolean prodEEFound = FALSE;
		for(i = 0; i < (int)termGrammar->rule[0].count; i++)
		{
			if(GET_PROD_EXI_EVENT(termGrammar->rule[0].prod[i].content) == EVENT_EE)
			{
				prodEEFound = TRUE;
				break;
			}
		}
		if(prodEEFound == FALSE) //	There is no production Gi,0 : EE so add one
		{
			tmp_err_code = addEEProduction(&termGrammar->rule[0]);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(maxOccurs >= 0) // {max occurs} is not unbounded
		{
			for(i = 0; i < maxOccurs - minOccurs; i++)
			{
				tmp_err_code = concatenateGrammars(particleGrammar, termGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		else // {max occurs} is unbounded
		{
			Index j = 0;
			unsigned int currRuleIndex = termGrammar->count;

			// Excluding the first rule
			for(i = 1; i < (int)termGrammar->count; i++)
			{
				for(j = 0; j < termGrammar->rule[i].count; j++)
				{
					if(GET_PROD_EXI_EVENT(termGrammar->rule[i].prod[j].content) == EVENT_EE)
					{
						if(!rulesEqual(termGrammar, i, termGrammar, 0))
						{
							/* Remove the EE production */
							delDynEntry(&termGrammar->rule[i].dynArray, j);

							if(termGrammar->rule[i].count == 0)
							{
								Index prodIterR;
								// Just copy all the production...
								for(prodIterR = 0; prodIterR < termGrammar->rule[0].count; prodIterR++)
								{
									tmp_err_code = addProduction(&termGrammar->rule[i],
																 GET_PROD_EXI_EVENT(termGrammar->rule[0].prod[prodIterR].content),
																 termGrammar->rule[0].prod[prodIterR].typeId,
																 termGrammar->rule[0].prod[prodIterR].qnameId,
																 GET_PROD_NON_TERM(termGrammar->rule[0].prod[prodIterR].content));
									if(tmp_err_code != ERR_OK)
										return tmp_err_code;
								}
							}
							else
							{
								/* Merge productions from RHS rule 0 into each left rule */
								tmp_err_code = addProductionsToARule(termGrammar,
																	 i,
																	 termGrammar,
																	 0,
																	 &currRuleIndex,
																	 0);
								if(tmp_err_code != ERR_OK)
									return tmp_err_code;
							}
						}
						break;
					}
				}
			}

			tmp_err_code = concatenateGrammars(particleGrammar, termGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

errorCode createElementTermGrammar(ProtoGrammar* elemGrammar, QNameID qnameID, Index grIndex)
{
	//TODO: enable support for {substitution group affiliation} property of the elements
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoRuleEntry* pRuleEntry;

	tmp_err_code = createProtoGrammar(2, elemGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProtoRule(elemGrammar, 3, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProduction(pRuleEntry, EVENT_SE_QNAME, grIndex, qnameID, 1);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProtoRule(elemGrammar, 3, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode createWildcardTermGrammar(String* wildcardArray, Index wildcardArraySize, UriTable* uriT, ProtoGrammar* wildcardGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoRuleEntry* pRuleEntry;
	QNameID qnameID;

	tmp_err_code = createProtoGrammar(2, wildcardGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProtoRule(wildcardGrammar, wildcardArraySize + 1, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(wildcardArraySize == 0 ||		// default is "##any"
		(wildcardArraySize == 1 &&
				(stringEqualToAscii(wildcardArray[0], "##any") || stringEqualToAscii(wildcardArray[0], "##other"))
		)
	  )
	{
		qnameID.uriId = URI_MAX;
		qnameID.lnId = LN_MAX;
		tmp_err_code = addProduction(pRuleEntry, EVENT_SE_ALL, INDEX_MAX, qnameID, 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(wildcardArraySize >= 1)
	{
		Index i;

		qnameID.lnId = LN_MAX;
		for(i = 0; i < wildcardArraySize; i++)
		{
			if(!lookupUri(uriT, wildcardArray[i], &qnameID.uriId))
			 	return UNEXPECTED_ERROR;
			tmp_err_code = addProduction(pRuleEntry, EVENT_SE_URI, INDEX_MAX, qnameID, 1);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else
		return UNEXPECTED_ERROR;

	tmp_err_code = addProtoRule(wildcardGrammar, 2, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode createSequenceModelGroupsGrammar(ProtoGrammar** grArray, unsigned int arrSize, ProtoGrammar* seqGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoRuleEntry* pRuleEntry;

	if(arrSize == 0)
	{
		tmp_err_code = createProtoGrammar(3, seqGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addProtoRule(seqGrammar, 3, &pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addEEProduction(pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		unsigned int i;

		tmp_err_code = createProtoGrammar(10, seqGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addProtoRule(seqGrammar, 5, &pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addEEProduction(pRuleEntry);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for(i = 0; i < arrSize; i++)
		{
			tmp_err_code = concatenateGrammars(seqGrammar, grArray[i]);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	return ERR_OK;
}

errorCode createChoiceModelGroupsGrammar(ProtoGrammarArray* pgArray, ProtoGrammar* modGrpGrammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Index i;
	unsigned int ruleIterTerm = 0;
	unsigned int prodIterTerm = 0;
	unsigned int currRuleIndex;
	unsigned int initialResultRulesCount;
	ProtoGrammar* tmpGrammar;
	ProtoRuleEntry* pRuleEntry;

	tmp_err_code = createProtoGrammar(10, modGrpGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProtoRule(modGrpGrammar, 5, &pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addEEProduction(pRuleEntry);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmpGrammar = pgArray->pg[0];
	if(tmpGrammar == NULL)
		return NULL_POINTER_REF;

	tmp_err_code = concatenateGrammars(modGrpGrammar, tmpGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 1; i < pgArray->count; i++)
	{

		tmpGrammar = pgArray->pg[i];

		if(tmpGrammar == NULL)
			return NULL_POINTER_REF;

		initialResultRulesCount = modGrpGrammar->count;

		for(ruleIterTerm = 1; ruleIterTerm < tmpGrammar->count; ruleIterTerm++)
		{
			tmp_err_code = addProtoRule(modGrpGrammar, 5, &pRuleEntry);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			for(prodIterTerm = 0; prodIterTerm < tmpGrammar->rule[ruleIterTerm].count; prodIterTerm++)
			{
				tmp_err_code = addProduction(pRuleEntry,
											 GET_PROD_EXI_EVENT(tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].content),
											 tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].typeId,
											 tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].qnameId,
											 GET_PROD_NON_TERM(tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].content) + ((GET_PROD_EXI_EVENT(tmpGrammar->rule[ruleIterTerm].prod[prodIterTerm].content) == EVENT_EE)?0:(initialResultRulesCount-1)));
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}

		currRuleIndex = modGrpGrammar->count;

		if(!rulesEqual(modGrpGrammar, 0, tmpGrammar, 0))
		{
			if(modGrpGrammar->rule[0].count == 0)
			{
				Index prodIterR;
				// Just copy all the production...
				for(prodIterR = 0; prodIterR < tmpGrammar->rule[0].count; prodIterR++)
				{
					tmp_err_code = addProduction(&modGrpGrammar->rule[0],
												 GET_PROD_EXI_EVENT(tmpGrammar->rule[0].prod[prodIterR].content),
												 tmpGrammar->rule[0].prod[prodIterR].typeId,
												 tmpGrammar->rule[0].prod[prodIterR].qnameId,
												 GET_PROD_NON_TERM(tmpGrammar->rule[0].prod[prodIterR].content) + ((GET_PROD_EXI_EVENT(tmpGrammar->rule[0].prod[prodIterR].content) == EVENT_EE)?0:(initialResultRulesCount-1)));
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}
			}
			else
			{
				/* Merge productions from RHS rule 0 into each left rule */
				tmp_err_code = addProductionsToARule(modGrpGrammar,
													 0,
													 tmpGrammar,
													 0,
													 &currRuleIndex,
													 initialResultRulesCount - 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
	}

	return ERR_OK;
}

errorCode createAllModelGroupsGrammar(ProtoGrammar* pTermArray, unsigned int pTermArraySize, ProtoGrammar* modGrpGrammar)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode addEEProduction(ProtoRuleEntry* rule)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production *prod;
	Index prodId;

	tmp_err_code = addEmptyDynEntry(&rule->dynArray, (void**)&prod, &prodId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	SET_PROD_EXI_EVENT(prod->content, EVENT_EE);
	prod->typeId = INDEX_MAX;
	SET_PROD_NON_TERM(prod->content, GR_VOID_NON_TERMINAL);
	prod->qnameId.uriId = URI_MAX;
	prod->qnameId.lnId = LN_MAX;

	return ERR_OK;
}

int compareQNameID(const void* qnameID1, const void* qnameID2, UriTable* uriTbl)
{
	/**
	 *  The strings in the string tables are sorted beforehand so simple comparison
	 *  of the indexes is enough.
	 */
	QNameID* qId1 = (QNameID*) qnameID1;
	QNameID* qId2 = (QNameID*) qnameID2;

	if(qId1->uriId == qId2->uriId)
	{
		// Within the same namespace
		if(qId1->lnId < qId2->lnId)
		{
			return -1;
		}
		else if(qId1->lnId > qId2->lnId)
		{
			return 1;
		}
	}
	else
	{
		// In different namespaces
		int i;
		i = stringCompare(GET_LN_P_URI_P_QNAME(uriTbl, qId1).lnStr, GET_LN_P_URI_P_QNAME(uriTbl, qId2).lnStr);
		if(i == 0)
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
		else
			return i;
	}

	return 0;
}

static char rulesEqual(ProtoGrammar* g1, Index ruleIndx1, ProtoGrammar* g2, Index ruleIndx2)
{
	Index i, j;
	boolean prodFound;

	// TODO: currently it does not follow the right hand side non-terminals to check if the state there is the same...
	// It might not be needed; if needed there will be a recursive call that might lag a lot

	if(g1->rule[ruleIndx1].count != g2->rule[ruleIndx2].count)
		return FALSE;

	for(i = 0; i < g1->rule[ruleIndx1].count; i++)
	{
		prodFound = FALSE;
		for(j = 0; j < g2->rule[ruleIndx2].count; j++)
		{
			if(GET_PROD_EXI_EVENT(g1->rule[ruleIndx1].prod[i].content) == GET_PROD_EXI_EVENT(g2->rule[ruleIndx2].prod[j].content) &&
					g1->rule[ruleIndx1].prod[i].typeId == g2->rule[ruleIndx2].prod[j].typeId &&
					g1->rule[ruleIndx1].prod[i].qnameId.uriId == g2->rule[ruleIndx2].prod[j].qnameId.uriId &&
					g1->rule[ruleIndx1].prod[i].qnameId.lnId == g2->rule[ruleIndx2].prod[j].qnameId.lnId)
			{
				prodFound = TRUE;
				break;
			}
		}

		if(!prodFound)
			return FALSE;
	}

	return TRUE;
}
