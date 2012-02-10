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
 * @file check_grammar.c
 * @brief Tests the EXI grammar module
 *
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <check.h>
#include "grammars.h"
#include "memManagement.h"

/* BEGIN: grammars tests */

START_TEST (test_createDocGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIStream testStream;
	EXIGrammar testGrammar;
	char buf[2];
	buf[0] = (char) 0xD4; /* 0b11010100 */
	buf[1] = (char) 0x60; /* 0b01100000 */

	testStream.context.bitPointer = 0;
	makeDefaultOpts(&testStream.header.opts);
	testStream.buffer = buf;
	testStream.bufLen = 2;
	testStream.bufContent = 2;
	testStream.ioStrm.readWriteToStream = NULL;
	testStream.ioStrm.stream = NULL;
	testStream.context.bufferIndx = 0;
	initAllocList(&testStream.memList);

	err = createDocGrammar(&testGrammar, &testStream, NULL);

	fail_unless (err == ERR_OK, "createDocGrammar returns an error code %d", err);

	//TODO: add more tests!
}
END_TEST

START_TEST (test_processNextProduction)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIStream strm;
	EXIEvent event;
	size_t nonTermID_out;
	ContentHandler handler;
	EXIGrammar testGrammar;

	initAllocList(&strm.memList);

	err = createDocGrammar(&testGrammar, &strm, NULL);
	fail_unless (err == ERR_OK, "createDocGrammar returns an error code %d", err);

	err = pushGrammar(&strm.gStack, &testGrammar);
	fail_unless (err == ERR_OK, "pushGrammar returns an error code %d", err);

	strm.context.nonTermID = 4;
	err = processNextProduction(&strm, &event, &nonTermID_out, &handler, NULL);
	fail_unless (err == INCONSISTENT_PROC_STATE, "processNextProduction does not return the correct error code");
}
END_TEST

START_TEST (test_pushGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIGrammarStack* testGrStack = NULL;
	EXIGrammar docGr;
	EXIStream strm;
	EXIGrammar testElementGrammar;

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

	err = createDocGrammar(&docGr, &strm, NULL);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &docGr);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);

	err = createBuildInElementGrammar(&testElementGrammar, &strm);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &testElementGrammar);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack == NULL);
	fail_if(testGrStack->nextInStack->grammar != &docGr);
}
END_TEST

START_TEST (test_popGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIGrammarStack* testGrStack = NULL;
	EXIGrammar docGr;
	EXIStream strm;
	EXIGrammar testElementGrammar;
	EXIGrammar* testGR;

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

	err = createDocGrammar(&docGr, &strm, NULL);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &docGr);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);

	err = createBuildInElementGrammar(&testElementGrammar, &strm);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &testElementGrammar);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack == NULL);

	popGrammar(&testGrStack, &testGR);
	fail_if(testGrStack->nextInStack != NULL);
	fail_if(testGR == NULL);
	fail_if(testGR != &testElementGrammar);
}
END_TEST

START_TEST (test_createBuildInElementGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIGrammar testElementGrammar;
	EXIStream strm;

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

	err = createBuildInElementGrammar(&testElementGrammar, &strm);
	fail_unless (err == ERR_OK, "createBuildInElementGrammar returns error code %d", err);

	//TODO: add more tests!
}
END_TEST

/* END: grammars tests */


/* BEGIN: events tests */

/* END: events tests */


/* BEGIN: rules tests */

START_TEST (test_insertZeroProduction)
{
	DynGrammarRule rule;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	Production prod0Arr[2];
	EXIEvent event = {EVENT_CH, {VALUE_TYPE_BOOLEAN, UINT16_MAX}};

	rule.part[0].prodArraySize = 0;
	rule.part0Dimension = 1;
	rule.part[0].prodArray = prod0Arr;

	tmp_err_code = insertZeroProduction(&rule, event, 5, 0, 0);
	fail_unless (tmp_err_code == ERR_OK, "insertZeroProduction returns an error code %d", tmp_err_code);
	fail_unless (rule.part[0].prodArraySize == 1);
}
END_TEST

/* END: rules tests */


Suite * grammar_suite (void)
{
  Suite *s = suite_create ("Grammar");

  {
	  /* Grammars test case */
	  TCase *tc_gGrammars = tcase_create ("Grammars");
	  tcase_add_test (tc_gGrammars, test_createDocGrammar);
	  tcase_add_test (tc_gGrammars, test_processNextProduction);
	  tcase_add_test (tc_gGrammars, test_pushGrammar);
	  tcase_add_test (tc_gGrammars, test_popGrammar);
	  tcase_add_test (tc_gGrammars, test_createBuildInElementGrammar);
	  suite_add_tcase (s, tc_gGrammars);
  }

  {
	  /* Events test case */
	  TCase *tc_gEvents = tcase_create ("Events");
	  suite_add_tcase (s, tc_gEvents);
  }

  {
	  /* Rules test case */
	  TCase *tc_gRules = tcase_create ("Rules");
	  tcase_add_test (tc_gRules, test_insertZeroProduction);
	  suite_add_tcase (s, tc_gRules);
  }

  return s;
}

int main (void)
{
	int number_failed;
	Suite *s = grammar_suite();
	SRunner *sr = srunner_create (s);
#ifdef _MSC_VER
	srunner_set_fork_status(sr, CK_NOFORK);
#endif
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
