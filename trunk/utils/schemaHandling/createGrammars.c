/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file createGrammars.c
 * @brief Generate EXI grammars from XML schema definition and stores them in EXIP format
 *
 * @date Oct 13, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id: decodeTestEXI.c 93 2011-03-30 15:39:41Z kjussakov $
 */

#include "procTypes.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"
#include "memManagement.h"
#include "grammarAugment.h"
#include "schemaOutputUtils.h"
#include "hashtable.h"
#include "sTables.h"
#include "grammars.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_GRAMMARS_COUNT 5000
#define INPUT_BUFFER_SIZE 200
#define OUT_EXIP     0
#define OUT_TEXT     1
#define OUT_SRC_DYN  2
#define OUT_SRC_STAT 3

static void printfHelp();

size_t readFileInputStream(void* buf, size_t readSize, void* stream);
size_t writeFileOutputStream(void* buf, size_t readSize, void* stream);

int main(int argc, char *argv[])
{
	FILE *infile;
	FILE *outfile = stdout;
	char buf[INPUT_BUFFER_SIZE];
	BinaryBuffer inBuffer;
	BinaryBuffer outBuffer;
	EXIPSchema schema;
	unsigned char outputFormat = OUT_EXIP;
	int currArgNumber = 1;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	char prefix[20];

	unsigned char mask_specified = FALSE;
	unsigned char mask_strict = FALSE;
	unsigned char mask_sc = FALSE;
	unsigned char mask_preserve = 0;

	inBuffer.buf = buf;
	inBuffer.bufLen = INPUT_BUFFER_SIZE;
	inBuffer.bufContent = 0;
	inBuffer.ioStrm.readWriteToStream = readFileInputStream;
	outBuffer.ioStrm.readWriteToStream = writeFileOutputStream;
	outBuffer.ioStrm.stream = outfile;

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
		inBuffer.ioStrm.stream = infile;

		if(argc > currArgNumber + 1)
		{
			outfile = fopen(argv[currArgNumber + 1], "wb" );
			if(!outfile)
			{
				fprintf(stderr, "Unable to open file %s", argv[currArgNumber + 1]);
				return 1;
			}
			outBuffer.ioStrm.stream = outfile;
		}

		tmp_err_code = generateSchemaInformedGrammars(&inBuffer, 1, SCHEMA_FORMAT_XSD_EXI, &schema);
		if(tmp_err_code != ERR_OK)
		{
			printf("\n Error occured: %d", tmp_err_code);
			exit(1);
		}
		fclose(infile);

		{
			EXIGrammar* tmpGrammar;
			AllocList memList;
			time_t now;

			if(ERR_OK != initAllocList(&memList))
			{
				printf("unexpected error!");
				exit(1);
			}

			time(&now);
			fprintf(outfile, "/** AUTO-GENERATED: %.24s\n  * Copyright (c) 2010 - 2011, Rumen Kyusakov, EISLAB, LTU\n  * $Id$ */\n\n",  ctime(&now));

			if(outputFormat == OUT_EXIP)
			{
				printf("\n ERROR: EXIP output format is not implemented yet!");
				exit(1);
			}
			else if(outputFormat == OUT_SRC_DYN)
			{
				// TODO: All strings should be defined with the function asciiToString()
				//		 also, some common strings should be defined just once in a common location

				fprintf(outfile, "#include \"procTypes.h\"\n");
				fprintf(outfile, "#include \"memManagement.h\"\n");
				fprintf(outfile, "#include \"stringManipulate.h\"\n\n");

				if(ERR_OK != dynExipSchemaOutput(&schema, prefix, mask_specified, mask_strict,
						mask_sc, mask_preserve, outfile))
				{
					printf("\n ERROR: OUT_SRC_DYN output format!");
					exit(1);
				}
			}

/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/

			else if(outputFormat == OUT_SRC_STAT)
			{
                Index count;
				Index uriId;
				Index stId, stIdMax;
				Index grIter;

				// TODO: revise this comments!
				// NOTE: Do not use without option mask! Also when strict == FALSE the memPairs are NULL which will create errors
				// When there is no mask specified this is not correct if the schema is used more than once
				// There is extra rule slot for each grammar to be use if
				// strict == FALSE by addUndeclaredProductions() when no mask is specified

				fprintf(outfile, "#include \"procTypes.h\"\n\n");
				fprintf(outfile, "#define CONST\n\n");

				fprintf(outfile, "extern Production static_prod_start_doc_part0[1];\n");
				fprintf(outfile, "extern Production static_prod_doc_end_part0[1];\n\n");

				staticStringTblDefsOutput(&schema.uriTable, prefix, outfile);

				for(grIter = 0; grIter < schema.grammarTable.count; grIter++)
				{
					tmpGrammar = &schema.grammarTable.grammar[grIter];
					if(mask_specified == TRUE)
					{
						if(ERR_OK != addUndeclaredProductions(&memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, &schema.simpleTypeTable))
						{
							printf("\n ERROR: OUT_SRC_STAT output format!");
							exit(1);
						}
						if(ERR_OK != augmentDocGrammar(&memList, mask_preserve, &schema.docGrammar))
						{
							printf("\n ERROR: OUT_SRC_STAT output format!");
							exit(1);
						}
					}
					staticProductionsOutput(&schema.grammarTable.grammar[grIter], prefix, grIter, outfile);
					staticRulesOutput(&schema.grammarTable.grammar[grIter], prefix, grIter, mask_specified, outfile);
				}

				/* The array of schema-informed EXI grammars in the EXIPSchema object */
				fprintf(outfile, "static CONST EXIGrammar %sgrammarTable[%d] =\n{\n", prefix, (int) schema.grammarTable.count);
				for(grIter = 0; grIter < schema.grammarTable.count; grIter++)
				{
					tmpGrammar = &schema.grammarTable.grammar[grIter];
					fprintf(outfile,"   {%srule_%d, %d, 0x%02x, %d},\n",
							prefix,
							(int) grIter,
							(int) tmpGrammar->count,
							(int) tmpGrammar->props,
							(int) tmpGrammar->contentIndex);
				}

				fprintf(outfile, "};\n\n");

				/* Build the Prefix and LN table structures */
				for(uriId = 0; uriId < schema.uriTable.count; uriId++)
				{
					/* Prefix table */
					staticPrefixOutput(schema.uriTable.uri[uriId].pfxTable, prefix, uriId, outfile);
					/* Ln table */
					staticLnEntriesOutput(&schema.uriTable.uri[uriId].lnTable, prefix, uriId, outfile);
				}

				/* Build the URI table structure */
				staticUriTableOutput(&schema.uriTable, prefix, outfile);

				/* Build the document grammar */
				staticDocGrammarOutput(&schema.docGrammar, prefix, outfile);

				/* Build the simple types structure */
				fprintf(outfile, "static CONST SimpleType %ssimpleTypes[%d] =\n{\n", prefix, (int) schema.simpleTypeTable.count);

                if(schema.simpleTypeTable.sType != NULL)
				{
					stIdMax = schema.simpleTypeTable.count;
					for(stId = 0; stId < stIdMax; stId++)
					{
						uint32_t maxMS, maxLS, minMS, minLS;

						maxMS = (uint32_t)(schema.simpleTypeTable.sType[stId].max >> 32);
						maxLS = (uint32_t)(schema.simpleTypeTable.sType[stId].max & 0xFFFFFFFF);
						minMS = (uint32_t)(schema.simpleTypeTable.sType[stId].min >> 32);
						minLS = (uint32_t)(schema.simpleTypeTable.sType[stId].min & 0xFFFFFFFF);

						fprintf(outfile,
								"    {%d, 0x%04x, 0x%08x%08x, 0x%08x%08x, %d}%s",
								schema.simpleTypeTable.sType[stId].exiType,
								schema.simpleTypeTable.sType[stId].facetPresenceMask,
								maxMS,
								maxLS,
								minMS,
								minLS,
								schema.simpleTypeTable.sType[stId].length,
								stId==(stIdMax-1) ? "\n};\n\n" : ",\n");
					}
				}

                /* Enum table entries */
                staticEnumTableOutput(&schema, prefix, outfile);

				/* Finally, build the schema structure */
				fprintf(outfile,
                        "CONST EXIPSchema %sschema =\n{\n",
                        prefix);

                
				fprintf(outfile, "    {NULL, NULL},\n");

                count = schema.uriTable.count;
				fprintf(outfile,
                        "    {{sizeof(UriEntry), %d, %d}, %suriEntry, %d},\n",
                        (int) count,
                        (int) count,
                        prefix,
                        (int) count);

				fprintf(outfile,
                        "    {%sdocGrammarRule, %d, %d, %d},\n",
                        prefix,
                        (int) schema.docGrammar.count,
                        (int) schema.docGrammar.props,
                        (int) schema.docGrammar.contentIndex);

                count = schema.simpleTypeTable.count;
				fprintf(outfile,
                        "    {{sizeof(SimpleType), %d, %d}, %ssimpleTypes, %d},\n",
                        (int) count,
                        (int) count,
                        prefix,
                        (int) count);

                count = schema.grammarTable.count;
				fprintf(outfile,
                        "    {{sizeof(EXIGrammar), %d, %d}, %sgrammarTable, %d},\n    %d,\n",
                        (int) count,
                        (int) count,
                        prefix,
                        (int) count,
                        (int) count);

                count = schema.enumTable.count;
				fprintf(outfile,
                        "    {{sizeof(EnumDefinition), %d, %d}, %s%s, %d}\n};\n\n",

                        (int) count,
                        (int) count,
                        count == 0?"":prefix, count == 0?"NULL":"enumTable",
						(int) count);

			}
            
/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
            
			else if(outputFormat == OUT_TEXT)
			{
				QNameID qnameId = {0, 0};
				fprintf(outfile, "\n/** GLOBAL ELEMENT GRAMMARS **/\n\n");
				for(qnameId.uriId = 0; qnameId.uriId < schema.uriTable.count; qnameId.uriId++)
				{
					for(qnameId.lnId = 0; qnameId.lnId < schema.uriTable.uri[qnameId.uriId].lnTable.count; qnameId.lnId++)
					{
						tmpGrammar = GET_ELEM_GRAMMAR_QNAMEID(&schema, qnameId);
						if(tmpGrammar != NULL)
						{
							if(mask_specified == TRUE)
							{
								if(ERR_OK != addUndeclaredProductions(&memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, &schema.simpleTypeTable))
								{
									printf("\n ERROR: OUT_TEXT output format!");
									exit(1);
								}
							}
							if(ERR_OK != textGrammarOutput(qnameId, GET_LN_URI_QNAME(schema.uriTable, qnameId).elemGrammar, tmpGrammar, &schema, outfile))
							{
								printf("\n ERROR: OUT_TEXT output format!");
								exit(1);
							}
						}
					}
				}

				fprintf(outfile, "\n/** GLOBAL TYPE GRAMMARS **/\n\n");
				for(qnameId.uriId = 0; qnameId.uriId < schema.uriTable.count; qnameId.uriId++)
				{
					for(qnameId.lnId = 0; qnameId.lnId < schema.uriTable.uri[qnameId.uriId].lnTable.count; qnameId.lnId++)
					{
						tmpGrammar = GET_TYPE_GRAMMAR_QNAMEID(&schema, qnameId);
						if(tmpGrammar != NULL)
						{
							if(mask_specified == TRUE && !IS_AUGMENTED(tmpGrammar->props))
							{
								if(ERR_OK != addUndeclaredProductions(&memList, mask_strict, mask_sc, mask_preserve, tmpGrammar, &schema.simpleTypeTable))
								{
									printf("\n ERROR: OUT_TEXT output format!");
									exit(1);
								}
							}
							if(ERR_OK != textGrammarOutput(qnameId, GET_LN_URI_QNAME(schema.uriTable, qnameId).typeGrammar, tmpGrammar, &schema, outfile))
							{
								printf("\n ERROR: OUT_TEXT output format!");
								exit(1);
							}
						}
					}
				}
			}
			freeAllocList(&memList);
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
