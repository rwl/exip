/*==================================================================================*\
|                                                                                    |
|                    EXIP - Efficient XML Interchange Processor                      |
|                                                                                    |
|------------------------------------------------------------------------------------|
| Copyright (c) 2010, EISLAB - Luleå University of Technology                        |
| All rights reserved.                                                               |
|                                                                                    |
| Redistribution and use in source and binary forms, with or without                 |
| modification, are permitted provided that the following conditions are met:        |
|     * Redistributions of source code must retain the above copyright               |
|       notice, this list of conditions and the following disclaimer.                |
|     * Redistributions in binary form must reproduce the above copyright            |
|       notice, this list of conditions and the following disclaimer in the          |
|       documentation and/or other materials provided with the distribution.         |
|     * Neither the name of the EISLAB - Luleå University of Technology nor the      |
|       names of its contributors may be used to endorse or promote products         |
|       derived from this software without specific prior written permission.        |
|                                                                                    |
| THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND    |
| ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED      |
| WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             |
| DISCLAIMED. IN NO EVENT SHALL EISLAB - LULEÅ UNIVERSITY OF TECHNOLOGY BE LIABLE    |
| FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES |
| (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;       |
| LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND        |
| ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT         |
| (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS      |
| SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                       |
|                                                                                    |
|                                                                                    |
|                                                                                    |
\===================================================================================*/

/**
 * @file createGrammars.c
 * @brief Generate EXI grammars from XML schema definition and stores them in EXIP format
 *
 * @date Oct 13, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id: decodeTestEXI.c 93 2011-03-30 15:39:41Z kjussakov $
 */

#include "procTypes.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"
#include "memManagement.h"
#include "grammarAugment.h"
#include "hashtable.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_GRAMMARS_COUNT 5000
#define INPUT_BUFFER_SIZE 200
#define SPRINTF_BUFFER_SIZE 300
#define OUT_EXIP     0
#define OUT_TEXT     1
#define OUT_SRC_DYN  2
#define OUT_SRC_STAT 3

static void printfHelp();

size_t readFileInputStream(void* buf, size_t readSize, void* stream);
size_t writeFileOutputStream(void* buf, size_t readSize, void* stream);

// Converts to NULL terminated ASCII representation
static errorCode stringToASCII(char* outBuf, unsigned int bufSize, String inStr);

static void getValueTypeString(char* buf, ValueType vt);

int main(int argc, char *argv[])
{
	FILE *infile;
	FILE *outfile = stdout;
	char buffer[INPUT_BUFFER_SIZE];
	IOStream inputStrm;
	IOStream outputStrm;
	EXIPSchema schema;
	unsigned char outputFormat = OUT_EXIP;
	unsigned int currArgNumber = 1;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	char prefix[20];

	unsigned char mask_specified = FALSE;
	unsigned char mask_strict = FALSE;
	unsigned char mask_sc = FALSE;
	unsigned char mask_preserve = 0;

	inputStrm.readWriteToStream = readFileInputStream;
	outputStrm.readWriteToStream = writeFileOutputStream;
	outputStrm.stream = outfile;

	if(argc > 1)
	{
		if(strcmp(argv[1], "-help") == 0)
		{
			printfHelp();
			return 0;
		}
		else if(strcmp(argv[1], "-exip") == 0)
		{
			outputFormat = OUT_EXIP;
			currArgNumber++;
		}
		else if(strcmp(argv[1], "-text") == 0)
		{
			outputFormat = OUT_TEXT;
			currArgNumber++;
		}
		else if(argv[1][0] == '-' &&
				argv[1][1] == 's' &&
				argv[1][2] == 'r' &&
				argv[1][3] == 'c')
		{
			if(strcmp(argv[1] + 4, "=static") == 0)
			{
				outputFormat = OUT_SRC_STAT;
			}
			else
				outputFormat = OUT_SRC_DYN;
			currArgNumber++;

			if(argc <= currArgNumber)
			{
				printfHelp();
				return 0;
			}

			if(argv[currArgNumber][0] == '-' &&
			   argv[currArgNumber][1] == 'p' &&
			   argv[currArgNumber][2] == 'f' &&
			   argv[currArgNumber][3] == 'x' &&
			   argv[currArgNumber][4] == '=')
			{
				strcpy(prefix, argv[currArgNumber] + 5);
				currArgNumber++;
			}
			else
			{
				strcpy(prefix, "prfx_"); // The default prefix
			}
		}

		if(argc <= currArgNumber)
		{
			printfHelp();
			return 0;
		}

		if(argv[currArgNumber][0] == '-' &&
		   argv[currArgNumber][1] == 'm' &&
		   argv[currArgNumber][2] == 'a' &&
		   argv[currArgNumber][3] == 's' &&
		   argv[currArgNumber][4] == 'k' &&
		   argv[currArgNumber][5] == '=')
		{
			mask_specified = TRUE;
			if(argv[currArgNumber][6] == '1')
				mask_strict = TRUE;
			else
				mask_strict = FALSE;

			if(argv[currArgNumber][6] == '1')
				mask_sc = TRUE;
			else
				mask_sc = FALSE;

			if(argv[currArgNumber][7] == '1')
				SET_PRESERVED(mask_preserve, PRESERVE_DTD);

			if(argv[currArgNumber][8] == '1')
				SET_PRESERVED(mask_preserve, PRESERVE_PREFIXES);

			if(argv[currArgNumber][9] == '1')
				SET_PRESERVED(mask_preserve, PRESERVE_LEXVALUES);

			if(argv[currArgNumber][10] == '1')
				SET_PRESERVED(mask_preserve, PRESERVE_COMMENTS);

			if(argv[currArgNumber][11] == '1')
				SET_PRESERVED(mask_preserve, PRESERVE_PIS);

			currArgNumber++;
		}

		if(argc <= currArgNumber)
		{
			printfHelp();
			return 0;
		}

		infile = fopen(argv[currArgNumber], "rb" );
		if(!infile)
		{
			fprintf(stderr, "Unable to open file %s", argv[currArgNumber]);
			return 1;
		}
		inputStrm.stream = infile;

		if(argc > currArgNumber + 1)
		{
			outfile = fopen(argv[currArgNumber + 1], "wb" );
			if(!outfile)
			{
				fprintf(stderr, "Unable to open file %s", argv[currArgNumber + 1]);
				return 1;
			}
			outputStrm.stream = outfile;
		}

		tmp_err_code = generateSchemaInformedGrammars(buffer, INPUT_BUFFER_SIZE, 0, &inputStrm, SCHEMA_FORMAT_XSD_EXI, &schema);
		if(tmp_err_code != ERR_OK)
		{
			printf("\n Error occured: %d", tmp_err_code);
			exit(1);
		}
		fclose(infile);

		{
			unsigned int i;
			unsigned int j;
			unsigned int k;
			size_t r;
			size_t p;
			char printfBuf[SPRINTF_BUFFER_SIZE];
			EXIGrammar* tmpGrammar;
			size_t tmp_prod_indx = 0;
			char conv_buff[SPRINTF_BUFFER_SIZE];
			AllocList memList;
			time_t now;
			struct hashtable *typeGrammrarsHash;
			struct hashtable *typeEmptyGrammrarsHash;
			size_t typeGrammarID;
			size_t typeEmptyGrammarID;
			String hashKey;
			QNameID typeGrammars[MAX_GRAMMARS_COUNT];
			unsigned int tgCount = 0;
			QNameID typeEmptyGrammars[MAX_GRAMMARS_COUNT];
			unsigned int tegCount = 0;
			unsigned int grammarPointer = 0;

			if(ERR_OK != initAllocList(&memList))
			{
				printf("unexpected error!");
				exit(1);
			}

			typeGrammrarsHash = create_hashtable(200, djbHash, stringEqual);
			typeEmptyGrammrarsHash = create_hashtable(200, djbHash, stringEqual);

			time(&now);
			sprintf(printfBuf, "/** AUTO-GENERATED: %.24s\n  * Copyright (c) 2010 - 2011, Rumen Kyusakov, EISLAB, LTU\n  * $Id$ */\n\n",  ctime(&now));
			fwrite(printfBuf, 1, strlen(printfBuf), outfile);

			if(outputFormat == OUT_EXIP)
			{
				printf("\n ERROR: EXIP output format is not implemented yet!");
				exit(1);
			}
			else if(outputFormat == OUT_SRC_DYN)
			{
				// TODO: All strings should be defined with the function asciiToString()
				//		 also, some common strings should be defined just once in a common location

				sprintf(printfBuf, "#include \"procTypes.h\"\n");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "#include \"memManagement.h\"\n");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "#include \"stringManipulate.h\"\n\n");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);


				sprintf(printfBuf, "errorCode get_%sSchema(EXIPSchema* schema);\n\n", prefix);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "errorCode get_%sSchema(EXIPSchema* schema)\n{\n\t errorCode tmp_err_code = UNEXPECTED_ERROR;\n\t\n\t", prefix);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "if(schema == NULL)\n\t return NULL_POINTER_REF;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				sprintf(printfBuf, "tmp_err_code = initAllocList(&schema->memList);\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "if(tmp_err_code != ERR_OK)\n\t return tmp_err_code;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				for(i = 0; i < schema.initialStringTables->rowCount; i++)
				{
					if(schema.initialStringTables->rows[i].pTable != NULL)
					{
						sprintf(printfBuf, "PrefixTable* pTable_%d = memManagedAllocate(&schema->memList, sizeof(PrefixTable));\n\t", i);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						sprintf(printfBuf, "if(pTable_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);

						sprintf(printfBuf, "pTable_%d->rowCount = %d;\n\t", i, schema.initialStringTables->rows[i].pTable->rowCount);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);

						for(k = 0; k < schema.initialStringTables->rows[i].pTable->rowCount; k++)
						{
							if(ERR_OK != stringToASCII(conv_buff, SPRINTF_BUFFER_SIZE, schema.initialStringTables->rows[i].pTable->string_val[k]))
							{
								printf("\n ERROR: OUT_SRC_DYN output format!");
								exit(1);
							}
							sprintf(printfBuf, "tmp_err_code += asciiToString(\"%s\", &pTable_%d->string_val[%d], &schema->memList, TRUE);\n\t", conv_buff, i, k);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}
						for(; k < MAXIMUM_NUMBER_OF_PREFIXES_PER_URI; k++)
						{
							sprintf(printfBuf, "getEmptyString(&pTable_%d->string_val[%d]);\n\t", i, k);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}
						sprintf(printfBuf, "if(tmp_err_code != ERR_OK)\n\t return UNEXPECTED_ERROR;\n\t");
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					}

					for(j = 0; j < schema.initialStringTables->rows[i].lTable->rowCount; j++)
					{
						tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeGrammar;
						grammarPointer = (unsigned int) tmpGrammar;
						hashKey.str = (char*) &grammarPointer;
						hashKey.length = sizeof(grammarPointer);
						typeGrammarID = hashtable_search(typeGrammrarsHash, &hashKey);
						if(tmpGrammar != NULL && typeGrammarID == SIZE_MAX)
						{
							if(mask_specified == TRUE)
							{
								if(ERR_OK != addUndeclaredProductions(&memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, schema.simpleTypeArray, schema.sTypeArraySize))
								{
									printf("\n ERROR: OUT_SRC_DYN output format!");
									exit(1);
								}
							}

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								for(k = 0; k < 3; k++)
								{
									if(tmpGrammar->ruleArray[r].prodCounts[k] > 0)
									{
										sprintf(printfBuf, "Production* prodArray_%d_%d_%d_%d = memManagedAllocate(&schema->memList, %d * sizeof(Production));\n\t", i, j, r, k, tmpGrammar->ruleArray[r].prodCounts[k]);
										fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										sprintf(printfBuf, "if(prodArray_%d_%d_%d_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i, j, r, k);
										fwrite(printfBuf, 1, strlen(printfBuf), outfile);

										for(p = 0; p < tmpGrammar->ruleArray[r].prodCounts[k]; p++)
										{
											sprintf(printfBuf, "prodArray_%d_%d_%d_%d[%d].event.eventType = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].event.eventType);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_%d_%d_%d_%d[%d].event.valueType.exiType = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].event.valueType.exiType);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_%d_%d_%d_%d[%d].event.valueType.simpleTypeID = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].event.valueType.simpleTypeID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_%d_%d_%d_%d[%d].nonTermID = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].nonTermID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_%d_%d_%d_%d[%d].uriRowID = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].uriRowID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_%d_%d_%d_%d[%d].lnRowID = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].lnRowID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										}
									}
								}
							}
							// #DOCUMENT# IMPORTANT! tmpGrammar->rulesDimension + (mask_specified == FALSE) because It must be assured that the schema informed grammars have one empty slot for the rule:  Element i, content2
							sprintf(printfBuf, "GrammarRule* ruleArray_%d_%d = memManagedAllocate(&schema->memList, %d * sizeof(GrammarRule));\n\t", i, j, tmpGrammar->rulesDimension + (mask_specified == FALSE));
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "if(ruleArray_%d_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								sprintf(printfBuf, "ruleArray_%d_%d[%d].part[0].bits = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].bits[0]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "ruleArray_%d_%d[%d].part[1].bits = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].bits[1]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "ruleArray_%d_%d[%d].part[2].bits = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].bits[2]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);

								sprintf(printfBuf, "ruleArray_%d_%d[%d].part[0].prodArraySize = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].prodCounts[0]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "ruleArray_%d_%d[%d].part[1].prodArraySize = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].prodCounts[1]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "ruleArray_%d_%d[%d].part[2].prodArraySize = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].prodCounts[2]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);

								if(tmpGrammar->ruleArray[r].prodCounts[0] > 0)
									sprintf(printfBuf, "ruleArray_%d_%d[%d].part[0].prodArray = prodArray_%d_%d_%d_0;\n\t", i, j, r, i, j, r);
								else
									sprintf(printfBuf, "ruleArray_%d_%d[%d].part[0].prodArray = NULL;\n\t", i, j, r);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								if(tmpGrammar->ruleArray[r].prodCounts[0] > 0)
									sprintf(printfBuf, "ruleArray_%d_%d[%d].part[1].prodArray = prodArray_%d_%d_%d_1;\n\t", i, j, r, i, j, r);
								else
									sprintf(printfBuf, "ruleArray_%d_%d[%d].part[1].prodArray = NULL;\n\t", i, j, r);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								if(tmpGrammar->ruleArray[r].prodCounts[0] > 0)
									sprintf(printfBuf, "ruleArray_%d_%d[%d].part[2].prodArray = prodArray_%d_%d_%d_2;\n\t", i, j, r, i, j, r);
								else
									sprintf(printfBuf, "ruleArray_%d_%d[%d].part[2].prodArray = NULL;\n\t", i, j, r);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							}

							sprintf(printfBuf, "EXIGrammar* grammar_%d_%d = memManagedAllocate(&schema->memList, sizeof(EXIGrammar));\n\t", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "if(grammar_%d_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							sprintf(printfBuf, "grammar_%d_%d->contentIndex = %d;\n\t", i, j, tmpGrammar->contentIndex);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_%d_%d->grammarType = %d;\n\t", i, j, tmpGrammar->grammarType);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_%d_%d->isNillable = %d;\n\t", i, j, tmpGrammar->isNillable);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_%d_%d->isAugmented = %d;\n\t", i, j, tmpGrammar->isAugmented);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_%d_%d->rulesDimension = %d;\n\t", i, j, tmpGrammar->rulesDimension);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_%d_%d->ruleArray = ruleArray_%d_%d;\n\t", i, j, i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							if(tgCount >= MAX_GRAMMARS_COUNT)
							{
								printf("\n ERROR: MAX_GRAMMARS_COUNT reached!");
								exit(1);
							}

							if(ERR_OK != hashtable_insert(typeGrammrarsHash, &hashKey, tgCount))
							{
								printf("\n ERROR: OUT_SRC_DYN output format!");
								exit(1);
							}
							typeGrammars[tgCount].uriRowId = i;
							typeGrammars[tgCount].lnRowId = j;
							tgCount++;
						}

						tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeEmptyGrammar;
						grammarPointer = (unsigned int) tmpGrammar;
						hashKey.str = (char*) &grammarPointer;
						hashKey.length = sizeof(grammarPointer);
						typeEmptyGrammarID = hashtable_search(typeEmptyGrammrarsHash, &hashKey);
						if(tmpGrammar != NULL && typeEmptyGrammarID == SIZE_MAX)
						{
							if(mask_specified == TRUE)
							{
								if(ERR_OK != addUndeclaredProductions(&memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, schema.simpleTypeArray, schema.sTypeArraySize))
								{
									printf("\n ERROR: OUT_SRC_DYN output format!");
									exit(1);
								}
							}

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								for(k = 0; k < 3; k++)
								{
									if(tmpGrammar->ruleArray[r].prodCounts[k] > 0)
									{
										sprintf(printfBuf, "Production* prodArray_empty_%d_%d_%d_%d = memManagedAllocate(&schema->memList, %d * sizeof(Production));\n\t", i, j, r, k, tmpGrammar->ruleArray[r].prodCounts[k]);
										fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										sprintf(printfBuf, "if(prodArray_empty_%d_%d_%d_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i, j, r, k);
										fwrite(printfBuf, 1, strlen(printfBuf), outfile);

										for(p = 0; p < tmpGrammar->ruleArray[r].prodCounts[k]; p++)
										{
											sprintf(printfBuf, "prodArray_empty_%d_%d_%d_%d[%d].event.eventType = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].event.eventType);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_empty_%d_%d_%d_%d[%d].event.valueType.exiType = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].event.valueType.exiType);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_empty_%d_%d_%d_%d[%d].event.valueType.simpleTypeID = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].event.valueType.simpleTypeID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_empty_%d_%d_%d_%d[%d].nonTermID = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].nonTermID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_empty_%d_%d_%d_%d[%d].uriRowID = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].uriRowID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											sprintf(printfBuf, "prodArray_empty_%d_%d_%d_%d[%d].lnRowID = %d;\n\t", i, j, r, k, p, tmpGrammar->ruleArray[r].prodArrays[k][p].lnRowID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										}
									}
								}
							}
							// #DOCUMENT# IMPORTANT! tmpGrammar->rulesDimension + (mask_specified == FALSE) because It must be assured that the schema informed grammars have one empty slot for the rule:  Element i, content2
							sprintf(printfBuf, "GrammarRule* ruleArray_empty_%d_%d = memManagedAllocate(&schema->memList, %d * sizeof(GrammarRule));\n\t", i, j, tmpGrammar->rulesDimension + (mask_specified == FALSE));
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "if(ruleArray_empty_%d_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[0].bits = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].bits[0]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[1].bits = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].bits[1]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[2].bits = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].bits[2]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);

								sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[0].prodArraySize = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].prodCounts[0]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[1].prodArraySize = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].prodCounts[1]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[2].prodArraySize = %d;\n\t", i, j, r, tmpGrammar->ruleArray[r].prodCounts[2]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);

								if(tmpGrammar->ruleArray[r].prodCounts[0] > 0)
									sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[0].prodArray = prodArray_empty_%d_%d_%d_0;\n\t", i, j, r, i, j, r);
								else
									sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[0].prodArray = NULL;\n\t", i, j, r);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								if(tmpGrammar->ruleArray[r].prodCounts[0] > 0)
									sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[1].prodArray = prodArray_empty_%d_%d_%d_1;\n\t", i, j, r, i, j, r);
								else
									sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[1].prodArray = NULL;\n\t", i, j, r);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								if(tmpGrammar->ruleArray[r].prodCounts[0] > 0)
									sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[2].prodArray = prodArray_empty_%d_%d_%d_2;\n\t", i, j, r, i, j, r);
								else
									sprintf(printfBuf, "ruleArray_empty_%d_%d[%d].part[2].prodArray = NULL;\n\t", i, j, r);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							}

							sprintf(printfBuf, "EXIGrammar* grammar_empty_%d_%d = memManagedAllocate(&schema->memList, sizeof(EXIGrammar));\n\t", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "if(grammar_empty_%d_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							sprintf(printfBuf, "grammar_empty_%d_%d->contentIndex = %d;\n\t", i, j, tmpGrammar->contentIndex);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_empty_%d_%d->grammarType = %d;\n\t", i, j, tmpGrammar->grammarType);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_empty_%d_%d->isNillable = %d;\n\t", i, j, tmpGrammar->isNillable);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_empty_%d_%d->isAugmented = %d;\n\t", i, j, tmpGrammar->isAugmented);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_empty_%d_%d->rulesDimension = %d;\n\t", i, j, tmpGrammar->rulesDimension);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							sprintf(printfBuf, "grammar_empty_%d_%d->ruleArray = ruleArray_%d_%d;\n\t", i, j, i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							if(tegCount >= MAX_GRAMMARS_COUNT)
							{
								printf("\n ERROR: MAX_GRAMMARS_COUNT reached!");
								exit(1);
							}

							if(ERR_OK != hashtable_insert(typeEmptyGrammrarsHash, &hashKey, tegCount))
							{
								printf("\n ERROR: OUT_SRC_DYN output format!");
								exit(1);
							}
							typeEmptyGrammars[tegCount].uriRowId = i;
							typeEmptyGrammars[tegCount].lnRowId = j;
							tegCount++;
						}
					}

					sprintf(printfBuf, "struct LocalNamesRow* LNrows_%d = memManagedAllocate(&schema->memList, %d * sizeof(struct LocalNamesRow));\n\t", i, schema.initialStringTables->rows[i].lTable->rowCount);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "if(LNrows_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);

					for(j = 0; j < schema.initialStringTables->rows[i].lTable->rowCount; j++)
					{
						if(ERR_OK != stringToASCII(conv_buff, SPRINTF_BUFFER_SIZE, schema.initialStringTables->rows[i].lTable->rows[j].string_val))
						{
							printf("\n ERROR: OUT_SRC_DYN output format!");
							exit(1);
						}

						tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeGrammar;
						grammarPointer = (unsigned int) tmpGrammar;
						hashKey.str = (char*) &grammarPointer;
						hashKey.length = sizeof(grammarPointer);
						typeGrammarID = hashtable_search(typeGrammrarsHash, &hashKey);

						if(tmpGrammar != NULL)
						{
							sprintf(printfBuf, "LNrows_%d[%d].typeGrammar = grammar_%d_%d;\n\t", i, j, typeGrammars[typeGrammarID].uriRowId, typeGrammars[typeGrammarID].lnRowId);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}
						else
						{
							sprintf(printfBuf, "LNrows_%d[%d].typeGrammar = NULL;\n\t", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}

						tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeEmptyGrammar;
						grammarPointer = (unsigned int) tmpGrammar;
						hashKey.str = (char*) &grammarPointer;
						hashKey.length = sizeof(grammarPointer);
						typeEmptyGrammarID = hashtable_search(typeEmptyGrammrarsHash, &hashKey);

						if(tmpGrammar != NULL)
						{
							sprintf(printfBuf, "LNrows_%d[%d].typeEmptyGrammar = grammar_empty_%d_%d;\n\t", i, j, typeEmptyGrammars[typeEmptyGrammarID].uriRowId, typeEmptyGrammars[typeEmptyGrammarID].lnRowId);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}
						else
						{
							sprintf(printfBuf, "LNrows_%d[%d].typeEmptyGrammar = NULL;\n\t", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}

						sprintf(printfBuf, "LNrows_%d[%d].string_val.str = \"%s\";\n\t", i, j, conv_buff);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						sprintf(printfBuf, "LNrows_%d[%d].string_val.length = %d;\n\t", i, j, schema.initialStringTables->rows[i].lTable->rows[j].string_val.length);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						sprintf(printfBuf, "LNrows_%d[%d].vCrossTable = NULL;\n\t", i, j);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					}

					sprintf(printfBuf, "LocalNamesTable* lTable_%d = memManagedAllocate(&schema->memList, sizeof(LocalNamesTable));\n\t", i);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "if(lTable_%d == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t", i);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);

					sprintf(printfBuf, "lTable_%d->arrayDimension = %d;\n\t", i, schema.initialStringTables->rows[i].lTable->rowCount);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "lTable_%d->rowCount = %d;\n\t", i, schema.initialStringTables->rows[i].lTable->rowCount);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "lTable_%d->rows = LNrows_%d;\n\t", i, i);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "lTable_%d->memPair.memBlock = NULL;\n\t", i); // TO BE fixed!
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);					// TO BE fixed!
					sprintf(printfBuf, "lTable_%d->memPair.allocIndx = 0;\n\t", i); // TO BE fixed!
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);				// TO BE fixed!
				}

				sprintf(printfBuf, "struct URIRow* uriRows = memManagedAllocate(&schema->memList, %d * sizeof(struct URIRow));\n\t", schema.initialStringTables->rowCount);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "if(uriRows == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				for(i = 0; i < schema.initialStringTables->rowCount; i++)
				{
					if(ERR_OK != stringToASCII(conv_buff, SPRINTF_BUFFER_SIZE, schema.initialStringTables->rows[i].string_val))
					{
						printf("\n ERROR: OUT_SRC_DYN output format!");
						exit(1);
					}
					if(schema.initialStringTables->rows[i].pTable != NULL)
					{
						sprintf(printfBuf, "uriRows[%d].pTable = pTable_%d;\n\t", i, i);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					}
					else
					{
						sprintf(printfBuf, "uriRows[%d].pTable = NULL;\n\t", i);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					}
					sprintf(printfBuf, "uriRows[%d].lTable = lTable_%d;\n\t", i, i);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "uriRows[%d].string_val.str = \"%s\";\n\t", i, conv_buff);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "uriRows[%d].string_val.length = %d;\n\t", i, schema.initialStringTables->rows[i].string_val.length);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				}

				sprintf(printfBuf, "URITable* uriTbl = memManagedAllocate(&schema->memList, sizeof(URITable));\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "if(uriTbl == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				sprintf(printfBuf, "uriTbl->arrayDimension = %d;\n\t", schema.initialStringTables->arrayDimension);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "uriTbl->rowCount = %d;\n\t", schema.initialStringTables->rowCount);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "uriTbl->rows = uriRows;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "uriTbl->memPair.memBlock = NULL;\n\t"); // TO BE fixed!
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);					// TO BE fixed!
				sprintf(printfBuf, "uriTbl->memPair.allocIndx = 0;\n\t"); // TO BE fixed!
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);				// TO BE fixed!

				sprintf(printfBuf, "QNameID* qnames = memManagedAllocate(&schema->memList, %d * sizeof(QNameID));\n\t", schema.globalElemGrammarsCount);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "if(qnames == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				for(i = 0; i < schema.globalElemGrammarsCount; i++)
				{
					sprintf(printfBuf, "qnames[%d].uriRowId = %d;\n\t", i, schema.globalElemGrammars[i].uriRowId);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "qnames[%d].lnRowId = %d;\n\t", i, schema.globalElemGrammars[i].lnRowId);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				}

				sprintf(printfBuf, "SimpleType* sTypes = memManagedAllocate(&schema->memList, %d * sizeof(SimpleType));\n\t", schema.sTypeArraySize);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "if(sTypes == NULL)\n\t return MEMORY_ALLOCATION_ERROR;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				for(i = 0; i < schema.sTypeArraySize; i++)
				{
					sprintf(printfBuf, "sTypes[%d].facetPresenceMask = %d;\n\t", i, schema.simpleTypeArray[i].facetPresenceMask);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "sTypes[%d].maxInclusive = %d;\n\t", i, schema.simpleTypeArray[i].maxInclusive);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "sTypes[%d].minInclusive = %d;\n\t", i, schema.simpleTypeArray[i].minInclusive);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					sprintf(printfBuf, "sTypes[%d].maxLength = %d;\n\t", i, schema.simpleTypeArray[i].maxLength);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				}

				sprintf(printfBuf, "schema->globalElemGrammars = qnames;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "schema->globalElemGrammarsCount = %d;\n\t", schema.globalElemGrammarsCount);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "schema->initialStringTables = uriTbl;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "schema->simpleTypeArray = sTypes;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "schema->sTypeArraySize = %d;\n\t", schema.sTypeArraySize);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "schema->isAugmented = %d;\n\t", mask_specified);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				sprintf(printfBuf, "schema->isStatic = FALSE;\n\t");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				sprintf(printfBuf, "return ERR_OK;\n}");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

			}
			else if(outputFormat == OUT_SRC_STAT)
			{
				// NOTE: Do not use without option mask! Also when strict == FALSE the memPairs are NULL which will create errors
				// When there is no mask specified this is not correct if the schema is used more than once
				// There is extra rule slot for each grammar to be use if
				// strict == FALSE by addUndeclaredProductions() when no mask is specified

				sprintf(printfBuf, "#include \"procTypes.h\"\n\n");
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				for(i = 0; i < schema.initialStringTables->rowCount; i++)
				{
					if(schema.initialStringTables->rows[i].pTable != NULL)
					{
						sprintf(printfBuf, "PrefixTable %spTable_%d = {{", prefix, i);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						for(k = 0; k < schema.initialStringTables->rows[i].pTable->rowCount; k++)
						{
							if(ERR_OK != stringToASCII(conv_buff, SPRINTF_BUFFER_SIZE, schema.initialStringTables->rows[i].pTable->string_val[k]))
							{
								printf("\n ERROR: OUT_SRC_STAT output format!");
								exit(1);
							}
							sprintf(printfBuf, "%s{\"%s\",%d}", k==0?"":",", conv_buff, schema.initialStringTables->rows[i].pTable->string_val[k].length);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}
						for(; k < MAXIMUM_NUMBER_OF_PREFIXES_PER_URI; k++)
						{
							sprintf(printfBuf, "%s{NULL,0}", k==0?"":",");
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}
						sprintf(printfBuf, "}, %d};\n\n", schema.initialStringTables->rows[i].pTable->rowCount);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);
					}

					for(j = 0; j < schema.initialStringTables->rows[i].lTable->rowCount; j++)
					{
						tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeGrammar;
						grammarPointer = (unsigned int) tmpGrammar;
						hashKey.str = (char*) &grammarPointer;
						hashKey.length = sizeof(grammarPointer);
						typeGrammarID = hashtable_search(typeGrammrarsHash, &hashKey);

						if(tmpGrammar != NULL && typeGrammarID == SIZE_MAX)
						{
							if(mask_specified == TRUE)
							{
								if(ERR_OK != addUndeclaredProductions(&memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, schema.simpleTypeArray, schema.sTypeArraySize))
								{
									printf("\n ERROR: OUT_SRC_STAT output format!");
									exit(1);
								}
							}

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								for(k = 0; k < 3; k++)
								{
									if (tmpGrammar->ruleArray[r].prodCounts[k])
									{
										sprintf(printfBuf, "Production %sprodArray_%d_%d_%d_%d[%d] = {", prefix, i, j, r, k, tmpGrammar->ruleArray[r].prodCounts[k]);
										fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										for(p = 0; p < tmpGrammar->ruleArray[r].prodCounts[k]; p++)
										{
											sprintf(printfBuf, "%s{{%d,{%d, %d}}, %d, %d, %d}", p==0?"":",", tmpGrammar->ruleArray[r].prodArrays[k][p].event.eventType, tmpGrammar->ruleArray[r].prodArrays[k][p].event.valueType.exiType, tmpGrammar->ruleArray[r].prodArrays[k][p].event.valueType.simpleTypeID, tmpGrammar->ruleArray[r].prodArrays[k][p].uriRowID, tmpGrammar->ruleArray[r].prodArrays[k][p].lnRowID, tmpGrammar->ruleArray[r].prodArrays[k][p].nonTermID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										}
										fwrite("};\n", 1, strlen("};\n"), outfile);
									}
								}
							}
							// #DOCUMENT# IMPORTANT! tmpGrammar->rulesDimension + (mask_specified == FALSE) because It must be assured that the schema informed grammars have one empty slot for the rule:  Element i, content2
							sprintf(printfBuf, "\nGrammarRule %sruleArray_%d_%d[%d] = {", prefix, i, j, tmpGrammar->rulesDimension + (mask_specified == FALSE));
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								sprintf(printfBuf, "%s{{", r==0?"":",");
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								if (tmpGrammar->ruleArray[r].prodCounts[0] > 0)
									sprintf(printfBuf, "%sprodArray_%d_%d_%d_0, ", prefix, i, j, r);
								else
									sprintf(printfBuf, "NULL, ");
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "%d, %d}, {", tmpGrammar->ruleArray[r].prodCounts[0], tmpGrammar->ruleArray[r].bits[0]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);

								if (tmpGrammar->ruleArray[r].prodCounts[1] > 0)
									sprintf(printfBuf, "%sprodArray_%d_%d_%d_1, ", prefix, i, j, r);
								else
									sprintf(printfBuf, "NULL, ");
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "%d, %d}, {", tmpGrammar->ruleArray[r].prodCounts[1], tmpGrammar->ruleArray[r].bits[1]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);

								if (tmpGrammar->ruleArray[r].prodCounts[2] > 0)
									sprintf(printfBuf, "%sprodArray_%d_%d_%d_2", prefix, i, j, r);
								else
									sprintf(printfBuf, "NULL");
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "%d, %d}} ", tmpGrammar->ruleArray[r].prodCounts[2], tmpGrammar->ruleArray[r].bits[2]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							}

							fwrite("};\n\n", 1, strlen("};\n\n"), outfile);

							sprintf(printfBuf, "EXIGrammar %sgrammar_%d_%d = {%sruleArray_%d_%d, %d, %d, %d, %d, %d};\n\n",
									prefix, i, j, prefix, i, j, tmpGrammar->rulesDimension, tmpGrammar->grammarType, tmpGrammar->isNillable, tmpGrammar->isAugmented, tmpGrammar->contentIndex);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							if(tgCount >= MAX_GRAMMARS_COUNT)
							{
								printf("\n ERROR: MAX_GRAMMARS_COUNT reached!");
								exit(1);
							}

							if(ERR_OK != hashtable_insert(typeGrammrarsHash, &hashKey, tgCount))
							{
								printf("\n ERROR: OUT_SRC_DYN output format!");
								exit(1);
							}
							typeGrammars[tgCount].uriRowId = i;
							typeGrammars[tgCount].lnRowId = j;
							tgCount++;
						}

						tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeEmptyGrammar;
						grammarPointer = (unsigned int) tmpGrammar;
						hashKey.str = (char*) &grammarPointer;
						hashKey.length = sizeof(grammarPointer);
						typeEmptyGrammarID = hashtable_search(typeEmptyGrammrarsHash, &hashKey);

						if(tmpGrammar != NULL && typeEmptyGrammarID == SIZE_MAX)
						{
							if(mask_specified == TRUE)
							{
								if(ERR_OK != addUndeclaredProductions(&memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, schema.simpleTypeArray, schema.sTypeArraySize))
								{
									printf("\n ERROR: OUT_SRC_STAT output format!");
									exit(1);
								}
							}

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								for(k = 0; k < 3; k++)
								{
									if (tmpGrammar->ruleArray[r].prodCounts[k])
									{
										sprintf(printfBuf, "Production %sprodArray_empty_%d_%d_%d_%d[%d] = {", prefix, i, j, r, k, tmpGrammar->ruleArray[r].prodCounts[k]);
										fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										for(p = 0; p < tmpGrammar->ruleArray[r].prodCounts[k]; p++)
										{
											sprintf(printfBuf, "%s{{%d,{%d, %d}}, %d, %d, %d}", p==0?"":",", tmpGrammar->ruleArray[r].prodArrays[k][p].event.eventType, tmpGrammar->ruleArray[r].prodArrays[k][p].event.valueType.exiType, tmpGrammar->ruleArray[r].prodArrays[k][p].event.valueType.simpleTypeID, tmpGrammar->ruleArray[r].prodArrays[k][p].uriRowID, tmpGrammar->ruleArray[r].prodArrays[k][p].lnRowID, tmpGrammar->ruleArray[r].prodArrays[k][p].nonTermID);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										}
										fwrite("};\n", 1, strlen("};\n"), outfile);
									}
								}
							}
							// #DOCUMENT# IMPORTANT! tmpGrammar->rulesDimension + (mask_specified == FALSE) because It must be assured that the schema informed grammars have one empty slot for the rule:  Element i, content2
							sprintf(printfBuf, "\nGrammarRule %sruleArray_empty_%d_%d[%d] = {", prefix, i, j, tmpGrammar->rulesDimension + (mask_specified == FALSE));
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								sprintf(printfBuf, "%s{{", r==0?"":",");
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								if (tmpGrammar->ruleArray[r].prodCounts[0] > 0)
									sprintf(printfBuf, "%sprodArray_empty_%d_%d_%d_0, ", prefix, i, j, r);
								else
									sprintf(printfBuf, "NULL, ");
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "%d, %d}, {", tmpGrammar->ruleArray[r].prodCounts[0], tmpGrammar->ruleArray[r].bits[0]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);

								if (tmpGrammar->ruleArray[r].prodCounts[1] > 0)
									sprintf(printfBuf, "%sprodArray_empty_%d_%d_%d_1, ", prefix, i, j, r);
								else
									sprintf(printfBuf, "NULL, ");
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "%d, %d}, {", tmpGrammar->ruleArray[r].prodCounts[1], tmpGrammar->ruleArray[r].bits[1]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								if (tmpGrammar->ruleArray[r].prodCounts[2] > 0)
									sprintf(printfBuf, "%sprodArray_empty_%d_%d_%d_2", prefix, i, j, r);
								else
									sprintf(printfBuf, "NULL");
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "%d, %d}}", tmpGrammar->ruleArray[r].prodCounts[2], tmpGrammar->ruleArray[r].bits[2]);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							}

							fwrite("};\n\n", 1, strlen("};\n\n"), outfile);

							sprintf(printfBuf, "EXIGrammar %sgrammar_empty_%d_%d = {%sruleArray_empty_%d_%d, %d, %d, %d, %d, %d};\n\n",
									prefix, i, j, prefix, i, j, tmpGrammar->rulesDimension, tmpGrammar->grammarType, tmpGrammar->isNillable, tmpGrammar->isAugmented, tmpGrammar->contentIndex);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							if(tegCount >= MAX_GRAMMARS_COUNT)
							{
								printf("\n ERROR: MAX_GRAMMARS_COUNT reached!");
								exit(1);
							}

							if(ERR_OK != hashtable_insert(typeEmptyGrammrarsHash, &hashKey, tegCount))
							{
								printf("\n ERROR: OUT_SRC_STAT output format!");
								exit(1);
							}
							typeEmptyGrammars[tegCount].uriRowId = i;
							typeEmptyGrammars[tegCount].lnRowId = j;
							tegCount++;
						}
					}

					if(schema.initialStringTables->rows[i].lTable->rowCount > 0)
					{
						sprintf(printfBuf, "struct LocalNamesRow %sLNrows_%d[%d] = {", prefix, i, schema.initialStringTables->rows[i].lTable->rowCount);
						fwrite(printfBuf, 1, strlen(printfBuf), outfile);

						for(j = 0; j < schema.initialStringTables->rows[i].lTable->rowCount; j++)
						{
							if(ERR_OK != stringToASCII(conv_buff, SPRINTF_BUFFER_SIZE, schema.initialStringTables->rows[i].lTable->rows[j].string_val))
							{
								printf("\n ERROR: OUT_SRC_STAT output format!");
								exit(1);
							}

							tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeGrammar;
							grammarPointer = (unsigned int) tmpGrammar;
							hashKey.str = (char*) &grammarPointer;
							hashKey.length = sizeof(grammarPointer);
							typeGrammarID = hashtable_search(typeGrammrarsHash, &hashKey);

							if(tmpGrammar != NULL)
							{
								sprintf(printfBuf, "%s{NULL, {\"%s\", %d}, &%sgrammar_%d_%d", j==0?"":",",
										conv_buff, schema.initialStringTables->rows[i].lTable->rows[j].string_val.length, prefix, typeGrammars[typeGrammarID].uriRowId, typeGrammars[typeGrammarID].lnRowId);
							}
							else
							{
								sprintf(printfBuf, "%s{NULL, {\"%s\", %d}, NULL", j==0?"":",",
								conv_buff, schema.initialStringTables->rows[i].lTable->rows[j].string_val.length);
							}
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);

							tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeEmptyGrammar;
							grammarPointer = (unsigned int) tmpGrammar;
							hashKey.str = (char*) &grammarPointer;
							hashKey.length = sizeof(grammarPointer);
							typeEmptyGrammarID = hashtable_search(typeEmptyGrammrarsHash, &hashKey);

							if(tmpGrammar != NULL)
							{
								sprintf(printfBuf, ", &%sgrammar_empty_%d_%d}", prefix, typeEmptyGrammars[typeEmptyGrammarID].uriRowId, typeEmptyGrammars[typeEmptyGrammarID].lnRowId);
							}
							else
							{
								sprintf(printfBuf, ", NULL}");
							}
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
						}
						fwrite("};\n", 1, strlen("};\n"), outfile);
					}

					if(schema.initialStringTables->rows[i].lTable->rowCount > 0)
						sprintf(printfBuf, "LocalNamesTable %slTable_%d = { %sLNrows_%d, %d, %d, {NULL, 0}};\n\n", prefix, i, prefix, i, schema.initialStringTables->rows[i].lTable->rowCount,  schema.initialStringTables->rows[i].lTable->rowCount);
					else
						sprintf(printfBuf, "LocalNamesTable %slTable_%d = { NULL, %d, %d, {NULL, 0}};\n\n", prefix, i, schema.initialStringTables->rows[i].lTable->rowCount,  schema.initialStringTables->rows[i].lTable->rowCount);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				}

				sprintf(printfBuf, "struct URIRow %suriRows[%d] = {", prefix, schema.initialStringTables->rowCount);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				for(i = 0; i < schema.initialStringTables->rowCount; i++)
				{
					if(ERR_OK != stringToASCII(conv_buff, SPRINTF_BUFFER_SIZE, schema.initialStringTables->rows[i].string_val))
					{
						printf("\n ERROR: OUT_SRC_STAT output format!");
						exit(1);
					}
					if(schema.initialStringTables->rows[i].pTable != NULL)
					{
						sprintf(printfBuf, "%s{&%spTable_%d, &%slTable_%d, {\"%s\", %d}}", i==0?"":",", prefix, i, prefix, i, conv_buff, schema.initialStringTables->rows[i].string_val.length);
					}
					else
					{
						sprintf(printfBuf, "%s{NULL, &%slTable_%d, {\"%s\", %d}}", i==0?"":",", prefix, i, conv_buff, schema.initialStringTables->rows[i].string_val.length);
					}
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				}

				fwrite("};\n", 1, strlen("};\n"), outfile);

				sprintf(printfBuf, "URITable %suriTbl = {%suriRows, %d, %d, {NULL, 0}};\n\n", prefix, prefix, schema.initialStringTables->rowCount, schema.initialStringTables->rowCount);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				sprintf(printfBuf, "QNameID %sqnames[%d] = {", prefix, schema.globalElemGrammarsCount);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				for(i = 0; i < schema.globalElemGrammarsCount; i++)
				{
					sprintf(printfBuf, "%s{%d, %d}", i==0?"":",", schema.globalElemGrammars[i].uriRowId, schema.globalElemGrammars[i].lnRowId);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				}
				fwrite("};\n\n", 1, strlen("};\n\n"), outfile);

				sprintf(printfBuf, "SimpleType %ssimpleTypes[%d] = {", prefix, schema.sTypeArraySize);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

				for(i = 0; i < schema.sTypeArraySize; i++)
				{
					sprintf(printfBuf, "%s{%d, %d, %d, %d}", i==0?"":",", schema.simpleTypeArray[i].facetPresenceMask, schema.simpleTypeArray[i].maxInclusive, schema.simpleTypeArray[i].minInclusive, schema.simpleTypeArray[i].maxLength);
					fwrite(printfBuf, 1, strlen(printfBuf), outfile);
				}
				fwrite("};\n\n", 1, strlen("};\n\n"), outfile);

				sprintf(printfBuf, "const EXIPSchema %sschema = {&%suriTbl, %sqnames, %d, %ssimpleTypes, %d, %d, {NULL, NULL}};\n", prefix, prefix, prefix, schema.globalElemGrammarsCount, prefix, schema.sTypeArraySize, mask_specified);
				fwrite(printfBuf, 1, strlen(printfBuf), outfile);

			}
			else if(outputFormat == OUT_TEXT)
			{
				for(i = 0; i < schema.initialStringTables->rowCount; i++)
				{
					for(j = 0; j < schema.initialStringTables->rows[i].lTable->rowCount; j++)
					{
						tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].typeGrammar;
						if(tmpGrammar != NULL)
						{
							if(mask_specified == TRUE)
							{
								if(ERR_OK != addUndeclaredProductions(&memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, schema.simpleTypeArray, schema.sTypeArraySize))
								{
									printf("\n ERROR: OUT_TEXT output format!");
									exit(1);
								}
							}
							sprintf(printfBuf, "Grammar [%d:%d]", i, j);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							fwrite(schema.initialStringTables->rows[i].string_val.str, 1, schema.initialStringTables->rows[i].string_val.length, outfile);
							fwrite(":", 1, 1, outfile);
							fwrite(schema.initialStringTables->rows[i].lTable->rows[j].string_val.str, 1, schema.initialStringTables->rows[i].lTable->rows[j].string_val.length, outfile);
							fwrite("\n", 1, 1, outfile);

							for(r = 0; r < tmpGrammar->rulesDimension; r++)
							{
								sprintf(printfBuf, "NT-%d: \n", r);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								for(k = 0; k < 3; k++)
								{
									for(p = 0; p < tmpGrammar->ruleArray[r].prodCounts[k]; p++)
									{
										tmp_prod_indx = tmpGrammar->ruleArray[r].prodCounts[k] - 1 - p;
										switch(tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].event.eventType)
										{
											case EVENT_SD:
												fwrite("\tSD ", 1, strlen("\tSD "), outfile);
												break;
											case EVENT_ED:
												fwrite("\tED ", 1, strlen("\tED "), outfile);
												break;
											case EVENT_SE_QNAME:
												sprintf(printfBuf, "\tSE ([%d:%d]", tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID, tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].lnRowID);
												fwrite(printfBuf, 1, strlen(printfBuf), outfile);
												fwrite(schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID].string_val.str, 1, schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID].string_val.length, outfile);
												fwrite(":", 1, 1, outfile);
												fwrite(schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID].lTable->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].lnRowID].string_val.str, 1, schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID].lTable->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].lnRowID].string_val.length, outfile);
												fwrite(") ", 1, 2, outfile);
												break;
											case EVENT_SE_URI:
												fwrite("\tSE (uri) ", 1, strlen("\tSE (uri) "), outfile);
												break;
											case EVENT_SE_ALL:
												fwrite("\tSE (*) ", 1, strlen("\tSE (*) "), outfile);
												break;
											case EVENT_EE:
												fwrite("\tEE ", 1, strlen("\tEE "), outfile);
												break;
											case EVENT_AT_QNAME:
												sprintf(printfBuf, "\tAT ([%d:%d]", tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID, tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].lnRowID);
												fwrite(printfBuf, 1, strlen(printfBuf), outfile);
												fwrite(schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID].string_val.str, 1, schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID].string_val.length, outfile);
												fwrite(":", 1, 1, outfile);
												fwrite(schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID].lTable->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].lnRowID].string_val.str, 1, schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].uriRowID].lTable->rows[tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].lnRowID].string_val.length, outfile);
												fwrite(") ", 1, 2, outfile);
												getValueTypeString(printfBuf, tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].event.valueType);
												fwrite(printfBuf, 1, strlen(printfBuf), outfile);
												break;
											case EVENT_AT_URI:
												fwrite("\tAT (uri) ", 1, strlen("\tAT (uri) "), outfile);
												break;
											case EVENT_AT_ALL:
												fwrite("\tAT (*) ", 1, strlen("\tAT (*) "), outfile);
												getValueTypeString(printfBuf, tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].event.valueType);
												fwrite(printfBuf, 1, strlen(printfBuf), outfile);
												break;
											case EVENT_CH:
												fwrite("\tCH ", 1, strlen("\tCH "), outfile);
												getValueTypeString(printfBuf, tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].event.valueType);
												fwrite(printfBuf, 1, strlen(printfBuf), outfile);
												break;
											case EVENT_NS:
												fwrite("\tNS ", 1, strlen("\tNS "), outfile);
												break;
											case EVENT_CM:
												fwrite("\tCM ", 1, strlen("\tCM "), outfile);
												break;
											case EVENT_PI:
												fwrite("\tPI ", 1, strlen("\tPI "), outfile);
												break;
											case EVENT_DT:
												fwrite("\tDT ", 1, strlen("\tDT "), outfile);
												break;
											case EVENT_ER:
												fwrite("\tER ", 1, strlen("\tER "), outfile);
												break;
											case EVENT_SC:
												fwrite("\tSC ", 1, strlen("\tSC "), outfile);
												break;
											case EVENT_VOID:
												fwrite(" ", 1, 1, outfile);
												break;
											default:
												return UNEXPECTED_ERROR;
										}
										sprintf(printfBuf, "\tNT-%u\t", (unsigned int) tmpGrammar->ruleArray[r].prodArrays[k][tmp_prod_indx].nonTermID);
										fwrite(printfBuf, 1, strlen(printfBuf), outfile);
										if(k > 0)
										{
											sprintf(printfBuf, "%d.", tmpGrammar->ruleArray[r].prodCounts[0]);
											fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											if(k > 1)
											{
												sprintf(printfBuf, "%d.", tmpGrammar->ruleArray[r].prodCounts[1]);
												fwrite(printfBuf, 1, strlen(printfBuf), outfile);
											}
										}
										sprintf(printfBuf, "%d\n", p);
										fwrite(printfBuf, 1, strlen(printfBuf), outfile);
									}
								}
								fwrite("\n", 1, 1, outfile);
							}
						}
					}
				}
			}
			freeAllocList(&memList);

			hashtable_destroy(typeGrammrarsHash);
			hashtable_destroy(typeEmptyGrammrarsHash);
		}
		freeAllocList(&schema.memList);
	}
	else
	{
		printfHelp();
		return 1;
	}
	return 0;
}

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Efficient XML Interchange Processor, Rumen Kyusakov, 2011 \n");
    printf("           Copyright (c) 2010 - 2011, EISLAB - Luleå University of Technology Version 0.2 \n");
    printf("  Usage:   exipg [options] <schema_in> [<schema_out>] \n\n");
    printf("           Options: [[-help | [-exip | -text | -src[= dynamic | static] [-pfx=<prefix>]] [-mask=<OPTIONS_MASK>]] ] \n");
    printf("           -help   :   Prints this help message\n\n");
    printf("           -exip   :   Format the output schema definitions in EXIP-specific format (Default) \n\n");
    printf("           -text   :   Format the output schema definitions in human readable text format \n\n");
    printf("           -src    :   Create source files for the grammars defined. If you know the EXI options \n\n");
    printf("                       to be used for processing in advance, you can create more efficient representation \n\n");
    printf("                       by specifying STRICT, SELF_CONTAINED and PRESERVE options in the OPTIONS_MASK\n\n");
    printf("                       Only documents for the specified values for this options will be able to be\n\n");
    printf("                       processed correctly by the EXIP instance.\n\n");
    printf("                       -src=dynamic creates a function that dynamically creates the grammar (Default)\n\n");
    printf("                       -src=static the grammar definitions are defined statically as global variables\n\n");
    printf("           -pfx    :   When in -src mode, this options allows you to specify a unique prefix for the\n\n");
    printf("                       generated global types. The default is \"prfx_\"\n\n");
    printf("           <OPTIONS_MASK>:   <STRICT><SELF_CONTAINED><dtd><prefixes><lexicalValues><comments><pis> := <0|1><0|1><0|1><0|1><0|1><0|1><0|1> \n\n");
    printf("           <schema_in>   :   Source XML schema file \n\n");
    printf("           <schema_out>  :   Destination schema file in the particular format (Default is the standard output) \n\n");
    printf("  Purpose: Manipulation of EXIP schemas\n");
    printf("\n" );
}

size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
}

size_t writeFileOutputStream(void* buf, size_t readSize, void* stream)
{
	FILE *outfile = (FILE*) stream;
	return fwrite(buf, 1, readSize, outfile);
}

static errorCode stringToASCII(char* outBuf, unsigned int bufSize, String inStr)
{
	if(inStr.length >= bufSize)
		return OUT_OF_BOUND_BUFFER;

	memcpy(outBuf, inStr.str, inStr.length);
	outBuf[inStr.length] = '\0';

	return ERR_OK;
}

static void getValueTypeString(char* buf, ValueType vt)
{
	switch(vt.exiType)
	{
		case 1:
			strcpy(buf, "[N/A] ");
			break;
		case 2:
			strcpy(buf, "[str] ");
			break;
		case 3:
			strcpy(buf, "[float] ");
			break;
		case 4:
			strcpy(buf, "[dec] ");
			break;
		case 5:
			strcpy(buf, "[date] ");
			break;
		case 6:
			strcpy(buf, "[bool] ");
			break;
		case 7:
			strcpy(buf, "[bin] ");
			break;
		case 8:
			strcpy(buf, "[list] ");
			break;
		case 9:
			strcpy(buf, "[qname] ");
			break;
		case 10:
			strcpy(buf, "[untyped] ");
			break;
		case 20:
			strcpy(buf, "[int] ");
			break;
		case 21:
			strcpy(buf, "[short] ");
			break;
		case 22:
			strcpy(buf, "[uint] ");
			break;
	}
}
