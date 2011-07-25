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
#include "schema.h"
#include "memManagement.h"
#include "bodyDecode.h"

/** This is the statically generated EXIP schema definition for the EXI Options document*/
extern const ExipSchema ops_schema;

// Content Handler API
static char ops_fatalError(const char code, const char* msg, void* app_data);
static char ops_startDocument(void* app_data);
static char ops_endDocument(void* app_data);
static char ops_startElement(QName qname, void* app_data);
static char ops_endElement(void* app_data);
static char ops_attribute(QName qname, void* app_data);
static char ops_stringData(const StringType value, void* app_data);
static char ops_intData(int32_t int_val, void* app_data);

struct ops_AppData
{
	EXIOptions* parsed_ops;
	EXIStream* o_strm;
};

errorCode decodeHeader(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	uint32_t bits_val = 0;
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


	if(strm->header.opts == NULL)
		return NULL_POINTER_REF;

    makeDefaultOpts(strm->header.opts);

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
		EXIStream options_strm;
		EXIOptions o_ops;
		ContentHandler opsHandler;
		struct ops_AppData appD;

		makeDefaultOpts(&o_ops);
		o_ops.strict = TRUE;
		tmp_err_code = initAllocList(&options_strm.memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		options_strm.buffer = strm->buffer;
		options_strm.context.bitPointer = strm->context.bitPointer;
		options_strm.context.bufferIndx = strm->context.bufferIndx;
		options_strm.bufLen = strm->bufLen;
		options_strm.header.opts = &o_ops;
		options_strm.context.nonTermID = GR_DOCUMENT;
		options_strm.context.curr_lnID = 0;
		options_strm.context.curr_uriID = 0;
		options_strm.context.expectATData = 0;
		options_strm.bufContent = strm->bufContent;
		options_strm.ioStrm = strm->ioStrm;

		initContentHandler(&opsHandler);
		opsHandler.fatalError = ops_fatalError;
		opsHandler.error = ops_fatalError;
		opsHandler.startDocument = ops_startDocument;
		opsHandler.endDocument = ops_endDocument;
		opsHandler.startElement = ops_startElement;
		opsHandler.attribute = ops_attribute;
		opsHandler.stringData = ops_stringData;
		opsHandler.endElement = ops_endElement;
		opsHandler.intData = ops_intData;

		appD.o_strm = &options_strm;
		appD.parsed_ops = strm->header.opts;
		decodeBody(&options_strm, &opsHandler, &ops_schema, &appD);

		strm->bufContent = options_strm.bufContent;
		strm->context.bitPointer = options_strm.context.bitPointer;
		strm->context.bufferIndx = options_strm.context.bufferIndx;

		if(strm->header.opts->compression == TRUE ||
			strm->header.opts->alignment != BIT_PACKED)
		{
			// Padding bits
			if(strm->context.bitPointer != 0)
			{
				strm->context.bitPointer = 0;
				strm->context.bufferIndx += 1;
			}
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

	if(o_appD->o_strm->context.curr_uriID != 4) // URI != http://www.w3.org/2009/exi
	{
		DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Wrong namespace in the EXI Options\n"));
		return EXIP_HANDLER_STOP;
	}

	switch(o_appD->o_strm->context.curr_lnID)
	{
		case 33:	// strict
			o_appD->parsed_ops->strict = TRUE;
		break;
		case 31:	// schemaId
			// TODO: implement this!
		break;
		case 7:		// compression
			o_appD->parsed_ops->compression = TRUE;
		break;
		case 14:	// fragment
			o_appD->parsed_ops->fragment = TRUE;
		break;
		case 13:	// dtd
			SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_DTD);
		break;
		case 29:	// prefixes
			SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_PREFIXES);
		break;
		case 26:	// lexicalValues
			SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_LEXVALUES);
		break;
		case 5:	// comments
			SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_COMMENTS);
		break;
		case 27:	// pis
			SET_PRESERVED(o_appD->parsed_ops->preserve, PRESERVE_PIS);
		break;
		case 4:	// alignment->byte
			o_appD->parsed_ops->alignment = BYTE_ALIGNMENT;
		break;
		case 28:	// alignment->pre-compress
			o_appD->parsed_ops->alignment = PRE_COMPRESSION;
		break;
		case 32:	// selfContained
			o_appD->parsed_ops->selfContained = TRUE;
		break;
		case 8:	// datatypeRepresentationMap
			// TODO: implement this!
		break;
	}

	return EXIP_HANDLER_OK;
}

static char ops_endElement(void* app_data)
{
	return EXIP_HANDLER_OK;
}

static char ops_attribute(QName qname, void* app_data)
{
	return EXIP_HANDLER_OK;
}

static char ops_stringData(const StringType value, void* app_data)
{
	return EXIP_HANDLER_OK;
}

static char ops_intData(int32_t int_val, void* app_data)
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
