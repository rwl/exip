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
	testStrm.header.has_options = TRUE;

	// III: Define an external stream for the output if any

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, NULL, SCHEMA_ID_EMPTY, NULL);
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
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("xsitypetest", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.w3.org/2001/XMLSchema-instance", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("type", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, TRUE, &valueType);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.w3.org/2001/XMLSchema", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("integer", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.qnameData(&testStrm, qname);
	fail_unless (tmp_err_code == ERR_OK, "serialize.qnameData returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.intData(&testStrm, 144);
	fail_unless (tmp_err_code == ERR_OK, "serialize.intData returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endElement(&testStrm);

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

/* END: SchemaLess tests */

Suite* exip_suite(void)
{
	Suite *s = suite_create("XSI:TYPE");

	{
	  /* Schema-less test case */
	  TCase *tc_SchLess = tcase_create ("SchemaLess");
	  tcase_add_test (tc_SchLess, test_default_options);
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

