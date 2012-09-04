/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file EXISerializer.c
 * @brief Implementation of the serializer of EXI streams
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "EXISerializer.h"
#include "grammars.h"
#include "memManagement.h"
#include "sTables.h"
#include "headerEncode.h"
#include "bodyEncode.h"
#include "grammarAugment.h"
#include "hashtable.h"
#include "stringManipulate.h"
#include "streamEncode.h"
#include "initSchemaInstance.h"
#include "emptyTypeGrammarGen.h"

/**
 * The handler to be used by the applications to serialize EXI streams
 */
const EXISerializer serialize ={startDocument,
								endDocument,
								startElement,
								endElement,
								attribute,
								intData,
								booleanData,
								stringData,
								floatData,
								binaryData,
								dateTimeData,
								decimalData,
								processingInstruction,
								namespaceDeclaration,
								encodeHeader,
								selfContained,
								initHeader,
								initStream,
								closeEXIStream};

void initHeader(EXIStream* strm)
{
	strm->header.has_cookie = FALSE;
	strm->header.has_options = FALSE;
	strm->header.is_preview_version = FALSE;
	strm->header.version_number = 1;
	makeDefaultOpts(&strm->header.opts);
}

errorCode initStream(EXIStream* strm, BinaryBuffer buffer, EXIPSchema* schema, unsigned char schemaIdMode, String* schemaID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">EXI stream initialization \n"));

	tmp_err_code = initAllocList(&(strm->memList));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	strm->buffer = buffer;
	strm->context.bitPointer = 0;
	strm->context.bufferIndx = 0;
	strm->context.currNonTermID = GR_DOCUMENT;
	strm->context.currElem.uriId = URI_MAX;
	strm->context.currElem.lnId = LN_MAX;
	strm->context.currAttr.uriId = URI_MAX;
	strm->context.currAttr.lnId = LN_MAX;
	strm->context.expectATData = 0;
	strm->context.attrTypeId = INDEX_MAX;
	strm->gStack = NULL;
	strm->valueTable.value = NULL;
	strm->schema = schema;

	tmp_err_code = createValueTable(&strm->valueTable, &strm->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(schema != NULL) // schema enabled encoding
	{
		if(schemaIdMode == SCHEMA_ID_NIL || schemaIdMode == SCHEMA_ID_EMPTY)
			return INVALID_EXIP_CONFIGURATION;

		tmp_err_code = addUndeclaredProductionsToAll(&strm->memList, strm->schema, &strm->header.opts);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(WITH_FRAGMENT(strm->header.opts.enumOpt))
		{
			/* Fragment document grammar */
			// TODO: create a Schema-informed Fragment Grammar from the EXIP schema object
			return NOT_IMPLEMENTED_YET;
		}
		else
		{
			tmp_err_code = augmentDocGrammar(&strm->memList, strm->header.opts.preserve, &strm->schema->docGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else
	{
		// schema-less encoding
		strm->schema = memManagedAllocate(&strm->memList, sizeof(EXIPSchema));
		if(strm->schema == NULL)
			return MEMORY_ALLOCATION_ERROR;

		if(schemaIdMode != SCHEMA_ID_EMPTY)
		{
			// fully schema-less - no built-in XML schema types
			tmp_err_code = initSchema(strm->schema, INIT_SCHEMA_SCHEMA_LESS_MODE);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			// no user defined schema information, only built-in XML schema types
			tmp_err_code = initSchema(strm->schema, INIT_SCHEMA_BUILD_IN_TYPES);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = addUndeclaredProductionsToAll(&strm->memList, strm->schema, &strm->header.opts);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		if(WITH_FRAGMENT(strm->header.opts.enumOpt))
		{
			tmp_err_code = createFragmentGrammar(strm->schema, NULL, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = augmentFragGrammar(&strm->memList, strm->header.opts.preserve, &strm->schema->docGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			tmp_err_code = createDocGrammar(strm->schema, NULL, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = augmentDocGrammar(&strm->memList, strm->header.opts.preserve, &strm->schema->docGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	tmp_err_code = pushGrammar(&strm->gStack, &strm->schema->docGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// EXI schemaID handling

	if(schemaIdMode == SCHEMA_ID_ABSENT)
	{
		if(schemaID != NULL)
			return INVALID_EXIP_CONFIGURATION;
	}
	else if(schemaIdMode == SCHEMA_ID_SET)
	{
		if(schemaID == NULL)
			return INVALID_EXIP_CONFIGURATION;
		tmp_err_code = cloneString(schemaID, &strm->header.opts.schemaID, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(schemaIdMode == SCHEMA_ID_NIL)
	{
		strm->header.opts.schemaID.length = SCHEMA_ID_NIL;
	}
	else if(schemaIdMode == SCHEMA_ID_EMPTY)
	{
		strm->header.opts.schemaID.length = SCHEMA_ID_EMPTY;
	}

	// #DOCUMENT#
	// Hashtable for fast look-up of global values in the table.
	// Only used when:
	// serializing &&
	// valuePartitionCapacity > 50  &&   //for small table full-scan will work better
	// valueMaxLength > 0 && // this is essentially equal to valuePartitionCapacity == 0
	// HASH_TABLE_USE == ON // build configuration parameter
#if HASH_TABLE_USE == ON
	if(strm->header.opts.valuePartitionCapacity > DEFAULT_VALUE_ENTRIES_NUMBER &&
			strm->header.opts.valueMaxLength > 0)
	{
		strm->valueTable.hashTbl = create_hashtable(INITIAL_HASH_TABLE_SIZE, djbHash, stringEqual);
		if(strm->valueTable.hashTbl == NULL)
			return HASH_TABLE_ERROR;
	}
#endif
	return ERR_OK;
}

errorCode startDocument(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	Index lastCodePart;
	Index typeId;
	GrammarRule* currentRule;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start doc serialization\n"));

	if(strm->context.currNonTermID != GR_DOCUMENT)
		return INCONSISTENT_PROC_STATE;

	tmp_err_code = lookupProduction(strm, EVENT_SD, VALUE_TYPE_NONE, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
}

errorCode endDocument(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	Index lastCodePart;
	Index typeId;
	GrammarRule* currentRule;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">End doc serialization\n"));

	tmp_err_code = lookupProduction(strm, EVENT_ED, VALUE_TYPE_NONE, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
}

errorCode startElement(EXIStream* strm, QName qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	Index lastCodePart;
	Index typeId;
	GrammarRule* currentRule;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start element serialization\n"));

	tmp_err_code = lookupProduction(strm, EVENT_SE_ALL, VALUE_TYPE_NONE, &currentRule, &typeId, &qname, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeProduction(strm, currentRule, codeLength, lastCodePart, &qname);
}

errorCode endElement(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	Index lastCodePart;
	Index typeId;
	GrammarRule* currentRule;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">End element serialization\n"));

	tmp_err_code = lookupProduction(strm, EVENT_EE, VALUE_TYPE_NONE, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
}

errorCode attribute(EXIStream* strm, QName qname, EXIType exiType)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	Index lastCodePart;
	Index typeId;
	GrammarRule* currentRule;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start attribute serialization\n"));

	tmp_err_code = lookupProduction(strm, EVENT_AT_ALL, exiType, &currentRule, &typeId, &qname, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeProduction(strm, currentRule, codeLength, lastCodePart, &qname);
}

errorCode intData(EXIStream* strm, Integer int_val)
{
	Index intTypeId;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start integer data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		intTypeId = strm->context.attrTypeId;
		strm->context.expectATData = FALSE;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		unsigned char codeLength;
		Index lastCodePart;
		GrammarRule* currentRule;

		tmp_err_code = lookupProduction(strm, EVENT_CH, VALUE_TYPE_INTEGER, &currentRule, &intTypeId, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return encodeIntData(strm, int_val, intTypeId);
}

errorCode booleanData(EXIStream* strm, unsigned char bool_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char isXsiNilAttr = FALSE;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start boolean data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
		if(strm->context.currAttr.uriId == 2 && strm->context.currAttr.lnId == 0)
		{
			// xsi:nill
			isXsiNilAttr = TRUE;
		}
	}
	else
	{
		Index typeId;
		unsigned char codeLength;
		Index lastCodePart;
		GrammarRule* currentRule;

		tmp_err_code = lookupProduction(strm, EVENT_CH, VALUE_TYPE_BOOLEAN, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = encodeBoolean(strm, bool_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(IS_SCHEMA(strm->gStack->grammar->props) && isXsiNilAttr && bool_val)
	{
		// In a schema-informed grammar && xsi:nil == TRUE
		EXIGrammar* currGrammar;
		EXIGrammar* tmpGrammar;
		popGrammar(&(strm->gStack), &currGrammar);

		tmp_err_code = getEmptyTypeGrammar(strm, currGrammar, &tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = pushGrammar(&(strm->gStack), tmpGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		strm->context.currNonTermID = GR_START_TAG_CONTENT;
	}

	return ERR_OK;
}

errorCode stringData(EXIStream* strm, const String str_val)
{
	QNameID qnameID;
	Index typeId;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start string data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
		qnameID = strm->context.currAttr;
		typeId = strm->context.attrTypeId;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		unsigned char codeLength;
		Index lastCodePart;
		GrammarRule* currentRule;

		tmp_err_code = lookupProduction(strm, EVENT_CH, VALUE_TYPE_STRING, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		qnameID = strm->context.currElem;
	}

	return encodeStringData(strm, str_val, qnameID, typeId);
}

errorCode floatData(EXIStream* strm, Float float_val)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start float data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Index typeId;
		unsigned char codeLength;
		Index lastCodePart;
		GrammarRule* currentRule;

		tmp_err_code = lookupProduction(strm, EVENT_CH, VALUE_TYPE_FLOAT, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return encodeFloatValue(strm, float_val);
}

errorCode binaryData(EXIStream* strm, const char* binary_val, Index nbytes)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start float data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Index typeId;
		unsigned char codeLength;
		Index lastCodePart;
		GrammarRule* currentRule;

		tmp_err_code = lookupProduction(strm, EVENT_CH, VALUE_TYPE_BINARY, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return encodeBinary(strm, (char *)binary_val, nbytes);
}

errorCode dateTimeData(EXIStream* strm, EXIPDateTime dt_val)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start dateTime data serialization\n"));

	if(strm->context.expectATData) // Value for an attribute
	{
		strm->context.expectATData = FALSE;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Index typeId;
		unsigned char codeLength;
		Index lastCodePart;
		GrammarRule* currentRule;

		tmp_err_code = lookupProduction(strm, EVENT_CH, VALUE_TYPE_DATE_TIME, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return encodeDateTimeValue(strm, dt_val);
}

errorCode decimalData(EXIStream* strm, Decimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode processingInstruction(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode namespaceDeclaration(EXIStream* strm, const String ns, const String prefix, unsigned char isLocalElementNS)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char codeLength;
	Index lastCodePart;
	SmallIndex uriId;
	Index typeId;
	GrammarRule* currentRule;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start namespace declaration\n"));

	tmp_err_code = lookupProduction(strm, EVENT_NS, VALUE_TYPE_NONE, &currentRule, &typeId, NULL, &codeLength, &lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = encodeProduction(strm, currentRule, codeLength, lastCodePart, NULL);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = encodeUri(strm, (String*) &ns, &uriId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->schema->uriTable.uri[uriId].pfxTable == NULL)
	{
		tmp_err_code = createPfxTable(&strm->schema->uriTable.uri[uriId].pfxTable, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = encodePfx(strm, uriId, (String*) &prefix);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return encodeBoolean(strm, isLocalElementNS);
}

errorCode selfContained(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode closeEXIStream(EXIStream* strm)
{
	errorCode tmp_err_code = ERR_OK;
	EXIGrammar* tmp;

	while(strm->gStack != NULL)
	{
		popGrammar(&strm->gStack, &tmp);
	}

	// Flush the buffer first if there is an output Stream
	if(strm->buffer.ioStrm.readWriteToStream != NULL)
	{
		if((Index)strm->buffer.ioStrm.readWriteToStream(strm->buffer.buf, strm->context.bufferIndx + 1, strm->buffer.ioStrm.stream) < strm->context.bufferIndx + 1)
			tmp_err_code = BUFFER_END_REACHED;
	}
	if(strm->schema != NULL)
	{
		if(strm->schema->isStatic == TRUE)
		{
			// Reseting the value cross table links to NULL
			Index i;
			Index j;
			for(i = 0; i < strm->schema->uriTable.count; i++)
			{
				for(j = 0; j < strm->schema->uriTable.uri[i].lnTable.count; j++)
				{
					strm->schema->uriTable.uri[i].lnTable.ln[j].vxTable.vx = NULL;
				}
			}
		}
		else
			freeAllocList(&strm->schema->memList);
	}
	freeAllMem(strm);
	return tmp_err_code;
}

errorCode serializeEvent(EXIStream* strm, unsigned char codeLength, Index lastCodePart, QName* qname)
{
	GrammarRule* currentRule;

	if(strm->context.currNonTermID >=  strm->gStack->grammar->count)
		return INCONSISTENT_PROC_STATE;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[strm->context.currNonTermID];
	else
		currentRule = &strm->gStack->grammar->rule[strm->context.currNonTermID];

	return encodeProduction(strm, currentRule, codeLength, lastCodePart, qname);
}
