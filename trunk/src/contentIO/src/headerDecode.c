/*==================================================================================*\
|                                                                                    |
|                    EXIP - Efficient XML Interchange Processor                      |
|                                                                                    |
|------------------------------------------------------------------------------------|
| Copyright (c) 2010, EISLAB - Luleå University of Technology                        |
| All rights reserved.                                                               |
|                                                                                    |
| Redistribution and use in source and binary forms, with or without                 |
| modification, are permitted provided that the following conditions are met:        |
|     * Redistributions of source code must retain the above copyright               |
|       notice, this list of conditions and the following disclaimer.                |
|     * Redistributions in binary form must reproduce the above copyright            |
|       notice, this list of conditions and the following disclaimer in the          |
|       documentation and/or other materials provided with the distribution.         |
|     * Neither the name of the EISLAB - Luleå University of Technology nor the      |
|       names of its contributors may be used to endorse or promote products         |
|       derived from this software without specific prior written permission.        |
|                                                                                    |
| THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND    |
| ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED      |
| WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             |
| DISCLAIMED. IN NO EVENT SHALL EISLAB - LULEÅ UNIVERSITY OF TECHNOLOGY BE LIABLE    |
| FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES |
| (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;       |
| LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND        |
| ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT         |
| (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS      |
| SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                       |
|                                                                                    |
|                                                                                    |
|                                                                                    |
\===================================================================================*/

/**
 * @file headerDecode.c
 * @brief Implementing the interface of EXI header decoder
 *
 * @date Aug 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "headerDecode.h"
#include "streamDecode.h"
#include "streamRead.h"
#include "contentHandler.h"
#include "memManagement.h"
#include "bodyDecode.h"
#include "grammars.h"
#include "EXIParser.h"
#include "sTables.h"
#include "stringManipulate.h"
#include "buildInGrammars.h"

/** This is the statically generated EXIP schema definition for the EXI Options document*/
extern const EXIPSchema ops_schema;

// Content Handler API
static char ops_fatalError(const char code, const char* msg, void* app_data);
static char ops_startDocument(void* app_data);
static char ops_endDocument(void* app_data);
static char ops_startElement(QName qname, void* app_data);
static char ops_endElement(void* app_data);
static char ops_attribute(QName qname, void* app_data);
static char ops_stringData(const String value, void* app_data);
static char ops_intData(Integer int_val, void* app_data);
static char ops_boolData(unsigned char bool_val, void* app_data);

struct ops_AppData
{
	EXIOptions* parsed_ops;
	EXIStream* o_strm;
	AllocList* permanentAllocList;
	unsigned char prevElementUriID;
	unsigned char prevElementLnID;
	unsigned char schemaIDOptions;
};

#define SCHEMA_ID_NO_ELEMENT  0
#define SCHEMA_ID_EL_NIL      1
#define SCHEMA_ID_EL_NIL_TRUE 2
#define SCHEMA_ID_EL_EMPTY    3
#define SCHEMA_ID_EL_STRING   4

errorCode decodeHeader(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int bits_val = 0;
	unsigned char smallVal = 0;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start EXI header decoding\n"));
	tmp_err_code = readBits(strm, 2, &bits_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(bits_val == 2)  // The header Distinguishing Bits i.e. no EXI Cookie
	{
		strm->header.has_cookie = 0;
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">No EXI cookie detected\n"));
	}
	else if(bits_val == 0)// ASCII code for $ = 00100100  (36)
	{
		tmp_err_code = readBits(strm, 6, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 36)
			return INVALID_EXI_HEADER;
		tmp_err_code = readBits(strm, 8, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 69)   // ASCII code for E = 01000101  (69)
			return INVALID_EXI_HEADER;
		tmp_err_code = readBits(strm, 8, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 88)   // ASCII code for X = 01011000  (88)
			return INVALID_EXI_HEADER;
		tmp_err_code = readBits(strm, 8, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 73)   // ASCII code for I = 01001001  (73)
			return INVALID_EXI_HEADER;

		strm->header.has_cookie = 1;
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">EXI cookie detected\n"));
		tmp_err_code = readBits(strm, 2, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		if(bits_val != 2)  // The header Distinguishing Bits are required
			return INVALID_EXI_HEADER;
	}
	else
	{
		return INVALID_EXI_HEADER;
	}

	// Read the Presence Bit for EXI Options
	tmp_err_code = readNextBit(strm, &smallVal);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

    makeDefaultOpts(&strm->header.opts);

	if(smallVal == 1) // There are EXI options
		strm->header.has_options = 1;
	else // The default values for EXI options
	{
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">No EXI options field in the header\n"));
		strm->header.has_options = 0;
	}

	// Read the Version type
	tmp_err_code = readNextBit(strm, &smallVal);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	strm->header.is_preview_version = smallVal;
	strm->header.version_number = 1;

	do
	{
		tmp_err_code = readBits(strm, 4, &bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		strm->header.version_number += bits_val;
		if(bits_val < 15)
			break;
	} while(1);

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">EXI version: %d\n", strm->header.version_number));

	if(strm->header.has_options == 1)
	{
		Parser optionsParser;
		struct ops_AppData appD;

		tmp_err_code = initParser(&optionsParser, strm->buffer, strm->bufLen, strm->bufContent, &strm->ioStrm, (EXIPSchema*) &ops_schema, &appD);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		optionsParser.strm.context.bitPointer = strm->context.bitPointer;
		optionsParser.strm.context.bufferIndx = strm->context.bufferIndx;

		makeDefaultOpts(&optionsParser.strm.header.opts);
		SET_STRICT(optionsParser.strm.header.opts.enumOpt);

		optionsParser.handler.fatalError = ops_fatalError;
		optionsParser.handler.error = ops_fatalError;
		optionsParser.handler.startDocument = ops_startDocument;
		optionsParser.handler.endDocument = ops_endDocument;
		optionsParser.handler.startElement = ops_startElement;
		optionsParser.handler.attribute = ops_attribute;
		optionsParser.handler.stringData = ops_stringData;
		optionsParser.handler.endElement = ops_endElement;
		optionsParser.handler.intData = ops_intData;
		optionsParser.handler.booleanData = ops_boolData;

		appD.o_strm = &optionsParser.strm;
		appD.parsed_ops = &strm->header.opts;
		appD.prevElementLnID = 0;
		appD.prevElementUriID = 0;
		appD.schemaIDOptions = SCHEMA_ID_NO_ELEMENT;
		appD.permanentAllocList = &strm->memList;

		optionsParser.strm.gStack = NULL;
		tmp_err_code = createDocGrammar(&optionsParser.documentGrammar, &optionsParser.strm, optionsParser.strm.schema);
		if(tmp_err_code != ERR_OK)
		{
			destroyParser(&optionsParser);
			return tmp_err_code;
		}

		tmp_err_code = pushGrammar(&optionsParser.strm.gStack, &optionsParser.documentGrammar);
		if(tmp_err_code != ERR_OK)
		{
			destroyParser(&optionsParser);
			return tmp_err_code;
		}

		optionsParser.strm.uriTable = optionsParser.strm.schema->initialStringTables;
		tmp_err_code = createValueTable(&(optionsParser.strm.vTable), &(optionsParser.strm.memList));
		if(tmp_err_code != ERR_OK)
		{
			destroyParser(&optionsParser);
			return tmp_err_code;
		}

		while(tmp_err_code == ERR_OK)
		{
			tmp_err_code = parseNext(&optionsParser);
		}

		destroyParser(&optionsParser);

		if(tmp_err_code != PARSING_COMPLETE)
			return tmp_err_code;

		strm->bufContent = optionsParser.strm.bufContent;
		strm->context.bitPointer = optionsParser.strm.context.bitPointer;
		strm->context.bufferIndx = optionsParser.strm.context.bufferIndx;

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

		// When the "schemaId" element in the EXI options document contains the xsi:nil attribute
		// with its value set to true, no schema information is used for processing the EXI body
		// (i.e. a schema-less EXI stream)
		if(appD.schemaIDOptions == SCHEMA_ID_EL_NIL_TRUE)
		{
			if(strm->schema != NULL)
			{
				if(strm->schema->isStatic == FALSE)
				{
					freeAllocList(&strm->schema->memList);
				}
				strm->schema = NULL;
			}
		}
		else if(appD.schemaIDOptions == SCHEMA_ID_EL_EMPTY)
		{
			// When the value of the "schemaId" element is empty, no user defined schema information
			// is used for processing the EXI body; however, the built-in XML schema types are available for use in the EXI body
			strm->schema = memManagedAllocate(&strm->memList, sizeof(EXIPSchema));
			if(strm->schema == NULL)
				return MEMORY_ALLOCATION_ERROR;

			tmp_err_code = generateSchemaBuildInGrammars(strm->schema);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

static char ops_fatalError(const char code, const char* msg, void* app_data)
{
	DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Error during parsing of the EXI Options\n"));
	return EXIP_HANDLER_STOP;
}

static char ops_startDocument(void* app_data)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start parsing the EXI Options\n"));
	return EXIP_HANDLER_OK;
}

static char ops_endDocument(void* app_data)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Complete parsing the EXI Options\n"));
	return EXIP_HANDLER_OK;
}

static char ops_startElement(QName qname, void* app_data)
{
	struct ops_AppData* o_appD = (struct ops_AppData*) app_data;

	if(o_appD->o_strm->context.curr_uriID == 4) // URI == http://www.w3.org/2009/exi
	{
		o_appD->prevElementUriID = 4;

		switch(o_appD->o_strm->context.curr_lnID)
		{
			case 33:	// strict
				SET_STRICT(o_appD->parsed_ops->enumOpt);
				o_appD->prevElementLnID = 33;
			break;
			case 31:	// schemaId
				o_appD->prevElementLnID = 31;
				o_appD->schemaIDOptions = SCHEMA_ID_EL_EMPTY;
			break;
			case 7:		// compression
				SET_COMPRESSION(o_appD->parsed_ops->enumOpt);
				o_appD->prevElementLnID = 7;
			break;
			case 14:	// fragment
				SET_FRAGMENT(o_appD->parsed_ops->enumOpt);
				o_appD->prevElementLnID = 14;
			break;
			case 13:	// dtd
				SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_DTD);
				o_appD->prevElementLnID = 13;
			break;
			case 29:	// prefixes
				SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_PREFIXES);
				o_appD->prevElementLnID = 29;
			break;
			case 26:	// lexicalValues
				SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_LEXVALUES);
				o_appD->prevElementLnID = 26;
			break;
			case 5:	// comments
				SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_COMMENTS);
				o_appD->prevElementLnID = 5;
			break;
			case 27:	// pis
				SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_PIS);
				o_appD->prevElementLnID = 27;
			break;
			case 4:	// alignment->byte
				SET_ALIGNMENT(o_appD->parsed_ops->enumOpt, BYTE_ALIGNMENT);
				o_appD->prevElementLnID = 4;
			break;
			case 28:	// alignment->pre-compress
				SET_ALIGNMENT(o_appD->parsed_ops->enumOpt, PRE_COMPRESSION);
				o_appD->prevElementLnID = 28;
			break;
			case 32:	// selfContained
				SET_SELF_CONTAINED(o_appD->parsed_ops->enumOpt);
				o_appD->prevElementLnID = 32;
			break;
			case 8:	// datatypeRepresentationMap
				o_appD->prevElementLnID = 8;
			break;
			case 36:	// uncommon
				o_appD->prevElementLnID = 36;
			break;
		}
	}
	else // URI != http://www.w3.org/2009/exi
	{
		// The previous element should be either uncommon or datatypeRepresentationMap otherwise it is an error
		// These are the only places where <any> element is allowed
		if(o_appD->prevElementUriID != 4 || o_appD->prevElementLnID != 36 || o_appD->prevElementLnID != 8)
		{
			DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Wrong namespace in the EXI Options\n"));
			return EXIP_HANDLER_STOP;
		}

		// Handle here the user defined meta-data that follows! http://www.w3.org/TR/2011/REC-exi-20110310/#key-userMetaData
	}

	return EXIP_HANDLER_OK;
}

static char ops_endElement(void* app_data)
{
	return EXIP_HANDLER_OK;
}

static char ops_attribute(QName qname, void* app_data)
{
	struct ops_AppData* o_appD = (struct ops_AppData*) app_data;

	if(o_appD->prevElementUriID == 4) // URI == http://www.w3.org/2009/exi
	{
		if(o_appD->prevElementLnID != 31) // schemaId
		{
			DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Corrupt EXI Options\n"));
			return EXIP_HANDLER_STOP;
		}
		else
		{
			if(o_appD->o_strm->context.curr_uriID == 2 && o_appD->o_strm->context.curr_lnID == 0) // xsi:nil
			{
				o_appD->schemaIDOptions = SCHEMA_ID_EL_NIL;
			}
			else
			{
				DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Corrupt EXI Options\n"));
				return EXIP_HANDLER_STOP;
			}
		}
	}
	else
	{
		// Handle here the user defined meta-data that follows! http://www.w3.org/TR/2011/REC-exi-20110310/#key-userMetaData
	}

	return EXIP_HANDLER_OK;
}

static char ops_stringData(const String value, void* app_data)
{
	struct ops_AppData* o_appD = (struct ops_AppData*) app_data;

	if(o_appD->prevElementUriID == 4) // URI == http://www.w3.org/2009/exi
	{
		if(o_appD->prevElementLnID != 31) // schemaId
		{
			DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Corrupt EXI Options\n"));
			return EXIP_HANDLER_STOP;
		}
		else
		{
			if(isStringEmpty(&value))
			{
				o_appD->schemaIDOptions = SCHEMA_ID_EL_EMPTY;
			}
			else
			{
				o_appD->schemaIDOptions = SCHEMA_ID_EL_STRING;
				if(cloneString(&value, &o_appD->parsed_ops->schemaID, o_appD->permanentAllocList) != ERR_OK)
				{
					DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Memory error\n"));
					return EXIP_HANDLER_STOP;
				}
			}
		}
	}
	else
	{
		// Handle here the user defined meta-data that follows! http://www.w3.org/TR/2011/REC-exi-20110310/#key-userMetaData
	}

	return EXIP_HANDLER_OK;
}

static char ops_intData(Integer int_val, void* app_data)
{
	struct ops_AppData* o_appD = (struct ops_AppData*) app_data;

	switch(o_appD->o_strm->context.curr_lnID)
	{
		case 37:	// valueMaxLength
			o_appD->parsed_ops->valueMaxLength = (unsigned int) int_val;
		break;
		case 38:	// valuePartitionCapacity
			o_appD->parsed_ops->valuePartitionCapacity = (unsigned int) int_val;
		break;
		case 2:	// blockSize
			o_appD->parsed_ops->blockSize = (unsigned int) int_val;
		break;
	}
	return EXIP_HANDLER_OK;
}

static char ops_boolData(unsigned char bool_val, void* app_data)
{
	struct ops_AppData* o_appD = (struct ops_AppData*) app_data;

	if(o_appD->schemaIDOptions == SCHEMA_ID_EL_NIL) // xsi:nil attribute
	{
		if(bool_val == TRUE)
			o_appD->schemaIDOptions = SCHEMA_ID_EL_NIL_TRUE;
		else
			o_appD->schemaIDOptions = SCHEMA_ID_EL_EMPTY;
	}
	else
	{
		// Handle here the user defined meta-data that follows! http://www.w3.org/TR/2011/REC-exi-20110310/#key-userMetaData
	}

	return EXIP_HANDLER_OK;
}
