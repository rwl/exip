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
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned int bitCount;
	unsigned int tmp_bits_val = 0;
	GrammarRule* currentRule;
	Index prodCount;
	SmallIndex currNonTermID = strm->gStack->currNonTermID;

	// TODO: GR_CONTENT_2 is only needed when schema deviations are allowed.
	//       Here and in many other places when schema deviations are fully disabled
	//       many parts of the code can be pruned during compile time using macro parameters in the build.
	//       This includes the EXI Profile features itself (excluding localValueCapping)
	if(currNonTermID == GR_CONTENT_2)
		currNonTermID = GET_CONTENT_INDEX(strm->gStack->grammar->props);

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n>Next production non-term-id: %u\n", (unsigned int) currNonTermID));

	if(currNonTermID >=  strm->gStack->grammar->count)
		return EXIP_INCONSISTENT_PROC_STATE;

#if BUILD_IN_GRAMMARS_USE
	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[currNonTermID];
	else
#endif
		currentRule = &strm->gStack->grammar->rule[currNonTermID];

#if DEBUG_CONTENT_IO == ON
	TRY(printGrammarRule(currNonTermID, currentRule, strm->schema));
#endif

	if(strm->context.isNilType == FALSE)
		prodCount = currentRule->pCount;
	else
	{
		prodCount = RULE_GET_AT_COUNT(currentRule->meta) + RULE_CONTAIN_EE(currentRule->meta);
		if(currNonTermID >= GET_CONTENT_INDEX(strm->gStack->grammar->props))
		{
			// Instead of content we have a single EE production encoded with zero bits because of xsi:nil=TRUE
			strm->context.isNilType = FALSE;
			if(handler->endElement != NULL)
			{
				TRY(handler->endElement(app_data));
			}

			return EXIP_ERR_OK;
		}
	}

	bitCount = getBitsFirstPartCode(strm->header.opts, strm->gStack->grammar, currentRule, currNonTermID, strm->context.isNilType);

	if(prodCount > 0)
	{
		if(bitCount == 0)
		{
			// encoded with zero bits
			return handleProduction(strm, &currentRule->production[0], nonTermID_out, handler, app_data);
		}
		else
		{
			TRY(decodeNBitUnsignedInteger(strm, bitCount, &tmp_bits_val));

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
							TRY(handler->endElement(app_data));
						}

						return EXIP_ERR_OK;
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
			TRY(handler->endElement(app_data));
		}

		return EXIP_ERR_OK;
	}

	// Production with length code 1 not found: search second or third level productions
	// Invoke state machine

	return stateMachineProdDecode(strm, currentRule, nonTermID_out, handler, app_data);
}

static errorCode handleProduction(EXIStream* strm, Production* prodHit, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	*nonTermID_out = GET_PROD_NON_TERM(prodHit->content);

	switch(GET_PROD_EXI_EVENT(prodHit->content))
	{
		case EVENT_ED:
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("> ED event:\n"));
			if(handler->endDocument != NULL)
			{
				TRY(handler->endDocument(app_data));
			}
		break;
		case EVENT_EE:
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("> EE event:\n"));
			strm->context.isNilType = FALSE;
			if(handler->endElement != NULL)
			{
				TRY(handler->endElement(app_data));
			}
		break;
		case EVENT_SC:
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("> SC event:\n"));
			if(handler->selfContained != NULL)
			{
				TRY(handler->selfContained(app_data));
			}
		break;
		default: // The event has content!
			return decodeEventContent(strm, prodHit, handler, nonTermID_out, app_data);
		break;
	}

	return EXIP_ERR_OK;
}

static errorCode stateMachineProdDecode(EXIStream* strm, GrammarRule* currentRule, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned int prodCnt = 0;
	unsigned int tmp_bits_val = 0;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))
	{
		// Built-in element grammar
#if BUILD_IN_GRAMMARS_USE
		QNameID voidQnameID = {URI_MAX, LN_MAX};

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

		if(strm->gStack->currNonTermID == GR_START_TAG_CONTENT)
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

		TRY(decodeNBitUnsignedInteger(strm, getBitsNumber(prodCnt - 1), &tmp_bits_val));
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
					TRY(handler->endElement(app_data));
				}

				*nonTermID_out = GR_VOID_NON_TERMINAL;

				TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_EE, GR_VOID_NON_TERMINAL, &voidQnameID, 1));
			break;
			case 1:
				// StartTagContent : AT(*) event
				*nonTermID_out = GR_START_TAG_CONTENT;

				TRY(decodeATWildcardEvent(strm, handler, nonTermID_out, app_data));
				TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_AT_QNAME, GR_START_TAG_CONTENT, &strm->context.currAttr, 1));
			break;
			case 2:
				// StartTagContent : NS event
				TRY(decodeNSEvent(strm, handler, nonTermID_out, app_data));
			break;
			case 3:
				// StartTagContent : SC event
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case 4:
				// SE(*) event
				strm->gStack->currNonTermID = GR_ELEMENT_CONTENT;

				TRY(decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data));
				TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, GR_ELEMENT_CONTENT, &strm->gStack->currQNameID, 1));
			break;
			case 5:
				// CH event
				DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">CH event\n"));

				*nonTermID_out = GR_ELEMENT_CONTENT;

				TRY(decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->gStack->currQNameID, app_data));
				TRY(insertZeroProduction((DynGrammarRule*) currentRule, EVENT_CH, *nonTermID_out, &voidQnameID, 1));
			break;
			case 6:
				// ER event
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case 7:
				// CM or PI event
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
		/* There are 3 possible states to exit the state machine: DT, CM or PI
		 * The state depends on the input event code from the stream and the
		 * available productions at level 2 and 3.*/
		unsigned int state = 0;
		unsigned int level3ProdCnt = 0;

		if(strm->gStack->currNonTermID == GR_DOC_CONTENT)
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
				TRY(decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val));
				if(tmp_bits_val == 0)
					state = 0;
				else if(tmp_bits_val == 1)
				{
					// CM or PI third level
					if(level3ProdCnt == 2)
					{
						TRY(decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val));
						state = tmp_bits_val + 1;
					}
					else if(level3ProdCnt == 1)
					{
						state = !IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + 1;
					}
					else
						return EXIP_INCONSISTENT_PROC_STATE;
				}
				else
					return EXIP_INCONSISTENT_PROC_STATE;
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
						TRY(decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val));
						state = tmp_bits_val + 1;
					}
					else if(level3ProdCnt == 1)
					{
						state = !IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + 1;
					}
					else
						return EXIP_INCONSISTENT_PROC_STATE;
				}
			}
			else
				return EXIP_INCONSISTENT_PROC_STATE;
		}
		else
		{
			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS))
				prodCnt += 1; // CM second level

			if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
				prodCnt += 1; // PI second level

			if(prodCnt == 2)
			{
				TRY(decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val));
				state = tmp_bits_val + 1;
			}
			else if(prodCnt == 1)
			{
				state = !IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) + 1;
			}
			else
				return EXIP_INCONSISTENT_PROC_STATE;
		}

		switch(state)
		{
			case 0:
				// DT event
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case 1:
				// CM event
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			case 2:
				// PI event
				return EXIP_NOT_IMPLEMENTED_YET;
			break;
			default:
				return EXIP_INCONSISTENT_PROC_STATE;
		}
	}
	else if(IS_FRAGMENT(strm->gStack->grammar->props))
	{
		// Fragment grammar
		// CM or PI event
		return EXIP_NOT_IMPLEMENTED_YET;
	}
	else
	{
		// Schema-informed element/type grammar
		QName qname;

		if(WITH_STRICT(strm->header.opts.enumOpt))
		{
			// Strict mode

			// Only available second level event if it is an entry grammar rule and is not Nil type grammar
			if(strm->gStack->currNonTermID == GR_START_TAG_CONTENT && strm->context.isNilType == FALSE)
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
					TRY(decodeNBitUnsignedInteger(strm, 1, &tmp_bits_val));
					if(tmp_bits_val > 1)
						return EXIP_INCONSISTENT_PROC_STATE;
				}
				else if(prodCnt == 1)
				{
					// zero bit encoded event
					tmp_bits_val = 0;
				}
				else
					return EXIP_INCONSISTENT_PROC_STATE;

				state = tmp_bits_val;

				if(!HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props))
					state += 1;

				switch(state)
				{
					case 0:
						// AT(xsi:type) event
						DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(xsi:type) event\n"));
						strm->context.currAttr.uriId = XML_SCHEMA_INSTANCE_ID;
						strm->context.currAttr.lnId = XML_SCHEMA_INSTANCE_TYPE_ID;
						qname.uri = &strm->schema->uriTable.uri[strm->context.currAttr.uriId].uriStr;
						qname.localName = &GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currAttr).lnStr;

						if(handler->attribute != NULL)  // Invoke handler method
						{
							TRY(handler->attribute(qname, app_data));
						}
						TRY(decodeQNameValue(strm, handler, nonTermID_out, app_data));
					break;
					case 1:
						// AT(xsi:nil) event
						TRY(decodeBoolean(strm, &nil));
						if(nil == TRUE)
							strm->context.isNilType = TRUE;

						DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(xsi:nil) event\n"));
						strm->context.currAttr.uriId = XML_SCHEMA_INSTANCE_ID;
						strm->context.currAttr.lnId = XML_SCHEMA_INSTANCE_NIL_ID;
						qname.uri = &strm->schema->uriTable.uri[strm->context.currAttr.uriId].uriStr;
						qname.localName = &GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currAttr).lnStr;

						if(handler->attribute != NULL)  // Invoke handler method
						{
							TRY(handler->attribute(qname, app_data));
						}

						if(handler->booleanData != NULL)  // Invoke handler method
						{
							TRY(handler->booleanData(nil, app_data));
						}
					break;
					default:
						return EXIP_INCONSISTENT_PROC_STATE;
				}
			}
			else
				return EXIP_INCONSISTENT_PROC_STATE;
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

			prodCnt = 2; // SE(*), CH(untyped) always available, position 7 and 8
			state_mask[7] = TRUE;
			state_mask[8] = TRUE;
			if(isContent2Grammar ||
					strm->gStack->currNonTermID < GET_CONTENT_INDEX(strm->gStack->grammar->props))
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

			if(strm->gStack->currNonTermID == GR_START_TAG_CONTENT)
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

			TRY(decodeNBitUnsignedInteger(strm, getBitsNumber(prodCnt - 1), &tmp_bits_val));

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
						TRY(handler->endElement(app_data));
					}
					*nonTermID_out = GR_VOID_NON_TERMINAL;
				break;
				case 1:
					// AT(xsi:type) event
					DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(xsi:type) event\n"));
					strm->context.currAttr.uriId = XML_SCHEMA_INSTANCE_ID;
					strm->context.currAttr.lnId = XML_SCHEMA_INSTANCE_TYPE_ID;
					qname.uri = &strm->schema->uriTable.uri[strm->context.currAttr.uriId].uriStr;
					qname.localName = &GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currAttr).lnStr;

					if(handler->attribute != NULL)  // Invoke handler method
					{
						TRY(handler->attribute(qname, app_data));
					}
					TRY(decodeQNameValue(strm, handler, nonTermID_out, app_data));
				break;
				case 2:
					// AT(xsi:nil) Element i, 0
					{
						boolean nil = FALSE;
						TRY(decodeBoolean(strm, &nil));
						if(nil == TRUE)
							strm->context.isNilType = TRUE;

						DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(xsi:nil) event\n"));
						strm->context.currAttr.uriId = XML_SCHEMA_INSTANCE_ID;
						strm->context.currAttr.lnId = XML_SCHEMA_INSTANCE_NIL_ID;
						qname.uri = &strm->schema->uriTable.uri[strm->context.currAttr.uriId].uriStr;
						qname.localName = &GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currAttr).lnStr;

						if(handler->attribute != NULL)  // Invoke handler method
						{
							TRY(handler->attribute(qname, app_data));
						}

						if(handler->booleanData != NULL)  // Invoke handler method
						{
							TRY(handler->booleanData(nil, app_data));
						}
					}
				break;
				case 3:
					// AT(*)
					*nonTermID_out = strm->gStack->currNonTermID;
					TRY(decodeATWildcardEvent(strm, handler, nonTermID_out, app_data));
				break;
				case 4:
					// third level AT: eighter AT (qname) [untyped value] or AT (*) [untyped value]
					return EXIP_NOT_IMPLEMENTED_YET;
				break;
				case 5:
					// NS Element i, 0
					TRY(decodeNSEvent(strm, handler, nonTermID_out, app_data));
				break;
				case 6:
					// SC event
					return EXIP_NOT_IMPLEMENTED_YET;
				break;
				case 7:
					// SE(*) content|same_rule
					if(isContent2Grammar || strm->gStack->currNonTermID < GET_CONTENT_INDEX(strm->gStack->grammar->props))
					{
						// currNonTermID should point to the content grammar rule
						if(GET_CONTENT_INDEX(strm->gStack->grammar->props) == GR_START_TAG_CONTENT)
							strm->gStack->currNonTermID = GR_CONTENT_2;
						else
							strm->gStack->currNonTermID = GET_CONTENT_INDEX(strm->gStack->grammar->props);
					}
					TRY(decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data));
				break;
				case 8:
					// CH [untyped value] content|same_rule
					DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">CH event\n"));
					if(isContent2Grammar || strm->gStack->currNonTermID < GET_CONTENT_INDEX(strm->gStack->grammar->props))
					{
						// nonTermID_out should point to the content grammar rule
						if(GET_CONTENT_INDEX(strm->gStack->grammar->props) == GR_START_TAG_CONTENT)
							*nonTermID_out = GR_CONTENT_2;
						else
							*nonTermID_out = GET_CONTENT_INDEX(strm->gStack->grammar->props);
					}
					else
						*nonTermID_out = strm->gStack->currNonTermID;
					TRY(decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, strm->gStack->currQNameID, app_data));
				break;
				case 9:
					// ER event
					return EXIP_NOT_IMPLEMENTED_YET;
				break;
				case 10:
					// third level: CM or PI event
					return EXIP_NOT_IMPLEMENTED_YET;
				break;
				default:
					return EXIP_INCONSISTENT_PROC_STATE;
			}
		}
	}

	return EXIP_ERR_OK;
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
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Decoding QName\n"));

	TRY(decodeUri(strm, &qnameID->uriId));
	qname->uri = &(strm->schema->uriTable.uri[qnameID->uriId].uriStr);
	TRY(decodeLn(strm, qnameID->uriId, &qnameID->lnId));
	qname->localName = &GET_LN_P_URI_P_QNAME(&strm->schema->uriTable, qnameID).lnStr;

	return decodePfxQname(strm, qname, qnameID->uriId);
}

errorCode decodeUri(EXIStream* strm, SmallIndex* uriId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned int tmp_val_buf = 0;
	unsigned char uriBits = getBitsNumber(strm->schema->uriTable.count);

	TRY(decodeNBitUnsignedInteger(strm, uriBits, &tmp_val_buf));
	if(tmp_val_buf == 0) // uri miss
	{
		String str;
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">URI miss\n"));
		TRY(decodeString(strm, &str));
		TRY(addUriEntry(&strm->schema->uriTable, str, uriId));
	}
	else // uri hit
	{
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">URI hit\n"));
		*uriId = tmp_val_buf - 1;
		if(*uriId >= strm->schema->uriTable.count)
			return EXIP_INVALID_EXI_INPUT;
	}

	return EXIP_ERR_OK;
}

errorCode decodeLn(EXIStream* strm, Index uriId, Index* lnId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	UnsignedInteger tmpVar = 0;

	TRY(decodeUnsignedInteger(strm, &tmpVar));

	if(tmpVar == 0) // local-name table hit
	{
		unsigned int l_lnId;
		unsigned char lnBits = getBitsNumber((unsigned int)(strm->schema->uriTable.uri[uriId].lnTable.count - 1));
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">local-name table hit\n"));
		TRY(decodeNBitUnsignedInteger(strm, lnBits, &l_lnId));

		if(l_lnId >= strm->schema->uriTable.uri[uriId].lnTable.count)
			return EXIP_INVALID_EXI_INPUT;
		*lnId = l_lnId;
	}
	else // local-name table miss
	{
		String lnStr;
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">local-name table miss\n"));

		TRY(allocateStringMemoryManaged(&(lnStr.str),(Index) (tmpVar - 1), &strm->memList));
		TRY(decodeStringOnly(strm, (Index)tmpVar - 1, &lnStr));

		if(strm->schema->uriTable.uri[uriId].lnTable.ln == NULL)
		{
			// Create local name table for this URI entry
			TRY(createDynArray(&strm->schema->uriTable.uri[uriId].lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER));
		}
		TRY(addLnEntry(&strm->schema->uriTable.uri[uriId].lnTable, lnStr, lnId));
	}

	return EXIP_ERR_OK;
}

errorCode decodePfxQname(EXIStream* strm, QName* qname, SmallIndex uriId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned char prefixBits = 0;
	unsigned int prefixID = 0;

	qname->prefix = NULL;

	if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES) == FALSE)
		return EXIP_ERR_OK;

	if(strm->schema->uriTable.uri[uriId].pfxTable == NULL || strm->schema->uriTable.uri[uriId].pfxTable->count == 0)
		return EXIP_ERR_OK;

	prefixBits = getBitsNumber(strm->schema->uriTable.uri[uriId].pfxTable->count - 1);

	if(prefixBits > 0)
	{
		TRY(decodeNBitUnsignedInteger(strm, prefixBits, &prefixID));

		if(prefixID >= strm->schema->uriTable.uri[uriId].pfxTable->count)
			return EXIP_INVALID_EXI_INPUT;
	}

	qname->prefix = &strm->schema->uriTable.uri[uriId].pfxTable->pfxStr[prefixID];

	return EXIP_ERR_OK;
}

errorCode decodePfx(EXIStream* strm, SmallIndex uriId, SmallIndex* pfxId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned int tmp_val_buf = 0;
	unsigned char prfxBits = getBitsNumber(strm->schema->uriTable.uri[uriId].pfxTable->count);

	TRY(decodeNBitUnsignedInteger(strm, prfxBits, &tmp_val_buf));

	if(tmp_val_buf == 0) // prefix miss
	{
		String str;
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Prefix miss\n"));
		TRY(decodeString(strm, &str));
		TRY(addPfxEntry(strm->schema->uriTable.uri[uriId].pfxTable, str, pfxId));
	}
	else // prefix hit
	{
		DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Prefix hit\n"));
		*pfxId = tmp_val_buf-1;
		if(*pfxId >= strm->schema->uriTable.uri[uriId].pfxTable->count)
			return EXIP_INVALID_EXI_INPUT;
	}

	return EXIP_ERR_OK;
}

errorCode decodeStringValue(EXIStream* strm, QNameID qnameID, String* value)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	UnsignedInteger tmpVar = 0;
	TRY(decodeUnsignedInteger(strm, &tmpVar));

	if(tmpVar == 0) // "local" value partition table hit
	{
#if VALUE_CROSSTABLE_USE
		unsigned int vxEntryId = 0;
		unsigned char vxBits;
		VxTable* vxTable;

		vxTable = GET_LN_URI_QNAME(strm->schema->uriTable, qnameID).vxTable;
		assert(vxTable);
		vxBits = getBitsNumber(vxTable->count - 1);
		TRY(decodeNBitUnsignedInteger(strm, vxBits, &vxEntryId));

		*value = strm->valueTable.value[vxTable->vx[vxEntryId].globalId].valueStr;
#else
		DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, ("> Local-value partition table hit but VALUE_CROSSTABLE_USE disabled \n"));
		return EXIP_INCONSISTENT_PROC_STATE;
#endif
	}
	else if(tmpVar == 1)// global value partition table hit
	{
		unsigned int valueEntryID = 0;
		unsigned char valueBits;
		
		valueBits = getBitsNumber(strm->valueTable.count - 1);
		TRY(decodeNBitUnsignedInteger(strm, valueBits, &valueEntryID));

		*value = strm->valueTable.value[valueEntryID].valueStr;
	}
	else  // "local" value partition and global value partition table miss
	{
		Index vStrLen = (Index) tmpVar - 2;

		TRY(allocateStringMemory(&value->str, vStrLen));
		TRY(decodeStringOnly(strm, vStrLen, value));

		if(vStrLen > 0 && vStrLen <= strm->header.opts.valueMaxLength && strm->header.opts.valuePartitionCapacity > 0)
		{
			// The value should be entered in the value partitions of the string tables
			TRY(addValueEntry(strm, *value, qnameID));
		}
	}
	return EXIP_ERR_OK;
}

errorCode decodeEventContent(EXIStream* strm, Production* prodHit, ContentHandler* handler,
							SmallIndex* nonTermID_out, void* app_data)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	QName qname;

	// TODO: implement all cases of events such as PI, CM etc.

	switch(GET_PROD_EXI_EVENT(prodHit->content))
	{
		case EVENT_SE_ALL:
			strm->gStack->currNonTermID = GET_PROD_NON_TERM(prodHit->content);
			assert(strm->context.isNilType == FALSE);

			TRY(decodeSEWildcardEvent(strm, handler, nonTermID_out, app_data));
		break;
		case EVENT_AT_ALL:
			TRY(decodeATWildcardEvent(strm, handler, nonTermID_out, app_data));
		break;
		case EVENT_SE_QNAME:
		{
			EXIGrammar* elemGrammar = NULL;

			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">SE(qname) event: \n"));
			assert(strm->context.isNilType == FALSE);

			qname.uri = &(strm->schema->uriTable.uri[prodHit->qnameId.uriId].uriStr);
			qname.localName = &(GET_LN_URI_QNAME(strm->schema->uriTable, prodHit->qnameId).lnStr);
#if DEBUG_CONTENT_IO == ON && EXIP_DEBUG_LEVEL == INFO
			printString(qname.uri);
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (" : "));
			printString(qname.localName);
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, ("\n"));
#endif
			TRY(decodePfxQname(strm, &qname, prodHit->qnameId.uriId));

			strm->gStack->currNonTermID = *nonTermID_out;

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
				TRY(pushGrammar(&(strm->gStack), elemGrammar));
			}
			else
			{
				return EXIP_INCONSISTENT_PROC_STATE;  // The event require the presence of Element Grammar previously created
			}

			strm->gStack->currQNameID = prodHit->qnameId;

			if(handler->startElement != NULL)  // Invoke handler method passing the element qname
			{
				TRY(handler->startElement(qname, app_data));
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
			TRY(decodePfxQname(strm, &qname, prodHit->qnameId.uriId));
			if(handler->attribute != NULL)  // Invoke handler method
			{
				TRY(handler->attribute(qname, app_data));
			}
			TRY(decodeValueItem(strm, prodHit->typeId, handler, nonTermID_out, prodHit->qnameId, app_data));
		}
		break;
		case EVENT_CH:
		{
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">CH event\n"));
			assert(strm->context.isNilType == FALSE);
			TRY(decodeValueItem(strm, prodHit->typeId, handler, nonTermID_out, strm->gStack->currQNameID, app_data));
		}
		break;
		case EVENT_NS:
			TRY(decodeNSEvent(strm, handler, nonTermID_out, app_data));
		break;
		default:
			return EXIP_NOT_IMPLEMENTED_YET;
	}

	return EXIP_ERR_OK;
}

errorCode decodeValueItem(EXIStream* strm, Index typeId, ContentHandler* handler, SmallIndex* nonTermID_out, QNameID localQNameID, void* app_data)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
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
			TRY(decodeUnsignedInteger(strm, &uintVal));

			if(handler->intData != NULL)  // Invoke handler method
			{
				// TODO: the cast to signed int can introduce errors. Check first!
				TRY(handler->intData((Integer) uintVal, app_data));
			}
		}
		break;
		case VALUE_TYPE_INTEGER:
		{
			Integer sintVal;
			TRY(decodeIntegerValue(strm, &sintVal));
			if(handler->intData != NULL)  // Invoke handler method
			{
				TRY(handler->intData(sintVal, app_data));
			}
		}
		break;
		case VALUE_TYPE_SMALL_INTEGER:
		{
			unsigned int uintVal;
			int base;
			int64_t upLimit;

			if(typeId >= strm->schema->simpleTypeTable.count)
				return EXIP_INVALID_EXI_INPUT;

			if(!HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MIN_INCLUSIVE)
					&& !HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MIN_EXCLUSIVE))
				return EXIP_INVALID_EXI_INPUT;
			if(!HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MAX_INCLUSIVE)
					&& !HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MAX_EXCLUSIVE))
				return EXIP_INVALID_EXI_INPUT;

			if(HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MIN_INCLUSIVE))
				base = strm->schema->simpleTypeTable.sType[typeId].min;
			else
				return EXIP_NOT_IMPLEMENTED_YET;

			if(HAS_TYPE_FACET(strm->schema->simpleTypeTable.sType[typeId].content, TYPE_FACET_MAX_INCLUSIVE))
				upLimit = strm->schema->simpleTypeTable.sType[typeId].max;
			else
				return EXIP_NOT_IMPLEMENTED_YET;

			TRY(decodeNBitUnsignedInteger(strm, getBitsNumber(upLimit - base), &uintVal));
			if(handler->intData != NULL)  // Invoke handler method
			{
				TRY(handler->intData((Integer) (base + uintVal), app_data));
			}
		}
		break;
		case VALUE_TYPE_FLOAT:
		{
			Float flVal;
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Float value\n"));
			TRY(decodeFloatValue(strm, &flVal));
			if(handler->floatData != NULL)  // Invoke handler method
			{
				TRY(handler->floatData(flVal, app_data));
			}
		}
		break;
		case VALUE_TYPE_BOOLEAN:
		{
			boolean bool_val;
			TRY(decodeBoolean(strm, &bool_val));
			if(handler->booleanData != NULL)  // Invoke handler method
			{
				TRY(handler->booleanData(bool_val, app_data));
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
			TRY(decodeBinary(strm, &binary_val, &nbytes));

			if(handler->binaryData != NULL)  // Invoke handler method
			{
				TRY_CATCH(handler->binaryData(binary_val, nbytes, app_data), EXIP_MFREE(binary_val));
			}
			
			/* Free the memory allocated by decodeBinary() */
			EXIP_MFREE(binary_val);
		}
		break;
		case VALUE_TYPE_DECIMAL:
		{
			Decimal decVal;

			TRY(decodeDecimalValue(strm, &decVal));
			if(handler->decimalData != NULL)  // Invoke handler method
			{
				TRY(handler->decimalData(decVal, app_data));
			}
		}
		break;
		case VALUE_TYPE_DATE_TIME:
		{
			EXIPDateTime dtVal;

			TRY(decodeDateTimeValue(strm, &dtVal));
			if(handler->dateTimeData != NULL)  // Invoke handler method
			{
				TRY(handler->dateTimeData(dtVal, app_data));
			}
		}
		break;
		case VALUE_TYPE_LIST:
		{
			UnsignedInteger itemCount;
			Index itemTypeId;
			unsigned int i;

			TRY(decodeUnsignedInteger(strm, &itemCount));

			itemTypeId = strm->schema->simpleTypeTable.sType[typeId].length; // The item typeID must be encoded in the length field
			if(itemTypeId >= strm->schema->simpleTypeTable.count)
				return EXIP_UNEXPECTED_ERROR;

			if(handler->listData != NULL)  // Invoke handler method
			{
				TRY(handler->listData(GET_EXI_TYPE(strm->schema->simpleTypeTable.sType[itemTypeId].content), (unsigned int) itemCount, app_data));
			}

			for(i = 0; i < itemCount; i++)
			{
				TRY(decodeValueItem(strm, itemTypeId, handler, nonTermID_out, localQNameID, app_data));
			}
		}
		break;
		case VALUE_TYPE_QNAME:
		{
			// Only allowed if the current production is AT(xsi:type)
			assert(localQNameID.uriId == XML_SCHEMA_INSTANCE_ID && localQNameID.lnId == XML_SCHEMA_INSTANCE_TYPE_ID);
			TRY(decodeQNameValue(strm, handler, nonTermID_out, app_data));
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
					return EXIP_UNEXPECTED_ERROR;

				TRY(decodeNBitUnsignedInteger(strm, getBitsNumber(eDefFound->count - 1), &indx));
				value = ((String*) eDefFound->values)[indx];
				freeable = FALSE;
			}
			else
			{
				TRY(decodeStringValue(strm, localQNameID, &value));

				if(value.length == 0 || value.length > strm->header.opts.valueMaxLength || strm->header.opts.valuePartitionCapacity == 0)
					freeable = TRUE;
			}

			if(handler->stringData != NULL)  // Invoke handler method
			{
				TRY(handler->stringData(value, app_data));
			}

			if(freeable)
				EXIP_MFREE(value.str);
		} break;
	}

	return EXIP_ERR_OK;
}

errorCode decodeNSEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	SmallIndex ns_uriId;
	SmallIndex pfxId;
	boolean bool = FALSE;

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">NS event:\n"));
	*nonTermID_out = GR_START_TAG_CONTENT;

	TRY(decodeUri(strm, &ns_uriId));

	if(strm->schema->uriTable.uri[ns_uriId].pfxTable == NULL)
	{
		TRY(createPfxTable(&strm->schema->uriTable.uri[ns_uriId].pfxTable));
	}

	TRY(decodePfx(strm, ns_uriId, &pfxId));
	TRY(decodeBoolean(strm, &bool));

	if(handler->namespaceDeclaration != NULL)  // Invoke handler method
	{
		TRY(handler->namespaceDeclaration(strm->schema->uriTable.uri[ns_uriId].uriStr, strm->schema->uriTable.uri[ns_uriId].pfxTable->pfxStr[pfxId], bool, app_data));
	}
	return EXIP_ERR_OK;
}

errorCode decodeSEWildcardEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	EXIGrammar* elemGrammar = NULL;
	QName qname;
	QNameID qnameId = {URI_MAX, LN_MAX};

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">SE(*) event\n"));

	// The content of SE event is the element qname
	TRY(decodeQName(strm, &qname, &qnameId));

	if(handler->startElement != NULL)  // Invoke handler method passing the element qname
	{
		TRY(handler->startElement(qname, app_data));
	}

	// New element grammar is pushed on the stack
	elemGrammar = GET_ELEM_GRAMMAR_QNAMEID(strm->schema, qnameId);
	*nonTermID_out = GR_START_TAG_CONTENT;

	if(elemGrammar != NULL)
	{
		// The grammar is found
		TRY(pushGrammar(&(strm->gStack), elemGrammar));
	}
	else
	{
#if BUILD_IN_GRAMMARS_USE
		Index dynArrIndx;
		EXIGrammar newElementGrammar;
		TRY(createBuiltInElementGrammar(&newElementGrammar, strm));
		TRY(addDynEntry(&strm->schema->grammarTable.dynArray, &newElementGrammar, &dynArrIndx));

		GET_LN_URI_QNAME(strm->schema->uriTable, qnameId).elemGrammar = dynArrIndx;
		TRY(pushGrammar(&(strm->gStack), &strm->schema->grammarTable.grammar[dynArrIndx]));
#elif EXI_PROFILE_DEFAULT
		{
			unsigned int prodCnt = 4;
			unsigned int tmp_bits_val = 0;
			QName attrQname;
			QNameID attrQnameId = {URI_MAX, LN_MAX};

			prodCnt += IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PREFIXES);
			prodCnt += WITH_SELF_CONTAINED(strm->header.opts.enumOpt);
			prodCnt += IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD);
			prodCnt += (IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS));

			// There should be a valid xsi:type switch, otherwise rise an error
			TRY(decodeNBitUnsignedInteger(strm, getBitsNumber(prodCnt  - 1), &tmp_bits_val));
			if(tmp_bits_val != 1)
			{
				DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Build-in element grammars are not supported by this configuration \n"));
				return EXIP_INCONSISTENT_PROC_STATE;
			}

			TRY(decodeQName(strm, &attrQname, &attrQnameId));
			if(attrQnameId.uriId != XML_SCHEMA_INSTANCE_ID || attrQnameId.lnId != XML_SCHEMA_INSTANCE_TYPE_ID)
			{
				DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Build-in element grammars are not supported by this configuration \n"));
				return EXIP_INCONSISTENT_PROC_STATE;
			}

			if(handler->attribute != NULL)  // Invoke handler method for xsi:type
			{
				TRY(handler->attribute(attrQname, app_data));
			}

			TRY(decodeQName(strm, &attrQname, &attrQnameId));

			if(handler->qnameData != NULL)  // Invoke handler method for the QName value of xsi:type
			{
				TRY(handler->qnameData(attrQname, app_data));
			}

			// New element grammar is pushed on the stack
			elemGrammar = GET_TYPE_GRAMMAR_QNAMEID(strm->schema, attrQnameId);

			if(elemGrammar != NULL)
			{
				// The grammar is found
				*nonTermID_out = GR_START_TAG_CONTENT;
				TRY(pushGrammar(&(strm->gStack), elemGrammar));
			}
			else
			{
				DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Build-in element grammars are not supported by this configuration \n"));
				return EXIP_INCONSISTENT_PROC_STATE;
			}
		}
#else
		DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">Build-in element grammars are not supported by this configuration \n"));
		assert(FALSE);
		return EXIP_INCONSISTENT_PROC_STATE;
#endif
	}

	strm->gStack->currQNameID = qnameId;
	return EXIP_ERR_OK;
}

errorCode decodeATWildcardEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	QName qname;
	QNameID qnameId = {URI_MAX, LN_MAX};

	DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">AT(*) event\n"));

	TRY(decodeQName(strm, &qname, &qnameId));

	if(qnameId.uriId == XML_SCHEMA_INSTANCE_ID &&
			(qnameId.lnId == XML_SCHEMA_INSTANCE_TYPE_ID || qnameId.lnId == XML_SCHEMA_INSTANCE_NIL_ID))
	{
		if(IS_SCHEMA(strm->gStack->grammar->props))
		{
			DEBUG_MSG(ERROR, DEBUG_CONTENT_IO, (">In schema-informed grammars, xsi:type and xsi:nil attributes MUST NOT be represented using AT(*) terminal\n"));
			return EXIP_INCONSISTENT_PROC_STATE;
		}
	}

	if(handler->attribute != NULL)  // Invoke handler method
	{
		TRY(handler->attribute(qname, app_data));
	}

	TRY(decodeValueItem(strm, INDEX_MAX, handler, nonTermID_out, qnameId, app_data));
	strm->context.currAttr = qnameId;

	return EXIP_ERR_OK;
}

static errorCode decodeQNameValue(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data)
{
	// TODO: Add the case when Preserve.lexicalValues option value is true - instead of Qname decode it as String
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	QName qname;
	QNameID qnameId;
	EXIGrammar* newGrammar = NULL;

	TRY(decodeQName(strm, &qname, &qnameId));

	if(handler->qnameData != NULL)  // Invoke handler method
	{
		TRY(handler->qnameData(qname, app_data));
	}

	// New type grammar is pushed on the stack if it exists
	newGrammar = GET_TYPE_GRAMMAR_QNAMEID(strm->schema, qnameId);

	if(newGrammar != NULL)
	{
		// The grammar is found
		EXIGrammar* currGr;

		popGrammar(&(strm->gStack), &currGr);

		*nonTermID_out = GR_START_TAG_CONTENT;
		TRY(pushGrammar(&(strm->gStack), newGrammar));
	}

	return EXIP_ERR_OK;
}
