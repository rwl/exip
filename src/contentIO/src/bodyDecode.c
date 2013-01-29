/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file bodyDecode.c
 * @brief Implementing an API for decoding EXI stream body
 * @date Oct 1, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "bodyDecode.h"
#include "sTables.h"
#include "memManagement.h"
#include "ioUtil.h"
#include "streamDecode.h"
#include "grammars.h"
#include "dynamicArray.h"
#include "stringManipulate.h"


static errorCode stateMachineProdDecode(EXIStream* strm, GrammarRule* currentRule, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data);
static errorCode handleProduction(EXIStream* strm, Production* prodHit, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data);

errorCode processNextProduction(EXIStream* strm, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int bitCount;
	unsigned int tmp_bits_val = 0;
	GrammarRule* currentRule;
	Index prodCount;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Next production non-term-id: %u\n", (unsigned int) strm->context.currNonTermID));

	if(strm->context.currNonTermID >=  strm->gStack->grammar->count)
		return INCONSISTENT_PROC_STATE;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[strm->context.currNonTermID];
	else
		currentRule = &strm->gStack->grammar->rule[strm->context.currNonTermID];

#if DEBUG_GRAMMAR == ON
	tmp_err_code = printGrammarRule(strm->context.currNonTermID, currentRule, strm->schema);
	if(tmp_err_code != ERR_OK)
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Error printing grammar rule\n"));
	}
#endif

	if(strm->context.isNilType == FALSE)
		prodCount = currentRule->pCount;
	else
	{
		prodCount = RULE_GET_AT_COUNT(currentRule->meta) + RULE_CONTAIN_EE(currentRule->meta);
		if(strm->context.currNonTermID >= GET_CONTENT_INDEX(strm->gStack->grammar->props))
		{
			// Instead of content we have a single EE production encoded with zero bits because of xsi:nil=TRUE
			strm->context.isNilType = FALSE;
			if(handler->endElement != NULL)
			{
				if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			return ERR_OK;
		}
	}

	bitCount = getBitsFirstPartCode(strm->header.opts, strm->gStack->grammar, currentRule, strm->context.currNonTermID, strm->context.isNilType);

	if(prodCount > 0)
	{
		if(bitCount == 0)
		{
			// encoded with zero bits
			return handleProduction(strm, &currentRule->production[0], nonTermID_out, handler, app_data);
		}
		else
		{
			tmp_err_code = decodeNBitUnsignedInteger(strm, bitCount, &tmp_bits_val);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(tmp_bits_val < prodCount)
			{
				if(strm->context.isNilType == TRUE && tmp_bits_val == (prodCount - 1))
				{
					// Because SE productions are ordered before the EE in case of emptyTypeGrammar this might be either AT or EE
					if(RULE_CONTAIN_EE(currentRule->meta))
					{
						// Always last so this is an EE event
						strm->context.isNilType = FALSE;
						if(handler->endElement != NULL)
						{
							if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
								return HANDLER_STOP_RECEIVED;
						}

						return ERR_OK;
					}
				}

				return handleProduction(strm, &currentRule->production[prodCount - 1 - tmp_bits_val], nonTermID_out, handler, app_data);
			}
		}
	}
	else if(strm->context.isNilType == TRUE)
	{
		// There is a single EE production encoded with zero bits
		strm->context.isNilType = FALSE;
		if(handler->endElement != NULL)
		{
			if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
				return HANDLER_STOP_RECEIVED;
		}

		return ERR_OK;
	}

	// Production with length code 1 not found: search second or third level productions
	// Invoke state machine

	return stateMachineProdDecode(strm, currentRule, nonTermID_out, handler, app_data);
}

static errorCode handleProduction(EXIStream* strm, Production* prodHit, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data)
{
	*nonTermID_out = GET_PROD_NON_TERM(prodHit->content);

	switch(GET_PROD_EXI_EVENT(prodHit->content))
	{
		case EVENT_ED:
			if(handler->endDocument != NULL)
			{
				if(handler->endDocument(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		break;
		case EVENT_EE:
			strm->context.isNilType = FALSE;
			if(handler->endElement != NULL)
			{
				if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		break;
		case EVENT_SC:
			if(handler->selfContained != NULL)
			{
				if(handler->selfContained(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		break;
		default: // The event has content!
			return decodeEventContent(strm, prodHit, handler, nonTermID_out, app_data);
		break;
	}

	return ERR_OK;
}

static errorCode stateMachineProdDecode(EXIStream* strm, GrammarRule* currentRule, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int prodCnt = 0;
	unsigned int tmp_bits_val = 0;
	QNameID voidQnameID = {URI_MAX, LN_MAX};

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))
	{
		// Built-in element grammar
		if(strm->context.currNonTermID == GR_START_TAG_CONTENT)
		{
			prodCnt = 4;
			prodCnt += IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES);
			prodCnt += WITH_SELF_CONTAINED(strm->header.opts.enumOpt);
		}
		else // GR_ELEMENT_CONTENT
			prodCnt = 2;

		prodCnt += IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD);
		prodCnt += IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS);

		tmp_err_code = decodeNBitUnsignedInteger(strm, getBitsNumber(prodCnt - 1), &tmp_bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(tmp_bits_val == prodCnt)
		{
			// 3th level, CM or PI productions
			return NOT_IMPLEMENTED_YET;
		}
		else
		{
			// 2nd level
			switch(tmp_bits_val + (strm->context.currNonTermID*7))
			{
				case 0:
					// StartTagContent : EE
					strm->context.isNilType = FALSE;
					if(handler->endElement != NULL)
					{
						if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
							return HANDLER_STOP_RECEIVED;
					}

					*nonTermID_out = GR_VOID_NON_TERMINAL;

					tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_EE, GR_VOID_NON_TERMINAL, &voidQnameID, 1);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 1:
					// StartTagContent : AT (*)

					tmp_err_code = decodeATWildcardEvent(strm, handler, nonTermID_out, app_data);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_AT_QNAME, GR_START_TAG_CONTENT, &strm->context.currAttr, 1);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 2:
					// StartTagContent : NS or SC or SE
					if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
					{
						// NS event
						tmp_err_code = decodeNSEvent(strm, handler, nonTermID_out, app_data);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
					else if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
					{
						// SC event
						return NOT_IMPLEMENTED_YET;
					}
					else
					{
						// SE event
						strm->gStack->lastNonTermID = GR_ELEMENT_CONTENT;

						tmp_err_code = decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &strm->context.currElem, 1);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
				break;
				case 3:
					// StartTagContent : SC or SE or CH
					if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
					{
						if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
						{
							// SC event
							return NOT_IMPLEMENTED_YET;
						}
						else
						{
							// SE event
							strm->gStack->lastNonTermID = GR_ELEMENT_CONTENT;

							tmp_err_code = decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;

							tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &strm->context.currElem, 1);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;
						}
					}
					else if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
					{
						// SE event
						strm->gStack->lastNonTermID = GR_ELEMENT_CONTENT;

						tmp_err_code = decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &strm->context.currElem, 1);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
					else
					{
						// CH event
						DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));

						*nonTermID_out = GR_ELEMENT_CONTENT;

						tmp_err_code = decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->context.currElem, app_data);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, *nonTermID_out, &voidQnameID, 1);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
				break;
				case 4:
					// StartTagContent : SE or CH or ER
					if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
					{
						if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
						{
							// SE event
							strm->gStack->lastNonTermID = GR_ELEMENT_CONTENT;

							tmp_err_code = decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;

							tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &strm->context.currElem, 1);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;
						}
						else
						{
							// CH event
							DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));

							*nonTermID_out = GR_ELEMENT_CONTENT;

							tmp_err_code = decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->context.currElem, app_data);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;

							tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, *nonTermID_out, &voidQnameID, 1);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;
						}

					}
					else if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
					{
						// CH event
						DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));

						*nonTermID_out = GR_ELEMENT_CONTENT;

						tmp_err_code = decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->context.currElem, app_data);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, *nonTermID_out, &voidQnameID, 1);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
					else
					{
						// ER event
						return NOT_IMPLEMENTED_YET;
					}
				break;
				case 5:
					// StartTagContent : CH or ER
					if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) && WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
					{
						// CH event
						DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));

						*nonTermID_out = GR_ELEMENT_CONTENT;

						tmp_err_code = decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->context.currElem, app_data);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, *nonTermID_out, &voidQnameID, 1);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					}
					else
					{
						// ER event
						return NOT_IMPLEMENTED_YET;
					}
				break;
				case 6:
					// StartTagContent : ER
					return NOT_IMPLEMENTED_YET;
				break;
				case 7:
					// ElementContent : SE (*)
					strm->gStack->lastNonTermID = GR_ELEMENT_CONTENT;

					tmp_err_code = decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &strm->context.currElem, 1);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 8:
					// ElementContent : CH (*)
					DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));

					*nonTermID_out = GR_ELEMENT_CONTENT;

					tmp_err_code = decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->context.currElem, app_data);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, *nonTermID_out, &voidQnameID, 1);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 9:
					// ElementContent : ER (*)
					return NOT_IMPLEMENTED_YET;
				break;
				default:
					return INCONSISTENT_PROC_STATE;
			}
		}
	}
	else if(IS_DOCUMENT(strm->gStack->grammar->props))
	{
		// Document grammar
		if(strm->context.currNonTermID == GR_DOC_CONTENT)
		{
			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
			{
				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS) > 0)
				{
					tmp_err_code = decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					if(tmp_bits_val == 0)
					{
						// DT event
						return NOT_IMPLEMENTED_YET;
					}
					else
					{
						// 3th level, CM or PI productions
						return NOT_IMPLEMENTED_YET;
					}
				}
				else
				{
					// 0-bit DT event
					return NOT_IMPLEMENTED_YET;
				}
			}
			else
			{
				// 3th level, CM or PI productions
				return NOT_IMPLEMENTED_YET;
			}
		}
		else // GR_DOC_END
		{
			// CM or PI event
			return NOT_IMPLEMENTED_YET;
		}
	}
	else if(IS_FRAGMENT(strm->gStack->grammar->props))
	{
		// Fragment grammar
		// CM or PI event
		return NOT_IMPLEMENTED_YET;
	}
	else
	{
		// Schema-informed element/type grammar
		// TODO: implement is_empty case
		QName qname;

		if(WITH_STRICT(strm->header.opts.enumOpt))
		{
			// Strict mode
			if(strm->context.currNonTermID == GR_START_TAG_CONTENT && strm->context.isNilType == FALSE)
			{
				boolean nil;

				*nonTermID_out = GR_START_TAG_CONTENT;

				if(HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props))
				{
					if(IS_NILLABLE(strm->gStack->grammar->props))
					{
						tmp_err_code = decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						if(tmp_bits_val == 0)
						{
							// AT(xsi:type) event
							return NOT_IMPLEMENTED_YET;
						}
						else
						{
							// AT(xsi:nil) event
							tmp_err_code = decodeBoolean(strm, &nil);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;

							if(nil == TRUE)
								strm->context.isNilType = TRUE;

							DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">AT(xsi:nil) event\n"));
							strm->context.currAttr.uriId = XML_SCHEMA_INSTANCE_ID;
							strm->context.currAttr.lnId = XML_SCHEMA_INSTANCE_NIL_ID;
							qname.uri = &strm->schema->uriTable.uri[strm->context.currAttr.uriId].uriStr;
							qname.localName = &GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currAttr).lnStr;

							if(handler->attribute != NULL)  // Invoke handler method
							{
								if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
									return HANDLER_STOP_RECEIVED;
							}

							if(handler->booleanData != NULL)  // Invoke handler method
							{
								if(handler->booleanData(nil, app_data) == EXIP_HANDLER_STOP)
									return HANDLER_STOP_RECEIVED;
							}
						}
					}
					else
					{
						// AT(xsi:type) event
						return NOT_IMPLEMENTED_YET;
					}
				}
				else if(IS_NILLABLE(strm->gStack->grammar->props))
				{
					// AT(xsi:nil) event
					tmp_err_code = decodeBoolean(strm, &nil);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					if(nil == TRUE)
						strm->context.isNilType = TRUE;

					DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">AT(xsi:nil) event\n"));
					strm->context.currAttr.uriId = XML_SCHEMA_INSTANCE_ID;
					strm->context.currAttr.lnId = XML_SCHEMA_INSTANCE_NIL_ID;
					qname.uri = &strm->schema->uriTable.uri[strm->context.currAttr.uriId].uriStr;
					qname.localName = &GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currAttr).lnStr;

					if(handler->attribute != NULL)  // Invoke handler method
					{
						if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
							return HANDLER_STOP_RECEIVED;
					}

					if(handler->booleanData != NULL)  // Invoke handler method
					{
						if(handler->booleanData(nil, app_data) == EXIP_HANDLER_STOP)
							return HANDLER_STOP_RECEIVED;
					}
				}
				else
					return INCONSISTENT_PROC_STATE;
			}
			else
				return INCONSISTENT_PROC_STATE;
		}
		else // Non-strict mode
		{
			return NOT_IMPLEMENTED_YET;
		}
	}

	return ERR_OK;
}

/*
 * #1#:
 * All productions in the built-in element grammar of the form LeftHandSide : EE are evaluated as follows:
 * - If a production of the form, LeftHandSide : EE with an event code of length 1 does not exist in
 *   the current element grammar, create one with event code 0 and increment the first part of the
 *   event code of each production in the current grammar with the non-terminal LeftHandSide on the left-hand side.
 * - Add the production created in step 1 to the grammar
 *
 * #2#
 * All productions in the built-in element grammar of the form LeftHandSide : CH RightHandSide are evaluated as follows:
 * - If a production of the form, LeftHandSide : CH RightHandSide with an event code of length 1 does not exist in
 *   the current element grammar, create one with event code 0 and increment the first part of the event code of
 *   each production in the current grammar with the non-terminal LeftHandSide on the left-hand side.
 * - Add the production created in step 1 to the grammar
 * - Evaluate the remainder of event sequence using RightHandSide.
 * */

errorCode decodeQName(EXIStream* strm, QName* qname, QNameID* qnameID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Decoding QName\n"));

	tmp_err_code = decodeUri(strm, &qnameID->uriId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	qname->uri = &(strm->schema->uriTable.uri[qnameID->uriId].uriStr);

	tmp_err_code = decodeLn(strm, qnameID->uriId, &qnameID->lnId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	qname->localName = &GET_LN_P_URI_P_QNAME(&strm->schema->uriTable, qnameID).lnStr;

	return decodePfxQname(strm, qname, qnameID->uriId);
}

errorCode decodeUri(EXIStream* strm, SmallIndex* uriId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_val_buf = 0;
	unsigned char uriBits = getBitsNumber(strm->schema->uriTable.count);

	tmp_err_code = decodeNBitUnsignedInteger(strm, uriBits, &tmp_val_buf);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(tmp_val_buf == 0) // uri miss
	{
		String str;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">URI miss\n"));
		tmp_err_code = decodeString(strm, &str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addUriEntry(&strm->schema->uriTable, str, uriId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // uri hit
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">URI hit\n"));
		*uriId = tmp_val_buf - 1;
		if(*uriId >= strm->schema->uriTable.count)
			return INVALID_EXI_INPUT;
	}

	return ERR_OK;
}

errorCode decodeLn(EXIStream* strm, Index uriId, Index* lnId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	UnsignedInteger tmpVar = 0;

	tmp_err_code = decodeUnsignedInteger(strm, &tmpVar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmpVar == 0) // local-name table hit
	{
		unsigned int l_lnId;
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->schema->uriTable.uri[uriId].lnTable.count - 1));
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">local-name table hit\n"));
		tmp_err_code = decodeNBitUnsignedInteger(strm, lnBits, &l_lnId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(l_lnId >= strm->schema->uriTable.uri[uriId].lnTable.count)
			return INVALID_EXI_INPUT;
		*lnId = l_lnId;
	}
	else // local-name table miss
	{
		String lnStr;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">local-name table miss\n"));

		tmp_err_code = allocateStringMemoryManaged(&(lnStr.str),(Index) (tmpVar - 1), &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = decodeStringOnly(strm, (Index)tmpVar - 1, &lnStr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(strm->schema->uriTable.uri[uriId].lnTable.ln == NULL)
		{
			// Create local name table for this URI entry
			tmp_err_code = createDynArray(&strm->schema->uriTable.uri[uriId].lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		tmp_err_code = addLnEntry(&strm->schema->uriTable.uri[uriId].lnTable, lnStr, lnId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

errorCode decodePfxQname(EXIStream* strm, QName* qname, SmallIndex uriId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char prefixBits = 0;
	unsigned int prefixID = 0;

	qname->prefix = NULL;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) == FALSE)
		return ERR_OK;

	if(strm->schema->uriTable.uri[uriId].pfxTable == NULL || strm->schema->uriTable.uri[uriId].pfxTable->count == 0)
		return ERR_OK;

	prefixBits = getBitsNumber(strm->schema->uriTable.uri[uriId].pfxTable->count - 1);

	if(prefixBits > 0)
	{
		tmp_err_code = decodeNBitUnsignedInteger(strm, prefixBits, &prefixID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(prefixID >= strm->schema->uriTable.uri[uriId].pfxTable->count)
			return INVALID_EXI_INPUT;
	}

	qname->prefix = &strm->schema->uriTable.uri[uriId].pfxTable->pfxStr[prefixID];

	return ERR_OK;
}

errorCode decodePfx(EXIStream* strm, SmallIndex uriId, SmallIndex* pfxId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int tmp_val_buf = 0;
	unsigned char prfxBits = getBitsNumber(strm->schema->uriTable.uri[uriId].pfxTable->count);

	tmp_err_code = decodeNBitUnsignedInteger(strm, prfxBits, &tmp_val_buf);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmp_val_buf == 0) // prefix miss
	{
		String str;
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Prefix miss\n"));
		tmp_err_code = decodeString(strm, &str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addPfxEntry(strm->schema->uriTable.uri[uriId].pfxTable, str, pfxId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // prefix hit
	{
		DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Prefix hit\n"));
		*pfxId = tmp_val_buf-1;
		if(*pfxId >= strm->schema->uriTable.uri[uriId].pfxTable->count)
			return INVALID_EXI_INPUT;
	}

	return ERR_OK;
}

errorCode decodeStringValue(EXIStream* strm, QNameID qnameID, String* value)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	UnsignedInteger tmpVar = 0;
	tmp_err_code = decodeUnsignedInteger(strm, &tmpVar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(tmpVar == 0) // "local" value partition table hit
	{
		unsigned int vxEntryId = 0;
		unsigned char vxBits;
		VxTable* vxTable;

		vxTable = &GET_LN_URI_QNAME(strm->schema->uriTable, qnameID).vxTable;
		vxBits = getBitsNumber(vxTable->count - 1);
		tmp_err_code = decodeNBitUnsignedInteger(strm, vxBits, &vxEntryId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		*value = strm->valueTable.value[vxTable->vx[vxEntryId].globalId].valueStr;
	}
	else if(tmpVar == 1)// global value partition table hit
	{
		unsigned int valueEntryID = 0;
		unsigned char valueBits;
		
		valueBits = getBitsNumber(strm->valueTable.count - 1);
		tmp_err_code = decodeNBitUnsignedInteger(strm, valueBits, &valueEntryID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		*value = strm->valueTable.value[valueEntryID].valueStr;
	}
	else  // "local" value partition and global value partition table miss
	{
		Index vStrLen = (Index) tmpVar - 2;

		tmp_err_code = allocateStringMemory(&value->str, vStrLen);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = decodeStringOnly(strm, vStrLen, value);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(vStrLen > 0 && vStrLen <= strm->header.opts.valueMaxLength && strm->header.opts.valuePartitionCapacity > 0)
		{
			// The value should be entered in the value partitions of the string tables

			tmp_err_code = addValueEntry(strm, *value, qnameID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	return ERR_OK;
}

errorCode decodeEventContent(EXIStream* strm, Production* prodHit, ContentHandler* handler,
							SmallIndex* nonTermID_out, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QName qname;

	// TODO: implement all cases of events such as PI, CM etc.

	switch(GET_PROD_EXI_EVENT(prodHit->content))
	{
		case EVENT_SE_ALL:
			strm->gStack->lastNonTermID = GET_PROD_NON_TERM(prodHit->content);
			assert(strm->context.isNilType == FALSE);

			tmp_err_code = decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		break;
		case EVENT_AT_ALL:
			tmp_err_code = decodeATWildcardEvent(strm, handler, nonTermID_out, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		break;
		case EVENT_SE_QNAME:
		{
			EXIGrammar* elemGrammar = NULL;

			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">SE(qname) event\n"));
			assert(strm->context.isNilType == FALSE);
			strm->context.currElem = prodHit->qnameId;
			qname.uri = &(strm->schema->uriTable.uri[prodHit->qnameId.uriId].uriStr);
			qname.localName = &(GET_LN_URI_QNAME(strm->schema->uriTable, prodHit->qnameId).lnStr);
			tmp_err_code = decodePfxQname(strm, &qname, prodHit->qnameId.uriId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->startElement != NULL)  // Invoke handler method passing the element qname
			{
				if(handler->startElement(qname, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			strm->gStack->lastNonTermID = *nonTermID_out;

			// New element grammar is pushed on the stack
			if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
			{
				elemGrammar = GET_ELEM_GRAMMAR_QNAMEID(strm->schema, prodHit->qnameId);
			}
			else
			{
				elemGrammar = &strm->schema->grammarTable.grammar[prodHit->typeId];
			}

			if(elemGrammar != NULL) // The grammar is found
			{
				*nonTermID_out = GR_START_TAG_CONTENT;
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
		case EVENT_AT_QNAME:
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">AT(qname) event\n"));
			strm->context.currAttr = prodHit->qnameId;
			qname.uri = &strm->schema->uriTable.uri[strm->context.currAttr.uriId].uriStr;
			qname.localName = &GET_LN_URI_QNAME(strm->schema->uriTable, prodHit->qnameId).lnStr;
			tmp_err_code = decodePfxQname(strm, &qname, prodHit->qnameId.uriId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->attribute != NULL)  // Invoke handler method
			{
				if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			tmp_err_code = decodeValueItem(strm, prodHit->typeId, handler, nonTermID_out, prodHit->qnameId, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		break;
		case EVENT_CH:
		{
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">CH event\n"));
			assert(strm->context.isNilType == FALSE);
			tmp_err_code = decodeValueItem(strm, prodHit->typeId, handler, nonTermID_out, strm->context.currElem, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		break;
		case EVENT_NS:
			tmp_err_code = decodeNSEvent(strm, handler, nonTermID_out, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		break;
		default:
			return NOT_IMPLEMENTED_YET;
	}

	return ERR_OK;
}

errorCode decodeValueItem(EXIStream* strm, Index typeId, ContentHandler* handler, SmallIndex* nonTermID_out, QNameID localQNameID, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIType exiType = VALUE_TYPE_NONE;

	if(typeId != INDEX_MAX)
		exiType = GET_EXI_TYPE(strm->schema->simpleTypeTable.sType[typeId].content);

	switch(exiType)
	{
		case VALUE_TYPE_NON_NEGATIVE_INT:
		{
			UnsignedInteger uintVal;
			tmp_err_code = decodeUnsignedInteger(strm, &uintVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->intData != NULL)  // Invoke handler method
			{
				// TODO: the cast to signed int can introduce errors. Check first!
				if(handler->intData((Integer) uintVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		break;
		case VALUE_TYPE_INTEGER:
		{
			Integer sintVal;
			tmp_err_code = decodeIntegerValue(strm, &sintVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->intData != NULL)  // Invoke handler method
			{
				if(handler->intData(sintVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		break;
		case VALUE_TYPE_SMALL_INTEGER:
		{
			unsigned int uintVal;
			int base;
			int64_t upLimit;

			if(typeId >= strm->schema->simpleTypeTable.count)
				return INVALID_EXI_INPUT;

			if(!HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MIN_INCLUSIVE)
					&& !HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MIN_EXCLUSIVE))
				return INVALID_EXI_INPUT;
			if(!HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MAX_INCLUSIVE)
					&& !HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MAX_EXCLUSIVE))
				return INVALID_EXI_INPUT;

			if(HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MIN_INCLUSIVE))
				base = strm->schema->simpleTypeTable.sType[typeId].min;
			else
				return NOT_IMPLEMENTED_YET;

			if(HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MIN_EXCLUSIVE))
				upLimit = strm->schema->simpleTypeTable.sType[typeId].max;
			else
				return NOT_IMPLEMENTED_YET;

			tmp_err_code = decodeNBitUnsignedInteger(strm, getBitsNumber(upLimit - base), &uintVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->intData != NULL)  // Invoke handler method
			{
				if(handler->intData((Integer) (base + uintVal), app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		break;
		case VALUE_TYPE_FLOAT:
		{
			Float flVal;
			DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Float value\n"));
			tmp_err_code = decodeFloatValue(strm, &flVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->floatData != NULL)  // Invoke handler method
			{
				if(handler->floatData(flVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		break;
		case VALUE_TYPE_BOOLEAN:
		{
			boolean bool_val;
			tmp_err_code = decodeBoolean(strm, &bool_val);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->booleanData != NULL)  // Invoke handler method
			{
				if(handler->booleanData(bool_val, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			// handle xsi:nil attribute
			if(IS_SCHEMA(strm->gStack->grammar->props) && localQNameID.uriId == XML_SCHEMA_INSTANCE_ID && localQNameID.lnId == XML_SCHEMA_INSTANCE_NIL_ID) // Schema-enabled grammar and http://www.w3.org/2001/XMLSchema-instance:nil
			{
				if(bool_val == TRUE)
				{
					// xsi:nil attribute equals to true & schema mode
					strm->context.isNilType = TRUE;
					*nonTermID_out = GR_START_TAG_CONTENT;
				}
			}
		}
		break;
		case VALUE_TYPE_BINARY:
		{
			Index nbytes;
			char *binary_val;
			//DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">Binary value\n"));
			tmp_err_code = decodeBinary(strm, &binary_val, &nbytes);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(handler->binaryData != NULL)  // Invoke handler method
			{
				if(handler->binaryData(binary_val, nbytes, app_data) == EXIP_HANDLER_STOP)
				{
					/* Free the memory allocated by decodeBinary() */
					EXIP_MFREE(binary_val);
					return HANDLER_STOP_RECEIVED;
				}
			}
			
			/* Free the memory allocated by decodeBinary() */
			EXIP_MFREE(binary_val);
		}
		break;
		case VALUE_TYPE_DECIMAL:
		{
			Decimal decVal;

			tmp_err_code = decodeDecimalValue(strm, &decVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->decimalData != NULL)  // Invoke handler method
			{
				if(handler->decimalData(decVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		break;
		case VALUE_TYPE_DATE_TIME:
		{
			EXIPDateTime dtVal;

			tmp_err_code = decodeDateTimeValue(strm, &dtVal);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			if(handler->dateTimeData != NULL)  // Invoke handler method
			{
				if(handler->dateTimeData(dtVal, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		}
		break;
		case VALUE_TYPE_LIST:
		{
			UnsignedInteger itemCount;
			Index itemTypeId;
			unsigned int i;

			tmp_err_code = decodeUnsignedInteger(strm, &itemCount);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			itemTypeId = strm->schema->simpleTypeTable.sType[typeId].length; // The item typeID must be encoded in the length field
			if(itemTypeId >= strm->schema->simpleTypeTable.count)
				return UNEXPECTED_ERROR;

			if(handler->listData != NULL)  // Invoke handler method
			{
				if(handler->listData(GET_EXI_TYPE(strm->schema->simpleTypeTable.sType[itemTypeId].content), (unsigned int) itemCount, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			for(i = 0; i < itemCount; i++)
			{
				tmp_err_code = decodeValueItem(strm, itemTypeId, handler, nonTermID_out, localQNameID, app_data);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		break;
		case VALUE_TYPE_QNAME:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		default: // VALUE_TYPE_STRING || VALUE_TYPE_NONE || VALUE_TYPE_UNTYPED
		{
			String value;
			boolean freeable = FALSE;

			/* ENUMERATION CHECK */
			if(typeId != INDEX_MAX && (HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_ENUMERATION)))
			{
				// There is enumeration defined
				EnumDefinition eDefSearch;
				EnumDefinition* eDefFound;
				unsigned int indx;

				eDefSearch.typeId = typeId;
				eDefFound = bsearch(&eDefSearch, strm->schema->enumTable.enumDef, strm->schema->enumTable.count, sizeof(EnumDefinition), compareEnumDefs);
				if(eDefFound == NULL)
					return UNEXPECTED_ERROR;

				tmp_err_code = decodeNBitUnsignedInteger(strm, getBitsNumber(eDefFound->count - 1), &indx);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				value = ((String*) eDefFound->values)[indx];
				freeable = FALSE;
			}
			else
			{
				tmp_err_code = decodeStringValue(strm, localQNameID, &value);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				if(value.length > strm->header.opts.valueMaxLength || strm->header.opts.valuePartitionCapacity == 0)
					freeable = TRUE;
			}

			if(handler->stringData != NULL)  // Invoke handler method
			{
				if(handler->stringData(value, app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}

			if(freeable)
				EXIP_MFREE(value.str);
		} break;
	}

	return ERR_OK;
}

errorCode decodeNSEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	SmallIndex ns_uriId;
	SmallIndex pfxId;
	boolean bool = FALSE;

	*nonTermID_out = GR_START_TAG_CONTENT;

	tmp_err_code = decodeUri(strm, &ns_uriId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(strm->schema->uriTable.uri[ns_uriId].pfxTable == NULL)
	{
		tmp_err_code = createPfxTable(&strm->schema->uriTable.uri[ns_uriId].pfxTable);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	tmp_err_code = decodePfx(strm, ns_uriId, &pfxId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = decodeBoolean(strm, &bool);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(handler->namespaceDeclaration != NULL)  // Invoke handler method
	{
		if(handler->namespaceDeclaration(strm->schema->uriTable.uri[ns_uriId].uriStr, strm->schema->uriTable.uri[ns_uriId].pfxTable->pfxStr[pfxId], bool, app_data) == EXIP_HANDLER_STOP)
			return HANDLER_STOP_RECEIVED;
	}
	return ERR_OK;
}

errorCode decodeSEWildcardEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIGrammar* elemGrammar = NULL;
	QName qname;
	QNameID qnameId = {URI_MAX, LN_MAX};
	Index dynArrIndx;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">SE(*) event\n"));

	// The content of SE event is the element qname
	tmp_err_code = decodeQName(strm, &qname, &qnameId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(handler->startElement != NULL)  // Invoke handler method passing the element qname
	{
		if(handler->startElement(qname, app_data) == EXIP_HANDLER_STOP)
			return HANDLER_STOP_RECEIVED;
	}

	// New element grammar is pushed on the stack
	elemGrammar = GET_ELEM_GRAMMAR_QNAMEID(strm->schema, qnameId);
	*nonTermID_out = GR_START_TAG_CONTENT;

	if(elemGrammar != NULL)
	{
		// The grammar is found
		tmp_err_code = pushGrammar(&(strm->gStack), elemGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		EXIGrammar newElementGrammar;
		tmp_err_code = createBuiltInElementGrammar(&newElementGrammar, strm);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addDynEntry(&strm->schema->grammarTable.dynArray, &newElementGrammar, &dynArrIndx);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		GET_LN_URI_QNAME(strm->schema->uriTable, qnameId).elemGrammar = dynArrIndx;
		tmp_err_code = pushGrammar(&(strm->gStack), &strm->schema->grammarTable.grammar[dynArrIndx]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	strm->context.currElem = qnameId;
	return ERR_OK;
}

errorCode decodeATWildcardEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QName qname;
	QNameID qnameId = {URI_MAX, LN_MAX};

	DEBUG_MSG(INFO, DEBUG_GRAMMAR, (">AT(*) event\n"));
	*nonTermID_out = GR_START_TAG_CONTENT;

	tmp_err_code = decodeQName(strm, &qname, &qnameId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(handler->attribute != NULL)  // Invoke handler method
	{
		if(handler->attribute(qname, app_data) == EXIP_HANDLER_STOP)
			return HANDLER_STOP_RECEIVED;
	}

	tmp_err_code = decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, qnameId, app_data);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	strm->context.currAttr = qnameId;

	return ERR_OK;
}
