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
#include "stringManipulate.h"
#include "grammarGenerator.h"
#include <stdio.h>
#include <string.h>

static void printfHelp();
static void parseSchema(char* fileName, EXIPSchema* schema);

#define OUT_EXI 0
#define OUT_XML 1
#define INPUT_BUFFER_SIZE 200

struct appData
{
	unsigned char outputFormat;
	unsigned char expectAttributeData;
	char nameBuf[100];             // needed for the OUT_XML Output Format
	struct element* stack;         // needed for the OUT_XML Output Format
	unsigned char unclosedElement; // needed for the OUT_XML Output Format
};


// Stuff needed for the OUT_XML Output Format
// ******************************************
struct element {
	struct element* next;
	char* name;
};

static void push(struct element** stack, struct element* el);
static struct element* pop(struct element** stack);
static struct element* createElement(char* name);
static void destroyElement(struct element* el);

// ******************************************

// Content Handler API
static char sample_fatalError(const char code, const char* msg, void* app_data);
static char sample_startDocument(void* app_data);
static char sample_endDocument(void* app_data);
static char sample_startElement(QName qname, void* app_data);
static char sample_endElement(void* app_data);
static char sample_attribute(QName qname, void* app_data);
static char sample_stringData(const String value, void* app_data);
static char sample_decimalData(Decimal value, void* app_data);
static char sample_intData(Integer int_val, void* app_data);
static char sample_floatData(Float fl_val, void* app_data);

size_t readFileInputStream(void* buf, size_t readSize, void* stream);

int main(int argc, char *argv[])
{
	FILE *infile;
	char sourceFileName[100];
	EXIPSchema schema;
	EXIPSchema* schemaPtr = NULL;
	struct appData parsingData;

	parsingData.outputFormat = OUT_EXI; // Default output option

	if(argc > 1)
	{
		if(strcmp(argv[1], "-help") == 0)
		{
			printfHelp();
			return 0;
		}
		else if(strcmp(argv[1], "-exi") == 0)
		{
			parsingData.outputFormat = OUT_EXI;
			if(argc == 2)
			{
				printfHelp();
				return 0;
			}
			else if(strcmp(argv[2], "-schema") == 0)
			{
				if(argc <= 4)
				{
					printfHelp();
					return 0;
				}

				parseSchema(argv[3], &schema);
				schemaPtr = &schema;
				strcpy(sourceFileName, argv[4]);
			}
			else
				strcpy(sourceFileName, argv[2]);
		}
		else if(strcmp(argv[1], "-xml") == 0)
		{
			parsingData.outputFormat = OUT_XML;
			if(argc == 2)
			{
				printfHelp();
				return 0;
			}
			else if(strcmp(argv[2], "-schema") == 0)
			{
				if(argc <= 4)
				{
					printfHelp();
					return 0;
				}

				parseSchema(argv[3], &schema);
				schemaPtr = &schema;
				strcpy(sourceFileName, argv[4]);
			}
			else
				strcpy(sourceFileName, argv[2]);
		}
		else if(strcmp(argv[1], "-schema") == 0)
		{
			if(argc <= 3)
			{
				printfHelp();
				return 0;
			}
			else
			{
				parseSchema(argv[2], &schema);
				schemaPtr = &schema;
				strcpy(sourceFileName, argv[3]);
			}
		}
		else
		{
			strcpy(sourceFileName, argv[1]);
		}

		infile = fopen(sourceFileName, "rb" );
		if(!infile)
		{
			fprintf(stderr, "Unable to open file %s", sourceFileName);
			return 1;
		}
		else
		{
			Parser testParser;
			char buffer[INPUT_BUFFER_SIZE];
			IOStream ioStrm;
			errorCode tmp_err_code = UNEXPECTED_ERROR;

			// Parsing steps:

			// I: First, define an external stream for the input to the parser if any
			ioStrm.readWriteToStream = readFileInputStream;
			ioStrm.stream = infile;

			// II: Second, initialize the parser object
			tmp_err_code = initParser(&testParser, buffer, INPUT_BUFFER_SIZE, 0, &ioStrm, schemaPtr, &parsingData);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			// III: Initialize the parsing data and hook the callback handlers to the parser object

			parsingData.expectAttributeData = 0;
			parsingData.stack = NULL;
			parsingData.unclosedElement = 0;

			testParser.handler.fatalError = sample_fatalError;
			testParser.handler.error = sample_fatalError;
			testParser.handler.startDocument = sample_startDocument;
			testParser.handler.endDocument = sample_endDocument;
			testParser.handler.startElement = sample_startElement;
			testParser.handler.attribute = sample_attribute;
			testParser.handler.stringData = sample_stringData;
			testParser.handler.endElement = sample_endElement;
			testParser.handler.decimalData = sample_decimalData;
			testParser.handler.intData = sample_intData;
			testParser.handler.floatData = sample_floatData;

			// IV: Parse the header of the stream

			tmp_err_code = parseHeader(&testParser);
			if(tmp_err_code != ERR_OK)
			{
				printf("\nError during parsing of the EXI header: %d", tmp_err_code);
				return 1;
			}

			// V: Parse the body of the EXI stream

			while(tmp_err_code == ERR_OK)
			{
				tmp_err_code = parseNext(&testParser);
			}

			// VI: Free the memory allocated by the parser object

			destroyParser(&testParser);
			fclose(infile);

			if(tmp_err_code == PARSING_COMPLETE)
				return ERR_OK;
			else
			{
				printf("\nError during parsing of the EXI body: %d", tmp_err_code);
				return 1;
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
    printf("  EXIP     Efficient XML Interchange Processor, Rumen Kyusakov, 2011 \n");
    printf("           Copyright (c) 2010 - 2011, EISLAB - Luleå University of Technology Version 0.2 \n");
    printf("  Usage:   exipd [options] <EXI_FileIn>\n\n");
    printf("           Options: [-help | [ -xml | -exi ] -schema <schema_file_in>] \n");
    printf("           -schema :   uses schema defined in <schema_file_in> for decoding\n");
    printf("           -exi    :   EXI formated output [default]\n");
    printf("           -xml    :   XML formated output\n");
    printf("           -help   :   Prints this help message\n\n");
    printf("  Purpose: This program tests the EXIP decoding functionality\n");
    printf("\n" );
}

static char sample_fatalError(const char code, const char* msg, void* app_data)
{
	printf("\n%d : FATAL ERROR: %s\n", code, msg);
	return EXIP_HANDLER_STOP;
}

static char sample_startDocument(void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
		printf("SD\n");
	else if(appD->outputFormat == OUT_XML)
		printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	return EXIP_HANDLER_OK;
}

static char sample_endDocument(void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
		printf("ED\n");
	else if(appD->outputFormat == OUT_XML)
		printf("\n");

	return EXIP_HANDLER_OK;
}

static char sample_startElement(QName qname, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
	{
		printf("SE ");
		printString(qname.uri);
		printf(" ");
		printString(qname.localName);
		printf("\n");
	}
	else if(appD->outputFormat == OUT_XML)
	{
		memcpy(appD->nameBuf, qname.uri->str, qname.uri->length);
		memcpy(appD->nameBuf + qname.uri->length, qname.localName->str, qname.localName->length);
		appD->nameBuf[qname.uri->length + qname.localName->length] = '\0';
		push(&(appD->stack), createElement(appD->nameBuf));
		if(appD->unclosedElement)
			printf(">\n");
		printf("<");
		if(!isStringEmpty(qname.uri))
		{
			printString(qname.uri);
			printf(":");
		}
		printString(qname.localName);
		appD->unclosedElement = 1;
	}

	return EXIP_HANDLER_OK;
}

static char sample_endElement(void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
		printf("EE\n");
	else if(appD->outputFormat == OUT_XML)
	{
		struct element* el;

		if(appD->unclosedElement)
			printf(">\n");
		appD->unclosedElement = 0;
		el = pop(&(appD->stack));
		printf("</%s>\n", el->name);
		destroyElement(el);
	}

	return EXIP_HANDLER_OK;
}

static char sample_attribute(QName qname, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
	{
		printf("AT ");
		printString(qname.uri);
		printf(" ");
		printString(qname.localName);
		printf("=\"");
	}
	else if(appD->outputFormat == OUT_XML)
	{
		printf(" ");
		if(!isStringEmpty(qname.uri))
		{
			printString(qname.uri);
			printf(":");
		}
		printString(qname.localName);
		printf("=\"");
	}
	appD->expectAttributeData = 1;

	return EXIP_HANDLER_OK;
}

static char sample_stringData(const String value, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			printString(&value);
			printf("\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			printf("CH ");
			printString(&value);
			printf("\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			printString(&value);
			printf("\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				printf(">\n");
			appD->unclosedElement = 0;
			printString(&value);
			printf("\n");
		}
	}

	return EXIP_HANDLER_OK;
}

static char sample_decimalData(Decimal value, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			printf("%.1f\"\n", (double) value);
			appD->expectAttributeData = 0;
		}
		else
		{
			printf("CH %.1f \n", (double) value);
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			printf("%.1f \"", (double) value);
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				printf(">\n");
			appD->unclosedElement = 0;
			printf("%.1f \n", (double) value);
		}
	}

	return EXIP_HANDLER_OK;
}

static char sample_intData(Integer int_val, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	char tmp_buf[30];
	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			sprintf(tmp_buf, "%d", (int) int_val);
			printf("%s", tmp_buf);
			printf("\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			printf("CH ");
			sprintf(tmp_buf, "%d", (int) int_val);
			printf("%s", tmp_buf);
			printf("\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			sprintf(tmp_buf, "%d", (int) int_val);
			printf("%s", tmp_buf);
			printf("\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				printf(">\n");
			appD->unclosedElement = 0;
			sprintf(tmp_buf, "%d", (int) int_val);
			printf("%s", tmp_buf);
			printf("\n");
		}
	}

	return EXIP_HANDLER_OK;
}

static char sample_floatData(Float fl_val, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	char tmp_buf[30];
	if(appD->outputFormat == OUT_EXI)
	{
		if(appD->expectAttributeData)
		{
			sprintf(tmp_buf, "%lldE%d", fl_val.mantissa, fl_val.exponent);
			printf("%s", tmp_buf);
			printf("\"\n");
			appD->expectAttributeData = 0;
		}
		else
		{
			printf("CH ");
			sprintf(tmp_buf, "%lldE%d", fl_val.mantissa, fl_val.exponent);
			printf("%s", tmp_buf);
			printf("\n");
		}
	}
	else if(appD->outputFormat == OUT_XML)
	{
		if(appD->expectAttributeData)
		{
			sprintf(tmp_buf, "%lldE%d", fl_val.mantissa, fl_val.exponent);
			printf("%s", tmp_buf);
			printf("\"");
			appD->expectAttributeData = 0;
		}
		else
		{
			if(appD->unclosedElement)
				printf(">\n");
			appD->unclosedElement = 0;
			sprintf(tmp_buf, "%lldE%d", fl_val.mantissa, fl_val.exponent);
			printf("%s", tmp_buf);
			printf("\n");
		}
	}

	return EXIP_HANDLER_OK;
}

// Stuff needed for the OUT_XML Output Format
// ******************************************
static void push(struct element** stack, struct element* el)
{
	if(*stack == NULL)
		*stack = el;
	else
	{
		el->next = *stack;
		*stack = el;
	}
}

static struct element* pop(struct element** stack)
{
	if(*stack == NULL)
		return NULL;
	else
	{
		struct element* result;
		result = *stack;
		*stack = (*stack)->next;
		return result;
	}
}

static struct element* createElement(char* name)
{
	struct element* el;
	el = malloc(sizeof(struct element));
	if(el == NULL)
		exit(1);
	el->next = NULL;
	el->name = malloc(strlen(name)+1);
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

size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
}

static void parseSchema(char* fileName, EXIPSchema* schema)
{
	FILE *schemaFile;
	unsigned long schemaLen;
	char *schemaBuffer;
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	schemaFile = fopen(fileName, "rb" );
	if(!schemaFile)
	{
		fprintf(stderr, "Unable to open file %s", fileName);
		exit(1);
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
			exit(1);
		}

		//Read file contents into buffer
		fread(schemaBuffer, schemaLen, 1, schemaFile);
		fclose(schemaFile);

		tmp_err_code = generateSchemaInformedGrammars(schemaBuffer, schemaLen, schemaLen, NULL, SCHEMA_FORMAT_XSD_EXI, schema);

		if(tmp_err_code != ERR_OK)
		{
			printf("\n Error occured: %d", tmp_err_code);
			exit(1);
		}

		free(schemaBuffer);
	}
}
