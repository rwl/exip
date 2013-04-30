/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file initSchemaInstance.h
 * @brief Initialization functions for the EXIPSchema object
 *
 * @date Nov 28, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef INITSCHEMAINSTANCE_H_
#define INITSCHEMAINSTANCE_H_

#include "errorHandle.h"
#include "procTypes.h"
#include "dynamicArray.h"
#include "memManagement.h"

/* Three different levels of EXIPSchema object initialization */
enum InitSchemaType
{
	/** Only the string tables and the SchemaGrammarTable are used */
	INIT_SCHEMA_SCHEMA_LESS_MODE = 0,
	/**
	 * When the value of the "schemaId" element is empty, no user defined schema information
	 * is used for processing the EXI body; however, the built-in XML schema types are available
	 * for use in the EXI body
	 */
	INIT_SCHEMA_BUILD_IN_TYPES   = 1,
	/** Full schema information with schema-informed grammars */
	INIT_SCHEMA_SCHEMA_ENABLED   = 2
};

typedef enum InitSchemaType InitSchemaType;

/**
 * @brief Generate a Schema-informed Type and TypeEmpty Grammars for all built-in XML Schema Types
 * 
 * It is used by generateSchemaInformedGrammars() and when the value of the "schemaId" element is empty,
 * no user defined schema information is used for processing the EXI body; however, the built-in XML schema
 * types are available for use in the EXI body.
 *
 * @param[in, out] schema schema for which the grammars are generated
 * @return Error handling code
 */
errorCode generateBuiltInTypesGrammars(EXIPSchema* schema);


/**
 * @brief Populate initial simple type array with the build-in simple types
 *
 * @param[in, out] simpleTypeTable Dynamic array storing the simple types definitions
 * @param[in, out] memList memory allocations
 * @return Error handling code
 */
errorCode createBuiltInTypesDefinitions(SimpleTypeTable* simpleTypeTable, AllocList* memList);

/**
 * @brief Initialize a EXIPSchema object
 * 
 * Generate all build in Schema-informed Element and Type Grammars when not in
 * schema-less mode. It is also used during grammar generation.
 *
 * @param[in, out] schema a resulting EXIPSchema container
 * @param[in] initializationType one of INIT_SCHEMA_SCHEMA_LESS_MODE, INIT_SCHEMA_BUILD_IN_TYPES or
 * INIT_SCHEMA_SCHEMA_ENABLED
 * @return Error handling code
 */
errorCode initSchema(EXIPSchema* schema, InitSchemaType initializationType);

#endif /* INITSCHEMAINSTANCE_H_ */
