/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file check_exip.c
 * @brief Tests the whole EXIP library with some test input data
 *
 * @date Nov 4, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "procTypes.h"
#include "EXISerializer.h"
#include "EXIParser.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"

#define MAX_PATH_LEN 200
#define OUTPUT_BUFFER_SIZE 2000
/* Location for external test data */
static char *dataDir;

static size_t writeFileOutputStream(void* buf, size_t readSize, void* stream);
static size_t readFileInputStream(void* buf, size_t readSize, void* stream);
static void parseSchema(const char* fileName, EXIPSchema* schema);

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

	tmp_err_code = parseHeader(&testParser, TRUE);
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

START_TEST (test_fragment_option)
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
	testStrm.header.has_cookie = TRUE;
	testStrm.header.has_options = TRUE;
	SET_FRAGMENT(testStrm.header.opts.enumOpt);

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

	tmp_err_code += asciiToString("Test", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("beta tests", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += asciiToString("Test2", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("beta tests -> second root element", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.endElement(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.endElement returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.endDocument(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.endDocument returns an error code %d", tmp_err_code);

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

START_TEST (test_value_part_zero)
{
	const String NS_EMPTY_STR = {NULL, 0};
	const String ELEM_COBS_STR = {"cobs", 4};
	const String ELEM_TM_STAMP_STR = {"timestamp", 9};
	const String ELEM_RPM_STR = {"rpm", 3};
	const String ELEM_ACC_RNDS_STR = {"accRounds", 9};
	const String ELEM_TEMP_STR = {"temp", 4};
	const String ELEM_LEFT_STR = {"left", 4};
	const String ELEM_RIGHT_STR = {"right", 5};
	const String ELEM_RSSI_STR = {"RSSI", 4};
	const String ELEM_MIN_STR = {"min", 3};
	const String ELEM_MAX_STR = {"max", 3};
	const String ELEM_AVG_STR = {"avg", 3};
	const String ELEM_BATTERY_STR = {"battery", 7};
	const String ATTR_NODE_ID_STR = {"nodeId", 6};

	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	EXIStream testStrm;
	String uri;
	String ln;
	QName qname = {&uri, &ln, NULL};
	String chVal;
	BinaryBuffer buffer;
	EXITypeClass valueType;
	char valStr[50];
	char buf[OUTPUT_BUFFER_SIZE];
	Parser testParser;

	buffer.buf = buf;
	buffer.bufLen = OUTPUT_BUFFER_SIZE;
	buffer.bufContent = 0;

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_options = TRUE;
	testStrm.header.opts.valuePartitionCapacity = 0;

	// III: Define an external stream for the output if any
	buffer.ioStrm.readWriteToStream = NULL;
	buffer.ioStrm.stream = NULL;

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, NULL);
	fail_unless (tmp_err_code == EXIP_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code += serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "exiHeader returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.startDocument(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	qname.uri = &NS_EMPTY_STR;
	qname.localName = &ELEM_COBS_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <cobs>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_NODE_ID_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // nodeId="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%d", 111);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_TM_STAMP_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <timestamp>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	chVal.str = "2012-12-31T12:09:3.44";
	chVal.length = strlen("2012-12-31T12:09:3.44");
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </timestamp>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_RPM_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <rpm>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%d", 222);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </rpm>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_ACC_RNDS_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <accRounds>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%d", 4212);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </accRounds>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_TEMP_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <temp>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_LEFT_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <left>
	sprintf(valStr, "%f", 32.2);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </left>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_RIGHT_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <right>
	sprintf(valStr, "%f", 34.23);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </right>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </temp>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_RSSI_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <RSSI>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_AVG_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <avg>
	sprintf(valStr, "%d", 123);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </avg>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_MAX_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <max>
	sprintf(valStr, "%d", 2746);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </max>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_MIN_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <min>
	sprintf(valStr, "%d", 112);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </min>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </RSSI>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_BATTERY_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <battery>
	sprintf(valStr, "%f", 1.214);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </battery>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </cobs>
	tmp_err_code += serialize.endDocument(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code += serialize.closeEXIStream(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	buffer.bufContent = OUTPUT_BUFFER_SIZE;
	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buffer, NULL);
	fail_unless (tmp_err_code == EXIP_OK, "initParser returns an error code %d", tmp_err_code);

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser, TRUE);
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

START_TEST (test_recursive_defs)
{
	const String NS_EMPTY_STR = {NULL, 0};
	const String ELEM_OBJ_STR = {"obj", 3};
	const String ELEM_STR_STR = {"str", 3};
	const String ELEM_LIST_STR = {"list", 4};

	const String ATTR_XSTR_STR = {"xsss", 4};
	const String ATTR_XTEMP_STR = {"x-template", 10};
	const String ATTR_NAME_STR = {"name", 4};
	const String ATTR_VAL_STR = {"val", 3};

	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	EXIStream testStrm;
	String uri;
	String ln;
	QName qname = {&uri, &ln, NULL};
	String chVal;
	BinaryBuffer buffer;
	EXITypeClass valueType;
	char valStr[50];
	char buf[OUTPUT_BUFFER_SIZE];
	Parser testParser;

	buffer.buf = buf;
	buffer.bufLen = OUTPUT_BUFFER_SIZE;
	buffer.bufContent = 0;

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_options = TRUE;
	SET_STRICT(testStrm.header.opts.enumOpt);
	SET_ALIGNMENT(testStrm.header.opts.enumOpt, BYTE_ALIGNMENT);

	// III: Define an external stream for the output if any
	buffer.ioStrm.readWriteToStream = NULL;
	buffer.ioStrm.stream = NULL;

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, NULL);
	fail_unless (tmp_err_code == EXIP_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code += serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "exiHeader returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.startDocument(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	qname.uri = &NS_EMPTY_STR;
	qname.localName = &ELEM_OBJ_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <obj>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_XSTR_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // xsss="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "http://obix.org/ns/schema/1.1");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_XTEMP_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // x-template="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "ipu_inst.xml");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_STR_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <str>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_NAME_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // name="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "interworkingProxyID");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_VAL_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // val="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "IPU_6LoWPAN");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </str>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_LIST_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <list>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_NAME_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // name="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "supportedTechnologies");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_OBJ_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <obj>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	qname.localName = &ELEM_STR_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <str>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_NAME_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // name="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "anPhysical");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_VAL_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // val="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "2003_2_4GHz");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </str>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	qname.localName = &ELEM_STR_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <str>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_NAME_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // name="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "anStandard");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_VAL_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // val="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "Bee_1_0");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </str>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);


	qname.localName = &ELEM_STR_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <str>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_NAME_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // name="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "anProfile");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_VAL_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // val="..."
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%s", "Bee_HA");
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </str>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endElement(&testStrm); // </obj>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endElement(&testStrm); // </list>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endElement(&testStrm); // </obj>
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endDocument(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);
	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code += serialize.closeEXIStream(&testStrm);
	fail_unless (tmp_err_code == EXIP_OK, "serialize.* returns an error code %d", tmp_err_code);

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

#define OUTPUT_BUFFER_SIZE_LARGE_DOC 20000
#define MAX_XSD_FILES_COUNT 10 // up to 10 XSD files

/* BEGIN: Schema-mode tests */
START_TEST (test_large_doc_str_pattern)
{
	const String NS_EMPTY_STR = {NULL, 0};

	const String ELEM_CONFIGURATION = {"configuration", 13};
	const String ELEM_CAPSWITCH = {"capable-switch", 14};
	const String ELEM_RESOURCES = {"resources", 9};
	const String ELEM_PORT = {"port", 4};
	const String ELEM_RESID = {"resource-id", 11};
	const String ELEM_ADMIN_STATE = {"admin-state", 11};
	const String ELEM_NORECEIVE = {"no-receive", 10};
	const String ELEM_NOFORWARD = {"no-forward", 10};
	const String ELEM_NOPACKET = {"no-packet-in", 12};

	const String ELEM_LOGSWITCHES = {"logical-switches", 16};
	const String ELEM_SWITCH = {"switch", 6};
	const String ELEM_ID = {"id", 2};
	const String ELEM_DATAPATHID = {"datapath-id", 11};
	const String ELEM_ENABLED = {"enabled", 7};
	const String ELEM_LOSTCONNBEH = {"lost-connection-behavior", 24};
	const String ELEM_CONTROLLERS = {"controllers", 11};
	const String ELEM_CONTROLLER = {"controller", 10};
	const String ELEM_ROLE = {"role", 4};
	const String ELEM_IPADDR = {"ip-address", 10};
	const String ELEM_PROTOCOL = {"protocol", 8};
	const String ELEM_STATE = {"state", 5};
	const String ELEM_CONNSTATE = {"connection-state", 16};
	const String ELEM_CURRVER = {"current-version", 15};

	const char * PORT_STR = "port";
	const char * SWITCH_STR = "switch";
	const char * STATE_UP_STR = "up";
	const char * DATAPATH_STR = "10:14:56:7C:89:46:7A:";
	const char * LOST_CONN_BEHAVIOR_STR = "failSecureMode";
	const char * CTRL_STR = "ctrl";
	const char * ROLE_STR = "equal";
	const char * IPADDR_STR = "10.10.10.";
	const char * PROTOCOL_STR = "tcp";
	const char * VER_STR = "1.0";

	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	FILE *outfile;
	char* sourceFile = "testOutputFile.exi";
	EXIPSchema schema;
//	struct timespec start;
//	struct timespec end;
	const char* schemafname = "exip/schema_demo.exi";
	EXIStream testStrm;
	String uri;
	String ln;
	QName qname = {&uri, &ln, NULL};
	String chVal;
	char buf[OUTPUT_BUFFER_SIZE_LARGE_DOC];
	BinaryBuffer buffer;
	int i, j;
	char strbuffer[32];

	buffer.buf = buf;
	buffer.bufLen = OUTPUT_BUFFER_SIZE_LARGE_DOC;
	buffer.bufContent = 0;

	parseSchema(schemafname, &schema);

	outfile = fopen(sourceFile, "wb" );
	fail_if(!outfile, "Unable to open file %s", sourceFile);

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_cookie = TRUE;
	testStrm.header.has_options = TRUE;
	testStrm.header.opts.valueMaxLength = 300;
	testStrm.header.opts.valuePartitionCapacity = INDEX_MAX;
	SET_STRICT(testStrm.header.opts.enumOpt);

	// III: Define an external stream for the output if any
	buffer.ioStrm.readWriteToStream = writeFileOutputStream;
	buffer.ioStrm.stream = outfile;
	//buffer.ioStrm.readWriteToStream = NULL;
	//buffer.ioStrm.stream = NULL;
// printf("line:%d: %d\n", __LINE__, tmp_err_code);
	//clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, &schema);
	fail_unless(tmp_err_code == EXIP_OK);

//printf("line:%d: %d\n", __LINE__, tmp_err_code);
//                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code += serialize.exiHeader(&testStrm);
// printf("line:%d: %d\n", __LINE__, tmp_err_code);
	tmp_err_code += serialize.startDocument(&testStrm);
// printf("line:%d: %d\n", __LINE__, tmp_err_code);
	qname.uri = &NS_EMPTY_STR;
	qname.localName = &ELEM_CONFIGURATION;
	EXITypeClass typeClass;
	tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

	qname.uri = &NS_EMPTY_STR;
	qname.localName = &ELEM_CAPSWITCH;
	tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

	qname.uri = &NS_EMPTY_STR;
	qname.localName = &ELEM_RESOURCES;
	tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

// printf("line:%d: %d\n", __LINE__, tmp_err_code);
	for (i = 0; i < 100; i++)
	{
		qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_PORT;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_RESID;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

	    sprintf(strbuffer, "%s%d", PORT_STR, i);
	    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_CONFIGURATION;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_ADMIN_STATE;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += asciiToString(STATE_UP_STR, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_NORECEIVE;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += serialize.booleanData(&testStrm, FALSE);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_NOFORWARD;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += serialize.booleanData(&testStrm, FALSE);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_NOPACKET;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += serialize.booleanData(&testStrm, TRUE);
		tmp_err_code += serialize.endElement(&testStrm);

		tmp_err_code += serialize.endElement(&testStrm);

		tmp_err_code += serialize.endElement(&testStrm);
	}
	tmp_err_code += serialize.endElement(&testStrm);

	qname.uri = &NS_EMPTY_STR;
	qname.localName = &ELEM_LOGSWITCHES;
	tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

	for (i = 0; i < 20; i++)
	{
	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_SWITCH;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_ID;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

		sprintf(strbuffer, "%s%d", SWITCH_STR, i);
	    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_DATAPATHID;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

		sprintf(strbuffer, "%s%d", DATAPATH_STR, 10 + i);
	    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_ENABLED;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += asciiToString("true", &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_LOSTCONNBEH;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += asciiToString(LOST_CONN_BEHAVIOR_STR, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);
// printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
//		if ( i == 0 )
//		{
	        qname.uri = &NS_EMPTY_STR;
	        qname.localName = &ELEM_RESOURCES;
	        tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			for (j = 0; j < 100; j++)
			{
	            qname.uri = &NS_EMPTY_STR;
	            qname.localName = &ELEM_PORT;
	            tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

				sprintf(strbuffer, "%s%d", PORT_STR, j);
	            tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
	            tmp_err_code += serialize.stringData(&testStrm, chVal);
				tmp_err_code += serialize.endElement(&testStrm);
			}
			tmp_err_code += serialize.endElement(&testStrm);
//		}
// printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_CONTROLLERS;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
// printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_CONTROLLER;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
// printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_ID;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
// printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
		sprintf(strbuffer, "%s%d", CTRL_STR, i);
	    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);
// printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_ROLE;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += asciiToString(ROLE_STR, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_IPADDR;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
		sprintf(strbuffer, "%s%d", IPADDR_STR, i);
	    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_PORT;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
		sprintf(strbuffer, "%d", 6620 + i);
	    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_PROTOCOL;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += asciiToString(PROTOCOL_STR, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_STATE;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_CONNSTATE;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
	    tmp_err_code += asciiToString(STATE_UP_STR, &chVal, &testStrm.memList, FALSE);
	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

// printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
	    qname.uri = &NS_EMPTY_STR;
	    qname.localName = &ELEM_CURRVER;
	    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
// printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
	    tmp_err_code += asciiToString(VER_STR, &chVal, &testStrm.memList, FALSE);

	    tmp_err_code += serialize.stringData(&testStrm, chVal);
		tmp_err_code += serialize.endElement(&testStrm);

		tmp_err_code += serialize.endElement(&testStrm);

		tmp_err_code += serialize.endElement(&testStrm);

		tmp_err_code += serialize.endElement(&testStrm);

		tmp_err_code += serialize.endElement(&testStrm);
	}

	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += serialize.endElement(&testStrm);
	tmp_err_code += serialize.endDocument(&testStrm);
	fail_unless(tmp_err_code == EXIP_OK);

//	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);

//                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
//                total += ((end.tv_sec * SEC2NANO) + end.tv_nsec) - ((start.tv_sec * SEC2NANO) + start.tv_nsec);

    fclose(outfile);

    /* DECODE */
    {
    	Parser testParser;
    	FILE *infile;

    	buffer.buf = buf;
    	buffer.bufContent = 0;
    	buffer.bufLen = OUTPUT_BUFFER_SIZE_LARGE_DOC;
    	unsigned int eventCount;

    	// Parsing steps:

    	// I.A: First, read in the schema
    	parseSchema(schemafname, &schema);

    	// I.B: Define an external stream for the input to the parser if any
    	infile = fopen(sourceFile, "rb" );
    	if(!infile)
    		fail("Unable to open file %s", sourceFile);

    	buffer.ioStrm.readWriteToStream = readFileInputStream;
    	buffer.ioStrm.stream = infile;

    	// II: Second, initialize the parser object
    	tmp_err_code = initParser(&testParser, buffer, &eventCount);
    	fail_unless (tmp_err_code == EXIP_OK, "initParser returns an error code %d", tmp_err_code);

    	// IV: Parse the header of the stream
		tmp_err_code = parseHeader(&testParser, FALSE);
		fail_unless (tmp_err_code == EXIP_OK, "parsing the header returns an error code %d", tmp_err_code);

		tmp_err_code = setSchema(&testParser, &schema);
		fail_unless (tmp_err_code == EXIP_OK, "setSchema() returns an error code %d", tmp_err_code);

		// V: Parse the body of the EXI stream
		while(tmp_err_code == EXIP_OK)
		{
			tmp_err_code = parseNext(&testParser);
		}

		// VI: Free the memory allocated by the parser object
		destroyParser(&testParser);
		fclose(infile);
		fail_unless (tmp_err_code == EXIP_PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
    }

    remove(sourceFile);
	destroySchema(&schema);
}
END_TEST

/* END: Schema-mode tests */

/* Helper functions */
static size_t writeFileOutputStream(void* buf, size_t readSize, void* stream)
{
	FILE *outfile = (FILE*) stream;
	return fwrite(buf, 1, readSize, outfile);
}

static size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
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


Suite* exip_suite(void)
{
	Suite *s = suite_create("EXIP");
	{
		/* Schema-less test case */
		TCase *tc_SchLess = tcase_create ("SchemaLess");
		tcase_add_test (tc_SchLess, test_default_options);
		tcase_add_test (tc_SchLess, test_fragment_option);
		tcase_add_test (tc_SchLess, test_value_part_zero);
		tcase_add_test (tc_SchLess, test_recursive_defs);
		suite_add_tcase (s, tc_SchLess);
	}
	{
		/* Schema-mode test case */
		TCase *tc_Schema = tcase_create ("Schema-mode");
		tcase_add_test (tc_Schema, test_large_doc_str_pattern);
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

