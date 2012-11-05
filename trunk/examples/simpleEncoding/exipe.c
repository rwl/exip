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

int main(int argc, char *argv[])
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	FILE *outfile;
	char sourceFile[50];
	EXIPSchema schema;
	EXIPSchema* schemaPtr = NULL;

	if(argc > 1)
	{
		if(strcmp(argv[1], "-help") == 0)
		{
			printfHelp();
			return 0;
		}
		else if(strcmp(argv[1], "-schema") == 0)
		{
			// Schema enables encoding is requested.
			// All the xsd files should be passed as arguments to exipe
			FILE *schemaFile;
 			BinaryBuffer buffer[MAX_XSD_FILES_COUNT]; // up to 10 XSD files
			char schemaFileName[50];
			unsigned int schemaFilesCount = 0;
			unsigned int i;

			if(argc <= 3)
			{
				printfHelp();
				return 0;
			}
			else
				schemaFilesCount = argc - 3;

			if(schemaFilesCount > MAX_XSD_FILES_COUNT)
			{
				fprintf(stderr, "Too many xsd files given as an input: %d", schemaFilesCount);
				exit(1);
			}

			for(i = 0; i < schemaFilesCount; i++)
			{
				strcpy(schemaFileName, argv[2 + i]);

				schemaFile = fopen(schemaFileName, "rb" );
				if(!schemaFile)
				{
					fprintf(stderr, "Unable to open file %s", schemaFileName);
					return 1;
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
						return 1;
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
			tmp_err_code = generateSchemaInformedGrammars(buffer, schemaFilesCount, SCHEMA_FORMAT_XSD_EXI, &schema);

			schemaPtr = &schema;
			for(i = 0; i < schemaFilesCount; i++)
			{
				free(buffer[i].buf);
			}

			if(tmp_err_code != ERR_OK)
			{
				printf("\n Error occured: %d", tmp_err_code);
				return 1;
			}
		}

		strcpy(sourceFile, argv[argc - 1]);

		outfile = fopen(sourceFile, "wb" );
		if(!outfile)
		{
			fprintf(stderr, "Unable to open file %s", sourceFile);
			return 1;
		}
		else
		{
			tmp_err_code = encode(schemaPtr, outfile, writeFileOutputStream);

			if(schemaPtr != NULL)
				destroySchema(schemaPtr);
			fclose(outfile);

			if(tmp_err_code != ERR_OK)
			{
				printf("\nError occurred: %d\n", tmp_err_code);
				exit(1);
			}
			else
			{
				printf("\nSuccessful encoding in %s\n", sourceFile);
				exit(1);
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
    printf("  Usage:   exipe [options] <EXI_FileOut>\n\n");
    printf("           Options: [-help | -schema <schema_files_in>] \n");
    printf("           -schema :   uses schema defined in <schema_files_in> for encoding\n");
    printf("           -help   :   Prints this help message\n\n");
    printf("           <schema_files_in>  :   space separated list of XSD files. The first XSD file should be the primary schema file\n\n");
    printf("  Purpose: This program tests the EXIP encoding functionality\n");
    printf("\n" );
}

size_t writeFileOutputStream(void* buf, size_t readSize, void* stream)
{
	FILE *outfile = (FILE*) stream;
	return fwrite(buf, 1, readSize, outfile);
}
