/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file EXIParser.c
 * @brief Implementation of a parser of EXI streams
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "EXIParser.h"
#include "procTypes.h"
#include "headerDecode.h"
#include "memManagement.h"
#include "bodyDecode.h"
#include "sTables.h"
#include "grammars.h"
#include "initSchemaInstance.h"

errorCode initParser(Parser* parser, BinaryBuffer buffer, EXIPSchema* schema, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	tmp_err_code = initAllocList(&parser->strm.memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	parser->strm.buffer = buffer;
	parser->strm.context.bitPointer = 0;
	parser->strm.context.bufferIndx = 0;
	parser->strm.context.currNonTermID = GR_DOC_CONTENT;
	parser->strm.context.currElem.lnId = 0;
	parser->strm.context.currElem.uriId = 0;
	parser->strm.context.currAttr.lnId = 0;
	parser->strm.context.currAttr.uriId = 0;
	parser->strm.context.expectATData = FALSE;
	parser->strm.context.isNilType = FALSE;
	parser->strm.context.attrTypeId = INDEX_MAX;
	parser->strm.gStack = NULL;
	parser->strm.valueTable.value = NULL;
	parser->app_data = app_data;
	parser->strm.schema = schema;
    makeDefaultOpts(&parser->strm.header.opts);

	initContentHandler(&parser->handler);

#if HASH_TABLE_USE == ON
	parser->strm.valueTable.hashTbl = NULL;
#endif

	return ERR_OK;
}

errorCode parseHeader(Parser* parser, boolean outOfBandOpts)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = decodeHeader(&parser->strm, outOfBandOpts);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	parser->strm.gStack = NULL;

	if(parser->strm.schema != NULL)
	{
		/* Schema enabled mode*/
		if(WITH_FRAGMENT(parser->strm.header.opts.enumOpt))
		{
			/* Fragment document grammar */
			// TODO: create a Schema-informed Fragment Grammar from the EXIP schema object
			return NOT_IMPLEMENTED_YET;
		}
	}
	else
	{
		parser->strm.schema = memManagedAllocate(&parser->strm.memList, sizeof(EXIPSchema));
		if(parser->strm.schema == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_err_code = initSchema(parser->strm.schema, INIT_SCHEMA_SCHEMA_LESS_MODE);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(WITH_FRAGMENT(parser->strm.header.opts.enumOpt))
		{
			tmp_err_code = createFragmentGrammar(parser->strm.schema, NULL, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			tmp_err_code = createDocGrammar(parser->strm.schema, NULL, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	tmp_err_code = pushGrammar(&parser->strm.gStack, &parser->strm.schema->docGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(parser->strm.header.opts.valuePartitionCapacity > 0)
	{
		tmp_err_code = createValueTable(&parser->strm.valueTable);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	// The parsing of the header is successful
	// TODO: Consider removing the startDocument all together instead of invoking it always here?
	if(parser->handler.startDocument != NULL)
	{
		if(parser->handler.startDocument(parser->app_data) == EXIP_HANDLER_STOP)
			return HANDLER_STOP_RECEIVED;
	}

	return ERR_OK;
}

errorCode parseNext(Parser* parser)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SmallIndex tmpNonTermID = GR_VOID_NON_TERMINAL;

	tmp_err_code = processNextProduction(&parser->strm, &tmpNonTermID, &parser->handler, parser->app_data);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmpNonTermID == GR_VOID_NON_TERMINAL)
	{
		EXIGrammar* grammar;
		popGrammar(&(parser->strm.gStack), &grammar);
		if(parser->strm.gStack == NULL) // There is no more grammars in the stack
		{
			parser->strm.context.currNonTermID = GR_VOID_NON_TERMINAL; // The stream is parsed
		}
		else
		{
			parser->strm.context.currNonTermID = parser->strm.gStack->lastNonTermID;
		}
	}
	else
	{
		parser->strm.context.currNonTermID = tmpNonTermID;
	}

	if(parser->strm.context.currNonTermID == GR_VOID_NON_TERMINAL)
		return PARSING_COMPLETE;

	return ERR_OK;
}

void destroyParser(Parser* parser)
{
	EXIGrammar* tmp;
	while(parser->strm.gStack != NULL)
	{
		popGrammar(&parser->strm.gStack, &tmp);
	}

	freeAllMem(&parser->strm);
}
