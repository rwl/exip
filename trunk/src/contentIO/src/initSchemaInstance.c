/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file initSchemaInstance.c
 * @brief Implements the initialization functions for the EXIPSchema object
 *
 * @date Nov 28, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "initSchemaInstance.h"
#include "sTables.h"
#include "eventsEXI.h"

#define DEFAULT_GRAMMAR_TABLE        300
#define DEFAULT_SIMPLE_GRAMMAR_TABLE  75
#define DEFAULT_ENUM_TABLE             5

errorCode initSchema(EXIPSchema* schema, unsigned char initializationType)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = initAllocList(&schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	schema->isStatic = FALSE;
	schema->docGrammar.contentIndex = 0;
	schema->docGrammar.count = 0;
	schema->docGrammar.props = 0;
	schema->docGrammar.rule = NULL;
	schema->simpleTypeTable.count = 0;
	schema->simpleTypeTable.sType = NULL;
	schema->grammarTable.count = 0;
	schema->grammarTable.grammar = NULL;
	schema->enumTable.count = 0;
	schema->enumTable.enumDef = NULL;

	/* Create and initialize initial string table entries */
	tmp_err_code = createDynArray(&schema->uriTable.dynArray, sizeof(UriEntry), DEFAULT_URI_ENTRIES_NUMBER, &schema->memList);
	if(tmp_err_code != ERR_OK)
	{
		freeAllocList(&schema->memList);
		return tmp_err_code;
	}

	tmp_err_code = createUriTableEntries(&schema->uriTable, initializationType != INIT_SCHEMA_SCHEMA_LESS_MODE, &schema->memList);
	if(tmp_err_code != ERR_OK)
	{
		freeAllocList(&schema->memList);
		return tmp_err_code;
	}

	if(initializationType == INIT_SCHEMA_SCHEMA_ENABLED)
	{
		/* Create and initialize enumDef table */
		tmp_err_code = createDynArray(&schema->enumTable.dynArray, sizeof(EnumDefinition), DEFAULT_ENUM_TABLE, &schema->memList);
		if(tmp_err_code != ERR_OK)
		{
			freeAllocList(&schema->memList);
			return tmp_err_code;
		}
	}

	/* Create the schema grammar table */
	tmp_err_code = createDynArray(&schema->grammarTable.dynArray, sizeof(EXIGrammar), DEFAULT_GRAMMAR_TABLE, &schema->memList);
	if(tmp_err_code != ERR_OK)
	{
		freeAllocList(&schema->memList);
		return tmp_err_code;
	}

	if(initializationType != INIT_SCHEMA_SCHEMA_LESS_MODE)
	{
		/* Create and initialize simple type table */
		tmp_err_code = createDynArray(&schema->simpleTypeTable.dynArray, sizeof(SimpleType), DEFAULT_SIMPLE_GRAMMAR_TABLE, &schema->memList);
		if(tmp_err_code != ERR_OK)
		{
			freeAllocList(&schema->memList);
			return tmp_err_code;
		}

		tmp_err_code = createBuiltInTypesDefinitions(&schema->simpleTypeTable, &schema->memList);
		if(tmp_err_code != ERR_OK)
		{
			freeAllocList(&schema->memList);
			return tmp_err_code;
		}

		// Must be done after createBuiltInTypesDefinitions()
		tmp_err_code = generateBuiltInTypesGrammars(schema);
		if(tmp_err_code != ERR_OK)
		{
			freeAllocList(&schema->memList);
		}
	}

	return tmp_err_code;
}

errorCode generateBuiltInTypesGrammars(EXIPSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;
	QNameID typeQnameID;
	Index typeId;
	EXIGrammar grammar;
	Index dynArrId;

	// URI id 3 -> http://www.w3.org/2001/XMLSchema
	typeQnameID.uriId = 3;

	grammar.count = 2;

	for(i = 0; i < schema->uriTable.uri[3].lnTable.count; i++)
	{
		grammar.contentIndex = 0;
		typeQnameID.lnId = i;
		tmp_err_code = getEXIDataTypeFromSimpleType(typeQnameID, &typeId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		grammar.props = 0;
		SET_SCHEMA(grammar.props);
		if((schema->simpleTypeTable.sType[typeId].facetPresenceMask & TYPE_FACET_NAMED_SUBTYPE_UNION) > 0)
			SET_NAMED_SUB_TYPE_OR_UNION(grammar.props);

		// One more rule slot for grammar augmentation when strict == FASLE
		grammar.rule = (GrammarRule*)memManagedAllocate(&schema->memList, sizeof(GrammarRule)*(grammar.count + 1));
		if(grammar.rule == NULL)
			return MEMORY_ALLOCATION_ERROR;

		if(typeId == SIMPLE_TYPE_ANY_TYPE)
		{
			// <xs:anyType> - The base complex type; complex ur-type
			grammar.contentIndex = 1;

			/* Initialize first rule Part 2 */
			grammar.rule[0].part[1].prod = NULL;
			grammar.rule[0].part[1].count = 0;
			grammar.rule[0].part[1].bits = 0;

			/* Initialize first rule Part 3 */
			grammar.rule[0].part[2].prod = NULL;
			grammar.rule[0].part[2].count = 0;
			grammar.rule[0].part[2].bits = 0;

			/* Initialize second rule Part 2 */
			grammar.rule[1].part[1].prod = NULL;
			grammar.rule[1].part[1].count = 0;
			grammar.rule[1].part[1].bits = 0;

			/* Initialize second rule Part 3 */
			grammar.rule[1].part[2].prod = NULL;
			grammar.rule[1].part[2].count = 0;
			grammar.rule[1].part[2].bits = 0;

			grammar.rule[0].part[0].prod = memManagedAllocate(&schema->memList, sizeof(Production)*4);
			if(grammar.rule[0].part[0].prod == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar.rule[0].part[0].prod[3].eventType = EVENT_AT_ALL;
			grammar.rule[0].part[0].prod[3].typeId = INDEX_MAX;
			grammar.rule[0].part[0].prod[3].nonTermID = 0;
			grammar.rule[0].part[0].prod[3].qnameId.uriId = URI_MAX;
			grammar.rule[0].part[0].prod[3].qnameId.lnId = LN_MAX;

			grammar.rule[0].part[0].prod[2].eventType = EVENT_SE_ALL;
			grammar.rule[0].part[0].prod[2].typeId = INDEX_MAX;
			grammar.rule[0].part[0].prod[2].nonTermID = 1;
			grammar.rule[0].part[0].prod[2].qnameId.uriId = URI_MAX;
			grammar.rule[0].part[0].prod[2].qnameId.lnId = LN_MAX;

			grammar.rule[0].part[0].prod[1].eventType = EVENT_EE;
			grammar.rule[0].part[0].prod[1].typeId = INDEX_MAX;
			grammar.rule[0].part[0].prod[1].nonTermID = GR_VOID_NON_TERMINAL;
			grammar.rule[0].part[0].prod[1].qnameId.uriId = URI_MAX;
			grammar.rule[0].part[0].prod[1].qnameId.lnId = LN_MAX;

			grammar.rule[0].part[0].prod[0].eventType = EVENT_CH;
			grammar.rule[0].part[0].prod[0].typeId = INDEX_MAX;
			grammar.rule[0].part[0].prod[0].nonTermID = 1;
			grammar.rule[0].part[0].prod[0].qnameId.uriId = URI_MAX;
			grammar.rule[0].part[0].prod[0].qnameId.lnId = LN_MAX;

			grammar.rule[0].part[0].count = 4;
			grammar.rule[0].part[0].bits = 2;

			grammar.rule[1].part[0].prod = memManagedAllocate(&schema->memList, sizeof(Production)*3);
			if(grammar.rule[1].part[0].prod == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar.rule[1].part[0].prod[2].eventType = EVENT_SE_ALL;
			grammar.rule[1].part[0].prod[2].typeId = INDEX_MAX;
			grammar.rule[1].part[0].prod[2].nonTermID = 1;
			grammar.rule[1].part[0].prod[2].qnameId.uriId = URI_MAX;
			grammar.rule[1].part[0].prod[2].qnameId.lnId = LN_MAX;

			grammar.rule[1].part[0].prod[1].eventType = EVENT_EE;
			grammar.rule[1].part[0].prod[1].typeId = INDEX_MAX;
			grammar.rule[1].part[0].prod[1].nonTermID = GR_VOID_NON_TERMINAL;
			grammar.rule[1].part[0].prod[1].qnameId.uriId = URI_MAX;
			grammar.rule[1].part[0].prod[1].qnameId.lnId = LN_MAX;

			grammar.rule[1].part[0].prod[0].eventType = EVENT_CH;
			grammar.rule[1].part[0].prod[0].typeId = INDEX_MAX;
			grammar.rule[1].part[0].prod[0].nonTermID = 1;
			grammar.rule[1].part[0].prod[0].qnameId.uriId = URI_MAX;
			grammar.rule[1].part[0].prod[0].qnameId.lnId = LN_MAX;

			grammar.rule[1].part[0].count = 3;
			grammar.rule[1].part[0].bits = 2;

		}
		else // a regular simple type
		{
			/* Initialize first rule Part 2 */
			grammar.rule[0].part[1].prod = NULL;
			grammar.rule[0].part[1].count = 0;
			grammar.rule[0].part[1].bits = 0;

			/* Initialize first rule Part 3 */
			grammar.rule[0].part[2].prod = NULL;
			grammar.rule[0].part[2].count = 0;
			grammar.rule[0].part[2].bits = 0;

			/* Initialize second rule Part 2 */
			grammar.rule[1].part[1].prod = NULL;
			grammar.rule[1].part[1].count = 0;
			grammar.rule[1].part[1].bits = 0;

			/* Initialize second rule Part 3 */
			grammar.rule[1].part[2].prod = NULL;
			grammar.rule[1].part[2].count = 0;
			grammar.rule[1].part[2].bits = 0;

			grammar.rule[0].part[0].prod = memManagedAllocate(&schema->memList, sizeof(Production));
			if(grammar.rule[0].part[0].prod == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar.rule[0].part[0].prod[0].eventType = EVENT_CH;
			grammar.rule[0].part[0].prod[0].typeId = typeId;
			grammar.rule[0].part[0].prod[0].nonTermID = 1;
			grammar.rule[0].part[0].prod[0].qnameId.uriId = URI_MAX;
			grammar.rule[0].part[0].prod[0].qnameId.lnId = LN_MAX;
			grammar.rule[0].part[0].count = 1;
			grammar.rule[0].part[0].bits = 0;

			grammar.rule[1].part[0].prod = memManagedAllocate(&schema->memList, sizeof(Production));
			if(grammar.rule[1].part[0].prod == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar.rule[1].part[0].prod[0].eventType = EVENT_EE;
			grammar.rule[1].part[0].prod[0].typeId = INDEX_MAX;
			grammar.rule[1].part[0].prod[0].nonTermID = GR_VOID_NON_TERMINAL;
			grammar.rule[1].part[0].prod[0].qnameId.uriId = URI_MAX;
			grammar.rule[1].part[0].prod[0].qnameId.lnId = LN_MAX;
			grammar.rule[1].part[0].count = 1;
			grammar.rule[1].part[0].bits = 0;
		}

		/** Add the grammar to the schema grammar table */
		addDynEntry(&schema->grammarTable.dynArray, &grammar, &dynArrId, &schema->memList);
		schema->uriTable.uri[3].lnTable.ln[i].typeGrammar = dynArrId;
	}

	return ERR_OK;
}

errorCode createBuiltInTypesDefinitions(SimpleTypeTable* simpleTypeTable, AllocList* memList)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SimpleType sType;
	Index elID;

	// TODO: complete the description of the types with all the facets needed

	// String
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// normalizedString
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// token
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// nmtoken
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// nmtokens
	sType.exiType = VALUE_TYPE_LIST;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// name
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// language
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// ncname
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// idref
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// idrefs
	sType.exiType = VALUE_TYPE_LIST;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// entity
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// entities
	sType.exiType = VALUE_TYPE_LIST;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// id
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// decimal
	sType.exiType = VALUE_TYPE_DECIMAL;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// integer
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// NonPositiveInteger
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// negativeInteger
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.maxInclusive = -1;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// long
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Int
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// short
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 32767;
	sType.minInclusive = -32768;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// byte
	sType.exiType = VALUE_TYPE_SMALL_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 127;
	sType.minInclusive = -128;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// NonNegativeInteger
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned Long
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned int
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned short
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 65535;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned byte
	sType.exiType = VALUE_TYPE_SMALL_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.maxInclusive = 255;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Positive Integer
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.maxInclusive = 0;
	sType.minInclusive = 1;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// boolean
	sType.exiType = VALUE_TYPE_BOOLEAN;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// base64 binary
	sType.exiType = VALUE_TYPE_BINARY;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// hex binary
	sType.exiType = VALUE_TYPE_BINARY;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// float
	sType.exiType = VALUE_TYPE_FLOAT;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// double
	sType.exiType = VALUE_TYPE_FLOAT;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any uri
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// qname
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// notation
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// duration
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// date time
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// time
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// date
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gYearMonth
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gYear
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gMonthDay
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gDay
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gMonth
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any simple type
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any type
	sType.exiType = VALUE_TYPE_NONE;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.maxInclusive = 0;
	sType.minInclusive = 0;
	sType.maxLength = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode getEXIDataTypeFromSimpleType(QNameID simpleXSDType, Index* typeId)
{
	if(simpleXSDType.uriId != 3) // != http://www.w3.org/2001/XMLSchema
		return UNEXPECTED_ERROR;

	// TODO: arrange the typeIds in a way that they are identical with the lnId
	//		so instead of switch it will be simple assignment *typeId = simpleXSDType.lnId

	switch(simpleXSDType.lnId)
	{
		case 0: // "ENTITIES"
			*typeId = SIMPLE_TYPE_ENTITIES;
		break;
		case 1: // "ENTITY"
			*typeId = SIMPLE_TYPE_ENTITY;
		break;
		case 2: // ID
			*typeId = SIMPLE_TYPE_ID;
		break;
		case 3: // IDREF
			*typeId = SIMPLE_TYPE_IDREF;
		break;
		case 4: // IDREFS
			*typeId = SIMPLE_TYPE_IDREFS;
		break;
		case 5: // NCName
			*typeId = SIMPLE_TYPE_NCNAME;
		break;
		case 6: // NMTOKEN
			*typeId = SIMPLE_TYPE_NMTOKEN;
		break;
		case 7: // NMTOKENS
			*typeId = SIMPLE_TYPE_NMTOKENS;
		break;
		case 8: // NOTATION
			*typeId = SIMPLE_TYPE_NOTATION;
		break;
		case 9: // Name
			*typeId = SIMPLE_TYPE_NAME;
		break;
		case 10: // QName
			*typeId = SIMPLE_TYPE_QNAME;
		break;
		case 11: // anySimpleType
			*typeId = SIMPLE_TYPE_ANY_SIMPLE_TYPE;
		break;
		case 12: // anyType
			*typeId = SIMPLE_TYPE_ANY_TYPE;
			// This is not a simple type!
			// It must be handled with creating a Complex Ur-Type Grammar: http://www.w3.org/TR/2011/REC-exi-20110310/#anyTypeGrammar
		break;
		case 13: // anyURI
			*typeId = SIMPLE_TYPE_ANY_URI;
		break;
		case 14: // base64Binary
			*typeId = SIMPLE_TYPE_BASE64_BINARY;
		break;
		case 15: // boolean
			*typeId = SIMPLE_TYPE_BOOLEAN;
		break;
		case 16: // byte
			*typeId = SIMPLE_TYPE_BYTE;
		break;
		case 17: // date
			*typeId = SIMPLE_TYPE_DATE;
		break;
		case 18: // dateTime
			*typeId = SIMPLE_TYPE_DATE_TIME;
		break;
		case 19: // decimal
			*typeId = SIMPLE_TYPE_DECIMAL;
		break;
		case 20: // double
			*typeId = SIMPLE_TYPE_DOUBLE;
		break;
		case 21: // duration
			*typeId = SIMPLE_TYPE_DURATION;
		break;
		case 22: // float
			*typeId = SIMPLE_TYPE_FLOAT;
		break;
		case 23: // gDay
			*typeId = SIMPLE_TYPE_GDAY;
		break;
		case 24: // gMonth
			*typeId = SIMPLE_TYPE_GMONTH;
		break;
		case 25: // gMonthDay
			*typeId = SIMPLE_TYPE_GMONTH_DAY;
		break;
		case 26: // gYear
			*typeId = SIMPLE_TYPE_GYEAR;
		break;
		case 27: // gYearMonth
			*typeId = SIMPLE_TYPE_GYEAR_MONTH;
		break;
		case 28: // hexBinary
			*typeId = SIMPLE_TYPE_HEX_BINARY;
		break;
		case 29: // int
			*typeId = SIMPLE_TYPE_INT;
		break;
		case 30: // integer
			*typeId = SIMPLE_TYPE_INTEGER;
		break;
		case 31: // language
			*typeId = SIMPLE_TYPE_LANGUAGE;
		break;
		case 32: // long
			*typeId = SIMPLE_TYPE_LONG;
		break;
		case 33: // negativeInteger
			*typeId = SIMPLE_TYPE_NEGATIVE_INTEGER;
		break;
		case 34: // nonNegativeInteger
			*typeId = SIMPLE_TYPE_NON_NEGATIVE_INTEGER;
		break;
		case 35: // nonPositiveInteger
			*typeId = SIMPLE_TYPE_NON_POSITIVE_INTEGER;
		break;
		case 36: // normalizedString
			*typeId = SIMPLE_TYPE_NORMALIZED_STRING;
		break;
		case 37: // positiveInteger
			*typeId = SIMPLE_TYPE_POSITIVE_INTEGER;
		break;
		case 38: // short
			*typeId = SIMPLE_TYPE_SHORT;
		break;
		case 39: // string
			*typeId = SIMPLE_TYPE_STRING;
		break;
		case 40: // time
			*typeId = SIMPLE_TYPE_TIME;
		break;
		case 41: // token
			*typeId = SIMPLE_TYPE_TOKEN;
		break;
		case 42: // unsignedByte
			*typeId = SIMPLE_TYPE_UNSIGNED_BYTE;
		break;
		case 43: // unsignedInt
			*typeId = SIMPLE_TYPE_UNSIGNED_INT;
		break;
		case 44: // unsignedLong
			*typeId = SIMPLE_TYPE_UNSIGNED_LONG;
		break;
		case 45: // unsignedShort
			*typeId = SIMPLE_TYPE_UNSIGNED_SHORT;
		break;
	}

	return ERR_OK;
}
