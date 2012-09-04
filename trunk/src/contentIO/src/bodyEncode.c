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
#include "eventsEXI.h"
#include "grammarRules.h"
#include "stringManipulate.h"
#include "grammars.h"
#include "memManagement.h"
#include "dynamicArray.h"

errorCode encodeStringData(EXIStream* strm, String strng, QNameID qnameID, Index typeId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char flag_StringLiteralsPartition = 0;
	Index vxEntryId = 0;
	VxTable* vxTable = &GET_LN_URI_QNAME(strm->schema->uriTable, qnameID).vxTable;

	/* ENUMERATION CHECK */
	if(typeId != INDEX_MAX && (strm->schema->simpleTypeTable.sType[typeId].facetPresenceMask & TYPE_FACET_ENUMERATION) != 0)
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
				return encodeNBitUnsignedInteger(strm, getBitsNumber(eDefFound->count), i);
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
				String copiedValue;
				tmp_err_code = cloneString(&strng, &copiedValue, &strm->memList);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				tmp_err_code = addValueEntry(strm, &copiedValue, qnameID);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
	}

	return ERR_OK;
}

errorCode lookupProduction(EXIStream* strm, EventType eventType, EXIType exiType, GrammarRule** currentRule, Index* typeId, QName* qname, unsigned char* codeLength, Index* lastCodePart)
{
	unsigned char b = 0;
	Index j = 0;
	Production* prodHit = NULL;
	Production* tmpProd = NULL;
	EXIType prodExiType = VALUE_TYPE_NONE;

	if(strm->context.currNonTermID >=  strm->gStack->grammar->count)
		return INCONSISTENT_PROC_STATE;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		*currentRule = (GrammarRule*) &((DynGrammarRule*) strm->gStack->grammar->rule)[strm->context.currNonTermID];
	else
		*currentRule = &strm->gStack->grammar->rule[strm->context.currNonTermID];

	for(b = 0; b < 3; b++)
	{
		for(j = 0; j < (*currentRule)->part[b].count; j++)
		{
			tmpProd = &(*currentRule)->part[b].prod[(*currentRule)->part[b].count - 1 - j];

			if(tmpProd->eventType != EVENT_SE_QNAME && tmpProd->typeId != INDEX_MAX)
				prodExiType = strm->schema->simpleTypeTable.sType[tmpProd->typeId].exiType;
			else
				prodExiType = VALUE_TYPE_NONE;
			if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props) || valueTypeClassesEqual(prodExiType, exiType))
			{
				if(tmpProd->eventType == eventType || // (1)
						(qname != NULL &&
						(((tmpProd->eventType == EVENT_AT_URI || tmpProd->eventType == EVENT_SE_URI) &&    // (2)
						stringEqual(strm->schema->uriTable.uri[tmpProd->qnameId.uriId].uriStr, *(qname->uri))) ||
						((tmpProd->eventType == EVENT_AT_QNAME || tmpProd->eventType == EVENT_SE_QNAME) && // (3)
						stringEqual(strm->schema->uriTable.uri[tmpProd->qnameId.uriId].uriStr, *(qname->uri)) &&
						stringEqual(GET_LN_URI_QNAME(strm->schema->uriTable, tmpProd->qnameId).lnStr, *(qname->localName))))
						)
				)
				{
					prodHit = tmpProd;
					*typeId = tmpProd->typeId;
					break;
				}
			}
		}
		if(prodHit != NULL)
			break;
	}

	if(prodHit == NULL)
		return INCONSISTENT_PROC_STATE;

	*codeLength = b + 1;
	*lastCodePart = j;

	return ERR_OK;
}

errorCode encodeProduction(EXIStream* strm, GrammarRule* currentRule, unsigned char codeLength, Index lastCodePart, QName* qname)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production* prodHit = NULL;

#if DEBUG_CONTENT_IO == ON
	{
		tmp_err_code = printGrammarRule(strm->context.currNonTermID, currentRule, strm->schema);
		if(tmp_err_code != ERR_OK)
		{
			DEBUG_MSG(INFO, DEBUG_CONTENT_IO, (">Error printing grammar rule\n"));
		}
	}
#endif

	if(codeLength - 1 > 2)
		return INCONSISTENT_PROC_STATE;

	if(lastCodePart >= currentRule->part[codeLength - 1].count)
		return INCONSISTENT_PROC_STATE;

	prodHit = &currentRule->part[codeLength - 1].prod[currentRule->part[codeLength - 1].count - 1 - lastCodePart];

	tmp_err_code = writeEventCode(strm, currentRule, codeLength, lastCodePart);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props) && codeLength > 1
		&& (prodHit->eventType == EVENT_CH || prodHit->eventType == EVENT_EE))  // If the current grammar is build-in Element grammar and the event code size is bigger than 1 and the event is CH or EE...
	{
		// #1# COMMENT and #2# COMMENT
		tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, prodHit->eventType, prodHit->nonTermID,
				&prodHit->qnameId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	switch(prodHit->eventType)
	{
		case EVENT_AT_ALL:
		{
			if(qname == NULL)
				return NULL_POINTER_REF;

			tmp_err_code = encodeQName(strm, *qname, prodHit->eventType, &strm->context.currAttr);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
			{
				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_AT_QNAME, prodHit->nonTermID, &strm->context.currAttr);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		break;
		case EVENT_SE_ALL:
		{
			if(qname == NULL)
				return NULL_POINTER_REF;

			tmp_err_code = encodeQName(strm, *qname, prodHit->eventType, &strm->context.currElem);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
			{
				tmp_err_code = insertZeroProduction((DynGrammarRule*) currentRule, EVENT_SE_QNAME, prodHit->nonTermID, &strm->context.currElem);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		break;
		case EVENT_AT_URI:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		case EVENT_SE_URI:
		{
			return NOT_IMPLEMENTED_YET;
		}
		break;
		case EVENT_AT_QNAME:
		{
			strm->context.currAttr.uriId = prodHit->qnameId.uriId;
			strm->context.currAttr.lnId = prodHit->qnameId.lnId;

			tmp_err_code = encodePfxQName(strm, qname, prodHit->eventType, prodHit->qnameId.uriId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		break;
		case EVENT_SE_QNAME:
		{
			strm->context.currElem.uriId = prodHit->qnameId.uriId;
			strm->context.currElem.lnId = prodHit->qnameId.lnId;

			tmp_err_code = encodePfxQName(strm, qname, prodHit->eventType, prodHit->qnameId.uriId);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		break;
		default:
			break; // Do nothing!
	}

	strm->context.currNonTermID = prodHit->nonTermID;

	if(prodHit->eventType == EVENT_SE_ALL ||
			prodHit->eventType == EVENT_SE_URI)
	{
		EXIGrammar* elemGrammar = NULL;

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

			tmp_err_code = addDynEntry(&strm->schema->grammarTable.dynArray, &newElementGrammar, &dynArrIndx, &strm->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			GET_LN_URI_QNAME(strm->schema->uriTable, strm->context.currElem).elemGrammar = dynArrIndx;

			strm->context.currNonTermID = GR_START_TAG_CONTENT;
			tmp_err_code = pushGrammar(&(strm->gStack), &strm->schema->grammarTable.grammar[dynArrIndx]);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
	else if(prodHit->eventType == EVENT_SE_QNAME)
	{
		EXIGrammar* elemGrammar = NULL;

		strm->gStack->lastNonTermID = strm->context.currNonTermID;

		// New element grammar is pushed on the stack
		if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))  // If the current grammar is build-in Element grammar ...
		{
			elemGrammar = GET_ELEM_GRAMMAR_QNAMEID(strm->schema, strm->context.currElem);
		}
		else
		{
			elemGrammar = &strm->schema->grammarTable.grammar[prodHit->typeId];
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
	else if(prodHit->eventType == EVENT_EE)
	{
		if(strm->context.currNonTermID == GR_VOID_NON_TERMINAL)
		{
			EXIGrammar* grammar;
			popGrammar(&(strm->gStack), &grammar);
			if(strm->gStack != NULL) // There is more grammars in the stack
				strm->context.currNonTermID = strm->gStack->lastNonTermID;
		}
	}
	else if(prodHit->eventType == EVENT_AT_ALL ||
			prodHit->eventType == EVENT_AT_QNAME ||
			prodHit->eventType == EVENT_AT_URI)
	{
		strm->context.expectATData = TRUE;
		strm->context.attrTypeId = prodHit->typeId;
	}

	return ERR_OK;
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

		tmp_err_code = cloneString(uri, &copiedURI, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addUriEntry(&strm->schema->uriTable, copiedURI, uriId, &strm->memList);
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
			tmp_err_code = createDynArray(&strm->schema->uriTable.uri[qnameID->uriId].lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER, &strm->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		tmp_err_code = cloneString(ln, &copiedLN, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLnEntry(&strm->schema->uriTable.uri[qnameID->uriId].lnTable, copiedLN, &qnameID->lnId, &strm->memList);
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

		tmp_err_code = cloneString(prefix, &copiedPrefix, &strm->memList);
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

	exiType = strm->schema->simpleTypeTable.sType[typeId].exiType;

	if(exiType == VALUE_TYPE_SMALL_INTEGER)
	{
		// TODO: take into account  minExclusive and  maxExclusive when they are supported
		unsigned int encoded_val;
		unsigned char numberOfBits;

		if(int_val > strm->schema->simpleTypeTable.sType[typeId].maxInclusive ||
				int_val < strm->schema->simpleTypeTable.sType[typeId].minInclusive)
			return INVALID_EXI_INPUT;

		encoded_val = (unsigned int) (int_val - strm->schema->simpleTypeTable.sType[typeId].minInclusive);
		numberOfBits = getBitsNumber(strm->schema->simpleTypeTable.sType[typeId].maxInclusive - strm->schema->simpleTypeTable.sType[typeId].minInclusive);

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
