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
 * @file buildInGrammars.c
 * @brief Implements a function for generation of build-in Schema type grammars.
 *
 * @date Nov 28, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "buildInGrammars.h"
#include "sTables.h"
#include "eventsEXI.h"

errorCode generateSchemaBuildInGrammars(EXIPSchema* schema)
{
	// TODO: As everywhere else, here too, when an error occur the memory allocated with initAllocList() is not freed!
	// to be checked everywhere!

	DynArray* simpleTypesArray; // A temporary array of simple type definitions
	AllocList tmpMemList;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint16_t i;

	tmp_err_code = initAllocList(&tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = initAllocList(&schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	schema->isStatic = FALSE;

	tmp_err_code = createURITable(&schema->initialStringTables, &schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createInitialEntries(&schema->memList, schema->initialStringTables, TRUE);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = generateBuildInTypesGrammars(schema->initialStringTables, &schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&simpleTypesArray, sizeof(SimpleType), 50, &tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createBuildInTypesDefinitions(simpleTypesArray, &tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	schema->sTypeArraySize = simpleTypesArray->elementCount;
	schema->simpleTypeArray = memManagedAllocate(&schema->memList, sizeof(SimpleType) * schema->sTypeArraySize);
	if(schema->simpleTypeArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	for(i = 0; i < schema->sTypeArraySize; i++)
	{
		schema->simpleTypeArray[i].facetPresenceMask = ((SimpleType*) simpleTypesArray->elements)[i].facetPresenceMask;
		schema->simpleTypeArray[i].maxInclusive = ((SimpleType*) simpleTypesArray->elements)[i].maxInclusive;
		schema->simpleTypeArray[i].minInclusive = ((SimpleType*) simpleTypesArray->elements)[i].minInclusive;
		schema->simpleTypeArray[i].maxLength = ((SimpleType*) simpleTypesArray->elements)[i].maxLength;
	}

	freeAllocList(&tmpMemList);

	schema->globalElemGrammarsCount = 0;
	schema->globalElemGrammars = NULL;

	return ERR_OK;
}

errorCode generateBuildInTypesGrammars(URITable* sTables, AllocList* memList)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;
	QNameID typeQnameID;
	ValueType vType;
	EXIGrammar* grammar;
	EXIGrammar* typeEmptyGrammar;

	typeEmptyGrammar = memManagedAllocate(memList, sizeof(EXIGrammar));
	if(typeEmptyGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	typeEmptyGrammar->contentIndex = 0;
	typeEmptyGrammar->isNillable = FALSE;
	typeEmptyGrammar->isAugmented = FALSE;
	typeEmptyGrammar->grammarType = GR_TYPE_SCHEMA_EMPTY_TYPE;
	typeEmptyGrammar->rulesDimension = 1;

	// One more rule slot for grammar augmentation when strict == FASLE
	typeEmptyGrammar->ruleArray = memManagedAllocate(memList, sizeof(GrammarRule)*2);
	if(typeEmptyGrammar->ruleArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	typeEmptyGrammar->ruleArray->part[0].bits = 0;
	typeEmptyGrammar->ruleArray->part[1].bits = 0;
	typeEmptyGrammar->ruleArray->part[2].bits = 0;

	typeEmptyGrammar->ruleArray->part[0].prodArraySize = 1;
	typeEmptyGrammar->ruleArray->part[1].prodArraySize = 0;
	typeEmptyGrammar->ruleArray->part[2].prodArraySize = 0;

	typeEmptyGrammar->ruleArray->part[0].prodArray = memManagedAllocate(memList, sizeof(Production));
	if(typeEmptyGrammar->ruleArray->part[0].prodArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	typeEmptyGrammar->ruleArray->part[1].prodArray = NULL;
	typeEmptyGrammar->ruleArray->part[2].prodArray = NULL;

	typeEmptyGrammar->ruleArray->part[0].prodArray[0].event = getEventDefType(EVENT_EE);
	typeEmptyGrammar->ruleArray->part[0].prodArray[0].qname.uriRowId = UINT16_MAX;
	typeEmptyGrammar->ruleArray->part[0].prodArray[0].qname.lnRowId = SIZE_MAX;
	typeEmptyGrammar->ruleArray->part[0].prodArray[0].nonTermID = GR_VOID_NON_TERMINAL;

	// URI id 3 -> http://www.w3.org/2001/XMLSchema
	typeQnameID.uriRowId = 3;

	for(i = 0; i < sTables->rows[3].lTable->rowCount; i++)
	{
		typeQnameID.lnRowId = i;
		tmp_err_code = getEXIDataTypeFromSimpleType(typeQnameID, &vType);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		grammar = memManagedAllocate(memList, sizeof(EXIGrammar));
		if(grammar == NULL)
			return MEMORY_ALLOCATION_ERROR;

		grammar->contentIndex = 0;
		grammar->grammarType = GR_TYPE_SCHEMA_TYPE;
		grammar->isNillable = FALSE;
		grammar->isAugmented = FALSE;
		grammar->rulesDimension = 2;

		// One more rule slot for grammar augmentation when strict == FASLE
		grammar->ruleArray = memManagedAllocate(memList, sizeof(GrammarRule)*(grammar->rulesDimension + 1));
		if(grammar->ruleArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		grammar->ruleArray[0].part[0].bits = 0;
		grammar->ruleArray[0].part[1].bits = 0;
		grammar->ruleArray[0].part[2].bits = 0;
		grammar->ruleArray[0].part[0].prodArraySize = 1;
		grammar->ruleArray[0].part[1].prodArraySize = 0;
		grammar->ruleArray[0].part[2].prodArraySize = 0;

		grammar->ruleArray[0].part[0].prodArray = memManagedAllocate(memList, sizeof(Production));
		if(grammar->ruleArray[0].part[0].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		grammar->ruleArray[0].part[1].prodArray = NULL;
		grammar->ruleArray[0].part[2].prodArray = NULL;

		grammar->ruleArray[0].part[0].prodArray[0].event.eventType = EVENT_CH;
		grammar->ruleArray[0].part[0].prodArray[0].event.valueType = vType;
		grammar->ruleArray[0].part[0].prodArray[0].nonTermID = 1;
		grammar->ruleArray[0].part[0].prodArray[0].qname.uriRowId = UINT16_MAX;
		grammar->ruleArray[0].part[0].prodArray[0].qname.lnRowId = SIZE_MAX;

		grammar->ruleArray[1].part[0].bits = 0;
		grammar->ruleArray[1].part[1].bits = 0;
		grammar->ruleArray[1].part[2].bits = 0;
		grammar->ruleArray[1].part[0].prodArraySize = 1;
		grammar->ruleArray[1].part[1].prodArraySize = 0;
		grammar->ruleArray[1].part[2].prodArraySize = 0;

		grammar->ruleArray[1].part[0].prodArray = memManagedAllocate(memList, sizeof(Production));
		if(grammar->ruleArray[1].part[0].prodArray == NULL)
			return MEMORY_ALLOCATION_ERROR;

		grammar->ruleArray[1].part[1].prodArray = NULL;
		grammar->ruleArray[1].part[2].prodArray = NULL;

		grammar->ruleArray[1].part[0].prodArray[0].event.eventType = EVENT_EE;
		grammar->ruleArray[1].part[0].prodArray[0].event.valueType.exiType = VALUE_TYPE_NONE;
		grammar->ruleArray[1].part[0].prodArray[0].event.valueType.simpleTypeID = UINT16_MAX;
		grammar->ruleArray[1].part[0].prodArray[0].nonTermID = GR_VOID_NON_TERMINAL;
		grammar->ruleArray[1].part[0].prodArray[0].qname.uriRowId = UINT16_MAX;
		grammar->ruleArray[1].part[0].prodArray[0].qname.lnRowId = SIZE_MAX;

		sTables->rows[3].lTable->rows[i].typeGrammar = grammar;
		sTables->rows[3].lTable->rows[i].typeEmptyGrammar = typeEmptyGrammar;
	}

	return ERR_OK;
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
