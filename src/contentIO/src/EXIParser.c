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

errorCode initParser(Parser* parser, BinaryBuffer buffer, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	TRY(initAllocList(&parser->strm.memList));

	parser->strm.buffer = buffer;
	parser->strm.context.bitPointer = 0;
	parser->strm.context.bufferIndx = 0;
	parser->strm.context.currAttr.lnId = 0;
	parser->strm.context.currAttr.uriId = 0;
	parser->strm.context.expectATData = FALSE;
	parser->strm.context.isNilType = FALSE;
	parser->strm.context.attrTypeId = INDEX_MAX;
	parser->strm.gStack = NULL;
	parser->strm.valueTable.value = NULL;
	parser->strm.valueTable.count = 0;
	parser->app_data = app_data;
	parser->strm.schema = NULL;
    makeDefaultOpts(&parser->strm.header.opts);

	initContentHandler(&parser->handler);

#if HASH_TABLE_USE
	parser->strm.valueTable.hashTbl = NULL;
#endif

	return ERR_OK;
}

errorCode parseHeader(Parser* parser, boolean outOfBandOpts)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	TRY(decodeHeader(&parser->strm, outOfBandOpts));

	if(parser->strm.header.opts.valuePartitionCapacity > 0)
	{
		TRY(createValueTable(&parser->strm.valueTable));
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

errorCode setSchema(Parser* parser, EXIPSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(parser->strm.header.opts.schemaIDMode == SCHEMA_ID_NIL)
	{
		// When the "schemaId" element in the EXI options document contains the xsi:nil attribute
		// with its value set to true, no schema information is used for processing the EXI body
		// (i.e. a schema-less EXI stream)
		parser->strm.schema = NULL;
#if EXI_PROFILE_DEFAULT
		DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, ("\n> EXI Profile mode require schema mode processing"));
		return INVALID_EXI_INPUT;
#endif
#if DEBUG_CONTENT_IO == ON && EXIP_DEBUG_LEVEL <= WARNING
		if(schema != NULL)
			DEBUG_MSG(WARNING, DEBUG_CONTENT_IO, ("\n> Ignored out-of-band schema information. Schema-less mode required"));
#endif
	}
	else if(parser->strm.header.opts.schemaIDMode == SCHEMA_ID_EMPTY)
	{
		// When the value of the "schemaId" element is empty, no user defined schema information
		// is used for processing the EXI body; however, the built-in XML schema types are available for use in the EXI body
#if DEBUG_CONTENT_IO == ON && EXIP_DEBUG_LEVEL <= WARNING
		if(schema != NULL)
			DEBUG_MSG(WARNING, DEBUG_CONTENT_IO, ("\n> Ignored out-of-band schema information. Schema mode built-in types required"));
#endif
		parser->strm.schema = memManagedAllocate(&parser->strm.memList, sizeof(EXIPSchema));
		if(parser->strm.schema == NULL)
			return MEMORY_ALLOCATION_ERROR;

		TRY(initSchema(parser->strm.schema, INIT_SCHEMA_BUILD_IN_TYPES));

		if(WITH_FRAGMENT(parser->strm.header.opts.enumOpt))
		{
			TRY(createFragmentGrammar(parser->strm.schema, NULL, 0));
		}
		else
		{
			TRY(createDocGrammar(parser->strm.schema, NULL, 0));
		}
	}
	else if(schema != NULL)
	{
		/* Schema enabled mode*/
		if(WITH_FRAGMENT(parser->strm.header.opts.enumOpt))
		{
			/* Fragment document grammar */
			// TODO: create a Schema-informed Fragment Grammar from the EXIP schema object
			return NOT_IMPLEMENTED_YET;
		}
		else
		{
			parser->strm.schema = schema;
		}
	}

	if(parser->strm.schema == NULL)
	{
		// Schema-less mode
		if(parser->strm.header.opts.schemaIDMode == SCHEMA_ID_SET)
		{
			DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, ("\n> Schema mode required, but NULL schema set"));
			return INVALID_EXIP_CONFIGURATION;
		}

#if EXI_PROFILE_DEFAULT
		DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, ("\n> EXI Profile mode require schema mode processing"));
		return INVALID_EXI_INPUT;
#endif

		parser->strm.schema = memManagedAllocate(&parser->strm.memList, sizeof(EXIPSchema));
		if(parser->strm.schema == NULL)
			return MEMORY_ALLOCATION_ERROR;

		TRY(initSchema(parser->strm.schema, INIT_SCHEMA_SCHEMA_LESS_MODE));

		if(WITH_FRAGMENT(parser->strm.header.opts.enumOpt))
		{
			TRY(createFragmentGrammar(parser->strm.schema, NULL, 0));
		}
		else
		{
			TRY(createDocGrammar(parser->strm.schema, NULL, 0));
		}
	}

	TRY(pushGrammar(&parser->strm.gStack, &parser->strm.schema->docGrammar));

	return ERR_OK;
}

errorCode parseNext(Parser* parser)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SmallIndex tmpNonTermID = GR_VOID_NON_TERMINAL;

	TRY(processNextProduction(&parser->strm, &tmpNonTermID, &parser->handler, parser->app_data));

	if(tmpNonTermID == GR_VOID_NON_TERMINAL)
	{
		EXIGrammar* grammar;
		popGrammar(&(parser->strm.gStack), &grammar);
		if(parser->strm.gStack == NULL) // There is no more grammars in the stack
		{
			return PARSING_COMPLETE; // The stream is parsed
		}
	}
	else
	{
		parser->strm.gStack->currNonTermID = tmpNonTermID;
	}

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
