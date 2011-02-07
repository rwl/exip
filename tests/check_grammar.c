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

/* BEGIN: grammars tests */

START_TEST (test_createDocGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIStream testStream;
	testStream.bitPointer = 0;
	struct EXIOptions options;
	makeDefaultOpts(&options);
	testStream.opts = &options;
	char buf[2];
	buf[0] = (char) 0b11010100;
	buf[1] = (char) 0b01100000;
	testStream.buffer = buf;
	testStream.bufLen = 2;
	testStream.bufferIndx = 0;

	struct EXIGrammar testGrammar;

	err = createDocGrammar(&testGrammar, &options, &testStream, NULL);

	fail_unless (err == ERR_OK, "getBuildInDocGrammar returns error code %d", err);

	//TODO: add more tests!
}
END_TEST

START_TEST (test_processNextProduction)
{
	fail("Test not implemented yet!");
}
END_TEST

START_TEST (test_pushGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIGrammarStack testGr;
	EXIGrammarStack* testGrStack = &testGr;
	struct EXIOptions options;
	makeDefaultOpts(&options);
	EXIStream strm;

	err = createDocGrammar(testGrStack, &options, &strm, NULL);
	fail_if(err != ERR_OK);

	struct EXIGrammar testElementGrammar;
	err = createBuildInElementGrammar(&testElementGrammar, &options, &strm);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &testElementGrammar);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack == NULL);
}
END_TEST

START_TEST (test_popGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIGrammarStack testGr;
	EXIGrammarStack* testGrStack = &testGr;
	struct EXIOptions options;
	makeDefaultOpts(&options);
	EXIStream strm;

	err = createDocGrammar(testGrStack, &options, &strm, NULL);
	fail_if(err != ERR_OK);

	struct EXIGrammar testElementGrammar;
	err = createBuildInElementGrammar(&testElementGrammar, &options, &strm);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &testElementGrammar);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack == NULL);

	struct EXIGrammar* testGR;
	err = popGrammar(&testGrStack, &testGR);
	fail_unless (err == ERR_OK, "popGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack != NULL);
	fail_if(testGR == NULL);
	fail_if(testGR->nextInStack != NULL);
}
END_TEST

START_TEST (test_createBuildInElementGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	struct EXIGrammar testElementGrammar;
	struct EXIOptions options;
	makeDefaultOpts(&options);
	EXIStream strm;

	err = createBuildInElementGrammar(&testElementGrammar, &options, &strm);
	fail_unless (err == ERR_OK, "createBuildInElementGrammar returns error code %d", err);

	//TODO: add more tests!
}
END_TEST

START_TEST (test_createGrammarPool)
{
	errorCode err = UNEXPECTED_ERROR;
	GrammarPool* testPool;
	err = createGrammarPool(&testPool);

	fail_unless (err == ERR_OK, "createGrammarPool returns error code %d", err);
}
END_TEST

START_TEST (test_checkGrammarInPool)
{
	errorCode err = UNEXPECTED_ERROR;
	GrammarPool* testPool;
	err = createGrammarPool(&testPool);

	fail_unless (err == ERR_OK, "createGrammarPool returns error code %d", err);

	uint32_t uriRowID = 10;
	uint32_t lnRowID = 22;
	struct EXIGrammar testElementGrammar;
	struct EXIOptions options;
	makeDefaultOpts(&options);
	EXIStream strm;

	unsigned char is_found = 1;
	struct EXIGrammar* result = NULL;

	err = checkGrammarInPool(testPool, uriRowID, lnRowID, &is_found, &result);

	fail_unless (err == ERR_OK, "checkGrammarInPool returns error code %d", err);
	fail_unless (is_found == 0, "checkGrammarInPool does not set is_found correctly %d", is_found);
	fail_unless (result == NULL, "checkGrammarInPool does not set result correctly");

	err = createBuildInElementGrammar(&testElementGrammar, &options, &strm);
	fail_unless (err == ERR_OK, "createBuildInElementGrammar returns error code %d", err);

	err = addGrammarInPool(testPool, uriRowID, lnRowID, &testElementGrammar);

	fail_unless (err == ERR_OK, "addGrammarInPool returns error code %d", err);

	err = checkGrammarInPool(testPool, uriRowID, lnRowID, &is_found, &result);

	fail_unless (err == ERR_OK, "checkGrammarInPool returns error code %d", err);
	fail_unless (is_found == 1, "checkGrammarInPool does not set is_found correctly %d", is_found);
	fail_unless (result != NULL, "checkGrammarInPool does not set result correctly");
}
END_TEST

START_TEST (test_addGrammarInPool)
{
	errorCode err = UNEXPECTED_ERROR;
	GrammarPool* testPool;
	err = createGrammarPool(&testPool);

	fail_unless (err == ERR_OK, "createGrammarPool returns error code %d", err);

	uint32_t uriRowID = 10;
	uint32_t lnRowID = 22;
	struct EXIGrammar testElementGrammar;
	struct EXIOptions options;
	makeDefaultOpts(&options);
	EXIStream strm;

	err = createBuildInElementGrammar(&testElementGrammar, &options, &strm);
	fail_unless (err == ERR_OK, "createBuildInElementGrammar returns error code %d", err);

	err = addGrammarInPool(testPool, uriRowID, lnRowID, &testElementGrammar);

	fail_unless (err == ERR_OK, "addGrammarInPool returns error code %d", err);
}
END_TEST

/* END: grammars tests */


/* BEGIN: events tests */

START_TEST (test_getEventCode1)
{
	EventCode test_code = getEventCode1(101);
	fail_unless(test_code.size == 1, "getEventCode1 creates event with unexpected number of parts");
	fail_unless(test_code.code[0] == 101, "getEventCode1 creates event code with wrong code number");
}
END_TEST

START_TEST (test_getEventCode2)
{
	EventCode test_code = getEventCode2(101, 203);
	fail_unless(test_code.size == 2, "test_getEventCode2 creates event with unexpected number of parts");
	fail_unless(test_code.code[0] == 101 && test_code.code[1] == 203, "getEventCode2 creates event code with wrong code number");
}
END_TEST

START_TEST (test_getEventCode3)
{
	EventCode test_code = getEventCode3(101, 203, 51);
	fail_unless(test_code.size == 3, "getEventCode3 creates event with unexpected number of parts");
	fail_unless(test_code.code[0] == 101 && test_code.code[1] == 203 && test_code.code[2] == 51,
			    "getEventCode3 creates event code with wrong code number");
}
END_TEST

/* END: events tests */


/* BEGIN: rules tests */

START_TEST (test_initGrammarRule)
{
	errorCode err = UNEXPECTED_ERROR;
	GrammarRule rule;
	EXIStream strm;
	err = initGrammarRule(&rule, &strm);

	fail_unless (err == ERR_OK, "initGrammarRule returns error code %d", err);

	fail_unless(rule.bits[0] == 0 && rule.bits[1] == 0 && rule.bits[2] == 0,
				"initGrammarRule does not initialize the number of bits used for the integers constituting the EventCode");
	fail_unless(rule.nonTermID == GR_VOID_NON_TERMINAL && rule.prodCount == 0 && rule.prodDimension == DEFAULT_PROD_ARRAY_DIM,
			    "initGrammarRule doesn't initialize the GrammarRule structure correctly");
	fail_if(rule.prodArray == NULL);
}
END_TEST

START_TEST (test_addProduction)
{
	errorCode err = UNEXPECTED_ERROR;
	GrammarRule rule;
	EXIStream strm;
	err = initGrammarRule(&rule, &strm);
	fail_unless (err == ERR_OK, "initGrammarRule returns error code %d", err);
	EventCode eCode = getEventCode2(20,12);
	EXIEvent event;
	event.eventType = EVENT_SE_ALL;
	event.valueType = VALUE_TYPE_NONE;
	unsigned int nonTermID = GR_DOC_CONTENT;

	err = addProduction(&rule, eCode, event, nonTermID);
	fail_unless (err == ERR_OK, "addProduction returns error code %d", err);

	fail_unless(rule.prodCount == 1 && rule.prodDimension == DEFAULT_PROD_ARRAY_DIM,
				"addProduction does not initialize prodCount and/or prodDimension correctly");
	fail_unless(rule.prodArray[0].code.size == 2 && rule.prodArray[0].code.code[0] == 20 &&
			    rule.prodArray[0].code.code[1] == 12,
			    "addProduction does not set the EventCode correctly");

	fail_unless(rule.prodArray[0].event.eventType == EVENT_SE_ALL, "addProduction does not set the EXI EventType correctly");
	fail_unless(rule.prodArray[0].nonTermID == GR_DOC_CONTENT, "addProduction does not set the nonTermID correctly");

	rule.prodCount = DEFAULT_PROD_ARRAY_DIM;

	err = addProduction(&rule, eCode, event, nonTermID);
	fail_unless (err == ERR_OK, "addProduction returns error code %d", err);

	fail_unless(rule.prodCount == DEFAULT_PROD_ARRAY_DIM + 1 && rule.prodDimension == 2*DEFAULT_PROD_ARRAY_DIM,
				"addProduction does not initialize prodCount and/or prodDimension correctly");
	fail_unless(rule.prodArray[DEFAULT_PROD_ARRAY_DIM].code.size == 2 && rule.prodArray[DEFAULT_PROD_ARRAY_DIM].code.code[0] == 20 &&
				rule.prodArray[DEFAULT_PROD_ARRAY_DIM].code.code[1] == 12,
				"addProduction does not set the EventCode correctly");

	fail_unless(rule.prodArray[DEFAULT_PROD_ARRAY_DIM].event.eventType == EVENT_SE_ALL, "addProduction does not set the EXI EventType correctly");
	fail_unless(rule.prodArray[DEFAULT_PROD_ARRAY_DIM].nonTermID == GR_DOC_CONTENT, "addProduction does not set the nonTermID correctly");
}
END_TEST

START_TEST (test_insertZeroProduction)
{
	errorCode err = UNEXPECTED_ERROR;
	GrammarRule rule;
	EXIStream strm;
	err = initGrammarRule(&rule, &strm);
	fail_unless (err == ERR_OK, "initGrammarRule returns error code %d", err);
	EventCode eCode = getEventCode2(0,0);
	EXIEvent event;
	event.eventType = EVENT_SE_ALL;
	event.valueType = VALUE_TYPE_NONE;
	rule.bits[0] = 0;
	rule.bits[1] = 0;
	unsigned int nonTermID = GR_DOC_CONTENT;
	err = addProduction(&rule, eCode, event, nonTermID);
	fail_unless (err == ERR_OK, "addProduction returns error code %d", err);
	fail_unless(rule.prodCount == 1 && rule.prodDimension == DEFAULT_PROD_ARRAY_DIM,
				"addProduction does not initialize prodCount and/or prodDimension correctly");
	fail_unless(rule.prodArray[0].code.size == 2 && rule.prodArray[0].code.code[0] == 0 &&
				rule.prodArray[0].code.code[1] == 0,
				"addProduction does not set the EventCode correctly");
	fail_unless(rule.prodArray[0].event.eventType == EVENT_SE_ALL, "addProduction does not set the EXI EventType correctly");
	fail_unless(rule.prodArray[0].nonTermID == GR_DOC_CONTENT, "addProduction does not set the nonTermID correctly");

	err = insertZeroProduction(&rule, event, nonTermID, 0, 0);
	fail_unless (err == ERR_OK, "insertZeroProduction returns error code %d", err);
	fail_unless (rule.prodCount == 2, "insertZeroProduction does not set prodCount properly");
	fail_unless (rule.bits[0] == 1 && rule.bits[1] == 0, "insertZeroProduction does not set rule.bits properly");
	fail_unless (rule.prodArray[1].lnRowID == 0 && rule.prodArray[1].uriRowID == 0,
				"insertZeroProduction does not set lnRowID and uriRowID properly" );
	fail_unless(rule.prodArray[1].code.size == 1 && rule.prodArray[1].code.code[0] == 0,
					"insertZeroProduction does not set the EventCode correctly");
	fail_unless(rule.prodArray[1].event.eventType == EVENT_SE_ALL, "insertZeroProduction does not set the EXI EventType correctly");
	fail_unless(rule.prodArray[1].nonTermID == GR_DOC_CONTENT, "insertZeroProduction does not set the nonTermID correctly");

	fail_unless(rule.prodArray[0].code.size == 2 && rule.prodArray[0].code.code[0] == 1 &&
					rule.prodArray[0].code.code[1] == 0,
					"insertZeroProduction does not set the EventCode of other productions correctly");

}
END_TEST

/* END: rules tests */


Suite * grammar_suite (void)
{
  Suite *s = suite_create ("Grammar");

  /* Grammars test case */
  TCase *tc_gGrammars = tcase_create ("Grammars");
  tcase_add_test (tc_gGrammars, test_createDocGrammar);
  tcase_add_test (tc_gGrammars, test_processNextProduction);
  tcase_add_test (tc_gGrammars, test_pushGrammar);
  tcase_add_test (tc_gGrammars, test_popGrammar);
  tcase_add_test (tc_gGrammars, test_createBuildInElementGrammar);
  tcase_add_test (tc_gGrammars, test_createGrammarPool);
  tcase_add_test (tc_gGrammars, test_checkGrammarInPool);
  tcase_add_test (tc_gGrammars, test_addGrammarInPool);
  suite_add_tcase (s, tc_gGrammars);

  /* Events test case */
  TCase *tc_gEvents = tcase_create ("Events");
  tcase_add_test (tc_gEvents, test_getEventCode1);
  tcase_add_test (tc_gEvents, test_getEventCode2);
  tcase_add_test (tc_gEvents, test_getEventCode3);
  suite_add_tcase (s, tc_gEvents);

  /* Rules test case */
  TCase *tc_gRules = tcase_create ("Rules");
  tcase_add_test (tc_gRules, test_initGrammarRule);
  tcase_add_test (tc_gRules, test_addProduction);
  tcase_add_test (tc_gRules, test_insertZeroProduction);
  suite_add_tcase (s, tc_gRules);

  return s;
}

int main (void)
{
	int number_failed;
	Suite *s = grammar_suite();
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
