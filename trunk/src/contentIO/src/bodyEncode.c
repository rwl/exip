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
 * @version 0.5
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
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	boolean flag_StringLiteralsPartition = FALSE;

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
			return EXIP_UNEXPECTED_ERROR;

		for(i = 0; i < eDefFound->count; i++)
		{
			if(stringEqual(((String*) eDefFound->values)[i], strng))
			{
				return encodeNBitUnsignedInteger(strm, getBitsNumber(eDefFound->count - 1), i);
			}
		}
		/* The enum value is not found! */
		return EXIP_UNEXPECTED_ERROR;
	}
#if VALUE_CROSSTABLE_USE
	{
		VxTable* vxTable = GET_LN_URI_QNAME(strm->schema->uriTable, qnameID).vxTable;
		Index vxEntryId = 0;
		flag_StringLiteralsPartition = lookupVx(&strm->valueTable, vxTable, strng, &vxEntryId);
		if(flag_StringLiteralsPartition && vxTable->vx[vxEntryId].globalId != INDEX_MAX) //  "local" value partition table hit; when INDEX_MAX -> compact identifier permanently unassigned
		{
			unsigned char vxBits;

			TRY(encodeUnsignedInteger(strm, 0));
			vxBits = getBitsNumber(vxTable->count - 1);
			TRY(encodeNBitUnsignedInteger(strm, vxBits, vxEntryId));
			return EXIP_OK;
		}
	}
#endif
	{ //  "local" value partition table miss
		Index valueEntryId = 0;
		flag_StringLiteralsPartition = lookupValue(&strm->valueTable, strng, &valueEntryId);
		if(flag_StringLiteralsPartition) // global value partition table hit
		{
			unsigned char valueBits;

			TRY(encodeUnsignedInteger(strm, 1));
			valueBits = getBitsNumber((unsigned int)(strm->valueTable.count - 1));
			TRY(encodeNBitUnsignedInteger(strm, valueBits, (unsigned int)(valueEntryId)));
		}
		else // "local" value partition and global value partition table miss
		{
			TRY(encodeUnsignedInteger(strm, (UnsignedInteger)(strng.length + 2)));
			TRY(encodeStringOnly(strm, &strng));

			if(strng.length > 0 && strng.length <= strm->header.opts.valueMaxLength && strm->header.opts.valuePartitionCapacity > 0)
			{
				// The value should be added in the value partitions of the string tables
				String clonedValue;

				TRY(cloneString(&strng, &clonedValue));
				TRY(addValueEntry(strm, clonedValue, qnameID));
			}
		}
	}

	return EXIP_OK;
}

errorCode encodeProduction(EXIStream* strm, EventTypeClass eventClass, boolean isSchemaType, QName* qname, EXITypeClass chTypeClass, Production* prodHit)
{
	GrammarRule* currentRule;
	EventCode ec;
	Index j = 0;
	Production* tmpProd = NULL;
	Index prodCount;
	unsigned int bitCount;
	boolean matchFound = FALSE;
	SmallIndex currNonTermID = strm->gStack->currNonTermID;

	// TODO: GR_CONTENT_2 is only needed when schema deviations are allowed.
	//       Here and in many other places when schema deviations are fully disabled
	//       many parts of the code can be pruned during compile time using macro parameters in the build.
	//       This includes the EXI Profile features itself (excluding localValueCapping)
	if(currNonTermID == GR_CONTENT_2)
		currNonTermID = GET_CONTENT_INDEX(strm->gStack->grammar->props);

	if(currNonTermID >=  strm->gStack->grammar->count)
		return EXIP_INCONSISTENT_PROC_STATE;

#if BUILD_IN_GRAMMARS_USE
	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
	{
		currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[currNonTermID];
		prodCount = currentRule->pCount;
	}
	else
#endif
	{
		currentRule = &strm->gStack->grammar->rule[currNonTermID];

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
					strm->gStack->currNonTermID = GR_VOID_NON_TERMINAL;
					strm->context.isNilType = FALSE;

					return writeEventCode(strm, ec);
				}
			}
		}
		else
			prodCount = currentRule->pCount;
	}

#if DEBUG_CONTENT_IO == ON
	{
		errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
		TRY(printGrammarRule(currNonTermID, currentRule, strm->schema));
	}
#endif

	bitCount = getBitsFirstPartCode(strm, currentRule, currNonTermID);

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
				else if(eventClass == EVENT_CH_CLASS)
				{
					EXIType exiType;
					if(tmpProd->typeId != INDEX_MAX)
						exiType = GET_EXI_TYPE(strm->schema->simpleTypeTable.sType[tmpProd->typeId].content);
					else
						exiType = VALUE_TYPE_NONE;

					if(exiType == VALUE_TYPE_NONE || exiType == VALUE_TYPE_UNTYPED)
						matchFound = TRUE;
					else if(chTypeClass == GET_VALUE_TYPE_CLASS(exiType))
						matchFound = TRUE;
					break;
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
		strm->gStack->currNonTermID = GET_PROD_NON_TERM(tmpProd->content);

		return writeEventCode(strm, ec);
	}
	else
	{
		// Production not found: encoded as second or third level production

		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Secodn/third level production \n"));

		ec.length = 2;
		ec.part[0] = prodCount;
		ec.bits[0] = bitCount;

		return stateMachineProdEncode(strm, eventClass, currentRule, qname, ec, prodHit);
	}
}

static errorCode stateMachineProdEncode(EXIStream* strm, EventTypeClass eventClass,
						GrammarRule* currentRule, QName* qname, EventCode ec, Production* prodHit)
{
	QNameID qnameID;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))
	{
		// Built-in element grammar
#if BUILD_IN_GRAMMARS_USE
		errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
		QNameID voidQnameID = {SMALL_INDEX_MAX, INDEX_MAX};
		if(strm->gStack->currNonTermID == GR_START_TAG_CONTENT)
		{
			ec.bits[1] = getBitsNumber(3 +
									   (IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) +
										WITH_SELF_CONTAINED(strm->header.opts.enumOpt) +
										IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD) +
										(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS) != 0)));
		}
		else if(strm->gStack->currNonTermID == GR_ELEMENT_CONTENT)
		{
			ec.bits[1] = getBitsNumber(1 + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD) +
										(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS) != 0));
		}
		else
			return EXIP_INCONSISTENT_PROC_STATE;

		switch(eventClass)
		{
			case EVENT_EE_CLASS:
				if(strm->gStack->currNonTermID != GR_START_TAG_CONTENT)
					return EXIP_INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_EE);
				ec.part[1] = 0;
				strm->gStack->currNonTermID = GR_VOID_NON_TERMINAL;

				// #1# COMMENT and #2# COMMENT
				// NOTE: In general, first you need to check if EE does not already exists, just then insert it
				// However, the encodeProduction(); will always use first level EE if exists so no such check is needed here.
				TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_EE, GR_VOID_NON_TERMINAL, &voidQnameID, 1));
			break;
			case EVENT_AT_CLASS:
				if(strm->gStack->currNonTermID != GR_START_TAG_CONTENT)
					return EXIP_INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_ALL);
				ec.part[1] = 1;
				strm->gStack->currNonTermID = GR_START_TAG_CONTENT;

				if(!lookupUri(&strm->schema->uriTable, *qname->uri, &qnameID.uriId))
				{
					qnameID.uriId = strm->schema->uriTable.count;
					qnameID.lnId = 0;
				}
				else if(!lookupLn(&strm->schema->uriTable.uri[qnameID.uriId].lnTable, *qname->localName,  &qnameID.lnId))
				{
					qnameID.lnId = strm->schema->uriTable.uri[qnameID.uriId].lnTable.count;
				}

				// If eventType == AT(qname) and qname == xsi:type check first if there is no
				// such production already at top level (see http://www.w3.org/XML/EXI/exi-10-errata#Substantive20120508)
				// If there is no -> insert one, otherwise don't insert it
				if(qnameID.uriId == XML_SCHEMA_INSTANCE_ID && qnameID.lnId == XML_SCHEMA_INSTANCE_TYPE_ID)
				{
					if(!RULE_CONTAIN_XSI_TYPE(((DynGrammarRule*) currentRule)->meta))
					{
						RULE_SET_CONTAIN_XSI_TYPE(((DynGrammarRule*) currentRule)->meta);
						TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_AT_QNAME, GR_START_TAG_CONTENT, &qnameID, 1));
					}
				}
				else
					TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_AT_QNAME, GR_START_TAG_CONTENT, &qnameID, 1));
			break;
			case EVENT_NS_CLASS:
				if(strm->gStack->currNonTermID != GR_START_TAG_CONTENT || !IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
					return EXIP_INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_NS);
				ec.part[1] = 2;
				strm->gStack->currNonTermID = GR_START_TAG_CONTENT;
			break;
			case EVENT_SC_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case EVENT_SE_CLASS:
				SET_PROD_EXI_EVENT(prodHit->content, EVENT_SE_ALL);
				if(strm->gStack->currNonTermID == GR_START_TAG_CONTENT)
					ec.part[1] = 2 + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) + WITH_SELF_CONTAINED(strm->header.opts.enumOpt);
				else
					ec.part[1] = 0;
				strm->gStack->currNonTermID = GR_ELEMENT_CONTENT;

				if(!lookupUri(&strm->schema->uriTable, *qname->uri, &qnameID.uriId))
				{
					qnameID.uriId = strm->schema->uriTable.count;
					qnameID.lnId = 0;
				}
				else if(!lookupLn(&strm->schema->uriTable.uri[qnameID.uriId].lnTable, *qname->localName,  &qnameID.lnId))
				{
					qnameID.lnId = strm->schema->uriTable.uri[qnameID.uriId].lnTable.count;
				}

				TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &qnameID, 1));
			break;
			case EVENT_CH_CLASS:
				SET_PROD_EXI_EVENT(prodHit->content, EVENT_CH);
				if(strm->gStack->currNonTermID == GR_START_TAG_CONTENT)
					ec.part[1] = 3 + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) + WITH_SELF_CONTAINED(strm->header.opts.enumOpt);
				else
					ec.part[1] = 1;
				strm->gStack->currNonTermID = GR_ELEMENT_CONTENT;

				// #1# COMMENT and #2# COMMENT
				// NOTE: In general, first you need to check if CH does not already exists, just then insert it
				// However, the encodeProduction(); will always use first level CH if exists so no such check is needed here.
				TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, GR_ELEMENT_CONTENT, &voidQnameID, 1));
			break;
			case EVENT_ER_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case EVENT_CM_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case EVENT_PI_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			default:
				return EXIP_INCONSISTENT_PROC_STATE;
		}
#else
		DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Build-in element grammars are not supported by this configuration \n"));
		assert(FALSE);
		return EXIP_INCONSISTENT_PROC_STATE;
#endif
	}
	else if(IS_DOCUMENT(strm->gStack->grammar->props))
	{
		// Document grammar
		switch(eventClass)
		{
			case EVENT_DT_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case EVENT_CM_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case EVENT_PI_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			default:
				return EXIP_INCONSISTENT_PROC_STATE;
		}
	}
	else if(IS_FRAGMENT(strm->gStack->grammar->props))
	{
		// Fragment grammar
		switch(eventClass)
		{
			case EVENT_CM_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case EVENT_PI_CLASS:
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			default:
				return EXIP_INCONSISTENT_PROC_STATE;
		}
	}
	else
	{
		// Schema-informed element/type grammar
		if(WITH_STRICT(strm->header.opts.enumOpt))
		{
			// Strict mode
			if(strm->gStack->currNonTermID != GR_START_TAG_CONTENT ||
					eventClass != EVENT_AT_CLASS ||
					!stringEqual(*qname->uri, XML_SCHEMA_INSTANCE))
				return EXIP_INCONSISTENT_PROC_STATE;
			if(stringEqualToAscii(*qname->localName, "type"))
			{
				// there is AT(xsi:type) as a second level production only if this
				// grammar is not already switched using AT(xsi:type) and
				// it has sub-type or is union
				if(!HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props) ||
						strm->gStack->grammar != (GET_ELEM_GRAMMAR_QNAMEID(strm->schema, strm->gStack->currQNameID)))
					return EXIP_INCONSISTENT_PROC_STATE;

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
					return EXIP_INCONSISTENT_PROC_STATE;

				SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_QNAME);
				prodHit->qnameId.uriId = XML_SCHEMA_INSTANCE_ID;
				prodHit->qnameId.lnId = XML_SCHEMA_INSTANCE_NIL_ID;

				if(!HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props) ||
						strm->gStack->grammar != (GET_ELEM_GRAMMAR_QNAMEID(strm->schema, strm->gStack->currQNameID)))
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
				return EXIP_INCONSISTENT_PROC_STATE;
		}
		else // Non-strict mode
		{
			unsigned int prod2Count = 0;
			// Create a copy of the content grammar if and only if there are AT
			// productions that point to the content grammar rule OR the content index is 0.
			// The content2 grammar rule is only needed in case the current rule is
			// equal the content grammar rule. Note that when content index is 0, the entry grammar
			// is the content2 grammar rule and by default the GR_START_TAG_CONTENT is pointing
			// to the content2 while GR_CONTENT_2 is pointing to content i.e. the roles are
			// reversed in this situation. It is implemented in this way in order to keep
			// all the rule processing in tact in the other parts of the implementation.
			boolean isContent2Grammar = FALSE;

			if(strm->gStack->currNonTermID == GR_CONTENT_2)
			{
				if(GET_CONTENT_INDEX(strm->gStack->grammar->props) != GR_START_TAG_CONTENT)
					isContent2Grammar = TRUE;
			}
			else if(GET_CONTENT_INDEX(strm->gStack->grammar->props) == GR_START_TAG_CONTENT &&
					strm->gStack->currNonTermID == GR_START_TAG_CONTENT)
			{
					isContent2Grammar = TRUE;
			}
			prod2Count = 2; // SE(*), CH(untyped) always available

			if(isContent2Grammar ||
					strm->gStack->currNonTermID < GET_CONTENT_INDEX(strm->gStack->grammar->props))
			{
				prod2Count += 2; // AT(*), AT(untyped) third level
			}

			if(!RULE_CONTAIN_EE(currentRule->meta))
				prod2Count += 1; // EE

			if(strm->gStack->currNonTermID == GR_START_TAG_CONTENT)
			{
				prod2Count += 2; // AT(xsi:type), AT(xsi:nil)

				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
					prod2Count += 1; // NS
				if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
					prod2Count += 1; // SC
			}

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
				prod2Count += 1; // ER

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
				prod2Count += 1; // CM or PI

			switch(eventClass)
			{
				case EVENT_EE_CLASS:
					assert(!RULE_CONTAIN_EE(currentRule->meta));

					SET_PROD_EXI_EVENT(prodHit->content, EVENT_EE);
					ec.length = 2;
					ec.part[1] = 0;
					ec.bits[1] = getBitsNumber(prod2Count - 1);
					strm->gStack->currNonTermID = GR_VOID_NON_TERMINAL;
					strm->context.isNilType = FALSE;
				break;
				case EVENT_AT_CLASS:

					if(!lookupUri(&strm->schema->uriTable, *qname->uri, &qnameID.uriId))
					{
						qnameID.uriId = strm->schema->uriTable.count;
						qnameID.lnId = 0;
					}
					else if(!lookupLn(&strm->schema->uriTable.uri[qnameID.uriId].lnTable, *qname->localName,  &qnameID.lnId))
					{
						qnameID.lnId = strm->schema->uriTable.uri[qnameID.uriId].lnTable.count;
					}

					if(qnameID.uriId == XML_SCHEMA_INSTANCE_ID)
					{
						if(qnameID.lnId == XML_SCHEMA_INSTANCE_NIL_ID)
						{
							if(strm->gStack->currNonTermID != GR_START_TAG_CONTENT)
								return EXIP_INCONSISTENT_PROC_STATE;

							SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_QNAME);
							prodHit->qnameId = qnameID;
							ec.length = 2;
							ec.part[1] = 1 + !RULE_CONTAIN_EE(currentRule->meta);
							ec.bits[1] = getBitsNumber(prod2Count - 1);
							prodHit->typeId = SIMPLE_TYPE_BOOLEAN;
						}
						else if(qnameID.lnId == XML_SCHEMA_INSTANCE_TYPE_ID)
						{
							if(strm->gStack->currNonTermID != GR_START_TAG_CONTENT)
								return EXIP_INCONSISTENT_PROC_STATE;

							SET_PROD_EXI_EVENT(prodHit->content, EVENT_AT_QNAME);
							prodHit->qnameId = qnameID;
							ec.length = 2;
							ec.part[1] = 0 + !RULE_CONTAIN_EE(currentRule->meta);
							ec.bits[1] = getBitsNumber(prod2Count - 1);
							prodHit->typeId = SIMPLE_TYPE_QNAME;
						}
						else
							return EXIP_NOT_IMPLEMENTED_YET;
					}
					else // All other cases of AT events
						return EXIP_NOT_IMPLEMENTED_YET;
				break;
				case EVENT_NS_CLASS:
					if(strm->gStack->currNonTermID != GR_START_TAG_CONTENT ||
						!IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
						return EXIP_INCONSISTENT_PROC_STATE;

					SET_PROD_EXI_EVENT(prodHit->content, EVENT_NS);
					ec.length = 2;
					ec.part[1] = !RULE_CONTAIN_EE(currentRule->meta) + 4;
					ec.bits[1] = getBitsNumber(prod2Count - 1);
					strm->gStack->currNonTermID = GR_START_TAG_CONTENT;
				break;
				case EVENT_SC_CLASS:
					return EXIP_NOT_IMPLEMENTED_YET;
				break;
				case EVENT_SE_CLASS:
					// SE(*) content|same_rule
					if(isContent2Grammar || strm->gStack->currNonTermID < GET_CONTENT_INDEX(strm->gStack->grammar->props))
					{
						// currNonTermID should point to the content grammar rule
						if(GET_CONTENT_INDEX(strm->gStack->grammar->props) == GR_START_TAG_CONTENT)
							strm->gStack->currNonTermID = GR_CONTENT_2;
						else
							strm->gStack->currNonTermID = GET_CONTENT_INDEX(strm->gStack->grammar->props);
					}
					SET_PROD_EXI_EVENT(prodHit->content, EVENT_SE_ALL);
					ec.length = 2;
					ec.part[1] = prod2Count - 2 - (IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD) == TRUE) - (IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS));
					ec.bits[1] = getBitsNumber(prod2Count - 1);
				break;
				case EVENT_CH_CLASS:
					// CH [untyped value] content|same_rule
					if(isContent2Grammar || strm->gStack->currNonTermID < GET_CONTENT_INDEX(strm->gStack->grammar->props))
					{
						// currNonTermID should point to the content grammar rule
						if(GET_CONTENT_INDEX(strm->gStack->grammar->props) == GR_START_TAG_CONTENT)
							strm->gStack->currNonTermID = GR_CONTENT_2;
						else
							strm->gStack->currNonTermID = GET_CONTENT_INDEX(strm->gStack->grammar->props);
					}
					SET_PROD_EXI_EVENT(prodHit->content, EVENT_CH);
					ec.length = 2;
					ec.part[1] = prod2Count - 1 - (IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD) == TRUE) - (IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS));
					ec.bits[1] = getBitsNumber(prod2Count - 1);
				break;
				case EVENT_ER_CLASS:
					return EXIP_NOT_IMPLEMENTED_YET;
				break;
				case EVENT_CM_CLASS:
					return EXIP_NOT_IMPLEMENTED_YET;
				break;
				case EVENT_PI_CLASS:
					return EXIP_NOT_IMPLEMENTED_YET;
				break;
				default:
					return EXIP_INCONSISTENT_PROC_STATE;
			}
		}
	}

	return writeEventCode(strm, ec);
}

errorCode encodeQName(EXIStream* strm, QName qname, EventType eventT, QNameID* qnameID)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Encoding QName\n"));

/******* Start: URI **********/
	TRY(encodeUri(strm, (String*) qname.uri, &qnameID->uriId));
/******* End: URI **********/

/******* Start: Local name **********/
	TRY(encodeLn(strm, (String*) qname.localName, qnameID));
/******* End: Local name **********/

	return encodePfxQName(strm, &qname, eventT, qnameID->uriId);
}

errorCode encodeUri(EXIStream* strm, String* uri, SmallIndex* uriId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned char uriBits = getBitsNumber(strm->schema->uriTable.count);

	if(lookupUri(&strm->schema->uriTable, *uri, uriId)) // uri hit
	{
		TRY(encodeNBitUnsignedInteger(strm, uriBits, *uriId + 1));
	}
	else  // uri miss
	{
		String copiedURI;
		TRY(encodeNBitUnsignedInteger(strm, uriBits, 0));
		TRY(encodeString(strm, uri));
		TRY(cloneStringManaged(uri, &copiedURI, &strm->memList));
		TRY(addUriEntry(&strm->schema->uriTable, copiedURI, uriId));
	}

	return EXIP_OK;
}

errorCode encodeLn(EXIStream* strm, String* ln, QNameID* qnameID)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	if(lookupLn(&strm->schema->uriTable.uri[qnameID->uriId].lnTable, *ln, &qnameID->lnId)) // local-name table hit
	{
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->schema->uriTable.uri[qnameID->uriId].lnTable.count - 1));
		TRY(encodeUnsignedInteger(strm, 0));
		TRY(encodeNBitUnsignedInteger(strm, lnBits, (unsigned int)(qnameID->lnId)));
	}
	else // local-name table miss
	{
		String copiedLN;
		TRY(encodeUnsignedInteger(strm, (UnsignedInteger)(ln->length + 1)));
		TRY(encodeStringOnly(strm,  ln));

		if(strm->schema->uriTable.uri[qnameID->uriId].lnTable.ln == NULL)
		{
			// Create local name table for this URI entry
			TRY(createDynArray(&strm->schema->uriTable.uri[qnameID->uriId].lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER));
		}

		TRY(cloneStringManaged(ln, &copiedLN, &strm->memList));
		TRY(addLnEntry(&strm->schema->uriTable.uri[qnameID->uriId].lnTable, copiedLN, &qnameID->lnId));
	}

	return EXIP_OK;
}

errorCode encodePfxQName(EXIStream* strm, QName* qname, EventType eventT, SmallIndex uriId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned char prefixBits = 0;
	SmallIndex prefixID = 0;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) == FALSE)
		return EXIP_OK;

	if(strm->schema->uriTable.uri[uriId].pfxTable == NULL || strm->schema->uriTable.uri[uriId].pfxTable->count == 0)
		return EXIP_OK;

	prefixBits = getBitsNumber(strm->schema->uriTable.uri[uriId].pfxTable->count - 1);

	if(prefixBits > 0)
	{
		if(qname == NULL)
			return EXIP_NULL_POINTER_REF;

		if(lookupPfx(strm->schema->uriTable.uri[uriId].pfxTable, *qname->prefix, &prefixID) == TRUE)
		{
			TRY(encodeNBitUnsignedInteger(strm, prefixBits, (unsigned int) prefixID));
		}
		else
		{
			if(eventT != EVENT_SE_ALL)
				return EXIP_INCONSISTENT_PROC_STATE;

			TRY(encodeNBitUnsignedInteger(strm, prefixBits, 0));
		}
	}

	return EXIP_OK;
}

errorCode encodePfx(EXIStream* strm, SmallIndex uriId, String* prefix)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	SmallIndex pfxId;
	unsigned char pfxBits = getBitsNumber(strm->schema->uriTable.uri[uriId].pfxTable->count);

	if(lookupPfx(strm->schema->uriTable.uri[uriId].pfxTable, *prefix, &pfxId)) // prefix hit
	{
		TRY(encodeNBitUnsignedInteger(strm, pfxBits, pfxId + 1));
	}
	else  // prefix miss
	{
		String copiedPrefix;
		TRY(encodeNBitUnsignedInteger(strm, pfxBits, 0));
		TRY(encodeString(strm, prefix));
		TRY(cloneStringManaged(prefix, &copiedPrefix, &strm->memList));
		TRY(addPfxEntry(strm->schema->uriTable.uri[uriId].pfxTable, copiedPrefix, &pfxId));
	}

	return EXIP_OK;
}

errorCode encodeIntData(EXIStream* strm, Integer int_val, QNameID qnameID, Index typeId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	EXIType exiType;

	if(typeId != INDEX_MAX)
		exiType = GET_EXI_TYPE(strm->schema->simpleTypeTable.sType[typeId].content);
	else
		exiType = VALUE_TYPE_NONE;

	if(exiType == VALUE_TYPE_SMALL_INTEGER)
	{
		// TODO: take into account  minExclusive and  maxExclusive when they are supported
		unsigned int encoded_val;
		unsigned char numberOfBits;

		if(int_val > strm->schema->simpleTypeTable.sType[typeId].max ||
				int_val < strm->schema->simpleTypeTable.sType[typeId].min)
			return EXIP_INVALID_EXI_INPUT;

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
	else if(exiType == VALUE_TYPE_STRING || exiType == VALUE_TYPE_UNTYPED || exiType == VALUE_TYPE_NONE)
	{
		//       1) Print Warning
		//       2) convert the int to sting
		//       3) encode string
		String tmpStr;

		DEBUG_MSG(WARNING, DEBUG_CONTENT_IO, ("\n>Integer to String conversion required \n"));
#if EXIP_IMPLICIT_DATA_TYPE_CONVERSION
		TRY(integerToString(int_val, &tmpStr));
		TRY(encodeStringData(strm, tmpStr, qnameID, typeId));
		EXIP_MFREE(tmpStr.str);
#else
		return EXIP_INVALID_EXI_INPUT;
#endif
	}
	else
	{
	    DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, ("\n>Production type is not an Integer\n"));
		return EXIP_INCONSISTENT_PROC_STATE;
	}
	return EXIP_OK;
}
