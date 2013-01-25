/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file dynOutputUtils.c
 * @brief Implement utility functions for storing EXIPSchema instances as dynamic code
 * @date May 7, 2012
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "schemaOutputUtils.h"
#include "hashtable.h"
#include "stringManipulate.h"

#define MAX_GRAMMARS_COUNT 5000

//TODO: split this code into separate functions
errorCode dynExipSchemaOutput(EXIPSchema* schema, char* prefix, unsigned char mask_specified,
		unsigned char mask_strict, unsigned char mask_sc, unsigned char mask_preserve, FILE* out)
{

	// TODO: must be fixed!
#if 0
	Index uriIter, lnIter, pfxIter, ruleIter, partIter, prodIter;
	EXIGrammar* tmpGrammar;
	uintptr_t grammarPointer = 0;
	String hashKey;
	Index typeGrammarID;
	struct hashtable *typeGrammarsHash;
	unsigned int tgCount = 0;
	QNameID typeGrammars[MAX_GRAMMARS_COUNT];

	typeGrammarsHash = create_hashtable(1000, djbHash, stringEqual);

	// TODO: needs to be fixed

	fprintf(out, "errorCode get_%sSchema(EXIPSchema* schema);\n\n", prefix);
	fprintf(out, "errorCode get_%sSchema(EXIPSchema* schema)\n{\n\t errorCode tmp_err_code = UNEXPECTED_ERROR;\n\t\n\t", prefix);
	fprintf(out, "if(schema == NULL)\n\t return NULL_POINTER_REF;\n\t");

	fprintf(out, "tmp_err_code = initAllocList(&schema->memList);\n\t");
	fprintf(out, "if(tmp_err_code != ERR_OK)\n\t return tmp_err_code;\n\t");

	for(uriIter = 0; uriIter < schema->uriTable.count; uriIter++)
	{
		if(schema->uriTable.uri[uriIter].pfxTable != NULL)
		{
			fprintf(out, "PfxTable* pfxTable_%d = memManagedAllocate(&schema->memList, sizeof(PfxTable));\n\t", uriIter);
			fprintf(out, "if(pTable_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", uriIter);
			fprintf(out, "pTable_%d->rowCount = %d;\n\t", uriIter, schema->uriTable.uri[uriIter].pfxTable->count);

			for(pfxIter = 0; pfxIter < schema->uriTable.uri[uriIter].pfxTable->count; pfxIter++)
			{
				fprintf(out, "tmp_err_code += asciiToString(\"");
				fwrite(schema->uriTable.uri[uriIter].pfxTable->pfxStr[pfxIter].str, sizeof(CharType), schema->uriTable.uri[uriIter].pfxTable->pfxStr[pfxIter].length, out);
				fprintf(out, "\", &pTable_%d->string_val[%d], &schema->memList, TRUE);\n\t", uriIter, pfxIter);
			}
			for(; pfxIter < MAXIMUM_NUMBER_OF_PREFIXES_PER_URI; pfxIter++)
			{
				fprintf(out, "getEmptyString(&pTable_%d->string_val[%d]);\n\t", uriIter, pfxIter);
			}
			fprintf(out, "if(tmp_err_code != ERR_OK)\n\t return UNEXPECTED_ERROR;\n\t");
		}

		for(lnIter = 0; lnIter < schema->uriTable.uri[uriIter].lnTable.count; lnIter++)
		{
			tmpGrammar = schema->uriTable.uri[uriIter].lnTable.ln[lnIter].typeGrammar;
			grammarPointer = (uintptr_t) tmpGrammar;
			hashKey.str = (CharType*) &grammarPointer;
			hashKey.length = sizeof(grammarPointer)/sizeof(CharType);
			typeGrammarID = hashtable_search(typeGrammarsHash, &hashKey);
			if(tmpGrammar != NULL && typeGrammarID == INDEX_MAX)
			{
				if(mask_specified == TRUE)
				{
					if(ERR_OK != addUndeclaredProductions(&schema->memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, &schema->simpleTypeTable))
					{
						printf("\n ERROR: OUT_SRC_DYN output format!");
						exit(1);
					}
				}

				for(ruleIter = 0; ruleIter < tmpGrammar->count; ruleIter++)
				{
					for(partIter = 0; partIter < 3; partIter++)
					{
						if(tmpGrammar->rule[ruleIter].part[partIter].count > 0)
						{
							fprintf(out, "Production* prod_%d_%d_%d_part%d = memManagedAllocate(&schema->memList, %d * sizeof(Production));\n\t", uriIter, lnIter, ruleIter, partIter, tmpGrammar->rule[ruleIter].part[partIter].count);
							fprintf(out, "if(prod_%d_%d_%d_part%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", uriIter, lnIter, ruleIter, partIter);

							for(prodIter = 0; prodIter < tmpGrammar->rule[ruleIter].part[partIter].count; prodIter++)
							{
								fprintf(out, "prod_%d_%d_%d_part%d[%d].evnt.eventType = %d;\n\t", uriIter, lnIter, ruleIter, partIter, prodIter, tmpGrammar->rule[ruleIter].part[partIter].prod[prodIter].evnt.eventType);
								fprintf(out, "prod_%d_%d_%d_part%d[%d].evnt.valueType.exiType = %d;\n\t", uriIter, lnIter, ruleIter, partIter, prodIter, tmpGrammar->rule[ruleIter].part[partIter].prod[prodIter].evnt.valueType.exiType);
								fprintf(out, "prod_%d_%d_%d_part%d[%d].evnt.valueType.simpleTypeId = %d;\n\t", uriIter, lnIter, ruleIter, partIter, prodIter, tmpGrammar->rule[ruleIter].part[partIter].prod[prodIter].evnt.valueType.simpleTypeId);
								fprintf(out, "prod_%d_%d_%d_part%d[%d].nonTermID = %d;\n\t", uriIter, lnIter, ruleIter, partIter, prodIter, tmpGrammar->rule[ruleIter].part[partIter].prod[prodIter].nonTermID);
								fprintf(out, "prod_%d_%d_%d_part%d[%d].qname.uriId = %d;\n\t", uriIter, lnIter, ruleIter, partIter, prodIter, tmpGrammar->rule[ruleIter].part[partIter].prod[prodIter].qname.uriId);
								fprintf(out, "prod_%d_%d_%d_part%d[%d].qname.lnId = %d;\n\t", uriIter, lnIter, ruleIter, partIter, prodIter, tmpGrammar->rule[ruleIter].part[partIter].prod[prodIter].qname.lnId);

							}
						}
					}
				}
				// #DOCUMENT# IMPORTANT! tmpGrammar->count + (mask_specified == FALSE) because It must be assured that the schema informed grammars have one empty slot for the rule:  Element i, content2
				fprintf(out, "GrammarRule* rule_%d_%d = memManagedAllocate(&schema->memList, %d * sizeof(GrammarRule));\n\t", uriIter, lnIter, tmpGrammar->count + (mask_specified == FALSE));
				fprintf(out, "if(rule_%d_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", uriIter, lnIter);

				for(ruleIter = 0; ruleIter < tmpGrammar->count; ruleIter++)
				{
					for(partIter = 0; partIter < 3; partIter++)
					{
						if(tmpGrammar->rule[ruleIter].part[partIter].count > 0)
							fprintf(out, "rule_%d_%d[%d].part[%d] = prod_%d_%d_%d_part%d;\n\t", uriIter, lnIter, ruleIter, partIter, uriIter, lnIter, ruleIter, partIter);
						else
							fprintf(out, "rule_%d_%d[%d].part[%d].prod = NULL;\n\t", uriIter, lnIter, ruleIter, partIter);

						fprintf(out, "rule_%d_%d[%d].part[%d].count = %d;\n\t", uriIter, lnIter, ruleIter, partIter, tmpGrammar->rule[ruleIter].part[partIter].count);
						fprintf(out, "rule_%d_%d[%d].part[%d].bits = %d;\n\t", uriIter, lnIter, ruleIter, partIter, tmpGrammar->rule[ruleIter].part[partIter].bits);

					}
				}

				fprintf(out, "EXIGrammar* grammar_%d_%d = memManagedAllocate(&schema->memList, sizeof(EXIGrammar));\n\t", uriIter, lnIter);
				fprintf(out, "if(grammar_%d_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", uriIter, lnIter);

				fprintf(out, "grammar_%d_%d->contentIndex = %d;\n\t", uriIter, lnIter, tmpGrammar->contentIndex);
				fprintf(out, "grammar_%d_%d->props = 0x%02x;\n\t", uriIter, lnIter, tmpGrammar->props);
				fprintf(out, "grammar_%d_%d->count = %d;\n\t", uriIter, lnIter, tmpGrammar->count);
				fprintf(out, "grammar_%d_%d->rule = rule_%d_%d;\n\t", uriIter, lnIter, uriIter, lnIter);

				if(tgCount >= MAX_GRAMMARS_COUNT)
				{
					printf("\n ERROR: MAX_GRAMMARS_COUNT reached!");
					exit(1);
				}

				if(ERR_OK != hashtable_insert(typeGrammarsHash, &hashKey, tgCount))
				{
					printf("\n ERROR: OUT_SRC_DYN output format!");
					exit(1);
				}
				typeGrammars[tgCount].uriId = uriIter;
				typeGrammars[tgCount].lnId = lnIter;
				tgCount++;
			}
		}
		fprintf(out, "struct LocalNamesRow* LNrows_%d = memManagedAllocate(&schema->memList, %d * sizeof(struct LocalNamesRow));\n\t", uriIter, schema->uriTable.uri[uriIter].lnTable.count);
		fprintf(out, "if(LNrows_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", uriIter);

		for(lnIter = 0; lnIter < schema->uriTable.uri[uriIter].lnTable.count; lnIter++)
		{
			tmpGrammar = schema->uriTable.uri[uriIter].lnTable.ln[lnIter].typeGrammar;
			grammarPointer = (uintptr_t) tmpGrammar;
			hashKey.str = (CharType*) &grammarPointer;
			hashKey.length = sizeof(grammarPointer)/sizeof(CharType);
			typeGrammarID = hashtable_search(typeGrammarsHash, &hashKey);

			if(tmpGrammar != NULL)
				fprintf(out, "LNrows_%d[%d].typeGrammar = grammar_%d_%d;\n\t", uriIter, lnIter, typeGrammars[typeGrammarID].uriId, typeGrammars[typeGrammarID].lnId);
			else
				fprintf(out, "LNrows_%d[%d].typeGrammar = NULL;\n\t", uriIter, lnIter);

			fprintf(out, "LNrows_%d[%d].string_val.str = \"", uriIter, lnIter);
			fwrite(schema->uriTable.uri[uriIter].lnTable.ln[lnIter].lnStr.str, sizeof(CharType), schema->uriTable.uri[uriIter].lnTable.ln[lnIter].lnStr.length, out);
			fprintf(out, "\";\n\t");
			fprintf(out, "LNrows_%d[%d].string_val.length = %d;\n\t", uriIter, lnIter, schema->uriTable.uri[uriIter].lnTable.ln[lnIter].lnStr.length);
			fprintf(out, "LNrows_%d[%d].vCrossTable = NULL;\n\t", uriIter, lnIter);

		}

		fprintf(out, "LocalNamesTable* lTable_%d = memManagedAllocate(&schema->memList, sizeof(LocalNamesTable));\n\t", uriIter);
		fprintf(out, "if(lTable_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", uriIter);

		fprintf(out, "lTable_%d->arrayDimension = %d;\n\t", uriIter, schema->uriTable.uri[uriIter].lnTable.count);
		fprintf(out, "lTable_%d->rowCount = %d;\n\t", uriIter, schema->uriTable.uri[uriIter].lnTable.count);
		fprintf(out, "lTable_%d->rows = LNrows_%d;\n\t", uriIter, uriIter);
		fprintf(out, "lTable_%d->memPair.memBlock = NULL;\n\t", uriIter); // TO BE fixed!
		fprintf(out, "lTable_%d->memPair.allocIndx = 0;\n\t", uriIter); // TO BE fixed!
	}

	fprintf(out, "struct URIRow* uriRows = memManagedAllocate(&schema->memList, %d * sizeof(struct URIRow));\n\t", schema->uriTable.count);

	fprintf(out, "if(uriRows == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t");


	for(uriIter = 0; uriIter < schema->uriTable.count; uriIter++)
	{
		if(schema->uriTable.uri[uriIter].pfxTable != NULL)
			fprintf(out, "uriRows[%d].pTable = pTable_%d;\n\t", uriIter, uriIter);
		else
			fprintf(out, "uriRows[%d].pTable = NULL;\n\t", uriIter);
		fprintf(out, "uriRows[%d].lTable = lTable_%d;\n\t", uriIter, uriIter);
		fprintf(out, "uriRows[%d].string_val.str = \"", uriIter);
		fwrite(schema->uriTable.uri[uriIter].uriStr.str, sizeof(CharType), schema->uriTable.uri[uriIter].uriStr.length, out);
		fprintf(out, "\";\n\t");
		fprintf(out, "uriRows[%d].string_val.length = %d;\n\t", uriIter, schema->uriTable.uri[uriIter].uriStr.length);

	}

	fprintf(out, "URITable* uriTbl = memManagedAllocate(&schema->memList, sizeof(URITable));\n\t");
	fprintf(out, "if(uriTbl == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t");
	fprintf(out, "uriTbl->arrayDimension = %d;\n\t", schema->uriTable.dynArray.arrayEntries);
	fprintf(out, "uriTbl->rowCount = %d;\n\t", schema->uriTable.count);
	fprintf(out, "uriTbl->rows = uriRows;\n\t");
	fprintf(out, "uriTbl->memPair.memBlock = NULL;\n\t"); // TO BE fixed!
	fprintf(out, "uriTbl->memPair.allocIndx = 0;\n\t"); // TO BE fixed!
	fprintf(out, "QNameID* qnames = memManagedAllocate(&schema->memList, %d * sizeof(QNameID));\n\t", schema->globalElemGrammarTable.count);
	fprintf(out, "if(qnames == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t");

	for(uriIter = 0; uriIter < schema->globalElemGrammarTable.count; uriIter++)
	{
		fprintf(out, "qnames[%d].uriId = %d;\n\t", uriIter, schema->globalElemGrammarTable.qname[uriIter].uriId);
		fprintf(out, "qnames[%d].lnId = %d;\n\t", uriIter, schema->globalElemGrammarTable.qname[uriIter].lnId);

	}

	fprintf(out, "SimpleType* sTypes = memManagedAllocate(&schema->memList, %d * sizeof(SimpleType));\n\t", schema->simpleTypeTable.count);
	fprintf(out, "if(sTypes == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t");

	for(uriIter = 0; uriIter < schema->simpleTypeTable.count; uriIter++)
	{
		fprintf(out, "sTypes[%d].facetPresenceMask = %d;\n\t", uriIter, schema->simpleTypeTable.sType[uriIter].facetPresenceMask);
		fprintf(out, "sTypes[%d].maxInclusive = %d;\n\t", uriIter, schema->simpleTypeTable.sType[uriIter].maxInclusive);
		fprintf(out, "sTypes[%d].minInclusive = %d;\n\t", uriIter, schema->simpleTypeTable.sType[uriIter].minInclusive);
		fprintf(out, "sTypes[%d].maxLength = %d;\n\t", uriIter, schema->simpleTypeTable.sType[uriIter].maxLength);

	}

	fprintf(out, "schema->globalElemGrammars = qnames;\n\t");
	fprintf(out, "schema->globalElemGrammarTable.count = %d;\n\t", schema->globalElemGrammarTable.count);
	fprintf(out, "schema->uriTable = uriTbl;\n\t");
	fprintf(out, "schema->simpleTypeArray = sTypes;\n\t");
	fprintf(out, "schema->simpleTypeTable.count = %d;\n\t", schema->simpleTypeTable.count);
	fprintf(out, "schema->isAugmented = %d;\n\t", mask_specified);
	fprintf(out, "schema->isStatic = FALSE;\n\t");
	fprintf(out, "return ERR_OK;\n}");

	hashtable_destroy(typeGrammarsHash);
#endif
	return ERR_OK;
}
