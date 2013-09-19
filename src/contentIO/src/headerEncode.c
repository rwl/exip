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
#include "ioUtil.h"
#include "streamEncode.h"

/** This is the statically generated EXIP schema definition for the EXI Options document*/
extern const EXIPSchema ops_schema;

static void closeOptionsStream(EXIStream* strm);
static errorCode serializeOptionsStream(EXIStream* options_strm, EXIOptions* opts, UriTable* uriTbl);

errorCode encodeHeader(EXIStream* strm)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start EXI header encoding\n"));

	if(strm->header.has_cookie)
	{
		TRY(writeNBits(strm, 8, 36)); // ASCII code for $ = 00100100  (36)
		TRY(writeNBits(strm, 8, 69)); // ASCII code for E = 01000101  (69)
		TRY(writeNBits(strm, 8, 88)); // ASCII code for X = 01011000  (88)
		TRY(writeNBits(strm, 8, 73)); // ASCII code for I = 01001001  (73)
	}

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encoding the header Distinguishing Bits\n"));
	TRY(writeNBits(strm, 2, 2));

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Write the Presence Bit for EXI Options\n"));
	TRY(writeNextBit(strm, strm->header.has_options));

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encode EXI version\n"));
	TRY(writeNextBit(strm, strm->header.is_preview_version));

	if(strm->header.version_number > 15)
	{
		TRY(writeNBits(strm, 4, 15));
		TRY(writeNBits(strm, 4, strm->header.version_number - 15 - 1));
	}
	else
	{
		TRY(writeNBits(strm, 4, strm->header.version_number - 1));
	}

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encode EXI options\n"));
	if(strm->header.has_options)
	{
		EXIStream options_strm;
		QNameID emptyQnameID = {URI_MAX, LN_MAX};

		makeDefaultOpts(&options_strm.header.opts);
		SET_STRICT(options_strm.header.opts.enumOpt);
		TRY(initAllocList(&options_strm.memList));

		options_strm.buffer = strm->buffer;
		options_strm.context.bitPointer = strm->context.bitPointer;
		options_strm.context.bufferIndx = strm->context.bufferIndx;
		options_strm.context.currAttr.lnId = LN_MAX;
		options_strm.context.currAttr.uriId = URI_MAX;
		options_strm.context.expectATData = FALSE;
		options_strm.context.isNilType = FALSE;
		options_strm.context.attrTypeId = 0;
		options_strm.gStack = NULL;
		options_strm.schema = (EXIPSchema*) &ops_schema;

		TRY_CATCH(createValueTable(&options_strm.valueTable), closeOptionsStream(&options_strm));
		TRY_CATCH(pushGrammar(&options_strm.gStack, emptyQnameID, (EXIGrammar*) &ops_schema.docGrammar), closeOptionsStream(&options_strm));
		TRY_CATCH(serializeOptionsStream(&options_strm, &strm->header.opts, &strm->schema->uriTable), closeOptionsStream(&options_strm));

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
	}

	return EXIP_OK;
}

static void closeOptionsStream(EXIStream* strm)
{
	while(strm->gStack != NULL)
	{
		popGrammar(&strm->gStack);
	}
	freeAllMem(strm);
}

static errorCode serializeOptionsStream(EXIStream* options_strm, EXIOptions* opts, UriTable* uriTbl)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	EventCode tmpEvCode;
	boolean hasUncommon = FALSE;
	boolean hasLesscommon = FALSE;
	boolean hasCommon = FALSE;
	unsigned int ruleContext = 0;

	// Implicit serialize.startDocument (0 bits)
	tmpEvCode.length = 1;
	tmpEvCode.part[0] = 0;
	tmpEvCode.bits[0] = 1;
	TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <header>

	// uncommon options
	if(GET_ALIGNMENT(opts->enumOpt) != BIT_PACKED ||
			WITH_SELF_CONTAINED(opts->enumOpt) ||
			opts->valueMaxLength != INDEX_MAX ||
			opts->valuePartitionCapacity != INDEX_MAX ||
			opts->drMap != NULL)
	{
		hasUncommon = TRUE;
		hasLesscommon = TRUE;
	}
	else if(opts->preserve != 0 || opts->blockSize != 1000000)
	{
		// lesscommon options
		hasLesscommon = TRUE;
	}

	if(hasLesscommon)
	{
		tmpEvCode.length = 1;
		tmpEvCode.part[0] = 0;
		tmpEvCode.bits[0] = 2;
		TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <lesscommon>
		if(hasUncommon)
		{
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 0;
			tmpEvCode.bits[0] = 2;
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <uncommon>
			ruleContext = 0;
			if(GET_ALIGNMENT(opts->enumOpt) != BIT_PACKED)
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 3;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <alignment>
				ruleContext = 1;
				if(GET_ALIGNMENT(opts->enumOpt) == BYTE_ALIGNMENT)
				{
					tmpEvCode.length = 1;
					tmpEvCode.part[0] = 0;
					tmpEvCode.bits[0] = 1;
					TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <byte>
				}
				else
				{
					tmpEvCode.length = 1;
					tmpEvCode.part[0] = 1;
					tmpEvCode.bits[0] = 1;
					TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <pre-compress>
				}
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <byte> or <pre-compress>
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <alignment>
			}
			if(WITH_SELF_CONTAINED(opts->enumOpt))
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 1 - ruleContext;
				tmpEvCode.bits[0] = 3;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <selfContained>
				ruleContext = 2;
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <selfContained>
			}
			if(opts->valueMaxLength != INDEX_MAX)
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 2 - ruleContext;
				tmpEvCode.bits[0] = 3 - (ruleContext == 2);
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <valueMaxLength>
				ruleContext = 3;
				TRY(serialize.intData(options_strm, opts->valueMaxLength));
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <valueMaxLength>
			}
			if(opts->valuePartitionCapacity != INDEX_MAX)
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 3 - ruleContext;
				tmpEvCode.bits[0] = 3 - (tmpEvCode.part[0] < 2);
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <valuePartitionCapacity>
				ruleContext = 4;
				TRY(serialize.intData(options_strm, opts->valuePartitionCapacity));
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <valuePartitionCapacity>
			}
			if(opts->drMap != NULL)
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 4 - ruleContext;
				tmpEvCode.bits[0] = 3 - (tmpEvCode.part[0] < 3) - (tmpEvCode.part[0] == 0);
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <datatypeRepresentationMap>
				ruleContext = 5;
				// TODO: not ready yet!
				return EXIP_NOT_IMPLEMENTED_YET;
			}
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 6 - ruleContext - (ruleContext > 0);
			tmpEvCode.bits[0] = getBitsNumber(tmpEvCode.part[0]);
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <uncommon>
		}
		if(opts->preserve != 0)
		{
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 1 - hasUncommon;
			tmpEvCode.bits[0] = 2;
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <preserve>
			ruleContext = 0;
			if(IS_PRESERVED(opts->preserve, PRESERVE_DTD))
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 3;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <dtd>
				ruleContext = 1;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <dtd>
			}
			if(IS_PRESERVED(opts->preserve, PRESERVE_PREFIXES))
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 1 - ruleContext;
				tmpEvCode.bits[0] = 3;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <prefixes>
				ruleContext = 2;
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <prefixes>
			}
			if(IS_PRESERVED(opts->preserve, PRESERVE_LEXVALUES))
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 2 - ruleContext;
				tmpEvCode.bits[0] = 3 - (ruleContext == 2);
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <lexicalValues>
				ruleContext = 3;
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <lexicalValues>
			}
			if(IS_PRESERVED(opts->preserve, PRESERVE_COMMENTS))
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 3 - ruleContext;
				tmpEvCode.bits[0] = 3 - (tmpEvCode.part[0] < 2);
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <comments>
				ruleContext = 4;
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <comments>
			}
			if(IS_PRESERVED(opts->preserve, PRESERVE_PIS))
			{
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 4 - ruleContext;
				tmpEvCode.bits[0] = 3 - (tmpEvCode.part[0] < 3) - (tmpEvCode.part[0] == 0);
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <pis>
				ruleContext = 5;
				tmpEvCode.length = 1;
				tmpEvCode.part[0] = 0;
				tmpEvCode.bits[0] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <pis>
			}
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 5 - ruleContext;
			tmpEvCode.bits[0] = getBitsNumber(tmpEvCode.part[0]);
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <preserve>
		}
		if(opts->blockSize != 1000000)
		{
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = opts->preserve != 0 ? 0 : (2 - hasUncommon);
			tmpEvCode.bits[0] = 2 - (opts->preserve != 0);
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <blockSize>
			TRY(serialize.intData(options_strm, opts->blockSize));
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 0;
			tmpEvCode.bits[0] = 0;
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <blockSize>
		}
		tmpEvCode.length = 1;
		tmpEvCode.part[0] = opts->blockSize != 1000000 ? 0 : (opts->preserve != 0 ? 1 : 3 - hasUncommon);
		tmpEvCode.bits[0] = getBitsNumber(tmpEvCode.part[0]);
		TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <lesscommon>
	}

	// common options if any...
	if(WITH_COMPRESSION(opts->enumOpt) || WITH_FRAGMENT(opts->enumOpt) || opts->schemaIDMode != SCHEMA_ID_ABSENT)
	{
		hasCommon = TRUE;
	}

	if(hasCommon)
	{
		tmpEvCode.length = 1;
		tmpEvCode.part[0] = 1 - hasLesscommon;
		tmpEvCode.bits[0] = 2;
		TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <common>
		ruleContext = 0;
		if(WITH_COMPRESSION(opts->enumOpt))
		{
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 0;
			tmpEvCode.bits[0] = 2;
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <compression>
			ruleContext = 1;
			tmpEvCode.bits[0] = 0;
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <compression>
		}
		if(WITH_FRAGMENT(opts->enumOpt))
		{
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 1 - ruleContext;
			tmpEvCode.bits[0] = 2;
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <fragment>
			ruleContext = 2;
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 0;
			tmpEvCode.bits[0] = 0;
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <fragment>
		}
		if(opts->schemaIDMode != SCHEMA_ID_ABSENT)
		{
			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 2 - ruleContext;
			tmpEvCode.bits[0] = 2 - (ruleContext == 2);
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <schemaId>
			ruleContext = 3;

			if(opts->schemaIDMode == SCHEMA_ID_EMPTY)
			{
				String empty;
				getEmptyString(&empty);
				TRY(serialize.stringData(options_strm, empty));
			}
			else if(opts->schemaID.length == SCHEMA_ID_NIL)
			{
				QName nil;
				nil.uri = &uriTbl->uri[XML_SCHEMA_INSTANCE_ID].uriStr;
				nil.localName = &uriTbl->uri[XML_SCHEMA_INSTANCE_ID].lnTable.ln[XML_SCHEMA_INSTANCE_NIL_ID].lnStr;
				nil.prefix = &uriTbl->uri[XML_SCHEMA_INSTANCE_ID].pfxTable->pfxStr[0];
				tmpEvCode.length = 2;
				tmpEvCode.part[0] = 1;
				tmpEvCode.bits[0] = 1;
				tmpEvCode.part[1] = 0;
				tmpEvCode.bits[1] = 0;
				TRY(serializeEvent(options_strm, tmpEvCode, &nil)); // serialize.attribute nil="true"
				TRY(serialize.booleanData(options_strm, TRUE));
			}
			else
			{
				TRY(serialize.stringData(options_strm, opts->schemaID));
			}

			tmpEvCode.length = 1;
			tmpEvCode.part[0] = 0;
			tmpEvCode.bits[0] = 0;
			TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <schemaId>
		}
		tmpEvCode.length = 1;
		tmpEvCode.part[0] = 3 - ruleContext;
		tmpEvCode.bits[0] = getBitsNumber(tmpEvCode.part[0]);
		TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <common>
	}

	if(WITH_STRICT(opts->enumOpt))
	{
		tmpEvCode.length = 1;
		tmpEvCode.part[0] = hasCommon? 0 : 2 - hasLesscommon;
		tmpEvCode.bits[0] = 2 - hasCommon;
		TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.startElement <strict>
		tmpEvCode.length = 1;
		tmpEvCode.part[0] = 0;
		tmpEvCode.bits[0] = 0;
		TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <strict>
	}

	tmpEvCode.length = 1;
	tmpEvCode.part[0] = WITH_STRICT(opts->enumOpt)? 0 : (hasCommon? 1 : 3 - hasLesscommon);
	tmpEvCode.bits[0] = getBitsNumber(tmpEvCode.part[0]);
	TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endElement <header>

	tmpEvCode.length = 1;
	tmpEvCode.part[0] = 0;
	tmpEvCode.bits[0] = 0;
	TRY(serializeEvent(options_strm, tmpEvCode, NULL)); // serialize.endDocument

	return tmp_err_code;
}
