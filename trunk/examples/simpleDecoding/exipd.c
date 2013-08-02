/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file exipd.c
 * @brief Command-line utility
 *
 * @date Nov 5, 2012
 * @author Rumen Kyusakov
 * @version 0.4.1
 * @par[Revision] $Id$
 */

#include "decodeTestEXI.h"
#include "grammarGenerator.h"

#define MAX_XSD_FILES_COUNT 10 // up to 10 XSD files

static void printfHelp();
static void parseSchema(char* xsdList, EXIPSchema* schema);

size_t readFileInputStream(void* buf, size_t readSize, void* stream);

int main(int argc, char *argv[])
{
	FILE *infile = stdin;
	char sourceFileName[500];
	EXIPSchema schema;
	EXIPSchema* schemaPtr = NULL;
	unsigned char outFlag = OUT_EXI; // Default output option
	unsigned int argIndex = 1;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	strcpy(sourceFileName, "stdin");

	if(argc > 1)
	{
		if(strcmp(argv[argIndex], "-help") == 0)
		{
			printfHelp();
			return 0;
		}
		else if(strcmp(argv[argIndex], "-exi") == 0)
		{
			outFlag = OUT_EXI;
			if(argc == 2)
			{
				printfHelp();
				return 0;
			}

			argIndex += 1;
		}
		else if(strcmp(argv[argIndex], "-xml") == 0)
		{
			outFlag = OUT_XML;
			if(argc == 2)
			{
				printfHelp();
				return 0;
			}

			argIndex += 1;
		}

		if(strstr(argv[argIndex], "-schema") != NULL)
		{
			char *xsdList = argv[argIndex] + 7;

			parseSchema(xsdList, &schema);
			schemaPtr = &schema;

			argIndex += 1;
		}
	}

	if(argIndex < argc)
	{
		strcpy(sourceFileName, argv[argIndex]);

		infile = fopen(sourceFileName, "rb" );
		if(!infile)
		{
			fprintf(stderr, "Unable to open file %s\n", sourceFileName);
			exit(1);
		}
	}

	tmp_err_code = decode(schemaPtr, outFlag, infile, readFileInputStream);

	if(schemaPtr != NULL)
		destroySchema(schemaPtr);
	fclose(infile);

	if(tmp_err_code != EXIP_ERR_OK)
	{
		printf("\nError (code: %d) during parsing of the EXI stream: %s\n", tmp_err_code, sourceFileName);
		return 1;
	}
	else
	{
		printf("\nSuccessful parsing of the EXI stream: %s\n", sourceFileName);
		return 0;
	}
}

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Copyright (c) 2010 - 2012, EISLAB - Luleå University of Technology Version 0.4 \n");
    printf("           Author: Rumen Kyusakov\n");
    printf("  Usage:   exipd [options] [exi_in]\n\n");
    printf("           Options: [-help | [ -xml | -exi ] -schema=<xsd_in>] \n");
    printf("           -schema :   uses schema defined in <xsd_in> for decoding. All referenced schema files should be included in <xsd_in>\n");
    printf("           <xsd_in>:   Comma-separated list of schema documents encoded in EXI with Preserve.prefixes. The first schema is the\n");
    printf("                       main one and the rest are schemas that are referenced from the main one through the <xs:import> statement.\n");
    printf("           -exi    :   EXI formated output [default]\n");
    printf("           -xml    :   XML formated output\n");
    printf("           -help   :   Prints this help message\n\n");
    printf("           exi_in  :   input file containing the EXI stream (stdin if none specified)\n\n");
    printf("  Purpose: This program tests the EXIP decoding functionality. The result is printed on the stdout.\n");
    printf("\n" );
}

size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
}

static void parseSchema(char* xsdList, EXIPSchema* schema)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	FILE *schemaFile;
	BinaryBuffer buffer[MAX_XSD_FILES_COUNT]; // up to 10 XSD files
	char schemaFileName[500];
	unsigned int schemaFilesCount = 0;
	unsigned int i;
	char *token;

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
	tmp_err_code = generateSchemaInformedGrammars(buffer, schemaFilesCount, SCHEMA_FORMAT_XSD_EXI, NULL, schema, NULL);

	for(i = 0; i < schemaFilesCount; i++)
	{
		free(buffer[i].buf);
	}

	if(tmp_err_code != EXIP_ERR_OK)
	{
		printf("\nGrammar generation error occurred: %d", tmp_err_code);
		exit(1);
	}
}
