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
#include "hashtable.h"
#include "stringManipulate.h"
#include "streamEncode.h"
#include "initSchemaInstance.h"

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
								listData,
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

errorCode initStream(EXIStream* strm, BinaryBuffer buffer, EXIPSchema* schema, SchemaIdMode schemaIdMode, String* schemaID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">EXI stream initialization \n"));

	tmp_err_code = checkOptionValues(&strm->header.opts);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = initAllocList(&(strm->memList));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	strm->buffer = buffer;
	strm->context.bitPointer = 0;
	strm->context.bufferIndx = 0;
	strm->context.currNonTermID = GR_DOC_CONTENT;
	strm->context.currElem.uriId = URI_MAX;
	strm->context.currElem.lnId = LN_MAX;
	strm->context.currAttr.uriId = URI_MAX;
	strm->context.currAttr.lnId = LN_MAX;
	strm->context.expectATData = FALSE;
	strm->context.isNilType = FALSE;
	strm->context.attrTypeId = INDEX_MAX;
	strm->gStack = NULL;
	strm->valueTable.value = NULL;
	strm->schema = schema;

	if(strm->header.opts.valuePartitionCapacity > 0)
	{
		tmp_err_code = createValueTable(&strm->valueTable);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	if(schema != NULL) // schema enabled encoding
	{
		if(schemaIdMode == SCHEMA_ID_NIL || schemaIdMode == SCHEMA_ID_EMPTY)
			return INVALID_EXIP_CONFIGURATION;

		if(WITH_FRAGMENT(strm->header.opts.enumOpt))
		{
			/* Fragment document grammar */
			// TODO: create a Schema-informed Fragment Grammar from the EXIP schema object
			return NOT_IMPLEMENTED_YET;
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
		}

		if(WITH_FRAGMENT(strm->header.opts.enumOpt))
		{
			tmp_err_code = createFragmentGrammar(strm->schema, NULL, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			tmp_err_code = createDocGrammar(strm->schema, NULL, 0);
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
		tmp_err_code = cloneStringManaged(schemaID, &strm->header.opts.schemaID, &strm->memList);
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
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start doc serialization\n"));

	if(strm->context.currNonTermID != GR_DOC_CONTENT)
		return INCONSISTENT_PROC_STATE;

	return ERR_OK;
}

errorCode endDocument(EXIStream* strm)
{
	Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">End doc serialization\n"));

	return encodeProduction(strm, EVENT_ED_CLASS, VALUE_TYPE_NONE_CLASS, NULL, &prodHit);
}

errorCode startElement(EXIStream* strm, QName qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start element serialization\n"));

	tmp_err_code = encodeProduction(strm, EVENT_SE_CLASS, VALUE_TYPE_NONE_CLASS, &qname, &prodHit);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(GET_PROD_EXI_EVENT(prodHit.content) == EVENT_SE_ALL)
	{
		EXIGrammar* elemGrammar = NULL;

		tmp_err_code = encodeQName(strm, qname, EVENT_SE_ALL, &strm->context.currElem);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		strm->gStack->lastNonTermID = strm->context.currNonTermID;
		// New element grammar is pushed on the stack
		elemGrammar = GET_ELEM_GRAMMAR_QNAMEID(strm->schema, strm->context.currElem);

		if(elemGrammar != NULL) // The grammar is found
		{
			strm->context.currNonTermID = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), elemGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			EXIGrammar newElementGrammar;
			Index dynArrIndx;
			tmp_err_code = createBuiltInElementGrammar(&newElementGrammar, strm);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = addDynEntry(&strm->schema->grammarTable.dynArray, &newElementGrammar, &dynArrIndx);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currElem).elemGrammar = dynArrIndx;

			strm->context.currNonTermID = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), &strm->schema->grammarTable.grammar[dynArrIndx]);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else if(GET_PROD_EXI_EVENT(prodHit.content) == EVENT_SE_QNAME)
	{
		EXIGrammar* elemGrammar = NULL;

		strm->context.currElem.uriId = prodHit.qnameId.uriId;
		strm->context.currElem.lnId = prodHit.qnameId.lnId;

		tmp_err_code = encodePfxQName(strm, &qname, EVENT_SE_QNAME, prodHit.qnameId.uriId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		strm->gStack->lastNonTermID = strm->context.currNonTermID;

		// New element grammar is pushed on the stack
		if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		{
			elemGrammar = GET_ELEM_GRAMMAR_QNAMEID(strm->schema, strm->context.currElem);
		}
		else
		{
			elemGrammar = &strm->schema->grammarTable.grammar[prodHit.typeId];
		}

		if(elemGrammar != NULL) // The grammar is found
		{
			strm->context.currNonTermID = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), elemGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			return INCONSISTENT_PROC_STATE;  // The event require the presence of Element Grammar previously created
		}
	}
	else
		return NOT_IMPLEMENTED_YET;

	return ERR_OK;
}

errorCode endElement(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">End element serialization\n"));

	tmp_err_code = encodeProduction(strm, EVENT_EE_CLASS, VALUE_TYPE_NONE_CLASS, NULL, &prodHit);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->context.currNonTermID == GR_VOID_NON_TERMINAL)
	{
		EXIGrammar* grammar;
		popGrammar(&(strm->gStack), &grammar);
		if(strm->gStack != NULL) // There is more grammars in the stack
			strm->context.currNonTermID = strm->gStack->lastNonTermID;
	}
	else
		return INCONSISTENT_PROC_STATE;

	return ERR_OK;
}

errorCode attribute(EXIStream* strm, QName qname, EXITypeClass exiType)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start attribute serialization\n"));

	tmp_err_code = encodeProduction(strm, EVENT_AT_CLASS, exiType, &qname, &prodHit);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(GET_PROD_EXI_EVENT(prodHit.content) == EVENT_AT_ALL)
	{
		tmp_err_code = encodeQName(strm, qname, EVENT_AT_ALL, &strm->context.currAttr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(IS_SCHEMA(strm->gStack->grammar->props) && strm->context.currAttr.uriId == XML_SCHEMA_INSTANCE_ID &&
				(strm->context.currAttr.uriId == XML_SCHEMA_INSTANCE_TYPE_ID || strm->context.currAttr.uriId == XML_SCHEMA_INSTANCE_NIL_ID))
		{
			DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">In schema-informed grammars, xsi:type and xsi:nil attributes MUST NOT be represented using AT(*) terminal\n"));
			return INCONSISTENT_PROC_STATE;
		}
	}
	else if(GET_PROD_EXI_EVENT(prodHit.content) == EVENT_AT_QNAME)
	{
		strm->context.currAttr.uriId = prodHit.qnameId.uriId;
		strm->context.currAttr.lnId = prodHit.qnameId.lnId;

		tmp_err_code = encodePfxQName(strm, &qname, EVENT_AT_QNAME, prodHit.qnameId.uriId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
		return NOT_IMPLEMENTED_YET;

	strm->context.expectATData = TRUE;
	strm->context.attrTypeId = prodHit.typeId;

	return ERR_OK;
}

errorCode intData(EXIStream* strm, Integer int_val)
{
	Index intTypeId;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start integer data serialization\n"));

	if(strm->context.expectATData > 0) // Value for an attribute or list item
	{
		intTypeId = strm->context.attrTypeId;
		strm->context.expectATData -= 1;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

		tmp_err_code = encodeProduction(strm, EVENT_CH_CLASS, VALUE_TYPE_INTEGER_CLASS, NULL, &prodHit);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		intTypeId = prodHit.typeId;
	}

	return encodeIntData(strm, int_val, intTypeId);
}

errorCode booleanData(EXIStream* strm, boolean bool_val)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	boolean isXsiNilAttr = FALSE;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start boolean data serialization\n"));

	if(strm->context.expectATData > 0) // Value for an attribute
	{
		strm->context.expectATData -= 1;
		if(strm->context.currAttr.uriId == XML_SCHEMA_INSTANCE_ID && strm->context.currAttr.lnId == XML_SCHEMA_INSTANCE_NIL_ID)
		{
			// xsi:nill
			isXsiNilAttr = TRUE;
		}
	}
	else
	{
		Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

		tmp_err_code = encodeProduction(strm, EVENT_CH_CLASS, VALUE_TYPE_BOOLEAN_CLASS, NULL, &prodHit);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = encodeBoolean(strm, bool_val);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(IS_SCHEMA(strm->gStack->grammar->props) && isXsiNilAttr && bool_val)
	{
		// In a schema-informed grammar && xsi:nil == TRUE
		strm->context.isNilType = TRUE;
		strm->context.currNonTermID = GR_START_TAG_CONTENT;
	}

	return ERR_OK;
}

errorCode stringData(EXIStream* strm, const String str_val)
{
	QNameID qnameID;
	Index typeId;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start string data serialization\n"));

	if(strm->context.expectATData > 0) // Value for an attribute
	{
		strm->context.expectATData -= 1;
		qnameID = strm->context.currAttr;
		typeId = strm->context.attrTypeId;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

		tmp_err_code = encodeProduction(strm, EVENT_CH_CLASS, VALUE_TYPE_STRING_CLASS, NULL, &prodHit);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		qnameID = strm->context.currElem;
		typeId = prodHit.typeId;
	}

	return encodeStringData(strm, str_val, qnameID, typeId);
}

errorCode floatData(EXIStream* strm, Float float_val)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start float data serialization\n"));

	if(strm->context.expectATData > 0) // Value for an attribute
	{
		strm->context.expectATData -= 1;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

		tmp_err_code = encodeProduction(strm, EVENT_CH_CLASS, VALUE_TYPE_FLOAT_CLASS, NULL, &prodHit);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return encodeFloatValue(strm, float_val);
}

errorCode binaryData(EXIStream* strm, const char* binary_val, Index nbytes)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start float data serialization\n"));

	if(strm->context.expectATData > 0) // Value for an attribute
	{
		strm->context.expectATData -= 1;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

		tmp_err_code = encodeProduction(strm, EVENT_CH_CLASS, VALUE_TYPE_BINARY_CLASS, NULL, &prodHit);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return encodeBinary(strm, (char *)binary_val, nbytes);
}

errorCode dateTimeData(EXIStream* strm, EXIPDateTime dt_val)
{
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start dateTime data serialization\n"));

	if(strm->context.expectATData > 0) // Value for an attribute
	{
		strm->context.expectATData -= 1;
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

		tmp_err_code = encodeProduction(strm, EVENT_CH_CLASS, VALUE_TYPE_DATE_TIME_CLASS, NULL, &prodHit);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return encodeDateTimeValue(strm, dt_val);
}

errorCode decimalData(EXIStream* strm, Decimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode listData(EXIStream* strm, unsigned int itemCount)
{
	Index typeId;
	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Start list data serialization\n"));

	if(strm->context.expectATData > 0) // Value for an attribute
	{
		strm->context.expectATData -= 1;
		typeId = strm->context.attrTypeId;

		// TODO: is it allowed to have list with elements lists??? To be checked...
	}
	else
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

		tmp_err_code = encodeProduction(strm, EVENT_CH_CLASS, VALUE_TYPE_LIST_CLASS, NULL, &prodHit);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	strm->context.expectATData = itemCount;
 	strm->context.attrTypeId = strm->schema->simpleTypeTable.sType[typeId].length; // The actual type of the list items

	return encodeUnsignedInteger(strm, (UnsignedInteger) itemCount);
}

errorCode processingInstruction(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode namespaceDeclaration(EXIStream* strm, const String ns, const String prefix, boolean isLocalElementNS)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SmallIndex uriId;
	Production prodHit = {0, INDEX_MAX, {URI_MAX, LN_MAX}};

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Start namespace declaration\n"));

	tmp_err_code = encodeProduction(strm, EVENT_NS_CLASS, VALUE_TYPE_NONE_CLASS, NULL, &prodHit);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = encodeUri(strm, (String*) &ns, &uriId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->schema->uriTable.uri[uriId].pfxTable == NULL)
	{
		tmp_err_code = createPfxTable(&strm->schema->uriTable.uri[uriId].pfxTable);
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

	freeAllMem(strm);
	return tmp_err_code;
}

errorCode serializeEvent(EXIStream* strm, EventCode ec, QName* qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	GrammarRule* currentRule;
	Production* tmpProd = NULL;

	if(strm->context.currNonTermID >=  strm->gStack->grammar->count)
		return INCONSISTENT_PROC_STATE;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[strm->context.currNonTermID];
	else
		currentRule = &strm->gStack->grammar->rule[strm->context.currNonTermID];

	tmp_err_code = writeEventCode(strm, ec);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(ec.length == 1)
	{
		tmpProd = &currentRule->production[currentRule->pCount - 1 - ec.part[0]];
	}
	else if(ec.length == 2)
		return NOT_IMPLEMENTED_YET;
	else // length == 3
		return NOT_IMPLEMENTED_YET;

	strm->context.currNonTermID = GET_PROD_NON_TERM(tmpProd->content);

	switch(GET_PROD_EXI_EVENT(tmpProd->content))
	{
		case EVENT_SD:
			return ERR_OK;
		break;
		case EVENT_ED:
			return ERR_OK;
		break;
		case EVENT_AT_QNAME:

			strm->context.currAttr.uriId = tmpProd->qnameId.uriId;
			strm->context.currAttr.lnId = tmpProd->qnameId.lnId;

			tmp_err_code = encodePfxQName(strm, qname, EVENT_AT_QNAME, tmpProd->qnameId.uriId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			strm->context.expectATData = TRUE;
			strm->context.attrTypeId = tmpProd->typeId;

		break;
		case EVENT_AT_URI:
			return NOT_IMPLEMENTED_YET;
		break;
		case EVENT_AT_ALL:
			if(qname == NULL)
				return NULL_POINTER_REF;

			tmp_err_code = encodeQName(strm, *qname, EVENT_AT_ALL, &strm->context.currAttr);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			strm->context.expectATData = TRUE;
			strm->context.attrTypeId = tmpProd->typeId;
		break;
		case EVENT_SE_QNAME:
		{
			EXIGrammar* elemGrammar = NULL;

			strm->context.currElem.uriId = tmpProd->qnameId.uriId;
			strm->context.currElem.lnId = tmpProd->qnameId.lnId;

			tmp_err_code = encodePfxQName(strm, qname, EVENT_SE_QNAME, tmpProd->qnameId.uriId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			strm->gStack->lastNonTermID = strm->context.currNonTermID;

			// New element grammar is pushed on the stack
			if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
			{
				elemGrammar = GET_ELEM_GRAMMAR_QNAMEID(strm->schema, strm->context.currElem);
			}
			else
			{
				elemGrammar = &strm->schema->grammarTable.grammar[tmpProd->typeId];
			}

			if(elemGrammar != NULL) // The grammar is found
			{
				strm->context.currNonTermID = GR_START_TAG_CONTENT;
				tmp_err_code = pushGrammar(&(strm->gStack), elemGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
			else
			{
				return INCONSISTENT_PROC_STATE;  // The event require the presence of Element Grammar previously created
			}
		}
		break;
		case EVENT_SE_URI:
			return NOT_IMPLEMENTED_YET;
		break;
		case EVENT_SE_ALL:
		{
			EXIGrammar* elemGrammar = NULL;

			if(qname == NULL)
				return NULL_POINTER_REF;

			tmp_err_code = encodeQName(strm, *qname, EVENT_SE_ALL, &strm->context.currElem);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			strm->gStack->lastNonTermID = strm->context.currNonTermID;
			// New element grammar is pushed on the stack
			elemGrammar = GET_ELEM_GRAMMAR_QNAMEID(strm->schema, strm->context.currElem);

			if(elemGrammar != NULL) // The grammar is found
			{
				strm->context.currNonTermID = GR_START_TAG_CONTENT;
				tmp_err_code = pushGrammar(&(strm->gStack), elemGrammar);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
			else
			{
				EXIGrammar newElementGrammar;
				Index dynArrIndx;
				tmp_err_code = createBuiltInElementGrammar(&newElementGrammar, strm);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = addDynEntry(&strm->schema->grammarTable.dynArray, &newElementGrammar, &dynArrIndx);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currElem).elemGrammar = dynArrIndx;

				strm->context.currNonTermID = GR_START_TAG_CONTENT;
				tmp_err_code = pushGrammar(&(strm->gStack), &strm->schema->grammarTable.grammar[dynArrIndx]);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		break;
		case EVENT_EE:
			assert(strm->context.currNonTermID == GR_VOID_NON_TERMINAL);

			EXIGrammar* grammar;
			popGrammar(&(strm->gStack), &grammar);
			if(strm->gStack != NULL) // There is more grammars in the stack
				strm->context.currNonTermID = strm->gStack->lastNonTermID;
		break;
		case EVENT_CH:
			return NOT_IMPLEMENTED_YET;
		break;
		case EVENT_NS:
			return NOT_IMPLEMENTED_YET;
		break;
		case EVENT_CM:
			return NOT_IMPLEMENTED_YET;
		break;
		case EVENT_PI:
			return NOT_IMPLEMENTED_YET;
		break;
		case EVENT_DT:
			return NOT_IMPLEMENTED_YET;
		break;
		case EVENT_ER:
			return NOT_IMPLEMENTED_YET;
		break;
		case EVENT_SC:
			return NOT_IMPLEMENTED_YET;
		break;
		default:
			return INCONSISTENT_PROC_STATE;
	}

	return ERR_OK;
}
