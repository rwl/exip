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
static errorCode decodeQNameValue(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data);

errorCode processNextProduction(EXIStream* strm, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int bitCount;
	unsigned int tmp_bits_val = 0;
	GrammarRule* currentRule;
	Index prodCount;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Next production non-term-id: %u\n", (unsigned int) strm->context.currNonTermID));

	if(strm->context.currNonTermID >=  strm->gStack->grammar->count)
		return INCONSISTENT_PROC_STATE;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[strm->context.currNonTermID];
	else
		currentRule = &strm->gStack->grammar->rule[strm->context.currNonTermID];

#if DEBUG_CONTENT_IO == ON
	tmp_err_code = printGrammarRule(strm->context.currNonTermID, currentRule, strm->schema);
	if(tmp_err_code != ERR_OK)
	{
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Error printing grammar rule\n"));
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
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("> ED event:\n"));
			if(handler->endDocument != NULL)
			{
				if(handler->endDocument(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		break;
		case EVENT_EE:
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("> EE event:\n"));
			strm->context.isNilType = FALSE;
			if(handler->endElement != NULL)
			{
				if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
					return HANDLER_STOP_RECEIVED;
			}
		break;
		case EVENT_SC:
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("> SC event:\n"));
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

		/* There are 8 possible states to exit the state machine: EE, AT (*), NS etc.
		 * The state depends on the input event code from the stream and the
		 * available productions at level 2.
		 * (Note this is the state for level 2 productions) */
		unsigned int state = 0;

		/* The state mask stores the availability of the productions on level 2.
		 * They are encoded ordered:
		 * Availability for EE is encoded in position 0,
		 * availability for AT(xsi:type) is encoded in position 1,
		 * and so on. */
		boolean state_mask[8] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
		unsigned int i;

		prodCnt = 2; // Always SE, CH, position 4, 5
		state_mask[4] = TRUE;
		state_mask[5] = TRUE;

		if(strm->context.currNonTermID == GR_START_TAG_CONTENT)
		{
			prodCnt += 2; // EE, AT, position 0, 1
			state_mask[0] = TRUE;
			state_mask[1] = TRUE;

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
			{
				prodCnt += 1; // NS, position 2
				state_mask[2] = TRUE;
			}

			if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
			{
				prodCnt += 1; // SC, position 3
				state_mask[3] = TRUE;
			}
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
		{
			prodCnt += 1; // ER position 6
			state_mask[6] = TRUE;
		}

		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
		{
			prodCnt += 1; // CM or PI, position 7
			state_mask[7] = TRUE;
		}

		tmp_err_code = decodeNBitUnsignedInteger(strm, getBitsNumber(prodCnt - 1), &tmp_bits_val);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		state = tmp_bits_val;

		for(i = 0; i <= state && state < 8; i++)
		{
			state = state + (state_mask[i] == 0);
		}

		switch(state)
		{
			case 0:
				// StartTagContent : EE event
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
				// StartTagContent : AT(*) event
				*nonTermID_out = GR_START_TAG_CONTENT;

				tmp_err_code = decodeATWildcardEvent(strm, handler, nonTermID_out, app_data);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_AT_QNAME, GR_START_TAG_CONTENT, &strm->context.currAttr, 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			break;
			case 2:
				// StartTagContent : NS event
				tmp_err_code = decodeNSEvent(strm, handler, nonTermID_out, app_data);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			break;
			case 3:
				// StartTagContent : SC event
				return NOT_IMPLEMENTED_YET;
			break;
			case 4:
				// SE(*) event
				strm->gStack->lastNonTermID = GR_ELEMENT_CONTENT;

				tmp_err_code = decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &strm->context.currElem, 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			break;
			case 5:
				// CH event
				DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">CH event\n"));

				*nonTermID_out = GR_ELEMENT_CONTENT;

				tmp_err_code = decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->context.currElem, app_data);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, *nonTermID_out, &voidQnameID, 1);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			break;
			case 6:
				// ER event
				return NOT_IMPLEMENTED_YET;
			break;
			case 7:
				// CM or PI event
				return NOT_IMPLEMENTED_YET;
			break;
			default:
				return INCONSISTENT_PROC_STATE;
		}
	}
	else if(IS_DOCUMENT(strm->gStack->grammar->props))
	{
		// Document grammar
		/* There are 3 possible states to exit the state machine: DT, CM or PI
		 * The state depends on the input event code from the stream and the
		 * available productions at level 2 and 3.*/
		unsigned int state = 0;
		unsigned int level3ProdCnt = 0;

		if(strm->context.currNonTermID == GR_DOC_CONTENT)
		{
			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
				prodCnt += 1; // DT event

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
				level3ProdCnt += 1;

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
				level3ProdCnt += 1;

			if(level3ProdCnt > 0)
				prodCnt += 1; // CM or PI third level

			if(prodCnt == 2)
			{
				tmp_err_code = decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				if(tmp_bits_val == 0)
					state = 0;
				else if(tmp_bits_val == 1)
				{
					// CM or PI third level
					if(level3ProdCnt == 2)
					{
						tmp_err_code = decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						state = tmp_bits_val + 1;
					}
					else if(level3ProdCnt == 1)
					{
						state = !IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + 1;
					}
					else
						return INCONSISTENT_PROC_STATE;
				}
				else
					return INCONSISTENT_PROC_STATE;
			}
			else if(prodCnt == 1)
			{
				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
					state = 0;
				else
				{
					// CM or PI third level
					if(level3ProdCnt == 2)
					{
						tmp_err_code = decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						state = tmp_bits_val + 1;
					}
					else if(level3ProdCnt == 1)
					{
						state = !IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + 1;
					}
					else
						return INCONSISTENT_PROC_STATE;
				}
			}
			else
				return INCONSISTENT_PROC_STATE;
		}
		else
		{
			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
				prodCnt += 1; // CM second level

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
				prodCnt += 1; // PI second level

			if(prodCnt == 2)
			{
				tmp_err_code = decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				state = tmp_bits_val + 1;
			}
			else if(prodCnt == 1)
			{
				state = !IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + 1;
			}
			else
				return INCONSISTENT_PROC_STATE;
		}

		switch(state)
		{
			case 0:
				// DT event
				return NOT_IMPLEMENTED_YET;
			break;
			case 1:
				// CM event
				return NOT_IMPLEMENTED_YET;
			break;
			case 2:
				// PI event
				return NOT_IMPLEMENTED_YET;
			break;
			default:
				return INCONSISTENT_PROC_STATE;
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
		QName qname;

		if(WITH_STRICT(strm->header.opts.enumOpt))
		{
			// Strict mode

			// Only available second level event if it is an entry grammar rule and is not Nil type grammar
			if(strm->context.currNonTermID == GR_START_TAG_CONTENT && strm->context.isNilType == FALSE)
			{
				/* There are 2 possible states to exit the state machine: AT(xsi:type) and AT(xsi:nil)
				 * (Note this is the state for level 2 productions) */
				unsigned int state = 0;
				boolean nil;

				*nonTermID_out = GR_START_TAG_CONTENT;

				if(HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props))
					prodCnt += 1;
				if(IS_NILLABLE(strm->gStack->grammar->props))
					prodCnt += 1;

				if(prodCnt == 2)
				{
					tmp_err_code = decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					if(tmp_bits_val > 1)
						return INCONSISTENT_PROC_STATE;
				}
				else if(prodCnt == 1)
				{
					// zero bit encoded event
					tmp_bits_val = 0;
				}
				else
					return INCONSISTENT_PROC_STATE;

				state = tmp_bits_val;

				if(!HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props))
					state += 1;

				switch(state)
				{
					case 0:
						// AT(xsi:type) event
						tmp_err_code = decodeQNameValue(strm, handler, nonTermID_out, app_data);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;
					break;
					case 1:
						// AT(xsi:nil) event
						tmp_err_code = decodeBoolean(strm, &nil);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						if(nil == TRUE)
							strm->context.isNilType = TRUE;

						DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(xsi:nil) event\n"));
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
					break;
					default:
						return INCONSISTENT_PROC_STATE;
				}
			}
			else
				return INCONSISTENT_PROC_STATE;
		}
		else // Non-strict mode
		{
			/* There are 11 possible states to exit the state machine: EE, AT(xsi:type), AT(xsi:nil) etc.
			 * The state depends on the input event code from the stream and the
			 * available productions at level 2.
			 * (Note this is the state for level 2 productions) */
			unsigned int state = 0;

			/* The state mask stores the availability of the productions on level 2.
			 * They are encoded ordered:
			 * Availability for EE is encoded in position 0,
			 * availability for AT(xsi:type) is encoded in position 1,
			 * and so on. */
			boolean state_mask[11] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
			unsigned int i;
			// Create a copy of the content grammar if and only if there are AT
			// productions that point to the content grammar rule OR the content index is 0.
			// The content2 grammar rule is only needed in case the current rule is
			// equal the content grammar
			boolean isContent2Grammar = FALSE;

			if(strm->context.currNonTermID == GET_CONTENT_INDEX(strm->gStack->grammar->props)
					&& HAS_CONTENT2(strm->gStack->grammar->props))
			{
				isContent2Grammar = TRUE;
			}

			prodCnt = 2; // SE(*), CH(untyped) always available, position 7 and 8
			state_mask[7] = TRUE;
			state_mask[8] = TRUE;
			if(isContent2Grammar ||
					strm->context.currNonTermID < GET_CONTENT_INDEX(strm->gStack->grammar->props))
			{
				prodCnt += 2; // AT(*), AT(untyped) third level, position 3 and 4
				state_mask[3] = TRUE;
				state_mask[4] = TRUE;
			}

			if(!RULE_CONTAIN_EE(currentRule->meta))
			{
				prodCnt += 1; // EE, position 0
				state_mask[0] = TRUE;
			}

			if(strm->context.currNonTermID == GR_START_TAG_CONTENT)
			{
				prodCnt += 2; // AT(xsi:type), AT(xsi:nil), position 1 and 2
				state_mask[1] = TRUE;
				state_mask[2] = TRUE;

				if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES))
				{
					prodCnt += 1; // NS, position 5
					state_mask[5] = TRUE;
				}
				if(WITH_SELF_CONTAINED(strm->header.opts.enumOpt))
				{
					prodCnt += 1; // SC, position 6
					state_mask[6] = TRUE;
				}
			}

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
			{
				prodCnt += 1; // ER, position 9
				state_mask[9] = TRUE;
			}

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
			{
				prodCnt += 1; // CM or PI, position 10
				state_mask[10] = TRUE;
			}

			tmp_err_code = decodeNBitUnsignedInteger(strm, getBitsNumber(prodCnt - 1), &tmp_bits_val);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			state = tmp_bits_val;

			for(i = 0; i <= state && state < 11; i++)
			{
				state = state + (state_mask[i] == 0);
			}

			switch(state)
			{
				case 0:
					// EE event
					strm->context.isNilType = FALSE;
					if(handler->endElement != NULL)
					{
						if(handler->endElement(app_data) == EXIP_HANDLER_STOP)
							return HANDLER_STOP_RECEIVED;
					}
					*nonTermID_out = GR_VOID_NON_TERMINAL;
				break;
				case 1:
					// AT(xsi:type) event
					tmp_err_code = decodeQNameValue(strm, handler, nonTermID_out, app_data);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 2:
					// AT(xsi:nil) Element i, 0
					{
						boolean nil = FALSE;
						tmp_err_code = decodeBoolean(strm, &nil);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						if(nil == TRUE)
							strm->context.isNilType = TRUE;

						DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(xsi:nil) event\n"));
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
				break;
				case 3:
					// AT(*)
					*nonTermID_out = strm->context.currNonTermID;

					tmp_err_code = decodeATWildcardEvent(strm, handler, nonTermID_out, app_data);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 4:
					// third level AT: eighter AT (qname) [untyped value] or AT (*) [untyped value]
					return NOT_IMPLEMENTED_YET;
				break;
				case 5:
					// NS Element i, 0
					tmp_err_code = decodeNSEvent(strm, handler, nonTermID_out, app_data);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 6:
					// SC event
					return NOT_IMPLEMENTED_YET;
				break;
				case 7:
					// SE(*) content
					strm->gStack->lastNonTermID = GET_CONTENT_INDEX(strm->gStack->grammar->props);

					tmp_err_code = decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 8:
					// CH [untyped value] content
					DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">CH event\n"));

					*nonTermID_out = GET_CONTENT_INDEX(strm->gStack->grammar->props);

					tmp_err_code = decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->context.currElem, app_data);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				break;
				case 9:
					// ER event
					return NOT_IMPLEMENTED_YET;
				break;
				case 10:
					// third level: CM or PI event
					return NOT_IMPLEMENTED_YET;
				break;
				default:
					return INCONSISTENT_PROC_STATE;
			}
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

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Decoding QName\n"));

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
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">URI miss\n"));
		tmp_err_code = decodeString(strm, &str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addUriEntry(&strm->schema->uriTable, str, uriId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // uri hit
	{
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">URI hit\n"));
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
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">local-name table hit\n"));
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
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">local-name table miss\n"));

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
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Prefix miss\n"));
		tmp_err_code = decodeString(strm, &str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addPfxEntry(strm->schema->uriTable.uri[uriId].pfxTable, str, pfxId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else // prefix hit
	{
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Prefix hit\n"));
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

			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">SE(qname) event: \n"));
			assert(strm->context.isNilType == FALSE);
			strm->context.currElem = prodHit->qnameId;
			qname.uri = &(strm->schema->uriTable.uri[prodHit->qnameId.uriId].uriStr);
			qname.localName = &(GET_LN_URI_QNAME(strm->schema->uriTable, prodHit->qnameId).lnStr);
#if DEBUG_CONTENT_IO == ON && EXIP_DEBUG_LEVEL == INFO
			printString(qname.uri);
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (" : "));
			printString(qname.localName);
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n"));
#endif
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
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(qname) event\n"));
			strm->context.currAttr = prodHit->qnameId;
			qname.uri = &strm->schema->uriTable.uri[strm->context.currAttr.uriId].uriStr;
			qname.localName = &GET_LN_URI_QNAME(strm->schema->uriTable, prodHit->qnameId).lnStr;
#if DEBUG_CONTENT_IO == ON && EXIP_DEBUG_LEVEL == INFO
			printString(qname.uri);
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (" : "));
			printString(qname.localName);
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n"));
#endif
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
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">CH event\n"));
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
	else if(localQNameID.uriId == XML_SCHEMA_INSTANCE_ID &&
			(localQNameID.lnId == XML_SCHEMA_INSTANCE_TYPE_ID || localQNameID.lnId == XML_SCHEMA_INSTANCE_NIL_ID))
	{
		exiType = VALUE_TYPE_QNAME;
	}

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

			if(HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MAX_INCLUSIVE))
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
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Float value\n"));
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
			//DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Binary value\n"));
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
			// Only allowed if the current production is AT(xsi:type)
			assert(localQNameID.uriId == XML_SCHEMA_INSTANCE_ID && localQNameID.lnId == XML_SCHEMA_INSTANCE_TYPE_ID);

			tmp_err_code = decodeQNameValue(strm, handler, nonTermID_out, app_data);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
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

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">NS event:\n"));
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

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">SE(*) event\n"));

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

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(*) event\n"));

	tmp_err_code = decodeQName(strm, &qname, &qnameId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(qnameId.uriId == XML_SCHEMA_INSTANCE_ID &&
			(qnameId.lnId == XML_SCHEMA_INSTANCE_TYPE_ID || qnameId.lnId == XML_SCHEMA_INSTANCE_NIL_ID))
	{
		if(IS_SCHEMA(strm->gStack->grammar->props))
		{
			DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">In schema-informed grammars, xsi:type and xsi:nil attributes MUST NOT be represented using AT(*) terminal\n"));
			return INCONSISTENT_PROC_STATE;
		}
	}

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

static errorCode decodeQNameValue(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data)
{
	// TODO: Add the case when Preserve.lexicalValues option value is true - instead of Qname decode it as String
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QName qname;
	QNameID qnameId;
	EXIGrammar* newGrammar = NULL;;

	tmp_err_code = decodeQName(strm, &qname, &qnameId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(handler->qnameData != NULL)  // Invoke handler method
	{
		if(handler->qnameData(qname, app_data) == EXIP_HANDLER_STOP)
			return HANDLER_STOP_RECEIVED;
	}

	// New type grammar is pushed on the stack if it exists
	newGrammar = GET_TYPE_GRAMMAR_QNAMEID(strm->schema, qnameId);

	if(newGrammar != NULL)
	{
		// The grammar is found
		EXIGrammar* currGr;

		popGrammar(&(strm->gStack), &currGr);

		*nonTermID_out = GR_START_TAG_CONTENT;
		tmp_err_code = pushGrammar(&(strm->gStack), newGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}
