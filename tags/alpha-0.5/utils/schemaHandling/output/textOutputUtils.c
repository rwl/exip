/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file textOutputUtils.c
 * @brief Implement utility functions for storing EXIPSchema instances as human readable descriptions
 * @date May 7, 2012
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "schemaOutputUtils.h"
#include "sTables.h"

static void writeValueTypeString(FILE* out, EXIType exiType);

errorCode textGrammarOutput(QNameID qnameid, Index grIndex, EXIGrammar* gr, EXIPSchema* schema, FILE* out)
{
	Index ruleIter, prodIter;
	Production* tmpProd;
	EXIType exiType = VALUE_TYPE_NONE;

	fprintf(out, "Grammar %d [%d:%d] ", (int) grIndex, (int) qnameid.uriId, (int) qnameid.lnId);
	fwrite(schema->uriTable.uri[qnameid.uriId].uriStr.str, sizeof(CharType), schema->uriTable.uri[qnameid.uriId].uriStr.length, out);
	fprintf(out, ":");
	fwrite(GET_LN_URI_QNAME(schema->uriTable, qnameid).lnStr.str, sizeof(CharType), GET_LN_URI_QNAME(schema->uriTable, qnameid).lnStr.length, out);
	fprintf(out, "\n");

	for(ruleIter = 0; ruleIter < gr->count; ruleIter++)
	{
		fprintf(out, "NT-%d: \n", (int) ruleIter);

		for(prodIter = 0; prodIter < gr->rule[ruleIter].pCount; prodIter++)
		{
			tmpProd = &gr->rule[ruleIter].production[gr->rule[ruleIter].pCount - 1 - prodIter];
			if(GET_PROD_EXI_EVENT(tmpProd->content) != EVENT_SE_QNAME && tmpProd->typeId != INDEX_MAX)
				exiType = GET_EXI_TYPE(schema->simpleTypeTable.sType[tmpProd->typeId].content);
			else
				exiType = VALUE_TYPE_NONE;
			switch(GET_PROD_EXI_EVENT(tmpProd->content))
			{
				case EVENT_SD:
					fprintf(out, "\tSD ");
					break;
				case EVENT_ED:
					fprintf(out, "\tED ");
					break;
				case EVENT_SE_QNAME:
					fprintf(out, "\tSE ([%d:%d]", (int) tmpProd->qnameId.uriId, (int) tmpProd->qnameId.lnId);
					fwrite(schema->uriTable.uri[tmpProd->qnameId.uriId].uriStr.str, sizeof(CharType), schema->uriTable.uri[tmpProd->qnameId.uriId].uriStr.length, out);
					fprintf(out, ":");
					fwrite(GET_LN_URI_QNAME(schema->uriTable, tmpProd->qnameId).lnStr.str, sizeof(CharType), GET_LN_URI_QNAME(schema->uriTable, tmpProd->qnameId).lnStr.length, out);
					fprintf(out, ") ");
					break;
				case EVENT_SE_URI:
					fprintf(out, "\tSE (uri) ");
					break;
				case EVENT_SE_ALL:
					fprintf(out, "\tSE (*) ");
					break;
				case EVENT_EE:
					fprintf(out, "\tEE ");
					break;
				case EVENT_AT_QNAME:
					fprintf(out, "\tAT ([%d:%d]", (int) tmpProd->qnameId.uriId, (int) tmpProd->qnameId.lnId);
					fwrite(schema->uriTable.uri[tmpProd->qnameId.uriId].uriStr.str, sizeof(CharType), schema->uriTable.uri[tmpProd->qnameId.uriId].uriStr.length, out);
					fprintf(out, ":");
					fwrite(GET_LN_URI_QNAME(schema->uriTable, tmpProd->qnameId).lnStr.str, sizeof(CharType), GET_LN_URI_QNAME(schema->uriTable, tmpProd->qnameId).lnStr.length, out);
					fprintf(out, ") ");
					writeValueTypeString(out, exiType);
					break;
				case EVENT_AT_URI:
					fprintf(out, "\tAT (uri) ");
					break;
				case EVENT_AT_ALL:
					fprintf(out, "\tAT (*) ");
					writeValueTypeString(out, exiType);
					break;
				case EVENT_CH:
					fprintf(out, "\tCH ");
					writeValueTypeString(out, exiType);
					break;
				case EVENT_NS:
					fprintf(out, "\tNS ");
					break;
				case EVENT_CM:
					fprintf(out, "\tCM ");
					break;
				case EVENT_PI:
					fprintf(out, "\tPI ");
					break;
				case EVENT_DT:
					fprintf(out, "\tDT ");
					break;
				case EVENT_ER:
					fprintf(out, "\tER ");
					break;
				case EVENT_SC:
					fprintf(out, "\tSC ");
					break;
				case EVENT_VOID:
					fprintf(out, " ");
					break;
				default:
					return UNEXPECTED_ERROR;
			}
			if(GET_PROD_NON_TERM(tmpProd->content) != GR_VOID_NON_TERMINAL)
			{
				fprintf(out, "\tNT-%u\t", (unsigned int) GET_PROD_NON_TERM(tmpProd->content));
			}
			fprintf(out, "%d\n", (int) prodIter);
		}
		fprintf(out, "\n");
	}

	return ERR_OK;
}

static void writeValueTypeString(FILE* out, EXIType exiType)
{
	switch(exiType)
	{
		case VALUE_TYPE_NONE:
			fprintf(out, "[N/A] ");
			break;
		case VALUE_TYPE_STRING:
			fprintf(out, "[str] ");
			break;
		case VALUE_TYPE_FLOAT:
			fprintf(out, "[float] ");
			break;
		case VALUE_TYPE_DECIMAL:
			fprintf(out, "[dec] ");
			break;
		case VALUE_TYPE_DATE_TIME:
			fprintf(out, "[date] ");
			break;
		case VALUE_TYPE_BOOLEAN:
			fprintf(out, "[bool] ");
			break;
		case VALUE_TYPE_BINARY:
			fprintf(out, "[bin] ");
			break;
		case VALUE_TYPE_LIST:
			fprintf(out, "[list] ");
			break;
		case VALUE_TYPE_QNAME:
			fprintf(out, "[qname] ");
			break;
		case VALUE_TYPE_UNTYPED:
			fprintf(out, "[untyped] ");
			break;
		case VALUE_TYPE_INTEGER:
			fprintf(out, "[int] ");
			break;
		case VALUE_TYPE_SMALL_INTEGER:
			fprintf(out, "[short] ");
			break;
		case VALUE_TYPE_NON_NEGATIVE_INT:
			fprintf(out, "[uint] ");
			break;
	}
}

errorCode recursiveTextGrammarOutput(QNameID qnameid, Index grIndex, EXIGrammar* gr, EXIPSchema* schema, FILE* out)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Index r, p;

	tmp_err_code = textGrammarOutput(qnameid, grIndex, gr, schema, out);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(r = 0; r < gr->count; r++)
	{
		for(p = 0; p < gr->rule[r].pCount; p++)
		{
			if(GET_PROD_EXI_EVENT(gr->rule[r].production[p].content) == EVENT_SE_QNAME &&
					gr->rule[r].production[p].typeId != INDEX_MAX)
			{
				tmp_err_code = recursiveTextGrammarOutput(gr->rule[r].production[p].qnameId, gr->rule[r].production[p].typeId, &schema->grammarTable.grammar[gr->rule[r].production[p].typeId], schema, out);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
	}

	return ERR_OK;
}

