/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file staticOutputUtils.c
 * @brief Implement utility functions for storing EXIPSchema instances as static code
 * @date May 7, 2012
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "schemaOutputUtils.h"
#include "hashtable.h"

static void setProdStrings(IndexStrings *indexStrings, Production *prod)
{
	char *indexMaxStr = "INDEX_MAX";
	char *smallIndexMaxStr = "SMALL_INDEX_MAX";

	if (prod->typeId == INDEX_MAX)
	{
		strcpy(indexStrings->buf1, indexMaxStr);
	}
	else
	{
		sprintf(indexStrings->buf1, "%d", (int) prod->typeId);
	}
	if (prod->qnameId.uriId == SMALL_INDEX_MAX)
	{
		strcpy(indexStrings->buf2, smallIndexMaxStr);
	}
	else
	{
		sprintf(indexStrings->buf2, "%d", (int) prod->qnameId.uriId);
	}
	if (prod->qnameId.lnId == INDEX_MAX)
	{
		strcpy(indexStrings->buf3, indexMaxStr);
	}
	else
	{
		sprintf(indexStrings->buf3, "%d", (int) prod->qnameId.lnId);
	}
	if (prod->nonTermID == SMALL_INDEX_MAX)
	{
		strcpy(indexStrings->buf4, smallIndexMaxStr);
	}
	else
	{
		sprintf(indexStrings->buf4, "%d", (int) prod->nonTermID);
	}
}

void staticStringDefOutput(String* str, char* varName, FILE* out)
{
	Index charIter, charMax;
	char displayStr[VAR_BUFFER_MAX_LENGTH];

	charMax = str->length;
	if(charMax > 0)
	{
		fprintf(out, "CONST CharType %s[] = {", varName);
		for(charIter = 0; charIter < charMax; charIter++)
		{
			fprintf(out, "0x%x", str->str[charIter]);
			if(charIter < charMax - 1)
				fprintf(out, ", ");
		}
		strncpy(displayStr, str->str, str->length);
		displayStr[str->length] = '\0';
		fprintf(out, "}; /* %s */\n", displayStr);
	}
}

void staticStringTblDefsOutput(UriTable* uriTbl, char* prefix, FILE* out)
{
	Index uriIter, pfxIter, lnIter;
	char varName[VAR_BUFFER_MAX_LENGTH];

	fprintf(out, "/** START_STRINGS_DEFINITONS */\n\n");

	for(uriIter = 0; uriIter < uriTbl->count; uriIter++)
	{
		// Printing of a uri string
		sprintf(varName, "%sURI_%d", prefix, (int) uriIter);
		staticStringDefOutput(&uriTbl->uri[uriIter].uriStr, varName, out);

		// Printing of a pfx strings if any
		if(uriTbl->uri[uriIter].pfxTable != NULL)
		{
			for(pfxIter = 0; pfxIter < uriTbl->uri[uriIter].pfxTable->count; pfxIter++)
			{
				sprintf(varName, "%sPFX_%d_%d", prefix, (int) uriIter, (int) pfxIter);
				staticStringDefOutput(&uriTbl->uri[uriIter].pfxTable->pfxStr[pfxIter], varName, out);
			}
		}

		// Printing of all local names for that uri
		for(lnIter = 0; lnIter < uriTbl->uri[uriIter].lnTable.count; lnIter++)
		{
			sprintf(varName, "%sLN_%d_%d", prefix, (int) uriIter, (int) lnIter);
			staticStringDefOutput(&uriTbl->uri[uriIter].lnTable.ln[lnIter].lnStr, varName, out);
		}
	}

	fprintf(out, "\n/** END_STRINGS_DEFINITONS */\n\n");
}

void staticProductionsOutput(EXIGrammar* gr, char* prefix, Index grId, FILE* out)
{
	Index ruleIter;
	char varName[VAR_BUFFER_MAX_LENGTH];
	Index prodIter;
	IndexStrings indexStrings;

	for(ruleIter = 0; ruleIter < gr->count; ruleIter++)
	{
		if (gr->rule[ruleIter].p1Count)
		{
			// Printing of the Production variable string
			sprintf(varName, "%sprod_%d_%d_part%d", prefix, (int) grId, (int) ruleIter, 1);

			fprintf(out, "static CONST Production %s[%d] =\n{\n", varName, (int) gr->rule[ruleIter].p1Count);

			for(prodIter = 0; prodIter < gr->rule[ruleIter].p1Count; prodIter++)
			{
				setProdStrings(&indexStrings, &gr->rule[ruleIter].prod1[prodIter]);
				fprintf(out,
						"    {\n        %d, %s,\n        {%s, %s},\n        %s\n    }%s",
						gr->rule[ruleIter].prod1[prodIter].eventType,
						indexStrings.buf1,
						indexStrings.buf2,
						indexStrings.buf3,
						indexStrings.buf4,
						prodIter==(gr->rule[ruleIter].p1Count - 1) ? "\n};\n\n" : ",\n");
			}
		}

		if (gr->rule[ruleIter].p2Count + gr->rule[ruleIter].p3Count)
		{
			// Printing of the Production variable string
			sprintf(varName, "%sprod_%d_%d_part%d", prefix, (int) grId, (int) ruleIter, 23);

			fprintf(out, "static CONST Production %s[%d] =\n{\n",
					varName, (int) gr->rule[ruleIter].p2Count + gr->rule[ruleIter].p3Count);

			for(prodIter = 0; prodIter < gr->rule[ruleIter].p2Count + gr->rule[ruleIter].p3Count; prodIter++)
			{
				setProdStrings(&indexStrings, &gr->rule[ruleIter].prod23[prodIter]);
				fprintf(out,
						"    {\n        %d, %s,\n        {%s, %s},\n        %s\n    }%s",
						gr->rule[ruleIter].prod23[prodIter].eventType,
						indexStrings.buf1,
						indexStrings.buf2,
						indexStrings.buf3,
						indexStrings.buf4,
						prodIter==(gr->rule[ruleIter].p2Count + gr->rule[ruleIter].p3Count - 1) ? "\n};\n\n" : ",\n");
			}
		}
	}
}

void staticRulesOutput(EXIGrammar* gr, char* prefix, Index grId, FILE* out)
{
	Index ruleIter;

	// #DOCUMENT# IMPORTANT! tmpGrammar->count + (mask_specified == FALSE) because It must be assured that the schema informed grammars have one empty slot for the rule:  Element i, content2
	fprintf(out,
		    "static CONST GrammarRule %srule_%d[%d] =\n{",
			prefix,
			(int) grId,
			(int) gr->count);
//			(int) (gr->count + (mask_specified == FALSE)));

	for(ruleIter = 0; ruleIter < gr->count; ruleIter++)
	{
		fprintf(out, "\n    {");
		if (gr->rule[ruleIter].p1Count > 0)
		{
			fprintf(out,
			        "%sprod_%d_%d_part%d, ",
					prefix,
					(int) grId,
					(int) ruleIter,
					(int) 1);
		}
		else
			fprintf(out, "NULL, ");

		if (gr->rule[ruleIter].p2Count + gr->rule[ruleIter].p3Count > 0)
		{
			fprintf(out,
			        "%sprod_%d_%d_part%d, ",
					prefix,
					(int) grId,
					(int) ruleIter,
					(int) 23);
		}
		else
			fprintf(out, "NULL, ");

		fprintf(out, "%d, ", gr->rule[ruleIter].p1Count);
		fprintf(out, "%d, ", gr->rule[ruleIter].p2Count);
		fprintf(out, "%d, ", gr->rule[ruleIter].p3Count);
		fprintf(out, "%d}%s", gr->rule[ruleIter].bits1, ruleIter != (gr->count-1)?",":"");

	}

	fprintf(out, "\n};\n\n");
}

void staticDocGrammarOutput(EXIGrammar* docGr, char* prefix, FILE* out)
{
	char varName[VAR_BUFFER_MAX_LENGTH];
	Index prodIter;
	IndexStrings indexStrings;

	// Printing of the Production variable string
	sprintf(varName, "%sprod_doc_content", prefix);

	/* Build the document grammar, DocContent productions */

	fprintf(out, "static CONST Production %s[%d] =\n{\n", varName, (int) docGr->rule[1].p1Count);

	for(prodIter = 0; prodIter < docGr->rule[1].p1Count; prodIter++)
	{
		setProdStrings(&indexStrings, &docGr->rule[1].prod1[prodIter]);
		fprintf(out,
				"    {\n        %d, %s,\n        {%s, %s},\n        %s\n    }%s",
				docGr->rule[1].prod1[prodIter].eventType,
				indexStrings.buf1,
				indexStrings.buf2,
				indexStrings.buf3,
				indexStrings.buf4,
				prodIter==(docGr->rule[1].p1Count - 1) ? "\n};\n\n" : ",\n");
	}

	// TODO: When mask_specified and DT, CM or PI are preserved, the docGrammarRule must contained these productions

	/* Build the document grammar rules */
	fprintf(out, "static CONST GrammarRule %sdocGrammarRule[3] =\n{\n", prefix);
	fprintf(out, "    {static_prod_start_doc, NULL, 1, 0, 0, 0},\n\
	{%s, NULL, %d, 0, 0, %d},\n\
    {static_prod_doc_end, NULL, 1, 0, 0, 0}\n};\n\n", varName, (int) docGr->rule[1].p1Count, docGr->rule[1].bits1);
}

void staticPrefixOutput(PfxTable* pfxTbl, char* prefix, Index uriId, FILE* out)
{
	Index pfxIter;
	if(pfxTbl != NULL)
	{
		fprintf(out, "static CONST PfxTable %spfxTable_%d =\n{\n    %d,\n    {\n", prefix, (int) uriId, (int) pfxTbl->count);

		for(pfxIter = 0; pfxIter < pfxTbl->count; pfxIter++)
		{
			if(pfxTbl->pfxStr[pfxIter].length > 0)
				fprintf(out, "        {%sPFX_%d_%d, %d},\n", prefix, (int) uriId, (int) pfxIter, (int) pfxTbl->pfxStr[pfxIter].length);
			else
				fprintf(out, "        {NULL, 0},\n");
		}
		for(; pfxIter < MAXIMUM_NUMBER_OF_PREFIXES_PER_URI; pfxIter++)
		{
			fprintf(out, "        {NULL, 0}%s", pfxIter==MAXIMUM_NUMBER_OF_PREFIXES_PER_URI-1 ? "\n    }\n};\n\n" : ",\n");
		}
	}
}

void staticLnEntriesOutput(LnTable* lnTbl, char* prefix, Index uriId, FILE* out)
{
	Index lnIter;
	char elemGrammar[20];
	char typeGrammar[20];

	if(lnTbl->count > 0)
	{
		fprintf(out, "static CONST LnEntry %sLnEntry_%d[%d] =\n{\n", prefix, (int) uriId, (int) lnTbl->count);

		for(lnIter = 0; lnIter < lnTbl->count; lnIter++)
		{
			if(lnTbl->ln[lnIter].elemGrammar == INDEX_MAX)
				strcpy(elemGrammar, "INDEX_MAX");
			else
				sprintf(elemGrammar, "%d", (int) lnTbl->ln[lnIter].elemGrammar);

			if(lnTbl->ln[lnIter].typeGrammar == INDEX_MAX)
				strcpy(typeGrammar, "INDEX_MAX");
			else
				sprintf(typeGrammar, "%d", (int) lnTbl->ln[lnIter].typeGrammar);

			fprintf(out, "    {\n        {{sizeof(VxEntry), 0, 0}, NULL, 0},\n");
			if(lnTbl->ln[lnIter].lnStr.length > 0)
				fprintf(out, "        {%sLN_%d_%d, %d},\n        %s, %s\n", prefix, (int) uriId, (int) lnIter, (int) lnTbl->ln[lnIter].lnStr.length, elemGrammar, typeGrammar);
			else
				fprintf(out, "        {NULL, 0},\n        %s, %s\n", elemGrammar, typeGrammar);
			fprintf(out, "%s", lnIter==(lnTbl->count-1)?"    }\n};\n\n":"    },\n");
		}
	} /* END if(lnTableSize > 0) */
}

void staticUriTableOutput(UriTable* uriTbl, char* prefix, FILE* out)
{
	Index uriIter;
	fprintf(out, "static CONST UriEntry %suriEntry[%d] =\n{\n", prefix, (int) uriTbl->count);

	for(uriIter = 0; uriIter < uriTbl->count; uriIter++)
	{
		if(uriTbl->uri[uriIter].lnTable.count > 0)
        {
			fprintf(out,
                    "    {\n        {{sizeof(LnEntry), %d, %d}, %sLnEntry_%d, %d},\n",
                    (int) uriTbl->uri[uriIter].lnTable.count,
                    (int) uriTbl->uri[uriIter].lnTable.count,
                    prefix,
                    (int) uriIter,
                    (int) uriTbl->uri[uriIter].lnTable.count);
        }
		else
        {
			fprintf(out, "    {\n        {{sizeof(LnEntry), 0, 0}, NULL, 0},\n");
        }

		if(uriTbl->uri[uriIter].pfxTable != NULL)
		{
			fprintf(out, "        &%spfxTable_%d,\n", prefix, (int) uriIter);
		}
		else
		{
			fprintf(out, "        NULL,\n");
		}

		if(uriTbl->uri[uriIter].uriStr.length > 0)
			fprintf(out, "        {%sURI_%d, %d}%s", prefix, (int) uriIter, (int) uriTbl->uri[uriIter].uriStr.length,
                uriIter==(uriTbl->count-1)?"\n    }\n};\n\n":"\n    },\n");
		else
			fprintf(out, "        {NULL, 0}%s", uriIter==(uriTbl->count-1)?"\n    }\n};\n\n":"\n    },\n");
	}
}

void staticEnumTableOutput(EXIPSchema* schema, char* prefix, FILE* out)
{
	EnumDefinition* tmpDef;
	char varName[VAR_BUFFER_MAX_LENGTH];
	Index i, j;

	if(schema->enumTable.count == 0)
		return;

	for(i = 0; i < schema->enumTable.count; i++)
	{
		tmpDef = &schema->enumTable.enumDef[i];
		switch(schema->simpleTypeTable.sType[tmpDef->typeId].exiType)
		{
			case VALUE_TYPE_STRING:
			{
				String* tmpStr;
				for(j = 0; j < tmpDef->count; j++)
				{
					tmpStr = &((String*) tmpDef->values)[j];
					sprintf(varName, "%sENUM_%d_%d", prefix, (int) i, (int) j);
					staticStringDefOutput(tmpStr, varName, out);
				}
				fprintf(out, "\nstatic CONST String %senumValues_%d[%d] = { \n", prefix, (int) i, (int) tmpDef->count);
				for(j = 0; j < tmpDef->count; j++)
				{
					tmpStr = &((String*) tmpDef->values)[j];
					if(tmpStr->str != NULL)
						fprintf(out, "   {%sENUM_%d_%d, %d}", prefix, (int) i, (int) j, (int) tmpStr->length);
					else
						fprintf(out, "   {NULL, 0}");

					if(j < tmpDef->count - 1)
						fprintf(out, ",\n");
					else
						fprintf(out, "\n};\n\n");
				}
			} break;
			case VALUE_TYPE_BOOLEAN:
				// NOT_IMPLEMENTED
				break;
			case VALUE_TYPE_DATE_TIME:
				// NOT_IMPLEMENTED
				break;
			case VALUE_TYPE_DECIMAL:
				// NOT_IMPLEMENTED
				break;
			case VALUE_TYPE_FLOAT:
				// NOT_IMPLEMENTED
				break;
			case VALUE_TYPE_INTEGER:
				// NOT_IMPLEMENTED
				break;
			case VALUE_TYPE_SMALL_INTEGER:
				// NOT_IMPLEMENTED
				break;
		}
	}

	fprintf(out, "static CONST EnumDefinition %senumTable[%d] = { \n", prefix, (int) schema->enumTable.count);
	for(i = 0; i < schema->enumTable.count; i++)
	{
		tmpDef = &schema->enumTable.enumDef[i];
		fprintf(out, "   {%d, %senumValues_%d, %d}", (int) tmpDef->typeId, prefix, (int) i, (int) tmpDef->count);

		if(i < schema->enumTable.count - 1)
			fprintf(out, ",\n");
		else
			fprintf(out, "\n};\n\n");
	}
}
