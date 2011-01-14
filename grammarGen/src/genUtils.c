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

errorCode concatenateGrammars(EXIStream* strm, struct EXIGrammar* left, struct EXIGrammar* right, struct EXIGrammar** result)
{
	*result = (struct EXIGrammar*) memManagedAllocate(strm, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = left->rulesDimension + right->rulesDimension;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(strm, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	/* The Non-terminal IDs must be unique within the particular grammar.
	 * To ensure this is true after the concatenating, the right Non-terminal IDs
	 * are re-numerated starting from the biggest left Non-terminal ID value + 1*/

	int i = 0;
	int j = 0;
	for(;i < left->rulesDimension; i++)
	{
		copyGrammarRule(strm, &(left->ruleArray[i]), &((*result)->ruleArray[i]));
		(*result)->ruleArray[i].nonTermID = GR_SCHEMA_GRAMMARS_FIRST + i;

		j = 0;
		for(; j < (*result)->ruleArray[i].prodCount; j++)
		{
			if((*result)->ruleArray[i].prodArray[j].eType == EVENT_EE)
			{
				(*result)->ruleArray[i].prodArray[j].eType = EVENT_VOID;
				(*result)->ruleArray[i].prodArray[j].nonTermID = GR_SCHEMA_GRAMMARS_FIRST + left->rulesDimension;
			}
		}
	}

	int i = 0;
	for(;i < right->rulesDimension; i++)
	{
		copyGrammarRule(strm, &(right->ruleArray[i]), &((*result)->ruleArray[left->rulesDimension + i]));
		(*result)->ruleArray[left->rulesDimension + i].nonTermID = GR_SCHEMA_GRAMMARS_FIRST + left->rulesDimension + i;
	}

	return ERR_OK;
}

errorCode createElementProtoGrammar(EXIStream* strm, StringType name, StringType target_ns,
									struct EXIGrammar* typeDef, QName scope, unsigned char nillable,
									struct EXIGrammar** result)
{
	*result = (struct EXIGrammar*) memManagedAllocate(strm, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = typeDef->rulesDimension;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(strm, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	int i = 0;
	for(;i < typeDef->rulesDimension; i++)
	{
		copyGrammarRule(strm, &(typeDef->ruleArray[i]), &((*result)->ruleArray[i]));
		(*result)->ruleArray[i].nonTermID = GR_SCHEMA_GRAMMARS_FIRST + i;
	}

	return ERR_OK;
}

errorCode createSimpleTypeGrammar(EXIStream* strm, QName simpleType, struct EXIGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	*result = (struct EXIGrammar*) memManagedAllocate(strm, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = 2;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(strm, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[0]), strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	(*result)->ruleArray[0].nonTermID = GR_SCHEMA_GRAMMARS_FIRST;
	EXIEvent event;
	event.eventType = EVENT_CH;
	tmp_err_code = getEXIDataType(simpleType, &(event.valueType));
	tmp_err_code = addProduction(&((*result)->ruleArray[0]), getEventCode1(0), event, GR_SCHEMA_GRAMMARS_FIRST + 1);

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[1]), strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	(*result)->ruleArray[1].nonTermID = GR_SCHEMA_GRAMMARS_FIRST + 1;
	tmp_err_code = addProduction(&((*result)->ruleArray[1]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);

	return ERR_OK;
}

errorCode createSimpleEmptyTypeGrammar(EXIStream* strm, struct EXIGrammar** result)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	*result = (struct EXIGrammar*) memManagedAllocate(strm, sizeof(struct EXIGrammar));
	if(*result == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*result)->nextInStack = NULL;
	(*result)->rulesDimension = 1;
	(*result)->ruleArray = (GrammarRule*) memManagedAllocate(strm, sizeof(GrammarRule)*((*result)->rulesDimension));
	if((*result)->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = initGrammarRule(&((*result)->ruleArray[0]), strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	(*result)->ruleArray[0].nonTermID = GR_SCHEMA_GRAMMARS_FIRST;
	tmp_err_code = addProduction(&((*result)->ruleArray[0]), getEventCode1(0), getEventDefType(EVENT_EE), GR_VOID_NON_TERMINAL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	return ERR_OK;
}

errorCode createComplexTypeGrammar(EXIStream* strm, StringType name, StringType target_ns,
		                           struct EXIGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType wildcardArray, unsigned int wildcardArraySize,
		                           struct EXIGrammar* contentTypeGrammar,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexEmptyTypeGrammar(EXIStream* strm, StringType name, StringType target_ns,
		                           struct EXIGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType wildcardArray, unsigned int wildcardArraySize,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexUrTypeGrammar(EXIStream* strm, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexUrEmptyTypeGrammar(EXIStream* strm, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createAttributeUseGrammar(EXIStream* strm, unsigned char required, StringType name, StringType target_ns,
										  QName simpleType, QName scope, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createParticleGrammar(EXIStream* strm, unsigned int minOccurs, unsigned int maxOccurs,
								struct EXIGrammar* termGrammar, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createElementTermGrammar(EXIStream* strm, StringType name, StringType target_ns,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createWildcardTermGrammar(EXIStream* strm, StringType wildcardArray, unsigned int wildcardArraySize,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createSequenceModelGroupsGrammar(EXIStream* strm, struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createChoiceModelGroupsGrammar(EXIStream* strm, struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createAllModelGroupsGrammar(EXIStream* strm, struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode getEXIDataType(QName simpleXSDType, ValueType* exiType)
{
	if(strEqualToAscii(simpleXSDType.localName, "string") ||
	   strEqualToAscii(simpleXSDType.localName, "duration") ||
	   strEqualToAscii(simpleXSDType.localName, "anyURI"))
	{
		*exiType = VALUE_TYPE_STRING;
		return ERR_OK;
	}
	else if(strEqualToAscii(simpleXSDType.localName, "boolean"))
	{
		*exiType = VALUE_TYPE_BOOLEAN;
		return ERR_OK;
	}
	else if(strEqualToAscii(simpleXSDType.localName, "integer") ||
			strEqualToAscii(simpleXSDType.localName, "nonPositiveInteger") ||
			strEqualToAscii(simpleXSDType.localName, "long") ||
			strEqualToAscii(simpleXSDType.localName, "nonNegativeInteger") ||
			strEqualToAscii(simpleXSDType.localName, "int") ||
			strEqualToAscii(simpleXSDType.localName, "short") ||
			strEqualToAscii(simpleXSDType.localName, "byte") ||
			strEqualToAscii(simpleXSDType.localName, "negativeInteger") ||
			strEqualToAscii(simpleXSDType.localName, "positiveInteger"))
	{
		*exiType = VALUE_TYPE_INTEGER;
		return ERR_OK;
	}
	else if(strEqualToAscii(simpleXSDType.localName, "float") ||
				strEqualToAscii(simpleXSDType.localName, "double"))
	{
		*exiType = VALUE_TYPE_FLOAT;
		return ERR_OK;
	}
	else if(strEqualToAscii(simpleXSDType.localName, "decimal"))
	{
		*exiType = VALUE_TYPE_DECIMAL;
		return ERR_OK;
	}
	else if(strEqualToAscii(simpleXSDType.localName, "hexBinary") ||
				strEqualToAscii(simpleXSDType.localName, "base64Binary"))
	{
		*exiType = VALUE_TYPE_BINARY;
		return ERR_OK;
	}
	else if(strEqualToAscii(simpleXSDType.localName, "dateTime") ||
			strEqualToAscii(simpleXSDType.localName, "time") ||
			strEqualToAscii(simpleXSDType.localName, "date") ||
			strEqualToAscii(simpleXSDType.localName, "gYearMonth") ||
			strEqualToAscii(simpleXSDType.localName, "gYear") ||
			strEqualToAscii(simpleXSDType.localName, "gMonthDay") ||
			strEqualToAscii(simpleXSDType.localName, "gDay") ||
			strEqualToAscii(simpleXSDType.localName, "gMonth"))
	{
		*exiType = VALUE_TYPE_DATE_TIME;
		return ERR_OK;
	}

	return INCONSISTENT_PROC_STATE;
}
