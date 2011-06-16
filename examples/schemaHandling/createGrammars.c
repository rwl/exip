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
#include "schema.h"
#include "grammarGenerator.h"
#include <stdio.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 200
#define OUT_EXIP 0
#define OUT_TEXT 1

static void printfHelp();

size_t readFileInputStream(void* buf, size_t readSize, void* stream);
size_t writeFileOutputStream(void* buf, size_t readSize, void* stream);

int main(int argc, char *argv[])
{
	FILE *infile;
	FILE *outfile = stdout;
	char buffer[INPUT_BUFFER_SIZE];
	IOStream inputStrm;
	IOStream outputStrm;
	ExipSchema schema;
	unsigned char outputFormat = OUT_EXIP;
	unsigned int currArgNumber = 1;
	errorCode tmp_err_code = UNEXPECTED_ERROR;

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

		if(outputFormat == OUT_EXIP)
		{
			printf("\n ERROR: EXIP output format is not implemented yet!");
			exit(1);
		}

		{
			unsigned int i;
			unsigned int j;
			size_t r;
			size_t p;
			char printfBuf[100];
			EXIGrammar* tmpGrammar;
			size_t tmp_prod_indx = 0;

			for(i = 0; i < schema.initialStringTables->rowCount; i++)
			{
				for(j = 0; j < schema.initialStringTables->rows[i].lTable->rowCount; j++)
				{
					tmpGrammar = schema.initialStringTables->rows[i].lTable->rows[j].globalGrammar;
					if(tmpGrammar != NULL)
					{
						fwrite("Grammar ", 1, strlen("Grammar "), outfile);
						fwrite(schema.initialStringTables->rows[i].string_val.str, 1, schema.initialStringTables->rows[i].string_val.length, outfile);
						fwrite(":", 1, 1, outfile);
						fwrite(schema.initialStringTables->rows[i].lTable->rows[j].string_val.str, 1, schema.initialStringTables->rows[i].lTable->rows[j].string_val.length, outfile);
						fwrite("\n", 1, 1, outfile);

						for(r = 0; r < tmpGrammar->rulesDimension; r++)
						{
							sprintf(printfBuf, "NT-%d: \n", r);
							fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							for(p = 0; p < tmpGrammar->ruleArray[r].prodCounts[0]; p++)
							{
								tmp_prod_indx = tmpGrammar->ruleArray[r].prodCounts[0] - 1 - p;
								switch(tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].event.eventType)
								{
									case EVENT_SD:
										fwrite("\tSD ", 1, strlen("\tSD "), outfile);
										break;
									case EVENT_ED:
										fwrite("\tED ", 1, strlen("\tED "), outfile);
										break;
									case EVENT_SE_QNAME:
										fwrite("\tSE (", 1, strlen("\tSE ("), outfile);
										fwrite(schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].uriRowID].string_val.str, 1, schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].uriRowID].string_val.length, outfile);
										fwrite(":", 1, 1, outfile);
										fwrite(schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].uriRowID].lTable->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].lnRowID].string_val.str, 1, schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].uriRowID].lTable->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].lnRowID].string_val.length, outfile);
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
										fwrite("\tAT (", 1, strlen("\tAT ("), outfile);
										fwrite(schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].uriRowID].string_val.str, 1, schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].uriRowID].string_val.length, outfile);
										fwrite(":", 1, 1, outfile);
										fwrite(schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].uriRowID].lTable->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].lnRowID].string_val.str, 1, schema.initialStringTables->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].uriRowID].lTable->rows[tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].lnRowID].string_val.length, outfile);
										fwrite(") ", 1, 2, outfile);
										break;
									case EVENT_AT_URI:
										fwrite("\tAT (uri) ", 1, strlen("\tAT (uri) "), outfile);
										break;
									case EVENT_AT_ALL:
										fwrite("\tAT (*) ", 1, strlen("\tAT (*) "), outfile);
										break;
									case EVENT_CH:
										fwrite("\tCH ", 1, strlen("\tCH "), outfile);
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
								sprintf(printfBuf, "\tNT-%u\t", (unsigned int) tmpGrammar->ruleArray[r].prodArrays[0][tmp_prod_indx].nonTermID);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
								sprintf(printfBuf, "%d\n", p);
								fwrite(printfBuf, 1, strlen(printfBuf), outfile);
							}
							fwrite("\n", 1, 1, outfile);
						}
					}
				}
			}
		}

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
    printf("  Usage:   exipSchema [options] <schema_in> [<schema_out>] \n\n");
    printf("           Options: [-help | [-exip | -text] ] \n");
    printf("           -help   :   Prints this help message\n\n");
    printf("           -exip   :   Format the output schema definitions in EXIP-specific format (Default) \n\n");
    printf("           -text   :   Format the output schema definitions in human readable text format \n\n");
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
