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
#include "grammars.h"

#ifndef DEFAULT_GRAMMAR_TABLE
# define DEFAULT_GRAMMAR_TABLE         300
#endif

#ifndef DEFAULT_SIMPLE_GRAMMAR_TABLE
# define DEFAULT_SIMPLE_GRAMMAR_TABLE   75
#endif

#ifndef DEFAULT_ENUM_TABLE
# define DEFAULT_ENUM_TABLE              5
#endif

errorCode initSchema(EXIPSchema* schema, InitSchemaType initializationType)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	TRY(initAllocList(&schema->memList));

	schema->staticGrCount = 0;
	SET_CONTENT_INDEX(schema->docGrammar.props, 0);
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
	TRY_CATCH(createDynArray(&schema->uriTable.dynArray, sizeof(UriEntry), DEFAULT_URI_ENTRIES_NUMBER), freeAllocList(&schema->memList));
	TRY_CATCH(createUriTableEntries(&schema->uriTable, initializationType != INIT_SCHEMA_SCHEMA_LESS_MODE), freeAllocList(&schema->memList));

	if(initializationType == INIT_SCHEMA_SCHEMA_ENABLED)
	{
		/* Create and initialize enumDef table */
		TRY_CATCH(createDynArray(&schema->enumTable.dynArray, sizeof(EnumDefinition), DEFAULT_ENUM_TABLE), freeAllocList(&schema->memList));
	}

	/* Create the schema grammar table */
	TRY_CATCH(createDynArray(&schema->grammarTable.dynArray, sizeof(EXIGrammar), DEFAULT_GRAMMAR_TABLE), freeAllocList(&schema->memList));

	if(initializationType != INIT_SCHEMA_SCHEMA_LESS_MODE)
	{
		/* Create and initialize simple type table */
		TRY_CATCH(createDynArray(&schema->simpleTypeTable.dynArray, sizeof(SimpleType), DEFAULT_SIMPLE_GRAMMAR_TABLE), freeAllocList(&schema->memList));
		TRY_CATCH(createBuiltInTypesDefinitions(&schema->simpleTypeTable, &schema->memList), freeAllocList(&schema->memList));

		// Must be done after createBuiltInTypesDefinitions()
		TRY_CATCH(generateBuiltInTypesGrammars(schema), freeAllocList(&schema->memList));

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
		typeQnameID.lnId = i;
		typeId = typeQnameID.lnId;

		grammar.props = 0;
		SET_SCHEMA_GR(grammar.props);
		if(HAS_TYPE_FACET(schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_NAMED_SUBTYPE_UNION))
			SET_NAMED_SUB_TYPE_OR_UNION(grammar.props);

		grammar.rule = (GrammarRule*) memManagedAllocate(&schema->memList, sizeof(GrammarRule)*(grammar.count));
		if(grammar.rule == NULL)
			return MEMORY_ALLOCATION_ERROR;

		if(typeId == SIMPLE_TYPE_ANY_TYPE)
		{
			// <xs:anyType> - The base complex type; complex ur-type
			SET_CONTENT_INDEX(grammar.props, 1);

			grammar.rule[0].production = memManagedAllocate(&schema->memList, sizeof(Production)*4);
			if(grammar.rule[0].production == NULL)
				return MEMORY_ALLOCATION_ERROR;

			SET_PROD_EXI_EVENT(grammar.rule[0].production[3].content, EVENT_AT_ALL);
			SET_PROD_NON_TERM(grammar.rule[0].production[3].content, 0);
			grammar.rule[0].production[3].typeId = INDEX_MAX;
			grammar.rule[0].production[3].qnameId.uriId = URI_MAX;
			grammar.rule[0].production[3].qnameId.lnId = LN_MAX;

			SET_PROD_EXI_EVENT(grammar.rule[0].production[2].content, EVENT_SE_ALL);
			SET_PROD_NON_TERM(grammar.rule[0].production[2].content, 1);
			grammar.rule[0].production[2].typeId = INDEX_MAX;
			grammar.rule[0].production[2].qnameId.uriId = URI_MAX;
			grammar.rule[0].production[2].qnameId.lnId = LN_MAX;

			SET_PROD_EXI_EVENT(grammar.rule[0].production[1].content, EVENT_EE);
			SET_PROD_NON_TERM(grammar.rule[0].production[1].content, GR_VOID_NON_TERMINAL);
			grammar.rule[0].production[1].typeId = INDEX_MAX;
			grammar.rule[0].production[1].qnameId.uriId = URI_MAX;
			grammar.rule[0].production[1].qnameId.lnId = LN_MAX;

			SET_PROD_EXI_EVENT(grammar.rule[0].production[0].content, EVENT_CH);
			SET_PROD_NON_TERM(grammar.rule[0].production[0].content, 1);
			grammar.rule[0].production[0].typeId = INDEX_MAX;
			grammar.rule[0].production[0].qnameId.uriId = URI_MAX;
			grammar.rule[0].production[0].qnameId.lnId = LN_MAX;

			grammar.rule[0].pCount = 4;
			grammar.rule[0].meta = 0;
			RULE_SET_CONTAIN_EE(grammar.rule[0].meta);

			grammar.rule[1].production = memManagedAllocate(&schema->memList, sizeof(Production)*3);
			if(grammar.rule[1].production == NULL)
				return MEMORY_ALLOCATION_ERROR;

			SET_PROD_EXI_EVENT(grammar.rule[1].production[2].content, EVENT_SE_ALL);
			SET_PROD_NON_TERM(grammar.rule[1].production[2].content, 1);
			grammar.rule[1].production[2].typeId = INDEX_MAX;
			grammar.rule[1].production[2].qnameId.uriId = URI_MAX;
			grammar.rule[1].production[2].qnameId.lnId = LN_MAX;

			SET_PROD_EXI_EVENT(grammar.rule[1].production[1].content, EVENT_EE);
			SET_PROD_NON_TERM(grammar.rule[1].production[1].content, GR_VOID_NON_TERMINAL);
			grammar.rule[1].production[1].typeId = INDEX_MAX;
			grammar.rule[1].production[1].qnameId.uriId = URI_MAX;
			grammar.rule[1].production[1].qnameId.lnId = LN_MAX;

			SET_PROD_EXI_EVENT(grammar.rule[1].production[0].content, EVENT_CH);
			SET_PROD_NON_TERM(grammar.rule[1].production[0].content, 1);
			grammar.rule[1].production[0].typeId = INDEX_MAX;
			grammar.rule[1].production[0].qnameId.uriId = URI_MAX;
			grammar.rule[1].production[0].qnameId.lnId = LN_MAX;

			grammar.rule[1].pCount = 3;
			grammar.rule[1].meta = 0;
			RULE_SET_CONTAIN_EE(grammar.rule[1].meta);
		}
		else // a regular simple type
		{
			grammar.rule[0].production = memManagedAllocate(&schema->memList, sizeof(Production));
			if(grammar.rule[0].production == NULL)
				return MEMORY_ALLOCATION_ERROR;

			SET_PROD_EXI_EVENT(grammar.rule[0].production[0].content, EVENT_CH);
			SET_PROD_NON_TERM(grammar.rule[0].production[0].content, 1);
			grammar.rule[0].production[0].typeId = typeId;
			grammar.rule[0].production[0].qnameId.uriId = URI_MAX;
			grammar.rule[0].production[0].qnameId.lnId = LN_MAX;
			grammar.rule[0].pCount = 1;
			grammar.rule[0].meta = 0;

			grammar.rule[1].production = memManagedAllocate(&schema->memList, sizeof(Production));
			if(grammar.rule[1].production == NULL)
				return MEMORY_ALLOCATION_ERROR;

			SET_PROD_EXI_EVENT(grammar.rule[1].production[0].content, EVENT_EE);
			SET_PROD_NON_TERM(grammar.rule[1].production[0].content, GR_VOID_NON_TERMINAL);
			grammar.rule[1].production[0].typeId = INDEX_MAX;
			grammar.rule[1].production[0].qnameId.uriId = URI_MAX;
			grammar.rule[1].production[0].qnameId.lnId = LN_MAX;
			grammar.rule[1].pCount = 1;
			grammar.rule[1].meta = 0;
			RULE_SET_CONTAIN_EE(grammar.rule[1].meta);
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
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_LIST);
	sType.max = 0;
	sType.min = 0;
	sType.length = SIMPLE_TYPE_ENTITY;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// entity
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// id
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// idref
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// idrefs
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_LIST);
	sType.max = 0;
	sType.min = 0;
	sType.length = SIMPLE_TYPE_IDREF;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// ncname
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// nmtoken
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// nmtokens
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_LIST);
	sType.max = 0;
	sType.min = 0;
	sType.length = SIMPLE_TYPE_NMTOKEN;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// notation
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// name
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// qname
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// any simple type
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// any type
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_NONE);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// any uri
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// base64 binary
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_BINARY);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// boolean
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_BOOLEAN);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// byte
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_SMALL_INTEGER);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MAX_INCLUSIVE);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MIN_INCLUSIVE);
	sType.max = 127;
	sType.min = -128;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// date
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DATE_TIME);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// date time
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DATE_TIME);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// decimal
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DECIMAL);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// double
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_FLOAT);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// duration
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// float
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_FLOAT);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// gDay
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DATE_TIME);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// gMonth
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DATE_TIME);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// gMonthDay
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DATE_TIME);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// gYear
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DATE_TIME);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// gYearMonth
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DATE_TIME);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// hex binary
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_BINARY);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// Int
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_INTEGER);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// integer
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_INTEGER);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// language
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// long
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_INTEGER);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// negativeInteger
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_INTEGER);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MAX_INCLUSIVE);
	sType.max = -1;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// NonNegativeInteger
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_NON_NEGATIVE_INT);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MIN_INCLUSIVE);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// NonPositiveInteger
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_INTEGER);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MAX_INCLUSIVE);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// normalizedString
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// Positive Integer
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_NON_NEGATIVE_INT);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MIN_INCLUSIVE);
	sType.max = 0;
	sType.min = 1;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// short
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_INTEGER);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MAX_INCLUSIVE);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MIN_INCLUSIVE);
	sType.max = 32767;
	sType.min = -32768;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// String
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// time
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_DATE_TIME);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// token
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_STRING);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// Unsigned byte
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_SMALL_INTEGER);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MAX_INCLUSIVE);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MIN_INCLUSIVE);
	sType.max = 255;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// Unsigned int
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_NON_NEGATIVE_INT);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MIN_INCLUSIVE);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// Unsigned Long
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_NON_NEGATIVE_INT);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MIN_INCLUSIVE);
	sType.max = 0;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	// Unsigned short
	sType.content = 0;
	SET_EXI_TYPE(sType.content, VALUE_TYPE_NON_NEGATIVE_INT);
	SET_TYPE_FACET(sType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MAX_INCLUSIVE);
	SET_TYPE_FACET(sType.content, TYPE_FACET_MIN_INCLUSIVE);
	sType.max = 65535;
	sType.min = 0;
	sType.length = 0;
	TRY(addDynEntry(&simpleTypeTable->dynArray, &sType, &elID));

	return ERR_OK;
}
