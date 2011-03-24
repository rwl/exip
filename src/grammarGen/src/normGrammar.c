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
 * @file normGrammar.c
 * @brief Implementing the code for normalizing EXI Proto-Grammars
 * @date Jan 31, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "normGrammar.h"
#include "dynamicArray.h"
#include "grammarRules.h"
#include "memManagement.h"

/** Store the information for a single already removed production - used for deleteNoTermProductions() */
struct removedProdInf
{
	size_t leftNonTermID;
	size_t rightNonTermID;
};

/** Store the information for a single assigned for addition grammar rule - used for deleteDuplicateTerminals() */
struct addedRuleInf
{
	size_t firstNonTermID; // smaller index, part of the union
	size_t secondNonTermID;  // higher index, part of the union
	size_t ruleNonTermID;
	GrammarRule* rule;
};

errorCode deleteNoTermProductions(AllocList* memList, struct EXIGrammar* grammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t i = 0;
	uint16_t j = 0;

	uint16_t k = 0;
	uint16_t l = 0;
	size_t t = 0;
	unsigned char rmPrFlag = 0;
	DynArray* removedProdArray;
	size_t rightNonTerm = 0;
	struct removedProdInf pr;
	size_t elId = 0;

	tmp_err_code = createDynArray(&removedProdArray, sizeof(struct removedProdInf), 5, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;


	for(i = 0; i < grammar->rulesDimension; i++) // For every rule in the grammar
	{
		for(j = 0; j < grammar->ruleArray[i].prodCount; j++) // For every production in a rule
		{
			if(grammar->ruleArray[i].prodArray[j].event.eventType == EVENT_VOID) // If there is no TERM symbol in this production
			{
				rightNonTerm = grammar->ruleArray[i].prodArray[j].nonTermID;
				// Register that we are replacing this production without a terminal symbol
				pr.leftNonTermID = i;
				pr.rightNonTermID = rightNonTerm;
				tmp_err_code = addDynElement(removedProdArray, &pr, &elId, memList);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				for(k = 0; k < grammar->rulesDimension; k++)  // For every rule in the grammar
				{
					if(k == rightNonTerm) // If the Left NON-TERM is eq to the previous Production rightNonTerm
					{
						for(l = 0; l < grammar->ruleArray[k].prodCount; l++) // For every production in this second rule
						{
							if(!(grammar->ruleArray[k].prodArray[l].event.eventType == EVENT_VOID &&
									grammar->ruleArray[k].prodArray[l].nonTermID == i)) // If the right-hand side is not identical to the left-hand side
							{
								struct removedProdInf* tmpPr;
								rmPrFlag = 0;   // Shows if a there is already such production replaced
								for(t = 0; t < removedProdArray->elementCount; t++)
								{
									tmpPr = &((struct removedProdInf* ) (removedProdArray->elements))[t];
									if(grammar->ruleArray[k].prodArray[l].nonTermID == tmpPr->rightNonTermID
										&& i == tmpPr->leftNonTermID
										&& grammar->ruleArray[k].prodArray[l].event.eventType == EVENT_VOID)
									{
										rmPrFlag = 1; // This production was already replaced - DO NOT INSERT IT!
										break;
									}
								}
								if(!rmPrFlag) // Insert this production
								{
									if(grammar->ruleArray[i].prodArray[j].event.eventType == EVENT_VOID) // This is the first production inserted - so replace the old one
									{
										grammar->ruleArray[i].prodArray[j].event.eventType = grammar->ruleArray[k].prodArray[l].event.eventType;
										grammar->ruleArray[i].prodArray[j].event.valueType = grammar->ruleArray[k].prodArray[l].event.valueType;
										grammar->ruleArray[i].prodArray[j].nonTermID = grammar->ruleArray[k].prodArray[l].nonTermID;
									}
									else // Second or above production for insertion - insert at the end of Production array
									{
										tmp_err_code = addProduction(&(grammar->ruleArray[i]), grammar->ruleArray[k].prodArray[l].code,
												grammar->ruleArray[k].prodArray[l].event, grammar->ruleArray[k].prodArray[l].nonTermID);
										if(tmp_err_code != ERR_OK)
											return tmp_err_code;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return ERR_OK;
}

errorCode deleteDuplicateTerminals(AllocList* memList, struct EXIGrammar* grammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t i = 0;
	uint16_t j = 0;
	uint16_t k = 0;
	uint16_t l = 0;
	uint16_t t = 0;
	uint16_t p = 0;
	unsigned char foundFlag = 0;

	DynArray* addedRulesArray;
	unsigned int lastAddedNonTermID;
	struct addedRuleInf tmp;

	// TODO: take into account the 'content' index:
	/* When G i  is a type grammar, if both k and l are smaller than content index
	 * of G i , k ⊔ l is also considered to be smaller than content for the purpose
	 * of index comparison purposes. Otherwise, if either k or l is not smaller than
	 * content, k ⊔ l is considered to be larger than content. */

	tmp_err_code = createDynArray(&addedRulesArray, sizeof(struct addedRuleInf), 5, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	lastAddedNonTermID = grammar->rulesDimension;

	for(i = 0; i < grammar->rulesDimension; i++) // For every rule in the grammar
	{
		for(j = 0; j < grammar->ruleArray[i].prodCount; j++) // For every production in a rule
		{
			for(k = j + 1; k < grammar->ruleArray[i].prodCount; k++)  // For every other successive production in the rule
			{
				if(grammar->ruleArray[i].prodArray[j].event.eventType == grammar->ruleArray[i].prodArray[k].event.eventType
						&& grammar->ruleArray[i].prodArray[j].event.valueType == grammar->ruleArray[i].prodArray[k].event.valueType) // If there are Duplicate Terminal Symbols
				{
					if(grammar->ruleArray[i].prodArray[j].nonTermID == grammar->ruleArray[i].prodArray[k].nonTermID) // identical productions
					{
						// delete the second one
						memcpy(grammar->ruleArray[i].prodArray + k, grammar->ruleArray[i].prodArray + k + 1,grammar->ruleArray[i].prodCount - k + 1);
						grammar->ruleArray[i].prodCount -= 1;
					}
					else  // not identical productions
					{
						foundFlag = 0;
						for(l = 0; l < addedRulesArray->elementCount; l++)  // Check if it is not already assigned for addition
						{
							tmp = ((struct addedRuleInf*) addedRulesArray->elements)[l];
							if(grammar->ruleArray[i].prodArray[j].nonTermID == tmp.firstNonTermID &&
									 grammar->ruleArray[i].prodArray[k].nonTermID == tmp.secondNonTermID)
							{
								foundFlag = 1;
								grammar->ruleArray[i].prodArray[j].nonTermID = tmp.ruleNonTermID;
								// delete the second one
								memcpy(grammar->ruleArray[i].prodArray + k, grammar->ruleArray[i].prodArray + k + 1,grammar->ruleArray[i].prodCount - k + 1);
								grammar->ruleArray[i].prodCount -= 1;
								break;
							}
						}

						if(!foundFlag) // Not already assigned for addition
						{
							tmp.firstNonTermID = grammar->ruleArray[i].prodArray[j].nonTermID;
							tmp.secondNonTermID = grammar->ruleArray[i].prodArray[k].nonTermID;

							tmp.rule = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule));
							if(tmp.rule == NULL)
								return MEMORY_ALLOCATION_ERROR;

							tmp_err_code = initGrammarRule(tmp.rule, memList);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;
							tmp.ruleNonTermID = lastAddedNonTermID;

							for(t = 0; t < grammar->rulesDimension; t++)  // For every rule in the grammar
							{
								if(t == tmp.firstNonTermID || t == tmp.secondNonTermID)  // if this rule is one of the two with Non-Term we are looking for
								{
									for(p = 0; p < grammar->ruleArray[t].prodCount; p++) // For every production in the rule
									{
										/* Just added to the new rule */
										tmp_err_code = addProduction(tmp.rule, grammar->ruleArray[t].prodArray[p].code, grammar->ruleArray[t].prodArray[p].event,
												grammar->ruleArray[t].prodArray[p].nonTermID);
										if(tmp_err_code != ERR_OK)
											return tmp_err_code;
									}
								}
							}
							grammar->ruleArray[i].prodArray[j].nonTermID = tmp.ruleNonTermID;
							// delete the second one
							memcpy(grammar->ruleArray[i].prodArray + k, grammar->ruleArray[i].prodArray + k + 1,grammar->ruleArray[i].prodCount - k + 1);
							grammar->ruleArray[i].prodCount -= 1;
							lastAddedNonTermID += 1;
						}
					}
				}
			}
		}
	}

	if(addedRulesArray->elementCount > 0) // there are rules for addition to this grammar
	{
		GrammarRule* newGrammarRuleArray;
		/* We do not use realloc() here as the previous GrammarRule pointer must be freed by the EXIP mem menager*/
		newGrammarRuleArray = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule)*(grammar->rulesDimension + addedRulesArray->elementCount));
		if(newGrammarRuleArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		for(i = 0; i < grammar->rulesDimension; i++)  // Copying the initial rules
		{
			tmp_err_code = copyGrammarRule(memList, &(grammar->ruleArray[i]), &(newGrammarRuleArray[i]), 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		for(i = 0; i < addedRulesArray->elementCount; i++)  // Copying the added rules
		{
			tmp_err_code = copyGrammarRule(memList, ((struct addedRuleInf*) addedRulesArray->elements)[i].rule, &(newGrammarRuleArray[((struct addedRuleInf*) addedRulesArray->elements)[i].ruleNonTermID]), 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		/* The old grammar->ruleArray will be garbage collected by EXIP mem menager*/
		grammar->ruleArray = newGrammarRuleArray;

		/* Recursive calls to the deleteDuplicateTerminals until everything is cleaned*/
		tmp_err_code = deleteDuplicateTerminals(memList, grammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode normalizeGrammar(AllocList* memList, struct EXIGrammar* grammar)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	tmp_err_code = deleteNoTermProductions(memList, grammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = deleteDuplicateTerminals(memList, grammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}
