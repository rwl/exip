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
 * @file encodeTestEXI.c
 * @brief Testing the EXI encoder
 *
 * @date Nov 4, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */
#include "EXISerializer.h"
#include "procTypes.h"
#include "stringManipulate.h"
#include "schema.h"
#include "grammarGenerator.h"
#include <stdio.h>
#include <string.h>

extern const EXISerializer serEXI;

#define OUTPUT_BUFFER_SIZE 200

#define TEST_HEADER_WITH_OPTIONS TRUE

static void printfHelp();
static void printError(errorCode err_code, EXIStream* strm, FILE *outfile);

size_t writeFileOutputStream(void* buf, size_t readSize, void* stream);

int main(int argc, char *argv[])
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	FILE *outfile;
	char sourceFile[50];
	ExipSchema schema;
	unsigned char hasSchema = FALSE;

	if(argc > 1)
	{
		if(strcmp(argv[1], "-help") == 0)
		{
			printfHelp();
			return 0;
		}
		else if(strcmp(argv[1], "-schema") == 0)
		{
			FILE *schemaFile;
			unsigned long schemaLen;
			char *schemaBuffer;
			char schemaFileName[50];

			strcpy(schemaFileName, argv[2]);

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
				schemaLen=ftell(schemaFile);
				fseek(schemaFile, 0, SEEK_SET);

				//Allocate memory
				schemaBuffer=(char *)malloc(schemaLen+1);
				if (!schemaBuffer)
				{
					fprintf(stderr, "Memory allocation error!");
					fclose(schemaFile);
					return 1;
				}

				//Read file contents into buffer
				fread(schemaBuffer, schemaLen, 1, schemaFile);
				fclose(schemaFile);

				tmp_err_code = generateSchemaInformedGrammars(schemaBuffer, schemaLen, schemaLen, NULL, SCHEMA_FORMAT_XSD_EXI, &schema);

				if(tmp_err_code != ERR_OK)
				{
					printf("\n Error occured: %d", tmp_err_code);
					return 1;
				}

				free(schemaBuffer);
				hasSchema = TRUE;
			}
			strcpy(sourceFile, argv[3]);
		}
		else
		{
			strcpy(sourceFile, argv[1]);
		}
		outfile = fopen(sourceFile, "wb" );
		if(!outfile)
		{
			fprintf(stderr, "Unable to open file %s", sourceFile);
			return 1;
		}
		else
		{
			EXIStream testStrm;
			EXIheader header;
			EXIOptions opts;
			StringType uri;
			StringType ln;
			QName qname= {&uri, &ln};
			StringType chVal;
			char buf[OUTPUT_BUFFER_SIZE];
			IOStream outStrm;

			tmp_err_code = makeDefaultOpts(&opts);
			if(tmp_err_code != ERR_OK)
				printError(tmp_err_code, &testStrm, outfile);

			outStrm.readWriteToStream = writeFileOutputStream;
			outStrm.stream = outfile;

			if(TEST_HEADER_WITH_OPTIONS == TRUE)
			{
				header.has_cookie = TRUE; // Let's try that as well...
				header.has_options = TRUE;
				opts.strict = TRUE;
				opts.valueMaxLength = 300;
				opts.valuePartitionCapacity = 50;
			}
			else
			{
				header.has_cookie = FALSE;
				header.has_options = FALSE;
			}

			header.opts = &opts;
			header.is_preview_version = FALSE;
			header.version_number = 1;

			if(hasSchema == TRUE)
			{
				tmp_err_code = serEXI.initStream(&testStrm, buf, OUTPUT_BUFFER_SIZE, &outStrm, &opts, &schema);
				if(tmp_err_code != ERR_OK)
					printError(tmp_err_code, &testStrm, outfile);
			}
			else
			{
				tmp_err_code = serEXI.initStream(&testStrm, buf, OUTPUT_BUFFER_SIZE, &outStrm, &opts, NULL);
				if(tmp_err_code != ERR_OK)
					printError(tmp_err_code, &testStrm, outfile);
			}

			tmp_err_code += serEXI.exiHeaderSer(&testStrm, &header);

			tmp_err_code += serEXI.startDocumentSer(&testStrm, FALSE, 0);

			tmp_err_code += asciiToString("http://www.ltu.se/EISLAB/schema-test", &uri, &testStrm.memList, FALSE);
			tmp_err_code += asciiToString("EXIPEncoder", &ln, &testStrm.memList, FALSE);
			tmp_err_code += serEXI.startElementSer(&testStrm, &qname, FALSE, 0);

			tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
			tmp_err_code += asciiToString("version", &ln, &testStrm.memList, FALSE);
			tmp_err_code += serEXI.attributeSer(&testStrm, &qname, VALUE_TYPE_STRING, FALSE, 0);

			tmp_err_code += asciiToString("0.2", &chVal, &testStrm.memList, FALSE);
			tmp_err_code += serEXI.stringDataSer(&testStrm, chVal, FALSE, 0);

			tmp_err_code += asciiToString("This is an example of serializing EXI streams using EXIP low level API", &chVal, &testStrm.memList, FALSE);
			tmp_err_code += serEXI.stringDataSer(&testStrm, chVal, FALSE, 0);

			tmp_err_code += serEXI.endElementSer(&testStrm, FALSE, 0);
			tmp_err_code += serEXI.endDocumentSer(&testStrm, FALSE, 0);

			if(tmp_err_code != ERR_OK)
				printError(tmp_err_code, &testStrm, outfile);

			tmp_err_code = serEXI.closeEXIStream(&testStrm);
			fclose(outfile);
		}
	}
	else
	{
		printfHelp();
		return 1;
	}
	return 0;
}

size_t writeFileOutputStream(void* buf, size_t readSize, void* stream)
{
	FILE *outfile = (FILE*) stream;
	return fwrite(buf, 1, readSize, outfile);
}

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Efficient XML Interchange Processor, Rumen Kyusakov, 2011 \n");
    printf("           Copyright (c) 2010 - 2011, EISLAB - Luleå University of Technology Version 0.2 \n");
    printf("  Usage:   exipe [options] <EXI_FileOut>\n\n");
    printf("           Options: [-help | -schema <schema_file_in>] \n");
    printf("           -schema :   uses schema defined in <schema_file_in> for encoding\n");
    printf("           -help   :   Prints this help message\n\n");
    printf("  Purpose: This program tests the EXIP encoding functionality\n");
    printf("\n" );
}

static void printError(errorCode err_code, EXIStream* strm, FILE *outfile)
{
	printf("\n Error occured: %d", err_code);
	serEXI.closeEXIStream(strm);
	fclose(outfile);
	exit(1);
}
