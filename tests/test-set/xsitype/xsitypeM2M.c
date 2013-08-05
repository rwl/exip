/*
* Copyright (C) Actility, SA. All Rights Reserved.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version
* 2 only, as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License version 2 for more details (a copy is
* included at /legal/license.txt).
*
* You should have received a copy of the GNU General Public License
* version 2 along with this work; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
* Please contact Actility, SA.,  4, rue Ampere 22300 LANNION FRANCE
* or visit www.actility.com if you need additional
* information or have any questions.
*/


/*
 Based on tests/check_xsi_type.c, this program encodes an EXI containing xsi:type
 and then decodes it :

<?xml version="1.0" encoding="UTF-8"?>
<m2m:subscription xmlns:m2m="http://uri.etsi.org/m2m" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <m2m:filterCriteria xsi:type="m2m:ContentInstanceFilterCriteriaType">
    <creator>http://www.creator.org/</creator>
  </m2m:filterCriteria>
</m2m:subscription>
*/

#include <stdio.h>
#include <stdlib.h>
#include "procTypes.h"
#include "EXISerializer.h"
#include "EXIParser.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"

#define OUTPUT_BUFFER_SIZE 64*1024
#define check(str)	if (tmp_err_code != EXIP_OK) { printf ("  =====> Err line %d (%s) code:%d\n", __LINE__, str, tmp_err_code); exit(0); }

static int parseSchema(char* xsdList, EXIPSchema* schema);

char *XSI = "http://www.w3.org/2001/XMLSchema-instance";
char *XS = "http://www.w3.org/2001/XMLSchema";
char *M2M = "http://uri.etsi.org/m2m";


static errorCode sample_fatalError(const errorCode code, const char* msg, void* app_data);
static errorCode sample_startDocument(void* app_data);
static errorCode sample_endDocument(void* app_data);
static errorCode sample_startElement(QName qname, void* app_data);
static errorCode sample_endElement(void* app_data);
static errorCode sample_attribute(QName qname, void* app_data);
static errorCode sample_stringData(const String value, void* app_data);
static errorCode sample_decimalData(Decimal value, void* app_data);
static errorCode sample_intData(Integer int_val, void* app_data);
static errorCode sample_floatData(Float fl_val, void* app_data);
static errorCode sample_booleanData(boolean bool_val, void* app_data);
static errorCode sample_dateTimeData(EXIPDateTime dt_val, void* app_data);
static errorCode sample_binaryData(const char* binary_val, Index nbytes, void* app_data);
static errorCode sample_qnameData(const QName qname, void* app_data);

int main(int ac, char **av) {
	EXIStream testStrm;
	EXIPSchema schema, *schemaPtr = NULL;
	String uri;
	String ln;
	QName qname= {&uri, &ln};
	String chVal;
	char buf[OUTPUT_BUFFER_SIZE];
	errorCode tmp_err_code = 0;
	BinaryBuffer buffer;
	EXITypeClass valueType;

	buffer.buf = buf;
	buffer.bufContent = 0;
	buffer.bufLen = OUTPUT_BUFFER_SIZE;
	buffer.ioStrm.readWriteToStream = NULL;
	buffer.ioStrm.stream = NULL;

	char files[256] = "";
	if	(ac == 2 && !strcmp(av[1], "-s")) {
		strcpy (files, "common.exs,subscription.exs,contentInstances.exs,contentInstance.exs,mime.exs");
	}

	if	(*files && parseSchema(files, &schema) == 0) {
		schemaPtr	= &schema;
	}

	serialize.initHeader(&testStrm);

	testStrm.header.has_options = TRUE;

	if	(schemaPtr) {
		tmp_err_code = asciiToString("subscription", &testStrm.header.opts.schemaID, &testStrm.memList, FALSE); check("")
		testStrm.header.opts.schemaIDMode = SCHEMA_ID_SET;
		printf ("schemaId %s\n", "subscription");
	}
	else {
		testStrm.header.opts.schemaIDMode = SCHEMA_ID_EMPTY;
	}

	tmp_err_code = serialize.initStream(&testStrm, buffer, schemaPtr); check("")
	tmp_err_code = serialize.exiHeader(&testStrm); check("")
	tmp_err_code = serialize.startDocument(&testStrm); check("SD")

	printf ("### startElement %s:%s\n", M2M, "subscription");
	tmp_err_code += asciiToString(M2M, &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("subscription", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); check("SE m2m:subscription")

	printf ("### startElement %s:%s\n", M2M, "filterCriteria");
	tmp_err_code += asciiToString(M2M, &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("filterCriteria", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); check("SE m2m:filterCriteria")

	printf ("### attribute %s:%s\n", XSI, "type");
	tmp_err_code += asciiToString(XSI, &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("type", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); check("xsi:type")

	printf ("### qnameData %s:%s\n", M2M, "ContentInstanceFilterCriteriaType");
	tmp_err_code += asciiToString(M2M, &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("ContentInstanceFilterCriteriaType", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.qnameData(&testStrm, qname); check("qnameData m2m:ContentInstanceFilterCriteriaType")

	// <creator>http://www.creator.org/</creator>
	printf ("### startElement %s:%s valueType=%d\n", "", "creator", valueType);
	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("creator", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); check("SE creator")

	printf ("### stringData %s\n", "http://www.creator.org");
	tmp_err_code += asciiToString("http://www.creator.org", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal); check("CH")

	tmp_err_code += serialize.endElement(&testStrm); check("EE creator")

	tmp_err_code += serialize.endElement(&testStrm); check("EE filterCriteria")

	tmp_err_code += serialize.endElement(&testStrm); check("EE subscription")

	tmp_err_code += serialize.endDocument(&testStrm); check("ED")

	// V: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm); check("")

	if	(tmp_err_code == EXIP_OK)
		printf ("ENCODING SUCCESS sz=%d\n", testStrm.context.bufferIndx + 1);

	printf ("START PARSING\n");

	//	DECODING

	Parser testParser;

	buffer.bufContent = testStrm.context.bufferIndx + 1;
	tmp_err_code = initParser(&testParser, buffer, NULL); check("")

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
	testParser.handler.booleanData = sample_booleanData;
	testParser.handler.dateTimeData = sample_dateTimeData;
	testParser.handler.binaryData = sample_binaryData;
	testParser.handler.qnameData = sample_qnameData;

	tmp_err_code = parseHeader(&testParser, FALSE); check("")

	// IV.1: Set the schema to be used for parsing.
	// The schemaID mode and schemaID field can be read at
	// parser.strm.header.opts.schemaIDMode and
	// parser.strm.header.opts.schemaID respectively
	// If schemaless mode, use setSchema(&parser, NULL);

	TRY(setSchema(&testParser, schemaPtr));

	while(tmp_err_code == EXIP_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	destroyParser(&testParser);

	if	(tmp_err_code == EXIP_PARSING_COMPLETE)
		printf ("PARSING SUCCESS\n");
	else
		printf ("tmp_err_code = %d\n", tmp_err_code);

	return	0;
}

static void printURI(const String *str) {
	if	(stringEqualToAscii(*str, XSI))
		printf ("xsi");
	else if	(stringEqualToAscii(*str, XS))
		printf ("xs");
	else if	(stringEqualToAscii(*str, M2M))
		printf ("m2m");
	else
		fwrite (str->str, sizeof(CharType), str->length, stdout);
}

void printQName(const QName qname) {
	printURI (qname.uri);
	printf(":");
	printString(qname.localName);
}

static errorCode sample_fatalError(const errorCode code, const char* msg, void* app_data)
{
	printf("\n%d : FATAL ERROR: %s\n", code, msg);
	return EXIP_HANDLER_STOP;
}

static errorCode sample_startDocument(void* app_data)
{
	printf("SD\n");
	return EXIP_OK;
}

static errorCode sample_endDocument(void* app_data)
{
	printf("ED\n");
	return EXIP_OK;
}

static errorCode sample_startElement(QName qname, void* app_data)
{
	printf("SE ");
	printQName (qname);
	printf("\n");
	return EXIP_OK;
}

static errorCode sample_endElement(void* app_data)
{
	printf("EE\n");
	return EXIP_OK;
}

int expectAttributeData = 0;

static errorCode sample_attribute(QName qname, void* app_data)
{
	printf("AT ");
	printQName (qname);
	printf("=\"");
	expectAttributeData = 1;
	return EXIP_OK;
}

static errorCode sample_stringData(const String value, void* app_data)
{
	if(expectAttributeData)
	{
		printString(&value);
		printf("\"\n");
		expectAttributeData = 0;
	}
	else
	{
		printf("CH ");
		printString(&value);
		printf("\n");
	}
	return EXIP_OK;
}

static errorCode sample_decimalData(Decimal value, void* app_data)
{
	return EXIP_OK;
}

static errorCode sample_intData(Integer int_val, void* app_data)
{
	char tmp_buf[30];
	if(expectAttributeData)
	{
		sprintf(tmp_buf, "%lld", (long long int) int_val);
		printf("intData %s", tmp_buf);
		printf("\"\n");
		expectAttributeData = 0;
	}
	else
	{
		printf("intData ");
		sprintf(tmp_buf, "%lld", (long long int) int_val);
		printf("%s", tmp_buf);
		printf("\n");
	}
	return EXIP_OK;
}

static errorCode sample_booleanData(boolean bool_val, void* app_data)
{
	return EXIP_OK;
}

static errorCode sample_floatData(Float fl_val, void* app_data)
{
	return EXIP_OK;
}

static errorCode sample_dateTimeData(EXIPDateTime dt_val, void* app_data)
{
	return EXIP_OK;
}

static errorCode sample_binaryData(const char* binary_val, Index nbytes, void* app_data)
{
	return EXIP_OK;
}

static errorCode sample_qnameData(const QName qname, void* app_data)
{
	//if(expectAttributeData)
	{
		printf ("qnameData : ");
		printQName (qname);
		printf("\"\n");
		expectAttributeData = 0;
	}
	return EXIP_OK;
}


#define MAX_XSD_FILES_COUNT 10 // up to 10 XSD files

static int parseSchema(char* xsdList, EXIPSchema* schema)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	FILE *schemaFile;
	BinaryBuffer buffer[MAX_XSD_FILES_COUNT]; // up to 10 XSD files
	char schemaFileName[50];
	unsigned int schemaFilesCount = 0;
	unsigned int i;
	char *token;

	for (token = strtok(xsdList, "=,"), i = 0; token != NULL; token = strtok(NULL, "=,"), i++)
	{
printf ("%d %s\n", i, token);
		schemaFilesCount++;
		if(schemaFilesCount > MAX_XSD_FILES_COUNT)
		{
			fprintf(stderr, "Too many xsd files given as an input: %d", schemaFilesCount);
			return 0;
		}

		strcpy(schemaFileName, token);
		schemaFile = fopen(schemaFileName, "rb" );
		if(!schemaFile)
		{
			fprintf(stderr, "Unable to open file %s", schemaFileName);
			continue;
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
				return -1;
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
	tmp_err_code = generateSchemaInformedGrammars(buffer, schemaFilesCount, SCHEMA_FORMAT_XSD_EXI, NULL, schema);

	for(i = 0; i < schemaFilesCount; i++)
	{
		free(buffer[i].buf);
	}

	if(tmp_err_code != EXIP_OK)
		printf("\nGrammar generation error occurred: %d\n", tmp_err_code);
	else
		printf("Grammar generation SUCCESS\n");
	return 0;
}
