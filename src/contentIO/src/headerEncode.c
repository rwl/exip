/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file headerEncode.c
 * @brief Implementing the interface of EXI header encoder
 *
 * @date Aug 23, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "headerEncode.h"
#include "streamWrite.h"
#include "memManagement.h"
#include "grammars.h"
#include "sTables.h"
#include "EXISerializer.h"
#include "stringManipulate.h"
#include "bodyEncode.h"

/** This is the statically generated EXIP schema definition for the EXI Options document*/
extern const EXIPSchema ops_schema;

static void closeOptionsStream(EXIStream* strm);

errorCode encodeHeader(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start EXI header encoding\n"));

	if(strm->header.has_cookie)
	{
		tmp_err_code = writeNBits(strm, 8, 36); // ASCII code for $ = 00100100  (36)
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = writeNBits(strm, 8, 69); // ASCII code for E = 01000101  (69)
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = writeNBits(strm, 8, 88); // ASCII code for X = 01011000  (88)
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = writeNBits(strm, 8, 73); // ASCII code for I = 01001001  (73)
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encoding the header Distinguishing Bits\n"));
	tmp_err_code = writeNBits(strm, 2, 2);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Write the Presence Bit for EXI Options\n"));
	tmp_err_code = writeNextBit(strm, strm->header.has_options);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encode EXI version\n"));
	tmp_err_code = writeNextBit(strm, strm->header.is_preview_version);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->header.version_number > 15)
	{
		tmp_err_code = writeNBits(strm, 4, 15);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = writeNBits(strm, 4, strm->header.version_number - 15 - 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		tmp_err_code = writeNBits(strm, 4, strm->header.version_number - 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encode EXI options\n"));
	if(strm->header.has_options)
	{
		EXIStream options_strm;
		unsigned char hasUncommon = FALSE;
		unsigned char hasLesscommon = FALSE;
		unsigned char hasCommon = FALSE;

		makeDefaultOpts(&options_strm.header.opts);
		SET_STRICT(options_strm.header.opts.enumOpt);
		tmp_err_code = initAllocList(&options_strm.memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		options_strm.buffer = strm->buffer;
		options_strm.context.bitPointer = strm->context.bitPointer;
		options_strm.context.bufferIndx = strm->context.bufferIndx;
		options_strm.context.currNonTermID = GR_DOCUMENT;
		options_strm.context.currElem.lnId = LN_MAX;
		options_strm.context.currElem.uriId = URI_MAX;
		options_strm.context.currAttr.lnId = LN_MAX;
		options_strm.context.currAttr.uriId = URI_MAX;
		options_strm.context.expectATData = 0;
		options_strm.context.attrTypeId = 0;
		options_strm.gStack = NULL;
		options_strm.schema = (EXIPSchema*) &ops_schema;

		tmp_err_code = createValueTable(&options_strm.valueTable);
		if(tmp_err_code != ERR_OK)
		{
			closeOptionsStream(&options_strm);
			return tmp_err_code;
		}

		tmp_err_code = pushGrammar(&options_strm.gStack, (EXIGrammar*) &ops_schema.docGrammar);
		if(tmp_err_code != ERR_OK)
		{
			closeOptionsStream(&options_strm);
			return tmp_err_code;
		}

		// TODO: Below, provide a checks for tmp_err_code and stop the execution on an error condition

		tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.startDocument
		tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.startElement <header>

		// uncommon options
		if(GET_ALIGNMENT(strm->header.opts.enumOpt) != BIT_PACKED ||
				WITH_SELF_CONTAINED(strm->header.opts.enumOpt) ||
				strm->header.opts.valueMaxLength != INDEX_MAX ||
				strm->header.opts.valuePartitionCapacity != INDEX_MAX ||
				strm->header.opts.drMap != NULL)
		{
			hasUncommon = TRUE;
			hasLesscommon = TRUE;
		}
		else if(strm->header.opts.preserve != 0 || strm->header.opts.blockSize != 1000000)
		{
			// lesscommon options
			hasLesscommon = TRUE;
		}

		if(hasLesscommon)
		{
			int nonTermAdj;
			tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.startElement <lesscommon>
			if(hasUncommon)
			{
				tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.startElement <uncommon>
				if(GET_ALIGNMENT(strm->header.opts.enumOpt) != BIT_PACKED)
				{
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.startElement <alignment>
					if(GET_ALIGNMENT(strm->header.opts.enumOpt) == BYTE_ALIGNMENT)
					{
						tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.startElement <byte>
					}
					else
					{
						tmp_err_code += serializeEvent(&options_strm, 1, 1, NULL); // serialize.startElement <pre-compress>
					}
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <byte> or <pre-compress>
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <alignment>
				}
				if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
				{
					nonTermAdj = options_strm.context.currNonTermID - 1;
					if(nonTermAdj < 0)
						nonTermAdj = 0;
					tmp_err_code += serializeEvent(&options_strm, 1, 1 - nonTermAdj, NULL); // serialize.startElement <selfContained>
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <selfContained>
				}
				if(strm->header.opts.valueMaxLength != INDEX_MAX)
				{
					nonTermAdj = options_strm.context.currNonTermID - 1;
					if(nonTermAdj < 0)
						nonTermAdj = 0;
					tmp_err_code += serializeEvent(&options_strm, 1, 2 - nonTermAdj, NULL); // serialize.startElement <valueMaxLength>
					tmp_err_code += serialize.intData(&options_strm, strm->header.opts.valueMaxLength);
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <valueMaxLength>
				}
				if(strm->header.opts.valuePartitionCapacity != INDEX_MAX)
				{
					nonTermAdj = options_strm.context.currNonTermID - 1;
					if(nonTermAdj < 0)
						nonTermAdj = 0;
					tmp_err_code += serializeEvent(&options_strm, 1, 3 - nonTermAdj, NULL); // serialize.startElement <valuePartitionCapacity>
					tmp_err_code += serialize.intData(&options_strm, strm->header.opts.valuePartitionCapacity);
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <valuePartitionCapacity>
				}
				if(strm->header.opts.drMap != NULL)
				{
					nonTermAdj = options_strm.context.currNonTermID - 1;
					if(nonTermAdj < 0)
						nonTermAdj = 0;
					tmp_err_code += serializeEvent(&options_strm, 1, 4 - nonTermAdj, NULL); // serialize.startElement <datatypeRepresentationMap>
					// TODO: not ready yet!
					return NOT_IMPLEMENTED_YET;
				}
				tmp_err_code += serializeEvent(&options_strm, 1, 6 - options_strm.context.currNonTermID, NULL); // serialize.endElement <uncommon>
			}
			if(strm->header.opts.preserve != 0)
			{
				tmp_err_code += serializeEvent(&options_strm, 1, 1 - options_strm.context.currNonTermID, NULL); // serialize.startElement <preserve>
				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
				{
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.startElement <dtd>
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <dtd>
				}
				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
				{
					tmp_err_code += serializeEvent(&options_strm, 1, 1 - options_strm.context.currNonTermID, NULL); // serialize.startElement <prefixes>
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <prefixes>
				}
				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_LEXVALUES))
				{
					tmp_err_code += serializeEvent(&options_strm, 1, 2 - options_strm.context.currNonTermID, NULL); // serialize.startElement <lexicalValues>
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <lexicalValues>
				}
				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
				{
					tmp_err_code += serializeEvent(&options_strm, 1, 3 - options_strm.context.currNonTermID, NULL); // serialize.startElement <comments>
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <comments>
				}
				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
				{
					tmp_err_code += serializeEvent(&options_strm, 1, 4 - options_strm.context.currNonTermID, NULL); // serialize.startElement <pis>
					tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <pis>
				}
				tmp_err_code += serializeEvent(&options_strm, 1, 5 - options_strm.context.currNonTermID, NULL); // serialize.endElement <preserve>
			}
			if(strm->header.opts.blockSize != 1000000)
			{
				tmp_err_code += serializeEvent(&options_strm, 1, 2 - options_strm.context.currNonTermID, NULL); // serialize.startElement <blockSize>
				tmp_err_code += serialize.intData(&options_strm, strm->header.opts.blockSize);
				tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <blockSize>
			}
			tmp_err_code += serializeEvent(&options_strm, 1, 3 - options_strm.context.currNonTermID, NULL); // serialize.endElement <lesscommon>
		}

		// common options if any...
		if(WITH_COMPRESSION(strm->header.opts.enumOpt) || WITH_FRAGMENT(strm->header.opts.enumOpt) || !isStringEmpty(&strm->header.opts.schemaID))
		{
			hasCommon = TRUE;
		}

		if(hasCommon)
		{
			tmp_err_code += serializeEvent(&options_strm, 1, 1 - options_strm.context.currNonTermID, NULL); // serialize.startElement <common>
			if(WITH_COMPRESSION(strm->header.opts.enumOpt))
			{
				tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.startElement <compression>
				tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <compression>
			}
			if(WITH_FRAGMENT(strm->header.opts.enumOpt))
			{
				tmp_err_code += serializeEvent(&options_strm, 1, 1 - options_strm.context.currNonTermID, NULL); // serialize.startElement <fragment>
				tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <fragment>
			}
			if(strm->header.opts.schemaID.length != 0) // SchemaID modes are encoded in the length part
			{
				tmp_err_code += serializeEvent(&options_strm, 1, 2 - options_strm.context.currNonTermID, NULL); // serialize.startElement <schemaId>
				if(strm->header.opts.schemaID.str != NULL)
				{
					tmp_err_code += serialize.stringData(&options_strm, strm->header.opts.schemaID);
				}
				else if(strm->header.opts.schemaID.length == SCHEMA_ID_NIL)
				{
					QName nil;
					nil.uri = &strm->schema->uriTable.uri[2].uriStr;
					nil.localName = &strm->schema->uriTable.uri[2].lnTable.ln[0].lnStr;
					nil.prefix = &strm->schema->uriTable.uri[2].pfxTable->pfxStr[0];
					tmp_err_code += serializeEvent(&options_strm, 2, 0, &nil); // serialize.attribute nil="true"
					tmp_err_code += serialize.booleanData(&options_strm, TRUE);
				}
				else if(strm->header.opts.schemaID.length == SCHEMA_ID_EMPTY)
				{
					String empty;
					getEmptyString(&empty);
					tmp_err_code += serialize.stringData(&options_strm, empty);
				}

				tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <schemaId>
			}
			tmp_err_code += serializeEvent(&options_strm, 1, 3 - options_strm.context.currNonTermID, NULL); // serialize.endElement <common>
		}

		if(WITH_STRICT(strm->header.opts.enumOpt))
		{
			tmp_err_code += serializeEvent(&options_strm, 1, 2 - options_strm.context.currNonTermID, NULL); // serialize.startElement <strict>
			tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endElement <strict>
		}

		tmp_err_code += serializeEvent(&options_strm, 1, 3 - options_strm.context.currNonTermID, NULL); // serialize.endElement <header>

		tmp_err_code += serializeEvent(&options_strm, 1, 0, NULL); // serialize.endDocument

		strm->buffer.bufContent = options_strm.buffer.bufContent;
		strm->context.bitPointer = options_strm.context.bitPointer;
		strm->context.bufferIndx = options_strm.context.bufferIndx;

		if(WITH_COMPRESSION(strm->header.opts.enumOpt) ||
				GET_ALIGNMENT(strm->header.opts.enumOpt) != BIT_PACKED)
		{
			// Padding bits
			if(strm->context.bitPointer != 0)
			{
				strm->context.bitPointer = 0;
				strm->context.bufferIndx += 1;
			}
		}

		closeOptionsStream(&options_strm);

		return tmp_err_code;
	}

	return ERR_OK;
}

static void closeOptionsStream(EXIStream* strm)
{
	EXIGrammar* tmp;
	while(strm->gStack != NULL)
	{
		popGrammar(&strm->gStack, &tmp);
	}
	freeAllMem(strm);
}
