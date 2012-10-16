/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammars.h
 * @brief Types and functions describing EXI grammars
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef GRAMMARS_H_
#define GRAMMARS_H_

#include "errorHandle.h"
#include "grammarRules.h"
#include "eventsEXI.h"
#include "procTypes.h"
#include "contentHandler.h"

/** The numbers of the grammars corresponding to the build-in XML Schema types */
#define BUILD_IN_GRAMMARS_COUNT 46

/**
 * Get global element EXIGrammar from the SchemaGrammarTable by given QNameID.
 * Returns NULL if the grammar does not exists in the SchemaGrammarTable
 * (i.e. the index of the grammar in the string table is INDEX_MAX)
 */
#define GET_ELEM_GRAMMAR_QNAMEID(schema, qnameID) GET_LN_URI_QNAME((schema)->uriTable, qnameID).elemGrammar == INDEX_MAX?NULL:&((schema)->grammarTable.grammar[GET_LN_URI_QNAME((schema)->uriTable, qnameID).elemGrammar])

/**
 * Get global type EXIGrammar from the SchemaGrammarTable by given QNameID.
 * Returns NULL if the grammar does not exists in the SchemaGrammarTable
 * (i.e. the index of the grammar in the string table is INDEX_MAX)
 */
#define GET_TYPE_GRAMMAR_QNAMEID(schema, qnameID) GET_LN_URI_QNAME((schema)->uriTable, qnameID).typeGrammar == INDEX_MAX?NULL:&((schema)->grammarTable.grammar[GET_LN_URI_QNAME((schema)->uriTable, qnameID).typeGrammar])

/**
 * @brief Process the next grammar production in the Current Grammar
 * 
 * Returns the terminal symbol of the production i.e. the EXI Event Type;
 * @param[in] strm EXI stream of bits
 * @param[out] nonTermID_out unique identifier of right-hand side Non-terminal
 * @param[in] handler content handler callbacks
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode processNextProduction(EXIStream* strm, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data);

/**
 * @brief Push a grammar on top of the Grammar Stack
 * 
 * @param[in, out] gStack the Grammar Stack
 * @param[in] grammar a grammar
 * @return Error handling code
 */
errorCode pushGrammar(EXIGrammarStack** gStack, EXIGrammar* grammar);

/**
 * @brief Pop a grammar off the top of the Grammar Stack
 * 
 * @param[in, out] gStack the Grammar stack
 * @param[out] grammar the popped out grammar
 */
void popGrammar(EXIGrammarStack** gStack, EXIGrammar** grammar);

/**
 * @brief Creates an instance of the EXI Built-in Document Grammar or Schema-Informed Document Grammar
 * 
 * If elQnameArr is NULL then it creates EXI Built-in Document Grammar, otherwise
 * it creates Schema-Informed Document Grammar with global elements stored (sorted) in elQnameArr
 *
 * @param[in, out] schema the schema describing the document
 * @param[in] elQnameArr array of QNameIds of global element definitions if any; NULL otherwise (schema-less mode)
 * @param[in] qnameCount the number of global element definitions if any; 0 otherwise
 * @return Error handling code
 */
errorCode createDocGrammar(EXIPSchema* schema, QNameID* elQnameArr, Index qnameCount);

/**
 * @brief Creates an instance of the EXI Built-in Fragment Grammar or Schema-Informed Fragment Grammar
 * 
 * If elQnameArr is NULL then it creates EXI Built-in Fragment Grammar, otherwise
 * it creates Schema-Informed Fragment Grammar
 *
 * @param[in, out] schema the schema describing the document
 * @param[in] elQnameArr array of QNameIds of element definitions if any; NULL otherwise (schema-less mode)
 * @param[in] qnameCount the number of global element definitions if any; 0 otherwise
 * @return Error handling code
 */
errorCode createFragmentGrammar(EXIPSchema* schema, QNameID* elQnameArr, Index qnameCount);

/**
 * @brief Creates an instance of EXI Built-in Element Grammar
 * 
 * @param[in] elementGrammar empty grammar container
 * @param[in, out] strm EXI stream for which the allocation is made
 * @return Error handling code
 */
errorCode createBuiltInElementGrammar(EXIGrammar* elementGrammar, EXIStream* strm);

#endif /* GRAMMARS_H_ */
