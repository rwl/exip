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

extern EXISerializer serEXI;

static void printfHelp();
static void printError(errorCode err_code, EXIStream* strm, FILE *outfile);

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
				EXIStream strm;

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

				tmp_err_code = generateSchemaInformedGrammars(schemaBuffer, schemaLen, SCHEMA_FORMAT_XSD_EXI,
														&strm, &schema);

				if(tmp_err_code != ERR_OK)
				{
					printf("\n Error occured: %d", tmp_err_code);
					serEXI.closeEXIStream(&strm);
					return 1;
				}
				// TODO: needs to figure out how to allocate the strings and other needed memory on a separate EXIStream
				serEXI.closeEXIStream(&strm);

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
			struct EXIOptions opts;
			StringType uri;
			StringType ln;
			QName testElQname= {&uri, &ln};
			QName testAtQname= {&uri, &ln};
			StringType attVal;
			StringType chVal;
			
			tmp_err_code = makeDefaultOpts(&opts);
			if(tmp_err_code != ERR_OK)
				printError(tmp_err_code, &testStrm, outfile);

			if(hasSchema == TRUE)
			{
				tmp_err_code = serEXI.initStream(&testStrm, 200, &opts, &schema);
				if(tmp_err_code != ERR_OK)
					printError(tmp_err_code, &testStrm, outfile);
			}
			else
			{
				tmp_err_code = serEXI.initStream(&testStrm, 200, &opts, NULL);
				if(tmp_err_code != ERR_OK)
					printError(tmp_err_code, &testStrm, outfile);
			}
			header.has_cookie = 0;
			header.has_options = 0;
			header.is_preview_version = 0;
			header.version_number = 1;
			tmp_err_code += serEXI.exiHeaderSer(&testStrm, &header);



			tmp_err_code += serEXI.startDocumentSer(&testStrm);

			tmp_err_code += asciiToString("", &uri, &testStrm);
			tmp_err_code += asciiToString("EXIPEncoder", &ln, &testStrm);
			tmp_err_code += serEXI.startElementSer(&testStrm, testElQname);

			tmp_err_code += asciiToString("version", &ln, &testStrm);
			tmp_err_code += serEXI.attributeSer(&testStrm, testAtQname);

			tmp_err_code += asciiToString("0.1", &attVal, &testStrm);
			tmp_err_code += serEXI.stringDataSer(&testStrm, attVal);

			tmp_err_code += asciiToString("This is an example of serializing EXI streams using EXIP low level API", &chVal, &testStrm);
			tmp_err_code += serEXI.stringDataSer(&testStrm, chVal);

			tmp_err_code += serEXI.endElementSer(&testStrm);
			tmp_err_code += serEXI.endDocumentSer(&testStrm);

			if(tmp_err_code != ERR_OK)
				printError(tmp_err_code, &testStrm, outfile);

			if(fwrite(testStrm.buffer, sizeof(char), testStrm.bufferIndx+1, outfile) < testStrm.bufferIndx+1)
			{
				fprintf(stderr, "Error writing to the file");
				fclose(outfile);
				return 1;
			}

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

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Efficient XML Interchange Processor, Rumen Kyusakov, 13.10.2010 \n");
    printf("           Copyright (c) 2010, EISLAB - Luleå University of Technology Version 0.1 \n");
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
