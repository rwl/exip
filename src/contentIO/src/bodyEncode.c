/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file bodyEncode.c
 * @brief Implementation of data and events serialization
 *
 * @date Mar 23, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "bodyEncode.h"
#include "sTables.h"
#include "streamEncode.h"
#include "ioUtil.h"
#include "stringManipulate.h"
#include "grammars.h"
#include "memManagement.h"
#include "dynamicArray.h"

extern const String XML_SCHEMA_INSTANCE;

/**
 * @brief Encodes second or third level production based on a state machine  */
static errorCode stateMachineProdEncode(EXIStream* strm, EventTypeClass eventClass, GrammarRule* currentRule,
										QName* qname, EventCode ec, Production* prodHit);

errorCode encodeStringData(EXIStream* strm, String strng, QNameID qnameID, Index typeId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	boolean flag_StringLiteralsPartition = 0;
	Index vxEntryId = 0;
	VxTable* vxTable = &GET_LN_URI_QNAME(strm->schema->uriTable, qnameID).vxTable;

	/* ENUMERATION CHECK */
	if(typeId != INDEX_MAX && HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content,TYPE_FACET_ENUMERATION))
	{
		// There is enumeration defined
		EnumDefinition eDefSearch;
		EnumDefinition* eDefFound;
		SmallIndex i;

		eDefSearch.typeId = typeId;
		eDefFound = bsearch(&eDefSearch, strm->schema->enumTable.enumDef, strm->schema->enumTable.count, sizeof(EnumDefinition), compareEnumDefs);
		if(eDefFound == NULL)
			return UNEXPECTED_ERROR;

		for(i = 0; i < eDefFound->count; i++)
		{
			if(stringEqual(((String*) eDefFound->values)[i], strng))
			{
				return encodeNBitUnsignedInteger(strm, getBitsNumber(eDefFound->count - 1), i);
			}
		}
		/* The enum value is not found! */
		return UNEXPECTED_ERROR;
	}

	flag_StringLiteralsPartition = lookupVx(&strm->valueTable, vxTable, strng, &vxEntryId);
	if(flag_StringLiteralsPartition && vxTable->vx[vxEntryId].globalId != INDEX_MAX) //  "local" value partition table hit; when INDEX_MAX -> compact identifier permanently unassigned
	{
		unsigned char vxBits;

		tmp_err_code = encodeUnsignedInteger(strm, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		vxBits = getBitsNumber(vxTable->count - 1);
		tmp_err_code = encodeNBitUnsignedInteger(strm, vxBits, vxEntryId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else //  "local" value partition table miss
	{
		Index valueEntryId = 0;
		flag_StringLiteralsPartition = lookupValue(&strm->valueTable, strng, &valueEntryId);
		if(flag_StringLiteralsPartition) // global value partition table hit
		{
			unsigned char valueBits;

			tmp_err_code = encodeUnsignedInteger(strm, 1);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			valueBits = getBitsNumber((unsigned int)(strm->valueTable.count - 1));
			tmp_err_code = encodeNBitUnsignedInteger(strm, valueBits, (unsigned int)(valueEntryId) );
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else // "local" value partition and global value partition table miss
		{
			tmp_err_code = encodeUnsignedInteger(strm, (UnsignedInteger)(strng.length + 2));
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			tmp_err_code = encodeStringOnly(strm, &strng);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(strng.length > 0 && strng.length <= strm->header.opts.valueMaxLength && strm->header.opts.valuePartitionCapacity > 0)
			{
				// The value should be added in the value partitions of the string tables
				String clonedValue;

				tmp_err_code = cloneString(&strng, &clonedValue);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = addValueEntry(strm, clonedValue, qnameID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
	}

	return ERR_OK;
}

errorCode encodeProduction(EXIStream* strm, EventTypeClass eventClass, boolean isSchemaType, QName* qname, Production* prodHit)
{
	GrammarRule* currentRule;
	EventCode ec;
	Index j = 0;
	Production* tmpProd = NULL;
	Index prodCount;
	unsigned int bitCount;
	boolean matchFound = FALSE;

	if(strm->context.currNonTermID >=  strm->gStack->grammar->count)
		return INCONSISTENT_PROC_STATE;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
	{
		currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[strm->context.currNonTermID];
		prodCount = currentRule->pCount;
	}
	else
	{
		currentRule = &strm->gStack->grammar->rule[strm->context.currNonTermID];

		if(strm->context.isNilType)
		{
			prodCount = RULE_GET_AT_COUNT(currentRule->meta);
			if(eventClass == EVENT_EE_CLASS)
			{
				if(RULE_CONTAIN_EE(currentRule->meta))
				{
					ec.length = 1;
					ec.part[0] = prodCount;
					ec.bits[0] = getBitsNumber(prodCount);
					strm->context.currNonTermID = GR_VOID_NON_TERMINAL;

					return writeEventCode(strm, ec);
				}
			}
		}
		else
			prodCount = currentRule->pCount;
	}

#if DEBUG_CONTENT_IO == ON
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		tmp_err_code = printGrammarRule(strm->context.currNonTermID, currentRule, strm->schema);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Error printing grammar rule\n"));
			return tmp_err_code;
		}
	}
#endif

	bitCount = getBitsFirstPartCode(strm->header.opts, strm->gStack->grammar, currentRule, strm->context.currNonTermID, strm->context.isNilType);

	if(isSchemaType == TRUE)
	{
		for(j = 0; j < prodCount; j++)
		{
			tmpProd = &currentRule->production[currentRule->pCount - 1 - j];

			if(GET_EVENT_CLASS(GET_PROD_EXI_EVENT(tmpProd->content)) == eventClass)
			{
				if(eventClass == EVENT_AT_CLASS || eventClass == EVENT_SE_CLASS)
				{
					assert(qname);
					if(tmpProd->qnameId.uriId == URI_MAX || (stringEqual(strm->schema->uriTable.uri[tmpProd->qnameId.uriId].uriStr, *(qname->uri)) &&
					   (tmpProd->qnameId.lnId == LN_MAX || stringEqual(GET_LN_URI_QNAME(strm->schema->uriTable, tmpProd->qnameId).lnStr, *(qname->localName)))))
					{
						matchFound = TRUE;
						break;
					}
				}
				else
				{
					matchFound = TRUE;
					break;
				}
			}
		}
	}

	if(matchFound == TRUE)
	{
		*prodHit = *tmpProd;
		ec.length = 1;
		ec.part[0] = j;
		ec.bits[0] = bitCount;
		strm->context.currNonTermID = GET_PROD_NON_TERM(tmpProd->content);

		return writeEventCode(strm, ec);
	}
	else
	{
		// Production not found: encoded as second or third level production
		ec.length = 2;
		ec.part[0] = prodCount;
		ec.bits[0] = bitCount;

		return stateMachineProdEncode(strm, eventClass, currentRule, qname, ec, prodHit);
	}
}

static errorCode stateMachineProdEncode(EXIStream* strm, EventTypeClass eventClass,
						GrammarRule* currentRule, QName* qname, EventCode ec, Production* prodHit)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QNameID voidQnameID = {SMALL_INDEX_MAX, INDEX_MAX};
	QNameID qnameID;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))
	{
		// Built-in element grammar

		if(strm->context.currNonTermID == GR_START_TAG_CONTENT)
		{
			ec.bits[1] = getBitsNumber(3 +
									   (IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) +
										WITH_SELF_CONTAINED(strm->header.opts.enumOpt) +
										IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD) +
										(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS) != 0)));
		}
		else if(strm->context.currNonTermID == GR_ELEMENT_CONTENT)
		{
			ec.bits[1] = getBitsNumber(1 + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD) +
										(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS) != 0));
		}
		else
			return INCONSISTENT_PROC_STATE;

		switch(eventClass)
		{
			case EVENT_EE_CLASS:
				if(strm->context.currNonTermID != GR_START_TAG_CONTENT)
					return INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_EE);
				ec.part[1] = 0;
				strm->context.currNonTermID = GR_VOID_NON_TERMINAL;

				// #1# COMMENT and #2# COMMENT
				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_EE, GR_VOID_NON_TERMINAL, &voidQnameID, 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			break;
			case EVENT_AT_CLASS:
				if(strm->context.currNonTermID != GR_START_TAG_CONTENT)
					return INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_ALL);
				ec.part[1] = 1;
				strm->context.currNonTermID = GR_START_TAG_CONTENT;

				if(!lookupUri(&strm->schema->uriTable, *qname->uri, &qnameID.uriId))
				{
					qnameID.uriId = strm->schema->uriTable.count;
					qnameID.lnId = 0;
				}
				else if(!lookupLn(&strm->schema->uriTable.uri[qnameID.uriId].lnTable, *qname->localName,  &qnameID.lnId))
				{
					qnameID.lnId = strm->schema->uriTable.uri[qnameID.uriId].lnTable.count;
				}

				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_AT_QNAME, GR_START_TAG_CONTENT, &qnameID, 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			break;
			case EVENT_NS_CLASS:
				if(strm->context.currNonTermID != GR_START_TAG_CONTENT || !IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
					return INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_NS);
				ec.part[1] = 2;
				strm->context.currNonTermID = GR_START_TAG_CONTENT;
			break;
			case EVENT_SC_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			case EVENT_SE_CLASS:
				SET_PROD_EXI_EVENT(prodHit->content, EVENT_SE_ALL);
				if(strm->context.currNonTermID == GR_START_TAG_CONTENT)
					ec.part[1] = 2 + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) + WITH_SELF_CONTAINED(strm->header.opts.enumOpt);
				else
					ec.part[1] = 0;
				strm->context.currNonTermID = GR_ELEMENT_CONTENT;

				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &strm->context.currElem, 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			break;
			case EVENT_CH_CLASS:
				SET_PROD_EXI_EVENT(prodHit->content, EVENT_CH);
				if(strm->context.currNonTermID == GR_START_TAG_CONTENT)
					ec.part[1] = 3 + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) + WITH_SELF_CONTAINED(strm->header.opts.enumOpt);
				else
					ec.part[1] = 1;
				strm->context.currNonTermID = GR_ELEMENT_CONTENT;

				// #1# COMMENT and #2# COMMENT
				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, GR_ELEMENT_CONTENT, &voidQnameID, 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			break;
			case EVENT_ER_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			case EVENT_CM_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			case EVENT_PI_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			default:
				return INCONSISTENT_PROC_STATE;
		}
	}
	else if(IS_DOCUMENT(strm->gStack->grammar->props))
	{
		// Document grammar
		switch(eventClass)
		{
			case EVENT_DT_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			case EVENT_CM_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			case EVENT_PI_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			default:
				return INCONSISTENT_PROC_STATE;
		}
	}
	else if(IS_FRAGMENT(strm->gStack->grammar->props))
	{
		// Fragment grammar
		switch(eventClass)
		{
			case EVENT_CM_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			case EVENT_PI_CLASS:
				return NOT_IMPLEMENTED_YET;
			break;
			default:
				return INCONSISTENT_PROC_STATE;
		}
	}
	else
	{
		// Schema-informed element/type grammar
		if(WITH_STRICT(strm->header.opts.enumOpt))
		{
			// Strict mode
			if(strm->context.currNonTermID != GR_START_TAG_CONTENT ||
					eventClass != EVENT_AT_CLASS ||
					!stringEqual(*qname->uri, XML_SCHEMA_INSTANCE))
				return INCONSISTENT_PROC_STATE;
			if(stringEqualToAscii(*qname->localName, "type"))
			{
				if(!HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props))
					return INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_QNAME);
				prodHit->qnameId.uriId = XML_SCHEMA_INSTANCE_ID;
				prodHit->qnameId.lnId = XML_SCHEMA_INSTANCE_TYPE_ID;
				ec.part[1] = 0;

				if(!IS_NILLABLE(strm->gStack->grammar->props))
					ec.bits[1] = 0;
				else
					ec.bits[1] = 1;
			}
			else if(stringEqualToAscii(*qname->localName, "nil"))
			{
				if(!IS_NILLABLE(strm->gStack->grammar->props))
					return INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_QNAME);
				prodHit->qnameId.uriId = XML_SCHEMA_INSTANCE_ID;
				prodHit->qnameId.lnId = XML_SCHEMA_INSTANCE_NIL_ID;

				if(!HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props))
				{
					ec.part[1] = 0;
					ec.bits[1] = 0;
				}
				else
				{
					ec.part[1] = 1;
					ec.bits[1] = 1;
				}
			}
			else
				return INCONSISTENT_PROC_STATE;
		}
		else // Non-strict mode
		{
			unsigned int prod2Count = 0;

			prod2Count += 5; // EE, AT (*), AT (*) [untyped value], SE (*), CH [untyped value]
			if(!RULE_CONTAIN_EE(currentRule->meta))
				prod2Count += 1;
			if(strm->context.currNonTermID == GR_START_TAG_CONTENT)
			{
				prod2Count += 2; // AT(xsi:type) Element i,0 and AT(xsi:nil) Element i,0
				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
					prod2Count += 1; // NS

				if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
					prod2Count += 1; // SC
			}
			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
				prod2Count += 1; // ER
			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
				prod2Count += 1; // CM & PI

			switch(eventClass)
			{
				case EVENT_EE_CLASS:
					assert(!RULE_CONTAIN_EE(currentRule->meta));

					SET_PROD_EXI_EVENT(prodHit->content, EVENT_EE);
					ec.length = 2;
					ec.part[1] = 0;
					ec.bits[1] = getBitsNumber(prod2Count - 1);
					strm->context.currNonTermID = GR_VOID_NON_TERMINAL;
				break;
				case EVENT_AT_CLASS:

					if(strm->context.currNonTermID >= GET_CONTENT_INDEX(strm->gStack->grammar->props))
						return INCONSISTENT_PROC_STATE;

					if(!lookupUri(&strm->schema->uriTable, *qname->uri, &qnameID.uriId))
					{
						qnameID.uriId = strm->schema->uriTable.count;
						qnameID.lnId = 0;
					}
					else if(!lookupLn(&strm->schema->uriTable.uri[qnameID.uriId].lnTable, *qname->localName,  &qnameID.lnId))
					{
						qnameID.lnId = strm->schema->uriTable.uri[qnameID.uriId].lnTable.count;
					}

					prodHit->qnameId = qnameID;

					if(qnameID.uriId == XML_SCHEMA_INSTANCE_ID)
					{
						if(qnameID.lnId == XML_SCHEMA_INSTANCE_NIL_ID)
						{
							if(strm->context.currNonTermID != GR_START_TAG_CONTENT)
								return INCONSISTENT_PROC_STATE;

							SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_QNAME);
							ec.length = 2;
							ec.part[1] = 1 + !RULE_CONTAIN_EE(currentRule->meta);
							ec.bits[1] = getBitsNumber(prod2Count - 1);
						}
						else if(qnameID.lnId == XML_SCHEMA_INSTANCE_TYPE_ID)
						{
							if(strm->context.currNonTermID != GR_START_TAG_CONTENT)
								return INCONSISTENT_PROC_STATE;

							SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_QNAME);
							ec.length = 2;
							ec.part[1] = 0 + !RULE_CONTAIN_EE(currentRule->meta);
							ec.bits[1] = getBitsNumber(prod2Count - 1);
						}
						else
							return NOT_IMPLEMENTED_YET;
					}
					else // All other cases of AT events
						return NOT_IMPLEMENTED_YET;
				break;
				case EVENT_NS_CLASS:
					if(strm->context.currNonTermID != GR_START_TAG_CONTENT ||
						!IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
						return INCONSISTENT_PROC_STATE;

					SET_PROD_EXI_EVENT(prodHit->content, EVENT_NS);
					ec.length = 2;
					ec.part[1] = !RULE_CONTAIN_EE(currentRule->meta) + 4;
					ec.bits[1] = getBitsNumber(prod2Count - 1);
					strm->context.currNonTermID = GR_START_TAG_CONTENT;
				break;
				case EVENT_SC_CLASS:
					return NOT_IMPLEMENTED_YET;
				break;
				case EVENT_SE_CLASS:
					return NOT_IMPLEMENTED_YET;
				break;
				case EVENT_CH_CLASS:
					return NOT_IMPLEMENTED_YET;
				break;
				case EVENT_ER_CLASS:
					return NOT_IMPLEMENTED_YET;
				break;
				case EVENT_CM_CLASS:
					return NOT_IMPLEMENTED_YET;
				break;
				case EVENT_PI_CLASS:
					return NOT_IMPLEMENTED_YET;
				break;
				default:
					return INCONSISTENT_PROC_STATE;
			}
		}
	}

	return writeEventCode(strm, ec);
}

errorCode encodeQName(EXIStream* strm, QName qname, EventType eventT, QNameID* qnameID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encoding QName\n"));

/******* Start: URI **********/
	tmp_err_code = encodeUri(strm, (String*) qname.uri, &qnameID->uriId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
/******* End: URI **********/

/******* Start: Local name **********/
	tmp_err_code = encodeLn(strm, (String*) qname.localName, qnameID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
/******* End: Local name **********/

	return encodePfxQName(strm, &qname, eventT, qnameID->uriId);
}

errorCode encodeUri(EXIStream* strm, String* uri, SmallIndex* uriId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char uriBits = getBitsNumber(strm->schema->uriTable.count);

	if(lookupUri(&strm->schema->uriTable, *uri, uriId)) // uri hit
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, uriBits, *uriId + 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else  // uri miss
	{
		String copiedURI;
		tmp_err_code = encodeNBitUnsignedInteger(strm, uriBits, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = encodeString(strm, uri);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = cloneStringManaged(uri, &copiedURI, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addUriEntry(&strm->schema->uriTable, copiedURI, uriId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode encodeLn(EXIStream* strm, String* ln, QNameID* qnameID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(lookupLn(&strm->schema->uriTable.uri[qnameID->uriId].lnTable, *ln, &qnameID->lnId)) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->schema->uriTable.uri[qnameID->uriId].lnTable.count - 1));
		tmp_err_code = encodeUnsignedInteger(strm, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeNBitUnsignedInteger(strm, lnBits, (unsigned int)(qnameID->lnId) );
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // local-name table miss
	{
		String copiedLN;
		tmp_err_code = encodeUnsignedInteger(strm, (UnsignedInteger)(ln->length + 1) );
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = encodeStringOnly(strm,  ln);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(strm->schema->uriTable.uri[qnameID->uriId].lnTable.ln == NULL)
		{
			// Create local name table for this URI entry
			tmp_err_code = createDynArray(&strm->schema->uriTable.uri[qnameID->uriId].lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		tmp_err_code = cloneStringManaged(ln, &copiedLN, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLnEntry(&strm->schema->uriTable.uri[qnameID->uriId].lnTable, copiedLN, &qnameID->lnId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode encodePfxQName(EXIStream* strm, QName* qname, EventType eventT, SmallIndex uriId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char prefixBits = 0;
	SmallIndex prefixID = 0;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) == FALSE)
		return ERR_OK;

	if(strm->schema->uriTable.uri[uriId].pfxTable == NULL || strm->schema->uriTable.uri[uriId].pfxTable->count == 0)
		return ERR_OK;

	prefixBits = getBitsNumber(strm->schema->uriTable.uri[uriId].pfxTable->count - 1);

	if(prefixBits > 0)
	{
		if(qname == NULL)
			return NULL_POINTER_REF;

		if(lookupPfx(strm->schema->uriTable.uri[uriId].pfxTable, *qname->prefix, &prefixID) == TRUE)
		{
			tmp_err_code = encodeNBitUnsignedInteger(strm, prefixBits, (unsigned int) prefixID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			if(eventT != EVENT_SE_ALL)
				return INCONSISTENT_PROC_STATE;

			tmp_err_code = encodeNBitUnsignedInteger(strm, prefixBits, 0);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

errorCode encodePfx(EXIStream* strm, SmallIndex uriId, String* prefix)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SmallIndex pfxId;
	unsigned char pfxBits = getBitsNumber(strm->schema->uriTable.uri[uriId].pfxTable->count);

	if(lookupPfx(strm->schema->uriTable.uri[uriId].pfxTable, *prefix, &pfxId)) // prefix hit
	{
		tmp_err_code = encodeNBitUnsignedInteger(strm, pfxBits, pfxId + 1);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else  // prefix miss
	{
		String copiedPrefix;
		tmp_err_code = encodeNBitUnsignedInteger(strm, pfxBits, 0);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		tmp_err_code = encodeString(strm, prefix);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = cloneStringManaged(prefix, &copiedPrefix, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addPfxEntry(strm->schema->uriTable.uri[uriId].pfxTable, copiedPrefix, &pfxId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode encodeIntData(EXIStream* strm, Integer int_val, Index typeId)
{
	EXIType exiType;

	exiType = GET_EXI_TYPE(strm->schema->simpleTypeTable.sType[typeId].content);

	if(exiType == VALUE_TYPE_SMALL_INTEGER)
	{
		// TODO: take into account  minExclusive and  maxExclusive when they are supported
		unsigned int encoded_val;
		unsigned char numberOfBits;

		if(int_val > strm->schema->simpleTypeTable.sType[typeId].max ||
				int_val < strm->schema->simpleTypeTable.sType[typeId].min)
			return INVALID_EXI_INPUT;

		encoded_val = (unsigned int) (int_val - strm->schema->simpleTypeTable.sType[typeId].min);
		numberOfBits = getBitsNumber(strm->schema->simpleTypeTable.sType[typeId].max - strm->schema->simpleTypeTable.sType[typeId].min);

		return encodeNBitUnsignedInteger(strm, numberOfBits, encoded_val);
	}
	else if(exiType == VALUE_TYPE_NON_NEGATIVE_INT)
	{
		return encodeUnsignedInteger(strm, (UnsignedInteger) int_val);
	}
	else if(exiType == VALUE_TYPE_INTEGER)
	{
		return encodeIntegerValue(strm, int_val);
	}
	else
	{
		return INCONSISTENT_PROC_STATE;
	}
}
