/*==================================================================*\
|                EXIP - Embeddable EXI Processor in   C              |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammars.c
 * @brief Defines grammar related functions
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id$
 */

#include "grammars.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "sTables.h"
#include "ioUtil.h"

#define DEF_DOC_GRAMMAR_RULE_NUMBER 2 // first rule is excluded
#define DEF_FRAG_GRAMMAR_RULE_NUMBER 1 // first rule is excluded
#define DEF_ELEMENT_GRAMMAR_RULE_NUMBER 2

errorCode createDocGrammar(EXIPSchema* schema, QNameID* elQnameArr, Index qnameCount)
{
	GrammarRule* tmp_rule;

	schema->docGrammar.count = DEF_DOC_GRAMMAR_RULE_NUMBER;
	schema->docGrammar.props = 0;
	SET_DOCUMENT_GR(schema->docGrammar.props);
	schema->docGrammar.rule = (GrammarRule*) memManagedAllocate(&schema->memList, sizeof(GrammarRule)*DEF_DOC_GRAMMAR_RULE_NUMBER);
	if(schema->docGrammar.rule == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	/* Rule for Document */
	/*
	 * Document :
	 *			  SD DocContent	0
	 */

	// IGNORED!

	/* Rule for document content */
	tmp_rule = &schema->docGrammar.rule[GR_DOC_CONTENT];

	if(elQnameArr != NULL)   // Creates Schema Informed Grammar
	{
		unsigned int e = 0;
		Index tmp_code1;

		SET_SCHEMA_GR(schema->docGrammar.props);
		tmp_code1 = qnameCount + 1;

		tmp_rule->production = (Production*) memManagedAllocate(&schema->memList, sizeof(Production)*tmp_code1);
		if(tmp_rule->production == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		/*
		 * DocContent :
		 *			   	SE (G-0)   DocEnd	0
		 *				SE (G-1)   DocEnd	1
		 *				⋮	⋮      ⋮
		 *				SE (G-n−1) DocEnd n-1
		 *			//	SE (*)     DocEnd	n		//  This is created as part of the Built-In grammar further on
		 */

		for(e = 0; e < qnameCount; e++)
		{
			SET_PROD_EXI_EVENT(tmp_rule->production[qnameCount - e].content, EVENT_SE_QNAME);
			SET_PROD_NON_TERM(tmp_rule->production[qnameCount - e].content, GR_DOC_END);
			tmp_rule->production[qnameCount - e].typeId = GET_LN_URI_QNAME(schema->uriTable, elQnameArr[e]).elemGrammar;
			tmp_rule->production[qnameCount - e].qnameId = elQnameArr[e];
		}
		tmp_rule->pCount = tmp_code1;
	}
	else
	{
		tmp_rule->production = (Production*) memManagedAllocate(&schema->memList, sizeof(Production));
		if(tmp_rule->production == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		tmp_rule->pCount = 1;
		tmp_rule->meta = 0;
	}

	/*
	 * DocContent :
	 *				SE (*) DocEnd	0
	 */
	SET_PROD_EXI_EVENT(tmp_rule->production[0].content, EVENT_SE_ALL);
	SET_PROD_NON_TERM(tmp_rule->production[0].content, GR_DOC_END);
	tmp_rule->production[0].typeId = INDEX_MAX;
	tmp_rule->production[0].qnameId.uriId = URI_MAX;
	tmp_rule->production[0].qnameId.lnId = LN_MAX;

	/* Rule for Document end */
	/* 
	 * DocEnd :
	 *			ED	        0
	 */
	tmp_rule = &schema->docGrammar.rule[GR_DOC_END];

	// TODO: consider ignoring this rule as well. In exipg generation as well ...

	/* Part 1 */
	tmp_rule->production = (Production*) memManagedAllocate(&schema->memList, sizeof(Production));
	if(tmp_rule->production == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	SET_PROD_EXI_EVENT(tmp_rule->production[0].content, EVENT_ED);
	SET_PROD_NON_TERM(tmp_rule->production[0].content, GR_VOID_NON_TERMINAL);
	tmp_rule->production[0].typeId = INDEX_MAX;
	tmp_rule->production[0].qnameId.uriId = URI_MAX;
	tmp_rule->production[0].qnameId.lnId = LN_MAX;

	tmp_rule->pCount = 1;
	tmp_rule->meta = 0;

	return EXIP_OK;
}

#if BUILD_IN_GRAMMARS_USE
errorCode createBuiltInElementGrammar(EXIGrammar* elementGrammar, EXIStream* strm)
{
	DynGrammarRule* tmp_rule;

	elementGrammar->count = DEF_ELEMENT_GRAMMAR_RULE_NUMBER;
	elementGrammar->props = 0;
	SET_BUILT_IN_ELEM_GR(elementGrammar->props);
	elementGrammar->rule = (GrammarRule*) EXIP_MALLOC(sizeof(DynGrammarRule)*DEF_ELEMENT_GRAMMAR_RULE_NUMBER);
	if(elementGrammar->rule == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	/* Rule for StartTagContent */
	/* StartTagContent :
	 *						EE	                    0.0
	 *						AT (*) StartTagContent	0.1
	 *						NS StartTagContent	    0.2
	 *						SC Fragment	            0.3
	 *						SE (*) ElementContent	0.4
	 *						CH ElementContent	    0.5
	 *						ER ElementContent	    0.6
	 *						CM ElementContent	    0.7.0
	 *						PI ElementContent	    0.7.1
	 */

	tmp_rule = &((DynGrammarRule*) elementGrammar->rule)[GR_START_TAG_CONTENT];

	/* Part 1 */
	tmp_rule->production = (Production*) EXIP_MALLOC(sizeof(Production)*DEFAULT_PROD_ARRAY_DIM);
	if(tmp_rule->production == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	/* The part 1 productions get added later... */
	tmp_rule->pCount = 0;
	tmp_rule->meta = 0;
	tmp_rule->prodDim = DEFAULT_PROD_ARRAY_DIM;

	/* Rule for ElementContent */
	/* ElementContent :
	 *						EE	                    0
	 *						SE (*) ElementContent	1.0
	 *						CH ElementContent	    1.1
	 *						ER ElementContent	    1.2
	 *						CM ElementContent	    1.3.0
	 *						PI ElementContent	    1.3.1
	 */
	tmp_rule = &((DynGrammarRule*) elementGrammar->rule)[GR_ELEMENT_CONTENT];

	/* Part 1 */
	tmp_rule->production = (Production*) EXIP_MALLOC(sizeof(Production)*DEFAULT_PROD_ARRAY_DIM);
	if(tmp_rule->production == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	/* EE	                  0 */
	SET_PROD_EXI_EVENT(tmp_rule->production[0].content, EVENT_EE);
	SET_PROD_NON_TERM(tmp_rule->production[0].content, GR_VOID_NON_TERMINAL);
	tmp_rule->production[0].typeId = INDEX_MAX;
	tmp_rule->production[0].qnameId.uriId = URI_MAX;
	tmp_rule->production[0].qnameId.lnId = LN_MAX;
	tmp_rule->pCount = 1;
	tmp_rule->meta = 0;
	tmp_rule->prodDim = DEFAULT_PROD_ARRAY_DIM;
	/* More part 1 productions get added later... */

	return EXIP_OK;
}

errorCode insertZeroProduction(DynGrammarRule* rule, EventType eventType, SmallIndex nonTermID, QNameID* qnameId, boolean hasSecondLevelProd)
{
	if(rule->pCount == rule->prodDim) // The dynamic array rule->production needs to be resized
	{
		void* ptr = EXIP_REALLOC(rule->production, sizeof(Production)*(rule->prodDim + DEFAULT_PROD_ARRAY_DIM));
		if(ptr == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		rule->production = ptr;
		rule->prodDim += DEFAULT_PROD_ARRAY_DIM;
	}

	SET_PROD_EXI_EVENT(rule->production[rule->pCount].content, eventType);
	SET_PROD_NON_TERM(rule->production[rule->pCount].content, nonTermID);
	rule->production[rule->pCount].typeId = INDEX_MAX;
	rule->production[rule->pCount].qnameId = *qnameId;

	rule->pCount += 1;
	return EXIP_OK;
}
#endif

errorCode pushGrammar(EXIGrammarStack** gStack, QNameID currQNameID, EXIGrammar* grammar)
{
	struct GrammarStackNode* node = (struct GrammarStackNode*)EXIP_MALLOC(sizeof(struct GrammarStackNode));
	if(node == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	node->grammar = grammar;
	node->currNonTermID = GR_START_TAG_CONTENT;
	node->currQNameID = currQNameID;
	node->nextInStack = *gStack;
	*gStack = node;
	return EXIP_OK;
}

void popGrammar(EXIGrammarStack** gStack)
{
	struct GrammarStackNode* node = *gStack;
	if((*gStack) != NULL)
	{
		node = *gStack;
		*gStack = (*gStack)->nextInStack;

		EXIP_MFREE(node);
	}
}

errorCode createFragmentGrammar(EXIPSchema* schema, QNameID* elQnameArr, Index qnameCount)
{
	GrammarRule* tmp_rule;

	schema->docGrammar.count = DEF_FRAG_GRAMMAR_RULE_NUMBER;
	schema->docGrammar.props = 0;
	schema->docGrammar.rule = (GrammarRule*) memManagedAllocate(&schema->memList, sizeof(GrammarRule)*DEF_FRAG_GRAMMAR_RULE_NUMBER);
	if(schema->docGrammar.rule == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	/* Rule for Fragment */
	/* Fragment : SD FragmentContent	0 */
	// IGNORED!

	/* Rule for Fragment content */
	tmp_rule = &schema->docGrammar.rule[GR_FRAGMENT_CONTENT];

	/* Part 1 */
	if(elQnameArr != NULL)   // Creates Schema Informed Grammar
	{
		unsigned int e = 0;
		Index tmp_code1;

		SET_SCHEMA_GR(schema->docGrammar.props);
		tmp_code1 = qnameCount + 2;

		tmp_rule->production = (Production*) memManagedAllocate(&schema->memList, sizeof(Production)*tmp_code1);
		if(tmp_rule->production == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		/*
		 * FragmentContent :
		 *			   	SE (F-0)   FragmentContent	0
		 *				SE (F-1)   FragmentContent	1
		 *				⋮	⋮      ⋮
		 *				SE (F-n−1) FragmentContent  n-1
		 *			//	SE (*)     FragmentContent	n		//  This is created as part of the Build-In grammar further on
 		 *			//	ED		   					n+1		//  This is created as part of the Build-In grammar further on
		 */

		for(e = 0; e < qnameCount; e++)
		{
			SET_PROD_EXI_EVENT(tmp_rule->production[qnameCount - e].content, EVENT_SE_QNAME);
			SET_PROD_NON_TERM(tmp_rule->production[qnameCount - e].content, GR_FRAGMENT_CONTENT);
			tmp_rule->production[qnameCount - e].typeId = GET_LN_URI_QNAME(schema->uriTable, elQnameArr[e]).elemGrammar;
			tmp_rule->production[qnameCount - e].qnameId = elQnameArr[e];
		}
		tmp_rule->pCount = tmp_code1;
	}
	else
	{
		tmp_rule->production = (Production*) memManagedAllocate(&schema->memList, sizeof(Production)*2);
		if(tmp_rule->production == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		/* Productions further on... */
		tmp_rule->pCount = 2;
	}

	/*
	 * FragmentContent :
	 *				SE (*) FragmentContent	0
	 *				ED						1
	 */

	SET_PROD_EXI_EVENT(tmp_rule->production[0].content, EVENT_ED);
	SET_PROD_NON_TERM(tmp_rule->production[0].content, GR_VOID_NON_TERMINAL);
	tmp_rule->production[0].typeId = INDEX_MAX;
	tmp_rule->production[0].qnameId.uriId = URI_MAX;
	tmp_rule->production[0].qnameId.lnId = LN_MAX;

	SET_PROD_EXI_EVENT(tmp_rule->production[1].content, EVENT_SE_ALL);
	SET_PROD_NON_TERM(tmp_rule->production[1].content, GR_FRAGMENT_CONTENT);
	tmp_rule->production[1].typeId = INDEX_MAX;
	tmp_rule->production[1].qnameId.uriId = URI_MAX;
	tmp_rule->production[1].qnameId.lnId = LN_MAX;

	return EXIP_OK;
}

unsigned int getBitsFirstPartCode(EXIStream* strm, GrammarRule* currentRule, SmallIndex currentRuleIndx)
{
	boolean secondLevelExists = FALSE;

	if(IS_BUILT_IN_ELEM(strm->gStack->grammar->props))
	{
		// Built-in element grammar
		// There is always a second level production
		return getBitsNumber(currentRule->pCount);
	}
	else if(IS_DOCUMENT(strm->gStack->grammar->props))
	{
		// Document grammar
		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
			secondLevelExists = TRUE;
		else if(currentRuleIndx == 0 && IS_PRESERVED(strm->header.opts.preserve, PRESERVE_DTD))
			secondLevelExists = TRUE;

		return getBitsNumber(currentRule->pCount - 1 + secondLevelExists);
	}
	else if(IS_FRAGMENT(strm->gStack->grammar->props))
	{
		// Fragment grammar
		if(IS_PRESERVED(strm->header.opts.preserve, PRESERVE_COMMENTS) || IS_PRESERVED(strm->header.opts.preserve, PRESERVE_PIS))
			secondLevelExists = TRUE;
		return getBitsNumber(currentRule->pCount - 1 + secondLevelExists);
	}
	else
	{
		// Schema-informed element/type grammar
		Index prodCount;

		if(strm->context.isNilType == FALSE)
			prodCount = currentRule->pCount;
		else
			prodCount = RULE_GET_AT_COUNT(currentRule->meta) + RULE_CONTAIN_EE(currentRule->meta);

		if(WITH_STRICT(strm->header.opts.enumOpt))
		{
			// Strict mode
			if(strm->context.isNilType == FALSE && currentRuleIndx == 0)
			{
				if(IS_NILLABLE(strm->gStack->grammar->props) || HAS_NAMED_SUB_TYPE_OR_UNION(strm->gStack->grammar->props))
					secondLevelExists = TRUE;
			}

			return getBitsNumber(prodCount - 1 + secondLevelExists);
		}
		else // Non-strict mode
		{
			// There is always a second level production
			return getBitsNumber(prodCount);
		}
	}
}

#if EXIP_DEBUG == ON

static void writeValueTypeString(EXIType exiType)
{
	switch(exiType)
	{
		case VALUE_TYPE_NONE:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[N/A]"));
			break;
		case VALUE_TYPE_STRING:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[str]"));
			break;
		case VALUE_TYPE_FLOAT:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[float]"));
			break;
		case VALUE_TYPE_DECIMAL:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[dec]"));
			break;
		case VALUE_TYPE_DATE_TIME:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[dateTime]"));
			break;
		case VALUE_TYPE_YEAR:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[gYear]"));
			break;
		case VALUE_TYPE_DATE:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[date]"));
			break;
		case VALUE_TYPE_MONTH:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[month]"));
			break;
		case VALUE_TYPE_TIME:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[time]"));
			break;
		case VALUE_TYPE_BOOLEAN:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[bool]"));
			break;
		case VALUE_TYPE_BINARY:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[bin]"));
			break;
		case VALUE_TYPE_LIST:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[list]"));
			break;
		case VALUE_TYPE_QNAME:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[qname]"));
			break;
		case VALUE_TYPE_UNTYPED:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[untyped]"));
			break;
		case VALUE_TYPE_INTEGER:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[int]"));
			break;
		case VALUE_TYPE_SMALL_INTEGER:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[short]"));
			break;
		case VALUE_TYPE_NON_NEGATIVE_INT:
			DEBUG_MSG(INFO, EXIP_DEBUG, ("[uint]"));
			break;
	}
}

errorCode printGrammarRule(SmallIndex nonTermID, GrammarRule* rule, EXIPSchema *schema)
{
	Index j = 0;
	Production* tmpProd;
	EXIType exiType = VALUE_TYPE_NONE;

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\n>RULE\n"));
	DEBUG_MSG(INFO, EXIP_DEBUG, ("NT-%u:", (unsigned int) nonTermID));

	DEBUG_MSG(INFO, EXIP_DEBUG, ("\n"));

	for(j = 0; j < rule->pCount; j++)
	{
		String *localName = NULL;
		tmpProd = &rule->production[rule->pCount - 1 - j];
		DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));

		if(GET_PROD_EXI_EVENT(tmpProd->content) != EVENT_SE_QNAME && tmpProd->typeId != INDEX_MAX)
			exiType = GET_EXI_TYPE(schema->simpleTypeTable.sType[tmpProd->typeId].content);
		else
			exiType = VALUE_TYPE_NONE;

		switch(GET_PROD_EXI_EVENT(tmpProd->content))
		{
			case EVENT_SD:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SD "));
				break;
			case EVENT_ED:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("ED "));
				break;
			case EVENT_SE_QNAME:
			{
				QNameID *qname = &tmpProd->qnameId;
				localName = &(GET_LN_URI_P_QNAME(schema->uriTable, qname).lnStr);
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (qname: %u:%u) ", (unsigned int) qname->uriId, (unsigned int) qname->lnId));
				break;
			}
			case EVENT_SE_URI:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (uri) "));
				break;
			case EVENT_SE_ALL:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SE (*) "));
				break;
			case EVENT_EE:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("EE "));
				break;
			case EVENT_AT_QNAME:
			{
				QNameID *qname = &tmpProd->qnameId;
				localName = &(GET_LN_URI_P_QNAME(schema->uriTable, qname).lnStr);
				DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (qname %u:%u) ", (unsigned int) tmpProd->qnameId.uriId, (unsigned int) tmpProd->qnameId.lnId));
				writeValueTypeString(exiType);
				break;
			}
			case EVENT_AT_URI:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (uri) "));
				break;
			case EVENT_AT_ALL:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("AT (*) "));
				writeValueTypeString(exiType);
				break;
			case EVENT_CH:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("CH "));
				writeValueTypeString(exiType);
				break;
			case EVENT_NS:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("NS "));
				break;
			case EVENT_CM:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("CM "));
				break;
			case EVENT_PI:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("PI "));
				break;
			case EVENT_DT:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("DT "));
				break;
			case EVENT_ER:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("ER "));
				break;
			case EVENT_SC:
				DEBUG_MSG(INFO, EXIP_DEBUG, ("SC "));
				break;
			case EVENT_VOID:
				DEBUG_MSG(INFO, EXIP_DEBUG, (" "));
				break;
			default:
				return EXIP_UNEXPECTED_ERROR;
		}
		DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));
		if(GET_PROD_NON_TERM(tmpProd->content) != GR_VOID_NON_TERMINAL)
		{
			DEBUG_MSG(INFO, EXIP_DEBUG, ("NT-%u", (unsigned int) GET_PROD_NON_TERM(tmpProd->content)));
		}
		DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));

		DEBUG_MSG(INFO, EXIP_DEBUG, ("%u", (unsigned int) j));

		if (localName != NULL)
		{
			DEBUG_MSG(INFO, EXIP_DEBUG, ("\t"));
			printString(localName);
		}
		DEBUG_MSG(INFO, EXIP_DEBUG, ("\n"));
	}
	return EXIP_OK;
}

#endif // EXIP_DEBUG
