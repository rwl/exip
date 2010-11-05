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
#include <stdio.h>
#include <string.h>

/**
 * The handler to be used by the applications to serialize EXI streams
 */
EXISerializer serEXI = {startDocumentSer,
						endDocumentSer,
						startElementSer,
						endElementSer,
						attributeSer,
						intDataSer,
						bigIntDataSer,
						booleanDataSer,
						stringDataSer,
						floatDataSer,
						bigFloatDataSer,
						binaryDataSer,
						dateTimeDataSer,
						decimalDataSer,
						bigDecimalDataSer,
						processingInstructionSer,
						initStream,
						encodeHeader,
						selfContainedSer};

static void printfHelp();

int main(int argc, char *argv[])
{
	FILE *outfile;
	char sourceFile[50];

	if(argc > 1)
	{
		if(strcmp(argv[1], "-help") == 0)
		{
			printfHelp();
			return 0;
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
			errorCode tmp_err_code = UNEXPECTED_ERROR;
			EXIStream testStrm;
			tmp_err_code = serEXI.initStream(&testStrm, 200);

			EXIheader header;
			header.has_cookie = 0;
			header.has_options = 0;
			header.is_preview_version = 0;
			header.version_number = 1;
			tmp_err_code = serEXI.exiHeaderSer(&testStrm, &header);

			tmp_err_code = serEXI.startDocumentSer(&testStrm);
			StringType uri;
			StringType ln;
			tmp_err_code = asciiToString("", &uri, &(testStrm.memStack));
			tmp_err_code = asciiToString("EXIPEncoder", &ln, &(testStrm.memStack));
			QName testElQname = {&uri, &ln};
			tmp_err_code = serEXI.startElementSer(&testStrm, testElQname);

			tmp_err_code = asciiToString("version", &ln, &(testStrm.memStack));
			QName testAtQname = {&uri, &ln};
			tmp_err_code = serEXI.attributeSer(&testStrm, testAtQname);

			StringType attVal;
			tmp_err_code = asciiToString("0.1", &attVal, &(testStrm.memStack));
			tmp_err_code = serEXI.stringDataSer(&testStrm, attVal);

			StringType chVal;
			tmp_err_code = asciiToString("This is an example of serializing EXI streams using EXIP low level API", &chVal, &(testStrm.memStack));
			tmp_err_code = serEXI.stringDataSer(&testStrm, chVal);

			tmp_err_code = serEXI.endElementSer(&testStrm);
			tmp_err_code = serEXI.endDocumentSer(&testStrm);

			if(fwrite(testStrm.buffer, sizeof(char), testStrm.bufferIndx+1, outfile) < testStrm.bufferIndx+1)
			{
				fprintf(stderr, "Error writing to the file");
				fclose(outfile);
				return 1;
			}

			tmp_err_code = freeAllMem(&(testStrm.memStack));
			fclose(outfile);
		}
	}
	else
	{
		printfHelp();
		return 1;
	}
}

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Efficient XML Interchange Processor, Rumen Kyusakov, 13.10.2010 \n");
    printf("           Copyright (c) 2010, EISLAB - Luleå University of Technology Version 0.1 \n");
    printf("  Usage:   exipe -help | <EXI_FileOut>\n\n");
    printf("  Purpose: This program tests the EXIP encoding functionality\n");
    printf("\n" );
}
