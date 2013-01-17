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
#include "grammars.h"

#define DEFAULT_GRAMMAR_TABLE        300
#define DEFAULT_SIMPLE_GRAMMAR_TABLE  75
#define DEFAULT_ENUM_TABLE             5

errorCode initSchema(EXIPSchema* schema, unsigned char initializationType)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = initAllocList(&schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	schema->staticGrCount = 0;
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
	tmp_err_code = createDynArray(&schema->uriTable.dynArray, sizeof(UriEntry), DEFAULT_URI_ENTRIES_NUMBER);
	if(tmp_err_code != ERR_OK)
	{
		freeAllocList(&schema->memList);
		return tmp_err_code;
	}

	tmp_err_code = createUriTableEntries(&schema->uriTable, initializationType != INIT_SCHEMA_SCHEMA_LESS_MODE);
	if(tmp_err_code != ERR_OK)
	{
		freeAllocList(&schema->memList);
		return tmp_err_code;
	}

	if(initializationType == INIT_SCHEMA_SCHEMA_ENABLED)
	{
		/* Create and initialize enumDef table */
		tmp_err_code = createDynArray(&schema->enumTable.dynArray, sizeof(EnumDefinition), DEFAULT_ENUM_TABLE);
		if(tmp_err_code != ERR_OK)
		{
			freeAllocList(&schema->memList);
			return tmp_err_code;
		}
	}

	/* Create the schema grammar table */
	tmp_err_code = createDynArray(&schema->grammarTable.dynArray, sizeof(EXIGrammar), DEFAULT_GRAMMAR_TABLE);
	if(tmp_err_code != ERR_OK)
	{
		freeAllocList(&schema->memList);
		return tmp_err_code;
	}

	if(initializationType != INIT_SCHEMA_SCHEMA_LESS_MODE)
	{
		/* Create and initialize simple type table */
		tmp_err_code = createDynArray(&schema->simpleTypeTable.dynArray, sizeof(SimpleType), DEFAULT_SIMPLE_GRAMMAR_TABLE);
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

		schema->staticGrCount = SIMPLE_TYPE_COUNT;
	}

	return tmp_err_code;
}

errorCode generateBuiltInTypesGrammars(EXIPSchema* schema)
{
	unsigned int i;
	QNameID typeQnameID;
	Index typeId;
	EXIGrammar grammar;
	Index dynArrId;

	// URI id 3 -> http://www.w3.org/2001/XMLSchema
	typeQnameID.uriId = XML_SCHEMA_NAMESPACE_ID;

	grammar.count = 2;

	for(i = 0; i < schema->uriTable.uri[XML_SCHEMA_NAMESPACE_ID].lnTable.count; i++)
	{
		grammar.contentIndex = 0;
		typeQnameID.lnId = i;
		typeId = typeQnameID.lnId;

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

			/* Initialize first rule Part 2 and 3 */
			grammar.rule[0].prod23 = NULL;
			grammar.rule[0].p2Count = 0;
			grammar.rule[0].p3Count = 0;

			/* Initialize second rule Part 2 and 3 */
			grammar.rule[1].prod23 = NULL;
			grammar.rule[1].p2Count = 0;
			grammar.rule[1].p3Count = 0;

			grammar.rule[0].prod1 = memManagedAllocate(&schema->memList, sizeof(Production)*4);
			if(grammar.rule[0].prod1 == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar.rule[0].prod1[3].eventType = EVENT_AT_ALL;
			grammar.rule[0].prod1[3].typeId = INDEX_MAX;
			grammar.rule[0].prod1[3].nonTermID = 0;
			grammar.rule[0].prod1[3].qnameId.uriId = URI_MAX;
			grammar.rule[0].prod1[3].qnameId.lnId = LN_MAX;

			grammar.rule[0].prod1[2].eventType = EVENT_SE_ALL;
			grammar.rule[0].prod1[2].typeId = INDEX_MAX;
			grammar.rule[0].prod1[2].nonTermID = 1;
			grammar.rule[0].prod1[2].qnameId.uriId = URI_MAX;
			grammar.rule[0].prod1[2].qnameId.lnId = LN_MAX;

			grammar.rule[0].prod1[1].eventType = EVENT_EE;
			grammar.rule[0].prod1[1].typeId = INDEX_MAX;
			grammar.rule[0].prod1[1].nonTermID = GR_VOID_NON_TERMINAL;
			grammar.rule[0].prod1[1].qnameId.uriId = URI_MAX;
			grammar.rule[0].prod1[1].qnameId.lnId = LN_MAX;

			grammar.rule[0].prod1[0].eventType = EVENT_CH;
			grammar.rule[0].prod1[0].typeId = INDEX_MAX;
			grammar.rule[0].prod1[0].nonTermID = 1;
			grammar.rule[0].prod1[0].qnameId.uriId = URI_MAX;
			grammar.rule[0].prod1[0].qnameId.lnId = LN_MAX;

			grammar.rule[0].p1Count = 4;
			grammar.rule[0].bits1 = 2;

			grammar.rule[1].prod1 = memManagedAllocate(&schema->memList, sizeof(Production)*3);
			if(grammar.rule[1].prod1 == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar.rule[1].prod1[2].eventType = EVENT_SE_ALL;
			grammar.rule[1].prod1[2].typeId = INDEX_MAX;
			grammar.rule[1].prod1[2].nonTermID = 1;
			grammar.rule[1].prod1[2].qnameId.uriId = URI_MAX;
			grammar.rule[1].prod1[2].qnameId.lnId = LN_MAX;

			grammar.rule[1].prod1[1].eventType = EVENT_EE;
			grammar.rule[1].prod1[1].typeId = INDEX_MAX;
			grammar.rule[1].prod1[1].nonTermID = GR_VOID_NON_TERMINAL;
			grammar.rule[1].prod1[1].qnameId.uriId = URI_MAX;
			grammar.rule[1].prod1[1].qnameId.lnId = LN_MAX;

			grammar.rule[1].prod1[0].eventType = EVENT_CH;
			grammar.rule[1].prod1[0].typeId = INDEX_MAX;
			grammar.rule[1].prod1[0].nonTermID = 1;
			grammar.rule[1].prod1[0].qnameId.uriId = URI_MAX;
			grammar.rule[1].prod1[0].qnameId.lnId = LN_MAX;

			grammar.rule[1].p1Count = 3;
			grammar.rule[1].bits1 = 2;

		}
		else // a regular simple type
		{
			/* Initialize first rule Part 2 and 3 */
			grammar.rule[0].prod23 = NULL;
			grammar.rule[0].p2Count = 0;
			grammar.rule[0].p3Count = 0;

			/* Initialize second rule Part 2 and 3 */
			grammar.rule[1].prod23 = NULL;
			grammar.rule[1].p2Count = 0;
			grammar.rule[1].p3Count = 0;

			grammar.rule[0].prod1 = memManagedAllocate(&schema->memList, sizeof(Production));
			if(grammar.rule[0].prod1 == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar.rule[0].prod1[0].eventType = EVENT_CH;
			grammar.rule[0].prod1[0].typeId = typeId;
			grammar.rule[0].prod1[0].nonTermID = 1;
			grammar.rule[0].prod1[0].qnameId.uriId = URI_MAX;
			grammar.rule[0].prod1[0].qnameId.lnId = LN_MAX;
			grammar.rule[0].p1Count = 1;
			grammar.rule[0].bits1 = 0;

			grammar.rule[1].prod1 = memManagedAllocate(&schema->memList, sizeof(Production));
			if(grammar.rule[1].prod1 == NULL)
				return MEMORY_ALLOCATION_ERROR;

			grammar.rule[1].prod1[0].eventType = EVENT_EE;
			grammar.rule[1].prod1[0].typeId = INDEX_MAX;
			grammar.rule[1].prod1[0].nonTermID = GR_VOID_NON_TERMINAL;
			grammar.rule[1].prod1[0].qnameId.uriId = URI_MAX;
			grammar.rule[1].prod1[0].qnameId.lnId = LN_MAX;
			grammar.rule[1].p1Count = 1;
			grammar.rule[1].bits1 = 0;
		}

		/** Add the grammar to the schema grammar table */
		addDynEntry(&schema->grammarTable.dynArray, &grammar, &dynArrId);
		schema->uriTable.uri[3].lnTable.ln[i].typeGrammar = dynArrId;
	}

	return ERR_OK;
}

errorCode createBuiltInTypesDefinitions(SimpleTypeTable* simpleTypeTable, AllocList* memList)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SimpleType sType;
	Index elID;

	// entities
	sType.exiType = VALUE_TYPE_LIST;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = SIMPLE_TYPE_ENTITY;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// entity
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// id
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// idref
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// idrefs
	sType.exiType = VALUE_TYPE_LIST;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = SIMPLE_TYPE_IDREF;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// ncname
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// nmtoken
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// nmtokens
	sType.exiType = VALUE_TYPE_LIST;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = SIMPLE_TYPE_NMTOKEN;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// notation
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// name
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// qname
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any simple type
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any type
	sType.exiType = VALUE_TYPE_NONE;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// any uri
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// base64 binary
	sType.exiType = VALUE_TYPE_BINARY;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// boolean
	sType.exiType = VALUE_TYPE_BOOLEAN;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// byte
	sType.exiType = VALUE_TYPE_SMALL_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.max = 127;
	sType.min = -128;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// date
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// date time
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// decimal
	sType.exiType = VALUE_TYPE_DECIMAL;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// double
	sType.exiType = VALUE_TYPE_FLOAT;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// duration
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// float
	sType.exiType = VALUE_TYPE_FLOAT;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gDay
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gMonth
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gMonthDay
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gYear
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// gYearMonth
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// hex binary
	sType.exiType = VALUE_TYPE_BINARY;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Int
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// integer
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// language
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// long
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// negativeInteger
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.max = -1;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// NonNegativeInteger
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// NonPositiveInteger
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// normalizedString
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Positive Integer
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.max = 0;
	sType.min = 1;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// short
	sType.exiType = VALUE_TYPE_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.max = 32767;
	sType.min = -32768;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// String
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// time
	sType.exiType = VALUE_TYPE_DATE_TIME;
	sType.facetPresenceMask = 0;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// token
	sType.exiType = VALUE_TYPE_STRING;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned byte
	sType.exiType = VALUE_TYPE_SMALL_INTEGER;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.max = 255;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned int
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned Long
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Unsigned short
	sType.exiType = VALUE_TYPE_NON_NEGATIVE_INT;
	sType.facetPresenceMask = 0;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_NAMED_SUBTYPE_UNION;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
	sType.facetPresenceMask = sType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
	sType.max = 65535;
	sType.min = 0;
	sType.length = 0;
	tmp_err_code = addDynEntry(&simpleTypeTable->dynArray, &sType, &elID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}
