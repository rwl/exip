/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file exipg.c
 * @brief Command-line utility for generating EXI grammars from XML schema definition
 *
 * @date Nov 5, 2012
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id: exipg.c 328 2013-10-30 16:00:10Z kjussakov $
 */

#include "createGrammars.h"
#include "grammarGenerator.h"

#define MAX_XSD_FILES_COUNT 10 // up to 10 XSD files
#define OUT_EXIP     0
#define OUT_TEXT     1
#define OUT_SRC_DYN  2
#define OUT_SRC_STAT 3

static void printfHelp();
static void parseSchema(char* xsdList, EXIPSchema* schema, unsigned char mask, EXIOptions maskOpt);

int main(int argc, char *argv[])
{
	FILE *outfile = stdout; // Default is the standard output
	EXIPSchema schema;
	unsigned char outputFormat = OUT_EXIP;
	int argIndex = 1;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	char prefix[20];
	unsigned char mask = FALSE;
	EXIOptions maskOpt;

	makeDefaultOpts(&maskOpt);

	if(argc == 1)
	{
		printfHelp();
		return 0;
	}

	if(strcmp(argv[argIndex], "-help") == 0)
	{
		printfHelp();
		return 0;
	}
	else if(strcmp(argv[argIndex], "-exip") == 0)
	{
		outputFormat = OUT_EXIP;
		argIndex++;
	}
	else if(strcmp(argv[argIndex], "-text") == 0)
	{
		outputFormat = OUT_TEXT;
		argIndex++;
	}
	else if(strcmp(argv[argIndex], "-dynamic") == 0)
	{
		outputFormat = OUT_SRC_DYN;
		argIndex++;
	}
	else if(strcmp(argv[argIndex], "-static") == 0)
	{
		outputFormat = OUT_SRC_STAT;
		argIndex++;
	}

	if(argc <= argIndex)
	{
		printfHelp();
		return 0;
	}

	if(argv[argIndex][0] == '-' &&
	   argv[argIndex][1] == 'p' &&
	   argv[argIndex][2] == 'f' &&
	   argv[argIndex][3] == 'x' &&
	   argv[argIndex][4] == '=')
	{
		strcpy(prefix, argv[argIndex] + 5);
		argIndex++;
	}
	else
	{
		strcpy(prefix, "prfx_"); // The default prefix
	}

	if(argc <= argIndex)
	{
		printfHelp();
		return 0;
	}

	if(argv[argIndex][0] == '-' &&
	   argv[argIndex][1] == 'o' &&
	   argv[argIndex][2] == 'p' &&
	   argv[argIndex][3] == 's' &&
	   argv[argIndex][4] == '=')
	{
		mask = TRUE;
		if(argv[argIndex][5] == '1')
			SET_STRICT(maskOpt.enumOpt);

		if(argv[argIndex][6] == '1')
			SET_SELF_CONTAINED(maskOpt.enumOpt);

		if(argv[argIndex][7] == '1')
			SET_PRESERVED(maskOpt.preserve, PRESERVE_DTD);

		if(argv[argIndex][8] == '1')
			SET_PRESERVED(maskOpt.preserve, PRESERVE_PREFIXES);

		if(argv[argIndex][9] == '1')
			SET_PRESERVED(maskOpt.preserve, PRESERVE_LEXVALUES);

		if(argv[argIndex][10] == '1')
			SET_PRESERVED(maskOpt.preserve, PRESERVE_COMMENTS);

		if(argv[argIndex][11] == '1')
			SET_PRESERVED(maskOpt.preserve, PRESERVE_PIS);

		argIndex++;
	}

	if(argc <= argIndex)
	{
		printfHelp();
		return 0;
	}

	if(strstr(argv[argIndex], "-schema") != NULL)
	{
		char *xsdList = argv[argIndex] + 7;

		parseSchema(xsdList, &schema, mask, maskOpt);

		argIndex += 1;
	}
	else
	{
		printfHelp();
		return 0;
	}

	if(argc > argIndex)
	{
		outfile = fopen(argv[argIndex], "wb" );
		if(!outfile)
		{
			fprintf(stderr, "Unable to open file %s", argv[argIndex]);
			return 1;
		}
		argIndex += 1;
	}

	switch(outputFormat)
	{
		case OUT_TEXT:
			tmp_err_code = toText(&schema, outfile);
		break;
		case OUT_EXIP:
			tmp_err_code = toEXIP(&schema, outfile);
		break;
		case OUT_SRC_STAT:
			tmp_err_code = toStaticSrc(&schema, prefix, outfile);
		break;
		case OUT_SRC_DYN:
			tmp_err_code = toDynSrc(&schema, outfile);
		break;
		default:
			printf("\nUnsupported output format!");
			exit(1);
		break;
	}

	destroySchema(&schema);

	fclose(outfile);

	if(tmp_err_code != EXIP_OK)
	{
		printf("\nError during grammar output!");
		exit(1);
	}

	return 0;
}

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Copyright (c) 2010 - 2012, EISLAB - Luleå University of Technology Version 0.5.1 \n");
    printf("           Author: Rumen Kyusakov\n");
    printf("  Usage:   exipg [options] -schema=<xsd_in> [grammar_out] \n\n");
    printf("           Options: [-help | [[-exip | -text | -dynamic | -static] [-pfx=<prefix>] [-ops=<ops_mask>]] ] \n");
    printf("           -help        :   Prints this help message\n");
    printf("           -exip        :   Format the output schema definitions in EXIP-specific format (Default)\n");
    printf("           -text        :   Format the output schema definitions in human readable text format\n");
    printf("           -dynamic     :   Create C code for the grammars defined. The output is a C function that dynamically generates the grammars\n");
    printf("           -static      :   Create C code for the grammars defined. The output is C structures describing the grammars\n");
    printf("           -pfx         :   When in -dynamic or -static mode, this option allows you to specify a unique prefix for the\n");
    printf("                            generated global types. The default is \"prfx_\"\n");
    printf("           ops_mask     :   The format is: <STRICT><SELF_CONTAINED><dtd><prefixes><lexicalValues><comments><pis> := <0|1><0|1><0|1><0|1><0|1><0|1><0|1>\n");
    printf("                            Use this argument only for specifying out-of-band options. That is if no options are specified in the header of the <xsd_in>\n");
    printf("                            EXI encoded schema files but some options are used during encoding.\n");
    printf("                            This argument is useful for generating the \"EXI Options\" grammar where STRICT is set and the rest are default options. \n");
    printf("                            In this way the bootstrapping of the code is easier. The mask to use for EXIOptions-xsd.exi is -ops=0001000 \n");
    printf("           -schema      :   The source schema definitions - all referenced schema files should be included in <xsd_in>\n");
    printf("           xsd_in       :   Comma-separated list of schema documents encoded in EXI with Preserve.prefixes. The first schema is the main one and\n");
    printf("                            the rest are schemas that are referenced from the main one through the <xs:import> statement.\n");
    printf("           grammar_out  :   Destination file for the grammar output (Default is the standard output) \n\n");
    printf("  Purpose: Manipulation of EXIP schemas\n");
    printf("\n" );
}

static void parseSchema(char* xsdList, EXIPSchema* schema, unsigned char mask, EXIOptions maskOpt)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	FILE *schemaFile;
	BinaryBuffer buffer[MAX_XSD_FILES_COUNT]; // up to 10 XSD files
	char schemaFileName[50];
	unsigned int schemaFilesCount = 0;
	unsigned int i;
	char *token;
	EXIOptions* opt = NULL;

	if(mask)
		opt = &maskOpt;

	for (token = strtok(xsdList, "=,"), i = 0; token != NULL; token = strtok(NULL, "=,"), i++)
	{
		schemaFilesCount++;
		if(schemaFilesCount > MAX_XSD_FILES_COUNT)
		{
			fprintf(stderr, "Too many xsd files given as an input: %d", schemaFilesCount);
			exit(1);
		}

		strcpy(schemaFileName, token);
		schemaFile = fopen(schemaFileName, "rb" );
		if(!schemaFile)
		{
			fprintf(stderr, "Unable to open file %s", schemaFileName);
			exit(1);
		}
		else
		{
			//Get file length
			fseek(schemaFile, 0, SEEK_END);
			buffer[i].bufLen = ftell(schemaFile) + 1;
			fseek(schemaFile, 0, SEEK_SET);

			//Allocate memory
			buffer[i].buf = (char *) malloc(buffer[i].bufLen);
			if (!buffer[i].buf)
			{
				fprintf(stderr, "Memory allocation error!");
				fclose(schemaFile);
				exit(1);
			}

			//Read file contents into buffer
			fread(buffer[i].buf, buffer[i].bufLen, 1, schemaFile);
			fclose(schemaFile);

			buffer[i].bufContent = buffer[i].bufLen;
			buffer[i].ioStrm.readWriteToStream = NULL;
			buffer[i].ioStrm.stream = NULL;
		}
	}

	// Generate the EXI grammars based on the schema information
	tmp_err_code = generateSchemaInformedGrammars(buffer, schemaFilesCount, SCHEMA_FORMAT_XSD_EXI, opt, schema, NULL);

	for(i = 0; i < schemaFilesCount; i++)
	{
		free(buffer[i].buf);
	}

	if(tmp_err_code != EXIP_OK)
	{
		printf("\nGrammar generation error occurred: %d", tmp_err_code);
		exit(1);
	}
}
