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
 * @file decodeTestEXI.c
 * @brief Testing the EXI decoder
 *
 * @date Oct 13, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */
#include "EXIParser.h"
#include "procTypes.h"
#include <stdio.h>
#include <string.h>

static void printfHelp();

// Content Handler API
void sample_fatalError(const char code, const char* msg);
void sample_startDocument();
void sample_endDocument();
void sample_startElement(QName qname);
void sample_endElement();
void sample_attributeString(QName qname, const StringType value);


int main(int argc, char *argv[])
{
	ContentHandler sampleHandler;
	sampleHandler.fatalError = sample_fatalError;
	sampleHandler.startDocument = sample_startDocument;
	sampleHandler.endDocument = sample_endDocument;
	sampleHandler.startElement = sample_startElement;
	sampleHandler.attributeString = sample_attributeString;

	FILE *infile;
	unsigned long fileLen;
	char *buffer;
	char sourceFile[50];
	if(argc > 1)
	{
		if(strcmp(argv[1], "help") == 0)
		{
			printfHelp();
			return 0;
		}
		strcpy(sourceFile, argv[1]);
		infile = fopen(sourceFile, "rb" );
		if(!infile)
		{
			fprintf(stderr, "Unable to open file %s", sourceFile);
			return 1;
		}
		else
		{
			//Get file length
			fseek(infile, 0, SEEK_END);
			fileLen=ftell(infile);
			fseek(infile, 0, SEEK_SET);

			//Allocate memory
			buffer=(char *)malloc(fileLen+1);
			if (!buffer)
			{
				fprintf(stderr, "Memory allocation error!");
				fclose(infile);
				return 1;
			}

			//Read file contents into buffer
			fread(buffer, fileLen, 1, infile);
			fclose(infile);

			// Parse the EXI string
			parseEXI(buffer, &sampleHandler);

			free(buffer);
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
    printf("  Usage:   decode <EXI_FileIn> | help  \n");
    printf("  Purpose: This program tests the EXIP decoding functionality\n");
    printf("\n" );
}

void sample_fatalError(const char code, const char* msg)
{
	printf("\n%d : FATAL ERROR: %s", code, msg);
}
void sample_startDocument()
{
	printf("\nstartDocument\n");
}
void sample_endDocument()
{
	printf("\nendDocument\n");
}
void sample_startElement(QName qname)
{
	printf("\nSTART ELEMENT: \nURI:");
	printString(qname.uri);
	printf("\nLN:");
	printString(qname.localName);
	printf("\n");
}
void sample_endElement()
{
	printf("\nendElement\n");
}
void sample_attributeString(QName qname, const StringType value)
{
	printf("\nATTRIBUTE: \nURI:");
	printString(qname.uri);
	printf("\nLN: ");
	printString(qname.localName);
	printf("\nVALUE: ");
	printString(&value);
	printf("\n");
}
