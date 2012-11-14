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
	unsigned char partIter;
	Production* tmpProdArr;
	Production* tmpProd;
	EXIType exiType = VALUE_TYPE_NONE;
	Index tmpCount;

	fprintf(out, "Grammar %d [%d:%d] ", (int) grIndex, (int) qnameid.uriId, (int) qnameid.lnId);
	fwrite(schema->uriTable.uri[qnameid.uriId].uriStr.str, sizeof(CharType), schema->uriTable.uri[qnameid.uriId].uriStr.length, out);
	fprintf(out, ":");
	fwrite(GET_LN_URI_QNAME(schema->uriTable, qnameid).lnStr.str, sizeof(CharType), GET_LN_URI_QNAME(schema->uriTable, qnameid).lnStr.length, out);
	fprintf(out, "\n");

	for(ruleIter = 0; ruleIter < gr->count; ruleIter++)
	{
		fprintf(out, "NT-%d: \n", (int) ruleIter);
		for(partIter = 0; partIter < 3; partIter++)
		{
			if(partIter == 0)
			{
				tmpProdArr = gr->rule[ruleIter].prod1;
				tmpCount = gr->rule[ruleIter].p1Count;
			}
			else if(partIter == 1)
			{
				tmpProdArr = gr->rule[ruleIter].prod23;
				tmpCount = gr->rule[ruleIter].p2Count;
			}
			else if(partIter == 2)
			{
				tmpCount = gr->rule[ruleIter].p3Count;
				if(tmpCount)
					tmpProdArr = gr->rule[ruleIter].prod23 + gr->rule[ruleIter].p2Count;
			}

			for(prodIter = 0; prodIter < tmpCount; prodIter++)
			{
				tmpProd = &tmpProdArr[tmpCount - 1 - prodIter];
				if(tmpProd->eventType != EVENT_SE_QNAME && tmpProd->typeId != INDEX_MAX)
					exiType = schema->simpleTypeTable.sType[tmpProd->typeId].exiType;
				else
					exiType = VALUE_TYPE_NONE;
				switch(tmpProd->eventType)
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
				if(tmpProd->nonTermID != GR_VOID_NON_TERMINAL)
				{
					fprintf(out, "\tNT-%u\t", (unsigned int) tmpProd->nonTermID);
				}
				if(partIter > 0)
				{
					fprintf(out, "%d.", (int) gr->rule[ruleIter].p1Count);
					if(partIter > 1)
					{
						fprintf(out, "%d.", (int) gr->rule[ruleIter].p2Count);
					}
				}
				fprintf(out, "%d\n", (int) prodIter);
			}
		}
		fprintf(out, "\n");
	}

	return ERR_OK;
}

static void writeValueTypeString(FILE* out, EXIType exiType)
{
	switch(exiType)
	{
		case 1:
			fprintf(out, "[N/A] ");
			break;
		case 2:
			fprintf(out, "[str] ");
			break;
		case 3:
			fprintf(out, "[float] ");
			break;
		case 4:
			fprintf(out, "[dec] ");
			break;
		case 5:
			fprintf(out, "[date] ");
			break;
		case 6:
			fprintf(out, "[bool] ");
			break;
		case 7:
			fprintf(out, "[bin] ");
			break;
		case 8:
			fprintf(out, "[list] ");
			break;
		case 9:
			fprintf(out, "[qname] ");
			break;
		case 10:
			fprintf(out, "[untyped] ");
			break;
		case 20:
			fprintf(out, "[int] ");
			break;
		case 21:
			fprintf(out, "[short] ");
			break;
		case 22:
			fprintf(out, "[uint] ");
			break;
	}
}
