/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file exipe.c
 * @brief Command-line utility for testing the EXI encoder
 *
 * @date Nov 5, 2012
 * @author Rumen Kyusakov
 * @version 0.4.1
 * @par[Revision] $Id$
 */

#include "encodeTestEXI.h"
#include "grammarGenerator.h"

#define MAX_XSD_FILES_COUNT 10 // up to 10 XSD files

static void printfHelp();
size_t writeFileOutputStream(void* buf, size_t readSize, void* stream);
static void parseSchema(char* xsdList, EXIPSchema* schema);

int main(int argc, char *argv[])
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	FILE *outfile = stdout; // Default is the standard output
	char sourceFile[100];
	EXIPSchema schema;
	EXIPSchema* schemaPtr = NULL;
	int argIndex = 1;

	strcpy(sourceFile, "stdout");

	if(argc >= 2)
	{
		if(strcmp(argv[argIndex], "-help") == 0)
		{
			printfHelp();
			return 0;
		}
		else if(strstr(argv[argIndex], "-schema") != NULL)
		{
			// Schema enabled encoding is requested.
			// All the xsd files should be passed as comma-separated list
			char* xsdList = argv[argIndex] + 7;

			parseSchema(xsdList, &schema);

			schemaPtr = &schema;
			argIndex++;
		}
	}

	if(argIndex < argc)
	{
		strcpy(sourceFile, argv[argIndex]);

		outfile = fopen(sourceFile, "wb" );
		if(!outfile)
		{
			fprintf(stderr, "Unable to open file %s", sourceFile);
			return 1;
		}
	}

	tmp_err_code = encode(schemaPtr, outfile, writeFileOutputStream);

	if(schemaPtr != NULL)
		destroySchema(schemaPtr);

	if(tmp_err_code != ERR_OK)
	{
		printf("\nError (code: %d) during encoding of: %s\n", tmp_err_code, sourceFile);
		exit(1);
	}
	else
	{
		printf("\nSuccessful encoding in %s\n", sourceFile);
		exit(1);
	}

	fclose(outfile);
}

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Copyright (c) 2010 - 2012, EISLAB - Luleå University of Technology Version 0.4 \n");
    printf("           Author: Rumen Kyusakov\n");
    printf("  Usage:   exipe [options] [exi_out]\n\n");
    printf("           Options: [-help | -schema=<xsd_in>] \n");
    printf("           -schema :   uses schema defined in <xsd_in> for encoding. All referenced schema files should be included in <xsd_in>\n");
    printf("           <xsd_in>:   Comma-separated list of schema documents encoded in EXI with Preserve.prefixes. The first schema is the\n");
    printf("                       main one and the rest are schemas that are referenced from the main one through the <xs:import> statement.\n");
    printf("           -help   :   Prints this help message\n\n");
    printf("           exi_out :   output file for the EXI stream (stdout if none specified)\n\n");
    printf("  Purpose: This program tests the EXIP encoding functionality\n");
    printf("\n" );
}

size_t writeFileOutputStream(void* buf, size_t readSize, void* stream)
{
	FILE *outfile = (FILE*) stream;
	return fwrite(buf, 1, readSize, outfile);
}

static void parseSchema(char* xsdList, EXIPSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	FILE *schemaFile;
	BinaryBuffer buffer[MAX_XSD_FILES_COUNT]; // up to 10 XSD files
	char schemaFileName[50];
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
	tmp_err_code = generateSchemaInformedGrammars(buffer, schemaFilesCount, SCHEMA_FORMAT_XSD_EXI, schema);

	for(i = 0; i < schemaFilesCount; i++)
	{
		free(buffer[i].buf);
	}

	if(tmp_err_code != ERR_OK)
	{
		printf("\nGrammar generation error occurred: %d", tmp_err_code);
		exit(1);
	}
}
