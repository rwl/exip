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
 * @file check_exip.c
 * @brief Tests the whole EXIP library with some test input data
 *
 * @date Nov 4, 2011
 * @author Rumen Kyusakov
 * @version 0.1
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

extern const EXISerializer serialize;

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

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults

	// III: Define an external stream for the output if any

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buf, OUTPUT_BUFFER_SIZE, NULL, NULL, SCHEMA_ID_ABSENT, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code = serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.exiHeader returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.startDocument(&testStrm, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startDocument returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.ltu.se/EISLAB/schema-test", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("EXIPEncoder", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("version", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_STRING, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("0.2", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("status", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_STRING, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("alpha", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("This is an example of serializing EXI streams using EXIP low level API", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal, FALSE, 0);

	tmp_err_code += serialize.endElement(&testStrm, FALSE, 0);
	tmp_err_code += serialize.endDocument(&testStrm, FALSE, 0);

	if(tmp_err_code != ERR_OK)
		fail_unless (tmp_err_code == ERR_OK, "serialization ended with error code %d", tmp_err_code);

	// V: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.closeEXIStream ended with error code %d", tmp_err_code);

	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buf, OUTPUT_BUFFER_SIZE, OUTPUT_BUFFER_SIZE, NULL, NULL, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initParser returns an error code %d", tmp_err_code);

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser);
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

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_cookie = TRUE;
	testStrm.header.has_options = TRUE;
	SET_FRAGMENT(testStrm.header.opts.enumOpt);

	// III: Define an external stream for the output if any

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buf, OUTPUT_BUFFER_SIZE, NULL, NULL, SCHEMA_ID_ABSENT, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code = serialize.exiHeader(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.exiHeader returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.startDocument(&testStrm, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startDocument returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("http://www.ltu.se/EISLAB/schema-test", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("EXIPEncoder", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("version", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_STRING, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("0.2", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("status", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_STRING, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.attribute returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("alpha", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("Test", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("beta tests", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code += serialize.endElement(&testStrm, FALSE, 0);

	tmp_err_code += serialize.endElement(&testStrm, FALSE, 0);

	tmp_err_code += asciiToString("Test2", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.startElement returns an error code %d", tmp_err_code);

	tmp_err_code += asciiToString("beta tests -> second root element", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.stringData returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.endElement(&testStrm, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.endElement returns an error code %d", tmp_err_code);

	tmp_err_code = serialize.endDocument(&testStrm, FALSE, 0);
	fail_unless (tmp_err_code == ERR_OK, "serialize.endDocument returns an error code %d", tmp_err_code);

	if(tmp_err_code != ERR_OK)
		fail_unless (tmp_err_code == ERR_OK, "serialization ended with error code %d", tmp_err_code);

	// V: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);
	fail_unless (tmp_err_code == ERR_OK, "serialize.closeEXIStream ended with error code %d", tmp_err_code);

	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buf, OUTPUT_BUFFER_SIZE, OUTPUT_BUFFER_SIZE, NULL, NULL, NULL);
	fail_unless (tmp_err_code == ERR_OK, "initParser returns an error code %d", tmp_err_code);

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser);
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

