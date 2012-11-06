/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file schemaOutputUtils.h
 * @brief Utility functions for storing EXIPSchema instances to external streams
 * @date May 7, 2012
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef SCHEMABUILDUTILS_H_
#define SCHEMABUILDUTILS_H_

#include "procTypes.h"

// Maximum number of characters in a variable name buffer
#define VAR_BUFFER_MAX_LENGTH 200

typedef struct
{
	char buf1[20];
	char buf2[20];
	char buf3[20];
	char buf4[20];
} IndexStrings;

/** TEXT OUTPUT DEFINITIONS */

/**
 * @brief Builds an EXI grammar text representation and stores it in out
 * @param[in] qnameid the qnameid of the EXI grammar
 * @param[in] grIndex index in the schema grammar table
 * @param[in] gr EXI grammar to be stored
 * @param[in] schema the EXIPSchema instance
 * @param[out] out output stream
 * @return Error handling code
 */
errorCode textGrammarOutput(QNameID qnameid, Index grIndex, EXIGrammar* gr, EXIPSchema* schema, FILE* out);


/** STATIC CODE OUTPUT DEFINITIONS */

/**
 * @brief Builds a static code representation of a CharType* array of a string
 * @param[in] str the String instance
 * @param[in] varName the name of the static code variable
 * @param[out] out output stream
 */
void staticStringDefOutput(String* str, char* varName, FILE* out);

/**
 * @brief Builds all the CharType* arrays of all strings in the string tables of a schema as a static code representation
 * @param[in] uriTbl the UriTable instance containing the string table entries
 * @param[in] prefix prefix for the definitions
 * @param[out] out output stream
 */
void staticStringTblDefsOutput(UriTable* uriTbl, char* prefix, FILE* out);

/**
 * @brief Builds an prefix table static code representation and stores it in out
 * @param[in] pfxTbl prefix string table to be stored
 * @param[in] prefix prefix for the definitions
 * @param[in] uriId uriId of the prefix
 * @param[out] out output stream
 */
void staticPrefixOutput(PfxTable* pfxTbl, char* prefix, Index uriId, FILE* out);

/**
 * @brief Builds all grammar productions for a grammar as a static code representation and stores it in out
 * @param[in] gr EXI grammar containing the productions to be stored
 * @param[in] prefix prefix for the definitions
 * @param[in] grId index of the grammar in the SchemmaGrammarTable
 * @param[out] out output stream
 */
void staticProductionsOutput(EXIGrammar* gr, char* prefix, Index grId, FILE* out);

/**
 * @brief Builds all grammar rules for a grammar as a static code representation and stores it in out
 * @param[in] gr EXI grammar containing the productions to be stored
 * @param[in] prefix prefix for the definitions
 * @param[in] grId index of the grammar in the SchemmaGrammarTable
 * @param[out] out output stream
 */
void staticRulesOutput(EXIGrammar* gr, char* prefix, Index grId, FILE* out);

/**
 * @brief Builds the document grammar as a static code representation and stores it in out
 * @param[in] docGr document grammar from the EXIPSchema object
 * @param[in] prefix prefix for the definitions
 * @param[out] out output stream
 */
void staticDocGrammarOutput(EXIGrammar* docGr, char* prefix, FILE* out);

/**
 * @brief Builds all the LN entries in a LnTable as a static code representation and stores it in out
 * @param[in] lnTbl the LnTable instance
 * @param[in] prefix prefix for the definitions
 * @param[in] uriId uriId for that LnTable
 * @param[out] out output stream
 */
void staticLnEntriesOutput(LnTable* lnTbl, char* prefix, Index uriId, FILE* out);

/**
 * @brief Builds all the URI entries in the UriTable as a static code representation and stores it in out
 * @param[in] uriTbl the UriTable instance
 * @param[in] prefix prefix for the definitions
 * @param[out] out output stream
 */
void staticUriTableOutput(UriTable* uriTbl, char* prefix, FILE* out);

/**
 * @brief Builds all the Enumeration definitions
 * @param[in] schema EXISchema instance
 * @param[in] prefix prefix for the definitions
 * @param[out] out output stream
 */
void staticEnumTableOutput(EXIPSchema* schema, char* prefix, FILE* out);


/** DYNAMIC CODE OUTPUT DEFINITIONS */

/**
 * @brief Builds an EXISchema dynamic code representation and stores it in out
 * Creates a code that returns a function definition that dynamically
 * creates EXISchema instances
 *
 * @param[in] schema EXISchema instance to be stored
 * @param[in] prefix prefix for the definitions
 * @param[in] mask_specified whether options and EXI header parameters are set
 * @param[in] mask_strict value of strict option
 * @param[in] mask_sc value of self contained option
 * @param[in] mask_preserve value of preserve option
 * @param[out] out output stream
 * @return Error handling code
 */
errorCode dynExipSchemaOutput(EXIPSchema* schema, char* prefix, unsigned char mask_specified,
		unsigned char mask_strict, unsigned char mask_sc, unsigned char mask_preserve, FILE* out);

#endif /* SCHEMABUILDUTILS_H_ */
