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
static void parseSchema(char** fileNames, unsigned int schemaFilesCount, EXIPSchema* schema);

size_t readFileInputStream(void* buf, size_t readSize, void* stream);

int main(int argc, char *argv[])
{
	FILE *infile;
	char sourceFileName[100];
	EXIPSchema schema;
	EXIPSchema* schemaPtr = NULL;
	unsigned char outFlag = OUT_EXI; // Default output option
	unsigned int argIndex = 1;

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

			argIndex = 2;
		}
		else if(strcmp(argv[argIndex], "-xml") == 0)
		{
			outFlag = OUT_XML;
			if(argc == 2)
			{
				printfHelp();
				return 0;
			}

			argIndex = 2;
		}

		if(strcmp(argv[argIndex], "-schema") == 0)
		{
			if(argc <= argIndex + 2)
			{
				printfHelp();
				return 0;
			}
			else
			{
				unsigned int schemaFilesCount = 0;

				schemaFilesCount = argc - argIndex - 2;

				argIndex++;

				parseSchema(argv + argIndex, schemaFilesCount, &schema);
				schemaPtr = &schema;

				argIndex += schemaFilesCount;
			}
		}

		strcpy(sourceFileName, argv[argIndex]);

		infile = fopen(sourceFileName, "rb" );
		if(!infile)
		{
			fprintf(stderr, "Unable to open file %s", sourceFileName);
			return 1;
		}
		else
		{
			errorCode tmp_err_code = UNEXPECTED_ERROR;
			tmp_err_code = decode(schemaPtr, outFlag, infile, readFileInputStream);

			if(schemaPtr != NULL)
				destroySchema(schemaPtr);
			fclose(infile);

			if(tmp_err_code != ERR_OK)
			{
				printf("\nError (code: %d) during parsing of the EXI stream: %s", tmp_err_code, sourceFileName);
				return 1;
			}
			else
			{
				printf("\nSuccessful parsing of the EXI stream: %s", sourceFileName);
				return 0;
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
    printf("  EXIP     Copyright (c) 2010 - 2012, EISLAB - Luleå University of Technology Version 0.4 \n");
    printf("           Authors: Rumen Kyusakov\n");
    printf("  Usage:   exipd [options] <EXI_FileIn>\n\n");
    printf("           Options: [-help | [ -xml | -exi ] -schema <schema_files_in>] \n");
    printf("           -schema :   uses schema defined in <schema_files_in> for decoding\n");
    printf("           -exi    :   EXI formated output [default]\n");
    printf("           -xml    :   XML formated output\n");
    printf("           -help   :   Prints this help message\n\n");
    printf("           <schema_files_in>  :   space separated list of XSD files. The first XSD file should be the primary schema file\n\n");
    printf("  Purpose: This program tests the EXIP decoding functionality\n");
    printf("\n" );
}

size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
}

static void parseSchema(char** fileNames, unsigned int schemaFilesCount, EXIPSchema* schema)
{
	FILE *schemaFile;
	BinaryBuffer buffer[MAX_XSD_FILES_COUNT];
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i;

	if(schemaFilesCount > MAX_XSD_FILES_COUNT)
	{
		fprintf(stderr, "Too many xsd files given as an input: %d", schemaFilesCount);
		exit(1);
	}

	for(i = 0; i < schemaFilesCount; i++)
	{
		schemaFile = fopen(fileNames[i], "rb" );
		if(!schemaFile)
		{
			fprintf(stderr, "Unable to open file %s", fileNames[i]);
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

	tmp_err_code = generateSchemaInformedGrammars(buffer, schemaFilesCount, SCHEMA_FORMAT_XSD_EXI, schema);
	if(tmp_err_code != ERR_OK)
	{
		printf("\n Error occured: %d", tmp_err_code);
		exit(1);
	}

	for(i = 0; i < schemaFilesCount; i++)
	{
		free(buffer[i].buf);
	}
}
