/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/*
 Based on tests/check_xsi_type.c, this program encodes an EXI containing xsi:type
 and then decodes it

*/

#include <stdio.h>
#include <stdlib.h>
#include "procTypes.h"
#include "EXISerializer.h"
#include "EXIParser.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"

#define OUTPUT_BUFFER_SIZE 2000
#define check(str)	if (tmp_err_code != EXIP_ERR_OK) { printf ("  =====> Err line %d (%s) code:%d\n", __LINE__, str, tmp_err_code); exit(0); }

static int parseSchema(char* xsdList, EXIPSchema* schema);

char *XSI = "http://www.w3.org/2001/XMLSchema-instance";
char *XS = "http://www.w3.org/2001/XMLSchema";
char *EXEMPLE = "http://www.exemple.com/XMLNameSpace";


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
	int opt;
	char files[1000] = "";

	while ((opt = getopt(ac, av, "s")) != -1) {
		switch (opt) {
		case 's' :
			strcpy (files, "Product.exs");
			break;
		}
	}

	printf ("### %s\n", av[0]);

	buffer.buf = buf;
	buffer.bufContent = 0;
	buffer.bufLen = OUTPUT_BUFFER_SIZE;
	buffer.ioStrm.readWriteToStream = NULL;
	buffer.ioStrm.stream = NULL;

	if	(*files && parseSchema(files, &schema) == 0) {
		schemaPtr	= &schema;
	}

	serialize.initHeader(&testStrm);

	testStrm.header.has_options = TRUE;

	testStrm.header.opts.schemaIDMode = SCHEMA_ID_EMPTY;

	if	(schemaPtr) {
		tmp_err_code = asciiToString("product", &testStrm.header.opts.schemaID, &testStrm.memList, FALSE); check("")
		testStrm.header.opts.schemaIDMode = SCHEMA_ID_SET;
		printf ("### schemaId %s\n", "product");
	}

	tmp_err_code = serialize.initStream(&testStrm, buffer, schemaPtr); check("")
	tmp_err_code = serialize.exiHeader(&testStrm); check("")
	tmp_err_code = serialize.startDocument(&testStrm); check("SD")

	printf ("### startElement %s:%s\n", EXEMPLE, "product");
	tmp_err_code += asciiToString(EXEMPLE, &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("product", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); check("SE exe:product")

	printf ("### startElement %s:%s\n", EXEMPLE, "subproduct");
	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("subproduct", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); check("SE exe:subproduct")

	printf ("### attribute %s:%s\n", XSI, "type");
	tmp_err_code += asciiToString(XSI, &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("type", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); check("xsi:type")

	printf ("### qnameData %s:%s\n", EXEMPLE, "ShirtType");
	tmp_err_code += asciiToString(EXEMPLE, &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("ShirtType", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.qnameData(&testStrm, qname); check("qnameData exe:ShirtType")

	printf ("### startElement %s:%s valueType=%d\n", "", "number", valueType);
	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("number", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); check("SE number")

	if	(schemaPtr) {
		printf ("### intData %s\n", "12345");
		tmp_err_code += serialize.intData(&testStrm, 12345);
	}
	else {
		printf ("### stringData %s\n", "12345");
		tmp_err_code += asciiToString("12345", &chVal, &testStrm.memList, FALSE);
		tmp_err_code += serialize.stringData(&testStrm, chVal); check("CH")
	}

	printf ("### EE\n");
	tmp_err_code += serialize.endElement(&testStrm); check("EE number")

	printf ("### startElement %s:%s valueType=%d\n", "", "size", valueType);
	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE); check("")
	tmp_err_code += asciiToString("size", &ln, &testStrm.memList, FALSE); check("")
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); check("SE size")

	if	(schemaPtr) {
		printf ("### intData %s\n", "33");
		tmp_err_code += serialize.intData(&testStrm, 33);
	}
	else {
		printf ("### stringData %s\n", "33");
		tmp_err_code += asciiToString("33", &chVal, &testStrm.memList, FALSE);
		tmp_err_code += serialize.stringData(&testStrm, chVal); check("CH")
	}

	printf ("### EE\n");
	tmp_err_code += serialize.endElement(&testStrm); check("EE size")

	printf ("### EE\n");
	tmp_err_code += serialize.endElement(&testStrm); check("EE product")

	printf ("### EE\n");
	tmp_err_code += serialize.endElement(&testStrm); check("EE subproduct")

	printf ("### ED\n");
	tmp_err_code += serialize.endDocument(&testStrm); check("ED")

	// V: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm); check("")

	if	(tmp_err_code == EXIP_ERR_OK)
		printf ("### ENCODING SUCCESS sz=%d\n", testStrm.context.bufferIndx + 1);

	printf ("### START PARSING\n");

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

	while(tmp_err_code == EXIP_ERR_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	destroyParser(&testParser);

	if	(tmp_err_code == EXIP_PARSING_COMPLETE)
		printf ("### PARSING SUCCESS\n");
	else
		printf ("### tmp_err_code = %d\n", tmp_err_code);

	return	0;
}

static void printURI(const String *str) {
	if	(stringEqualToAscii(*str, XSI))
		printf ("xsi");
	else if	(stringEqualToAscii(*str, XS))
		printf ("xs");
	else if	(stringEqualToAscii(*str, EXEMPLE))
		printf ("ABC");
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
	printf("\n### %d : FATAL ERROR: %s\n", code, msg);
	return EXIP_HANDLER_STOP;
}

static errorCode sample_startDocument(void* app_data)
{
	printf("### SD\n");
	return EXIP_ERR_OK;
}

static errorCode sample_endDocument(void* app_data)
{
	printf("### ED\n");
	return EXIP_ERR_OK;
}

static errorCode sample_startElement(QName qname, void* app_data)
{
	printf("### SE ");
	printQName (qname);
	printf("\n");
	return EXIP_ERR_OK;
}

static errorCode sample_endElement(void* app_data)
{
	printf("### EE\n");
	return EXIP_ERR_OK;
}

int expectAttributeData = 0;

static errorCode sample_attribute(QName qname, void* app_data)
{
	printf("### AT ");
	printQName (qname);
	printf("=\"");
	expectAttributeData = 1;
	return EXIP_ERR_OK;
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
		printf("### CH ");
		printString(&value);
		printf("\n");
	}
	return EXIP_ERR_OK;
}

static errorCode sample_decimalData(Decimal value, void* app_data)
{
	return EXIP_ERR_OK;
}

static errorCode sample_intData(Integer int_val, void* app_data)
{
	char tmp_buf[30];
	if(expectAttributeData)
	{
		sprintf(tmp_buf, "%lld", (long long int) int_val);
		printf("### intData %s", tmp_buf);
		printf("\"\n");
		expectAttributeData = 0;
	}
	else
	{
		printf("### intData ");
		sprintf(tmp_buf, "%lld", (long long int) int_val);
		printf("%s", tmp_buf);
		printf("\n");
	}
	return EXIP_ERR_OK;
}

static errorCode sample_booleanData(boolean bool_val, void* app_data)
{
	return EXIP_ERR_OK;
}

static errorCode sample_floatData(Float fl_val, void* app_data)
{
	return EXIP_ERR_OK;
}

static errorCode sample_dateTimeData(EXIPDateTime dt_val, void* app_data)
{
	return EXIP_ERR_OK;
}

static errorCode sample_binaryData(const char* binary_val, Index nbytes, void* app_data)
{
	return EXIP_ERR_OK;
}

static errorCode sample_qnameData(const QName qname, void* app_data)
{
	//if(expectAttributeData)
	{
		printf ("### qnameData : ");
		printQName (qname);
		printf("\"\n");
		expectAttributeData = 0;
	}
	return EXIP_ERR_OK;
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
printf ("### %d %s\n", i, token);
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

	if(tmp_err_code != EXIP_ERR_OK)
		printf("\n### Grammar generation error occurred: %d\n", tmp_err_code);
	else
		printf("### Grammar generation SUCCESS\n");
	return 0;
}
