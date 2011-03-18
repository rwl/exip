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

errorCode concatenateGrammars(AllocList* memList, struct EXIGrammar* left, struct EXIGrammar* right, struct EXIGrammar** result)
{
	uint16_t i = 0;
	uint16_t j = 0;
	*result = (struct EXIGrammar*) memManagedAllocate(memList, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = left->rulesDimension + right->rulesDimension;
	(*result)->grammarType = left->grammarType;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* The Non-terminal IDs must be unique within the particular grammar.
	 * To ensure this is true after the concatenating, the right Non-terminal IDs
	 * are re-numerated starting from the biggest left Non-terminal ID value + 1*/

	for(i = 0;i < left->rulesDimension; i++)
	{
		copyGrammarRule(memList, &(left->ruleArray[i]), &((*result)->ruleArray[i]), 0);

		for(j = 0; j < (*result)->ruleArray[i].prodCount; j++)
		{
			if((*result)->ruleArray[i].prodArray[j].event.eventType == EVENT_EE)
			{
				(*result)->ruleArray[i].prodArray[j].event.eventType = EVENT_VOID;
				(*result)->ruleArray[i].prodArray[j].nonTermID = left->rulesDimension;
			}
		}
	}

	for(i = 0;i < right->rulesDimension; i++)
	{
		copyGrammarRule(memList, &(right->ruleArray[i]), &((*result)->ruleArray[left->rulesDimension + i]), left->rulesDimension);
	}

	return ERR_OK;
}

errorCode createElementProtoGrammar(AllocList* memList, StringType name, StringType target_ns,
									struct EXIGrammar* typeDef, QName scope, unsigned char nillable,
									struct EXIGrammar** result)
{
	// TODO: Element-i,0 : Type-j,0 - this basically means that the Element Grammar equals to the type grammar
	// Here only needs to add already normalized type grammar in a element grammar pool
	// So remove the code below as it is not needed
	// Also name and target_ns should be added to metaStrTable if not already there

	int i = 0;

	*result = (struct EXIGrammar*) memManagedAllocate(memList, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = typeDef->rulesDimension;
	(*result)->grammarType = GR_TYPE_SCHEMA_ELEM;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(i = 0; i < typeDef->rulesDimension; i++)
	{
		copyGrammarRule(memList, &(typeDef->ruleArray[i]), &((*result)->ruleArray[i]), 0);
	}

	return ERR_OK;
}

errorCode createSimpleTypeGrammar(AllocList* memList, QName simpleType, struct EXIGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIEvent event;

	*result = (struct EXIGrammar*) memManagedAllocate(memList, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = 2;
	(*result)->grammarType = GR_TYPE_SCHEMA_TYPE;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[0]), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	event.eventType = EVENT_CH;
	tmp_err_code = getEXIDataType(simpleType, &(event.valueType));
	tmp_err_code = addProduction(&((*result)->ruleArray[0]), getEventCode1(0), event, 1);

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[1]), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = addProduction(&((*result)->ruleArray[1]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);

	return ERR_OK;
}

errorCode createSimpleEmptyTypeGrammar(AllocList* memList, struct EXIGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	*result = (struct EXIGrammar*) memManagedAllocate(memList, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = 1;
	(*result)->grammarType = GR_TYPE_SCHEMA_EMPTY_TYPE;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[0]), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = addProduction(&((*result)->ruleArray[0]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	return ERR_OK;
}

errorCode createComplexTypeGrammar(AllocList* memList, StringType name, StringType target_ns,
		                           struct EXIGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType* wildcardArray, unsigned int wildcardArraySize,
		                           struct EXIGrammar* contentTypeGrammar,
								   struct EXIGrammar** result)
{
	//TODO: Implement the case when there are wildcards i.e. wildcardArray is not empty
	//TODO: Consider freeing the intermediate grammars which are not longer needed resulting from the use of concatenateGrammars()

	struct EXIGrammar* tmpGrammar;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;

	tmpGrammar = &(attrUsesArray[0]);
	for(i = 1; i < attrUsesArraySize; i++)
	{
		tmp_err_code = concatenateGrammars(memList, tmpGrammar, &(attrUsesArray[i]), &tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = concatenateGrammars(memList, tmpGrammar, contentTypeGrammar, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->grammarType = GR_TYPE_SCHEMA_TYPE;

	return ERR_OK;
}

errorCode createComplexEmptyTypeGrammar(AllocList* memList, StringType name, StringType target_ns,
		                           struct EXIGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType* wildcardArray, unsigned int wildcardArraySize,
								   struct EXIGrammar** result)
{
	//TODO: Implement the case when there are wildcards i.e. wildcardArray is not empty
	//TODO: Consider freeing the intermediate grammars which are not longer needed resulting from the use of concatenateGrammars()

	struct EXIGrammar* tmpGrammar;

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;
	struct EXIGrammar* emptyContent;

	tmpGrammar = &(attrUsesArray[0]);
	for(i = 1; i < attrUsesArraySize; i++)
	{
		tmp_err_code = concatenateGrammars(memList, tmpGrammar, &(attrUsesArray[i]), &tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = createSimpleEmptyTypeGrammar(memList, &emptyContent);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = concatenateGrammars(memList, tmpGrammar, emptyContent, result);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->grammarType = GR_TYPE_SCHEMA_EMPTY_TYPE;

	return ERR_OK;
}

errorCode createComplexUrTypeGrammar(AllocList* memList, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexUrEmptyTypeGrammar(AllocList* memList, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createAttributeUseGrammar(AllocList* memList, unsigned char required, StringType name, StringType target_ns,
										  QName simpleType, QName scope, struct EXIGrammar** result, DynArray* regProdQname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIEvent event1;
	struct productionQname pqRow;
	size_t elIndx = 0;

	*result = (struct EXIGrammar*) memManagedAllocate(memList, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = 2;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[0]), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	event1.eventType = EVENT_AT_QNAME;
	tmp_err_code = getEXIDataType(simpleType, &event1.valueType);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProduction(&((*result)->ruleArray[0]), getEventCode1(0), event1, 1);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->ruleArray[0].prodArray[0].uriRowID = 9999;
	(*result)->ruleArray[0].prodArray[0].lnRowID = 9999;

	pqRow.p_uriRowID = &((*result)->ruleArray[0].prodArray[0].uriRowID);
	pqRow.p_lnRowID = &((*result)->ruleArray[0].prodArray[0].lnRowID);
	pqRow.qname.uri = &target_ns;
	pqRow.qname.localName = &name;

	tmp_err_code = addDynElement(regProdQname, &pqRow, &elIndx, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(!required)
	{
		tmp_err_code = addProduction(&((*result)->ruleArray[0]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[1]), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = addProduction(&((*result)->ruleArray[1]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode createParticleGrammar(AllocList* memList, unsigned int minOccurs, int32_t maxOccurs,
								struct EXIGrammar* termGrammar, struct EXIGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	struct EXIGrammar* tmpGrammar;
	uint16_t i = 0;

	tmpGrammar = termGrammar;
	for(i = 0; i < minOccurs; i++)
	{
		tmp_err_code = concatenateGrammars(memList, tmpGrammar, termGrammar, &tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	if(maxOccurs - minOccurs > 0 || maxOccurs < 0) // Only if maxOccurs is unbounded or maxOccurs > minOccurs
	{
		unsigned char prodEEFound = 0;
		for(i = 0; i < termGrammar->ruleArray[0].prodCount; i++)
		{
			if(termGrammar->ruleArray[0].prodArray[i].nonTermID == GR_VOID_NON_TERMINAL && termGrammar->ruleArray[0].prodArray[i].event.eventType == EVENT_EE)
			{
				prodEEFound = 1;
				break;
			}
		}
		if(!prodEEFound) //	There is no production Gi,0 : EE so add one
		{
			tmp_err_code = addProduction(&(termGrammar->ruleArray[0]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(maxOccurs >= 0) // {max occurs} is not unbounded
		{
			for(i = 0; i < maxOccurs - minOccurs; i++)
			{
				tmp_err_code = concatenateGrammars(memList, tmpGrammar, termGrammar, &tmpGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
			*result = tmpGrammar;
		}
		else // {max occurs} is unbounded
		{
			uint16_t j = 0;
			// Excluding the first rule
			for(i = 1; i < termGrammar->rulesDimension; i++)
			{
				for(j = 0; j < termGrammar->ruleArray[i].prodCount; j++)
				{
					if(termGrammar->ruleArray[i].prodArray[j].nonTermID == GR_VOID_NON_TERMINAL && termGrammar->ruleArray[i].prodArray[j].event.eventType == EVENT_EE)
					{
						termGrammar->ruleArray[i].prodArray[j].nonTermID = 0;
						termGrammar->ruleArray[i].prodArray[j].event.eventType = EVENT_VOID;
					}
				}
			}

			tmp_err_code = concatenateGrammars(memList, tmpGrammar, termGrammar, result);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else // maxOccurs == minOccurs
	{
		*result = tmpGrammar;
	}

	return ERR_OK;
}

errorCode createElementTermGrammar(AllocList* memList, StringType name, StringType target_ns,
								   struct EXIGrammar** result, DynArray* regProdQname)
{
	//TODO: enable support for {substitution group affiliation} property of the elements

	errorCode tmp_err_code = UNEXPECTED_ERROR;
	struct productionQname pqRow;
	size_t elIndx = 0;
	*result = (struct EXIGrammar*) memManagedAllocate(memList, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = 2;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[0]), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = addProduction(&((*result)->ruleArray[0]), getEventCode1(0), getEventDefType(EVENT_SE_QNAME), 1);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	(*result)->ruleArray[0].prodArray[0].uriRowID = 9999;
	(*result)->ruleArray[0].prodArray[0].lnRowID = 9999;

	pqRow.p_uriRowID = &((*result)->ruleArray[0].prodArray[0].uriRowID);
	pqRow.p_lnRowID = &((*result)->ruleArray[0].prodArray[0].lnRowID);
	pqRow.qname.uri = &target_ns;
	pqRow.qname.localName = &name;

	tmp_err_code = addDynElement(regProdQname, &pqRow, &elIndx, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[1]), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = addProduction(&((*result)->ruleArray[1]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode createWildcardTermGrammar(AllocList* memList, StringType* wildcardArray, unsigned int wildcardArraySize,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createSequenceModelGroupsGrammar(AllocList* memList, ProtoGrammarsStack* pGrammars, struct EXIGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(pGrammars == NULL)
	{
		tmp_err_code = createSimpleEmptyTypeGrammar(memList, result);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		struct EXIGrammar* tmpGrammar = (struct EXIGrammar*) pGrammars;
		pGrammars = pGrammars->nextInStack;
		while(pGrammars != NULL)
		{
			tmp_err_code = concatenateGrammars(memList, tmpGrammar, pGrammars, &tmpGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			pGrammars = pGrammars->nextInStack;
		}
		*result = tmpGrammar;
	}
	return ERR_OK;
}

errorCode createChoiceModelGroupsGrammar(AllocList* memList, struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	*result = (struct EXIGrammar*) memManagedAllocate(memList, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = 1;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(memList, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[0]), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(pTermArraySize == 0)
	{
		tmp_err_code = addProduction(&((*result)->ruleArray[0]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		// TODO: check this case. Probably something is missing here. As maybe the
		//       particle term grammars should be added as a rules here. Link: http://www.w3.org/TR/2009/CR-exi-20091208/#choiceGroupTerms
	}

	return ERR_OK;
}

errorCode createAllModelGroupsGrammar(AllocList* memList, struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode getEXIDataType(QName simpleXSDType, ValueType* exiType)
{
	if(strEqualToAscii(*simpleXSDType.localName, "string") ||
	   strEqualToAscii(*simpleXSDType.localName, "duration") ||
	   strEqualToAscii(*simpleXSDType.localName, "anyURI"))
	{
		*exiType = VALUE_TYPE_STRING;
		return ERR_OK;
	}
	else if(strEqualToAscii(*simpleXSDType.localName, "boolean"))
	{
		*exiType = VALUE_TYPE_BOOLEAN;
		return ERR_OK;
	}
	else if(strEqualToAscii(*simpleXSDType.localName, "integer") ||
			strEqualToAscii(*simpleXSDType.localName, "nonPositiveInteger") ||
			strEqualToAscii(*simpleXSDType.localName, "long") ||
			strEqualToAscii(*simpleXSDType.localName, "nonNegativeInteger") ||
			strEqualToAscii(*simpleXSDType.localName, "int") ||
			strEqualToAscii(*simpleXSDType.localName, "short") ||
			strEqualToAscii(*simpleXSDType.localName, "byte") ||
			strEqualToAscii(*simpleXSDType.localName, "negativeInteger") ||
			strEqualToAscii(*simpleXSDType.localName, "positiveInteger"))
	{
		*exiType = VALUE_TYPE_INTEGER;
		return ERR_OK;
	}
	else if(strEqualToAscii(*simpleXSDType.localName, "float") ||
				strEqualToAscii(*simpleXSDType.localName, "double"))
	{
		*exiType = VALUE_TYPE_FLOAT;
		return ERR_OK;
	}
	else if(strEqualToAscii(*simpleXSDType.localName, "decimal"))
	{
		*exiType = VALUE_TYPE_DECIMAL;
		return ERR_OK;
	}
	else if(strEqualToAscii(*simpleXSDType.localName, "hexBinary") ||
				strEqualToAscii(*simpleXSDType.localName, "base64Binary"))
	{
		*exiType = VALUE_TYPE_BINARY;
		return ERR_OK;
	}
	else if(strEqualToAscii(*simpleXSDType.localName, "dateTime") ||
			strEqualToAscii(*simpleXSDType.localName, "time") ||
			strEqualToAscii(*simpleXSDType.localName, "date") ||
			strEqualToAscii(*simpleXSDType.localName, "gYearMonth") ||
			strEqualToAscii(*simpleXSDType.localName, "gYear") ||
			strEqualToAscii(*simpleXSDType.localName, "gMonthDay") ||
			strEqualToAscii(*simpleXSDType.localName, "gDay") ||
			strEqualToAscii(*simpleXSDType.localName, "gMonth"))
	{
		*exiType = VALUE_TYPE_DATE_TIME;
		return ERR_OK;
	}

	return INCONSISTENT_PROC_STATE;
}

int qnamesCompare(const StringType uri1, const StringType ln1, const StringType uri2, const StringType ln2)
{
	int uri_cmp_res = str_compare(uri1, uri2);
	if(uri_cmp_res == 0) // equal URIs
	{
		return str_compare(ln1, ln2);
	}
	return uri_cmp_res;
}
