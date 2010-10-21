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

#define OUT_EXI 0
#define OUT_XML 1

// Default output option
unsigned char outputFormat = OUT_EXI;

// Stuff needed for the OUT_XML Output Format
// ******************************************
unsigned char unclosedElement = 0;
struct element {
	struct element* next;
	char* name;
};

char nameBuf[100];

struct element* stack = NULL;

static void push(struct element* el);
static struct element* pop();
static struct element* createElement(char* name);
static void destroyElement(struct element* el);

// ******************************************

// Content Handler API
void sample_fatalError(const char code, const char* msg);
void sample_startDocument();
void sample_endDocument();
void sample_startElement(QName qname);
void sample_endElement();
void sample_attributeString(QName qname, const StringType value);
void sample_stringData(const StringType value);


int main(int argc, char *argv[])
{
	ContentHandler sampleHandler;
	sampleHandler.fatalError = sample_fatalError;
	sampleHandler.startDocument = sample_startDocument;
	sampleHandler.endDocument = sample_endDocument;
	sampleHandler.startElement = sample_startElement;
	sampleHandler.attributeString = sample_attributeString;
	sampleHandler.stringData = sample_stringData;
	sampleHandler.endElement = sample_endElement;


	// NOTE: It is mandatory to initialize the callbacks you don't explicitly implement as NULL
	sampleHandler.binaryData = NULL;
	sampleHandler.error = NULL;
	sampleHandler.floatData = NULL;
	sampleHandler.intData = NULL;
	sampleHandler.processingInstruction = NULL;
	sampleHandler.selfContained = NULL;
	sampleHandler.warning = NULL;


	FILE *infile;
	unsigned long fileLen;
	char *buffer;
	char sourceFile[50];
	if(argc > 1)
	{
		if(strcmp(argv[1], "-help") == 0)
		{
			printfHelp();
			return 0;
		}
		else if(strcmp(argv[1], "-exi") == 0)
		{
			outputFormat = OUT_EXI;
			if(argc == 2)
			{
				printfHelp();
				return 0;
			}
			else
				strcpy(sourceFile, argv[2]);
		}
		else if(strcmp(argv[1], "-xml") == 0)
		{
			outputFormat = OUT_XML;
			if(argc == 2)
			{
				printfHelp();
				return 0;
			}
			else
				strcpy(sourceFile, argv[2]);
		}
		else
		{
			strcpy(sourceFile, argv[1]);
		}
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
    printf("  Usage:   exipd [options] <EXI_FileIn>\n\n");
    printf("           Options:\n");
    printf("           -exi   :   EXI formated output [default]\n");
    printf("           -xml   :   XML formated output\n");
    printf("           -help  :   Prints this help message\n\n");
    printf("  Purpose: This program tests the EXIP decoding functionality\n");
    printf("\n" );
}

void sample_fatalError(const char code, const char* msg)
{
	printf("\n%d : FATAL ERROR: %s\n", code, msg);
}

void sample_startDocument()
{
	if(outputFormat == OUT_EXI)
		printf("SD\n");
	else if(outputFormat == OUT_XML)
		printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
}

void sample_endDocument()
{
	if(outputFormat == OUT_EXI)
		printf("ED\n");
	else if(outputFormat == OUT_XML)
		printf("\n");
}

void sample_startElement(QName qname)
{
	if(outputFormat == OUT_EXI)
	{
		printf("SE ");
		printString(qname.uri);
		printf(" ");
		printString(qname.localName);
		printf("\n");
	}
	else if(outputFormat == OUT_XML)
	{
		memcpy(nameBuf, qname.uri->str, qname.uri->length);
		memcpy(nameBuf + qname.uri->length, qname.localName->str, qname.localName->length);
		nameBuf[qname.uri->length + qname.localName->length] = '\0';
		push(createElement(nameBuf));
		if(unclosedElement)
			printf(">\n");
		printf("<");
		printString(qname.uri);
		printString(qname.localName);
		unclosedElement = 1;
	}
}

void sample_endElement()
{
	if(outputFormat == OUT_EXI)
		printf("EE\n");
	else if(outputFormat == OUT_XML)
	{
		struct element* el = pop();
		printf("</%s>\n", el->name);
		destroyElement(el);
	}
}

void sample_attributeString(QName qname, const StringType value)
{
	if(outputFormat == OUT_EXI)
	{
		printf("AT ");
		printString(qname.uri);
		printf(" ");
		printString(qname.localName);
		printf("=\"");
		printString(&value);
		printf("\"\n");
	}
	else if(outputFormat == OUT_XML)
	{
		printf(" ");
		printString(qname.uri);
		printString(qname.localName);
		printf("=\"");
		printString(&value);
		printf("\"");
	}
}

void sample_stringData(const StringType value)
{
	if(outputFormat == OUT_EXI)
	{
		printf("CH ");
		printString(&value);
		printf("\n");
	}
	else if(outputFormat == OUT_XML)
	{
		if(unclosedElement)
			printf(">\n");
		unclosedElement = 0;
		printString(&value);
		printf("\n");
	}
}

// Stuff needed for the OUT_XML Output Format
// ******************************************
static void push(struct element* el)
{
	if(stack == NULL)
		stack = el;
	else
	{
		el->next = stack;
		stack = el;
	}
}

static struct element* pop()
{
	if(stack == NULL)
		return NULL;
	else
	{
		struct element* result;
		result = stack;
		stack = stack->next;
		return result;
	}
}

static struct element* createElement(char* name)
{
	struct element* el;
	el = malloc(sizeof(struct element));
	if(el == NULL)
		exit(1);
	el->next == NULL;
	el->name = malloc(strlen(name));
	if(el->name == NULL)
		exit(1);
	strcpy(el->name, name);
	return el;
}

static void destroyElement(struct element* el)
{
	free(el->name);
	free(el);
}
// ******************************************
