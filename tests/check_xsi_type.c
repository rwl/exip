/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file check_xsi_type.c
 * @brief Tests the encoding and decoding of xsi:type attributes
 *
 * @date Feb 7, 2013
 * @author Rumen Kyusakov
 * @version 0.4.2
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "procTypes.h"
#include "EXISerializer.h"
#include "EXIParser.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"

#define OUTPUT_BUFFER_SIZE 2000

/* BEGIN: SchemaLess tests */

START_TEST (test_default_options)
{
	EXIStream testStrm;
	Parser testParser;
	String uri;
	String ln;
	QName qname= {&uri, &ln};
	String chVal;
	char buf[OUTPUT_BUFFER_SIZE];
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	BinaryBuffer buffer;
	EXITypeClass valueType;

	buffer.buf = buf;
	buffer.bufContent = 0;
	buffer.bufLen = OUTPUT_BUFFER_SIZE;
	buffer.ioStrm.readWriteToStream = NULL;
	buffer.ioStrm.stream = NULL;

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_options = TRUE;
	testStrm.header.opts.schemaIDMode = SCHEMA_ID_EMPTY;

	// III: Define an external stream for the output if any

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, NULL);
	fail_unless (tmp_err_code == EXIP_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code = serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.exiHeader returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.startDocument(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.startDocument returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.ltu.se/EISLAB/schema-test", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("EXIPEncoder", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("version", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("0.2", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("status", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("alpha", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("This is an example of serializing EXI streams using EXIP low level API", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("xsitypetest", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.w3.org/2001/XMLSchema-instance", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("type", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.w3.org/2001/XMLSchema", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("integer", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.qnameData(&testStrm, qname);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.qnameData returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.intData(&testStrm, 144);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.intData returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += serialize.endElement(&testStrm);
	tmp_err_code += serialize.endDocument(&testStrm);

	if(tmp_err_code != EXIP_OK)
		fail_unless (tmp_err_code == EXIP_OK, "serialization ended with error code %d", tmp_err_code);

	// V: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.closeEXIStream ended with error code %d", tmp_err_code);


	buffer.bufContent = OUTPUT_BUFFER_SIZE;
	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buffer, NULL);
	fail_unless (tmp_err_code == EXIP_OK, "initParser returns an error code %d", tmp_err_code);

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser, FALSE);
	fail_unless (tmp_err_code == EXIP_OK, "parsing the header returns an error code %d", tmp_err_code);

	tmp_err_code = setSchema(&testParser, NULL);
	fail_unless (tmp_err_code == EXIP_OK, "setSchema() returns an error code %d", tmp_err_code);

	// V: Parse the body of the EXI stream

	while(tmp_err_code == EXIP_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&testParser);
	fail_unless (tmp_err_code == EXIP_PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
}
END_TEST

/* END: SchemaLess tests */

/* BEGIN: Schema-mode tests */

char *XSI = "http://www.w3.org/2001/XMLSchema-instance";
char *XS = "http://www.w3.org/2001/XMLSchema";
char *EXEMPLE = "http://www.exemple.com/XMLNameSpace";
#define MAX_XSD_FILES_COUNT 10 // up to 10 XSD files
static char *dataDir;
#define MAX_PATH_LEN 200

#define PRINTOUTS 0

static void parseSchema(const char* fileName, EXIPSchema* schema);

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

START_TEST (test_simple_schema)
{
	const char* schemafname = "xsitype/Product.exs";
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

	parseSchema(schemafname, &schema);
	schemaPtr = &schema;

	serialize.initHeader(&testStrm);

	testStrm.header.has_options = TRUE;
	testStrm.header.opts.schemaIDMode = SCHEMA_ID_EMPTY;

	if	(schemaPtr) {
		tmp_err_code = asciiToString("product", &testStrm.header.opts.schemaID, &testStrm.memList, FALSE);
		fail_unless (tmp_err_code == EXIP_OK, "asciiToString returns an error code %d", tmp_err_code);
		testStrm.header.opts.schemaIDMode = SCHEMA_ID_SET;
	}

	tmp_err_code = serialize.initStream(&testStrm, buffer, schemaPtr);
	fail_unless (tmp_err_code == EXIP_OK, "serialize returns an error code %d", tmp_err_code);
	tmp_err_code = serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize returns an error code %d", tmp_err_code);
	tmp_err_code = serialize.startDocument(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString(EXEMPLE, &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("product", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);

	tmp_err_code += asciiToString(XSI, &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("type", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);

	tmp_err_code += asciiToString(EXEMPLE, &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("ShirtType", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.qnameData(&testStrm, qname);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("number", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == EXIP_OK, "serialize returns an error code %d", tmp_err_code);

	if	(schemaPtr) {
		tmp_err_code += serialize.intData(&testStrm, 12345);
	}
	else {
		tmp_err_code += asciiToString("12345", &chVal, &testStrm.memList, FALSE);
		tmp_err_code += serialize.stringData(&testStrm, chVal);
	}

	fail_unless (tmp_err_code == EXIP_OK, "serialize returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("size", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);

	if	(schemaPtr) {
		tmp_err_code += serialize.intData(&testStrm, 33);
	}
	else {
		tmp_err_code += asciiToString("33", &chVal, &testStrm.memList, FALSE);
		tmp_err_code += serialize.stringData(&testStrm, chVal);
	}

	tmp_err_code += serialize.endElement(&testStrm);
	tmp_err_code += serialize.endElement(&testStrm);
	tmp_err_code += serialize.endDocument(&testStrm);

	// V: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);

	fail_unless (tmp_err_code == EXIP_OK, "serialize returns an error code %d", tmp_err_code);

	//	DECODING

	Parser testParser;

	buffer.bufContent = testStrm.context.bufferIndx + 1;
	tmp_err_code = initParser(&testParser, buffer, NULL);

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

	tmp_err_code = parseHeader(&testParser, FALSE);

	// IV.1: Set the schema to be used for parsing.
	// The schemaID mode and schemaID field can be read at
	// parser.strm.header.opts.schemaIDMode and
	// parser.strm.header.opts.schemaID respectively
	// If schemaless mode, use setSchema(&parser, NULL);

	tmp_err_code = setSchema(&testParser, schemaPtr);
	fail_unless (tmp_err_code == EXIP_OK, "setSchema() returns an error code %d", tmp_err_code);

	while(tmp_err_code == EXIP_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	destroyParser(&testParser);
	fail_unless (tmp_err_code == EXIP_PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
}
END_TEST

#if PRINTOUTS
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
#endif

static errorCode sample_fatalError(const errorCode code, const char* msg, void* app_data)
{
#if PRINTOUTS
	printf("\n### %d : FATAL ERROR: %s\n", code, msg);
#endif
	return EXIP_HANDLER_STOP;
}

static errorCode sample_startDocument(void* app_data)
{
#if PRINTOUTS
	printf("### SD\n");
#endif
	return EXIP_OK;
}

static errorCode sample_endDocument(void* app_data)
{
#if PRINTOUTS
	printf("### ED\n");
#endif
	return EXIP_OK;
}

static errorCode sample_startElement(QName qname, void* app_data)
{
#if PRINTOUTS
	printf("### SE ");
	printQName (qname);
	printf("\n");
#endif
	return EXIP_OK;
}

static errorCode sample_endElement(void* app_data)
{
#if PRINTOUTS
	printf("### EE\n");
#endif
	return EXIP_OK;
}

int expectAttributeData = 0;

static errorCode sample_attribute(QName qname, void* app_data)
{
#if PRINTOUTS
	printf("### AT ");
	printQName (qname);
	printf("=\"");
#endif
	expectAttributeData = 1;
	return EXIP_OK;
}

static errorCode sample_stringData(const String value, void* app_data)
{
	if(expectAttributeData)
	{
#if PRINTOUTS
		printString(&value);
		printf("\"\n");
#endif
		expectAttributeData = 0;
	}
	else
	{
#if PRINTOUTS
		printf("CH ");
		printString(&value);
		printf("\n");
#endif
	}
	return EXIP_OK;
}

static errorCode sample_decimalData(Decimal value, void* app_data)
{
	return EXIP_OK;
}

static errorCode sample_intData(Integer int_val, void* app_data)
{
	if(expectAttributeData)
	{
#if PRINTOUTS
		char tmp_buf[30];
		sprintf(tmp_buf, "%lld", (long long int) int_val);
		printf("### intData %s", tmp_buf);
		printf("\"\n");
#endif
		expectAttributeData = 0;
	}
	else
	{
#if PRINTOUTS
		char tmp_buf[30];
		printf("### intData ");
		sprintf(tmp_buf, "%lld", (long long int) int_val);
		printf("%s", tmp_buf);
		printf("\n");
#endif
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
#if PRINTOUTS
	printf ("### qnameData : ");
	printQName (qname);
	printf("\"\n");
#endif
	expectAttributeData = 0;
	return EXIP_OK;
}

static void parseSchema(const char* fileName, EXIPSchema* schema)
{
	FILE *schemaFile;
	BinaryBuffer buffer;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	size_t pathlen = strlen(dataDir);
	char exipath[MAX_PATH_LEN + strlen(fileName)];

	memcpy(exipath, dataDir, pathlen);
	exipath[pathlen] = '/';
	memcpy(&exipath[pathlen+1], fileName, strlen(fileName)+1);
	schemaFile = fopen(exipath, "rb" );
	if(!schemaFile)
	{
		fail("Unable to open file %s", exipath);
	}
	else
	{
		//Get file length
		fseek(schemaFile, 0, SEEK_END);
		buffer.bufLen = ftell(schemaFile) + 1;
		fseek(schemaFile, 0, SEEK_SET);

		//Allocate memory
		buffer.buf = (char *)malloc(buffer.bufLen);
		if (!buffer.buf)
		{
			fclose(schemaFile);
			fail("Memory allocation error!");
		}

		//Read file contents into buffer
		fread(buffer.buf, buffer.bufLen, 1, schemaFile);
		fclose(schemaFile);

		buffer.bufContent = buffer.bufLen;
		buffer.ioStrm.readWriteToStream = NULL;
		buffer.ioStrm.stream = NULL;

		tmp_err_code = generateSchemaInformedGrammars(&buffer, 1, SCHEMA_FORMAT_XSD_EXI, NULL, schema, NULL);

		if(tmp_err_code != EXIP_OK)
		{
			fail("\n Error reading schema: %d", tmp_err_code);
		}

		free(buffer.buf);
	}
}

/* END: Schema-mode tests */

Suite* exip_suite(void)
{
	Suite *s = suite_create("XSI:TYPE");

	{
	  /* Schema-less test case */
	  TCase *tc_SchLess = tcase_create ("SchemaLess");
	  tcase_add_test (tc_SchLess, test_default_options);
	  suite_add_tcase (s, tc_SchLess);
	}
	{
		/* Schema-mode test case */
		TCase *tc_Schema = tcase_create ("Schema-mode");
		tcase_add_test (tc_Schema, test_simple_schema);
		suite_add_tcase (s, tc_Schema);
	}

	return s;
}

int main (int argc, char *argv[])
{
	int number_failed;
	Suite *s = exip_suite();
	SRunner *sr = srunner_create (s);

	if (argc < 2)
	{
		printf("ERR: Expected test data directory\n");
		exit(1);
	}
	if (strlen(argv[1]) > MAX_PATH_LEN)
	{
		printf("ERR: Test data pathname too long: %u", (unsigned int) strlen(argv[1]));
		exit(1);
	}

	dataDir = argv[1];

#ifdef _MSC_VER
	srunner_set_fork_status(sr, CK_NOFORK);
#endif
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

