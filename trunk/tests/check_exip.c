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

#include <stdlib.h>
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
	errorCode tmp_err_code = UNEXPECTED_ERROR;
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
	tmp_err_code = serialize.initStream(&testStrm, buffer, NULL, SCHEMA_ID_ABSENT, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code = serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.exiHeader returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.startDocument(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startDocument returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.ltu.se/EISLAB/schema-test", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("EXIPEncoder", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("version", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("0.2", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("status", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("alpha", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("This is an example of serializing EXI streams using EXIP low level API", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);

	tmp_err_code += serialize.endElement(&testStrm);
	tmp_err_code += serialize.endDocument(&testStrm);

	if(tmp_err_code != ERR_OK)
		fail_unless (tmp_err_code == ERR_OK, "serialization ended with error code %d", tmp_err_code);

	// V: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.closeEXIStream ended with error code %d", tmp_err_code);


	buffer.bufContent = OUTPUT_BUFFER_SIZE;
	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buffer, NULL, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initParser returns an error code %d", tmp_err_code);

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser, TRUE);
	fail_unless (tmp_err_code == ERR_OK, "parsing the header returns an error code %d", tmp_err_code);

	// V: Parse the body of the EXI stream

	while(tmp_err_code == ERR_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&testParser);
	fail_unless (tmp_err_code == PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
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
	errorCode tmp_err_code = UNEXPECTED_ERROR;
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
	tmp_err_code = serialize.initStream(&testStrm, buffer, NULL, SCHEMA_ID_ABSENT, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code = serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.exiHeader returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.startDocument(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startDocument returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.ltu.se/EISLAB/schema-test", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("EXIPEncoder", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("version", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("0.2", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("status", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("alpha", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("Test", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("beta tests", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += asciiToString("Test2", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("beta tests -> second root element", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.endElement(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.endElement returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.endDocument(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.endDocument returns an error code %d", tmp_err_code);

	if(tmp_err_code != ERR_OK)
		fail_unless (tmp_err_code == ERR_OK, "serialization ended with error code %d", tmp_err_code);

	// V: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.closeEXIStream ended with error code %d", tmp_err_code);

	buffer.bufContent = OUTPUT_BUFFER_SIZE;

	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buffer, NULL, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initParser returns an error code %d", tmp_err_code);

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser, FALSE);
	fail_unless (tmp_err_code == ERR_OK, "parsing the header returns an error code %d", tmp_err_code);

	// V: Parse the body of the EXI stream

	while(tmp_err_code == ERR_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&testParser);
	fail_unless (tmp_err_code == PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
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

	errorCode tmp_err_code = UNEXPECTED_ERROR;
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
	tmp_err_code = serialize.initStream(&testStrm, buffer, NULL, SCHEMA_ID_ABSENT, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code += serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "exiHeader returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.startDocument(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);

	qname.uri = &NS_EMPTY_STR;
	qname.localName = &ELEM_COBS_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <cobs>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ATTR_NODE_ID_STR;
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType); // nodeId="..."
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%d", 111);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_TM_STAMP_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <timestamp>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	chVal.str = "2012-12-31T12:09:3.44";
	chVal.length = strlen("2012-12-31T12:09:3.44");
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </timestamp>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_RPM_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <rpm>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%d", 222);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </rpm>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_ACC_RNDS_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <accRounds>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	sprintf(valStr, "%d", 4212);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </accRounds>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_TEMP_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <temp>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_LEFT_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <left>
	sprintf(valStr, "%f", 32.2);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </left>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_RIGHT_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <right>
	sprintf(valStr, "%f", 34.23);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </right>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </temp>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_RSSI_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <RSSI>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_AVG_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <avg>
	sprintf(valStr, "%d", 123);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </avg>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_MAX_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <max>
	sprintf(valStr, "%d", 2746);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </max>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_MIN_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <min>
	sprintf(valStr, "%d", 112);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </min>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </RSSI>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	qname.localName = &ELEM_BATTERY_STR;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType); // <battery>
	sprintf(valStr, "%f", 1.214);
	chVal.str = valStr;
	chVal.length = strlen(valStr);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm); // </battery>
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	tmp_err_code += serialize.endElement(&testStrm); // </cobs>
	tmp_err_code += serialize.endDocument(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);
	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code += serialize.closeEXIStream(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.* returns an error code %d", tmp_err_code);

	buffer.bufContent = OUTPUT_BUFFER_SIZE;
	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buffer, NULL, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initParser returns an error code %d", tmp_err_code);

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser, TRUE);
	fail_unless (tmp_err_code == ERR_OK, "parsing the header returns an error code %d", tmp_err_code);

	// V: Parse the body of the EXI stream

	while(tmp_err_code == ERR_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&testParser);
	fail_unless (tmp_err_code == PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
}
END_TEST

/* END: SchemaLess tests */

Suite* exip_suite(void)
{
	Suite *s = suite_create("EXIP");

	{
	  /* Schema-less test case */
	  TCase *tc_SchLess = tcase_create ("SchemaLess");
	  tcase_add_test (tc_SchLess, test_default_options);
	  tcase_add_test (tc_SchLess, test_fragment_option);
	  tcase_add_test (tc_SchLess, test_value_part_zero);
	  suite_add_tcase (s, tc_SchLess);
	}

	return s;
}

int main (void)
{
	int number_failed;
	Suite *s = exip_suite();
	SRunner *sr = srunner_create (s);
#ifdef _MSC_VER
	srunner_set_fork_status(sr, CK_NOFORK);
#endif
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

