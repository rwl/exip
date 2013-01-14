/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file check_grammar.c
 * @brief Tests the EXI grammar module
 *
 * @date Sep 13, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <check.h>
#include "grammars.h"
#include "bodyDecode.h"
#include "memManagement.h"

/* BEGIN: grammars tests */

START_TEST (test_createDocGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIPSchema schema;
	EXIStream testStream;
	char buf[2];
	buf[0] = (char) 0xD4; /* 0b11010100 */
	buf[1] = (char) 0x60; /* 0b01100000 */

	initAllocList(&schema.memList);

	testStream.context.bitPointer = 0;
	makeDefaultOpts(&testStream.header.opts);
	testStream.buffer.buf = buf;
	testStream.buffer.bufLen = 2;
	testStream.buffer.bufContent = 2;
	testStream.buffer.ioStrm.readWriteToStream = NULL;
	testStream.buffer.ioStrm.stream = NULL;
	testStream.context.bufferIndx = 0;
	initAllocList(&testStream.memList);

	err = createDocGrammar(&schema, NULL, 0);

	fail_unless (err == ERR_OK, "createDocGrammar returns an error code %d", err);

	freeAllocList(&schema.memList);
}
END_TEST

START_TEST (test_processNextProduction)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIStream strm;
	SmallIndex nonTermID_out;
	ContentHandler handler;
	EXIPSchema schema;

	initAllocList(&strm.memList);
	initAllocList(&schema.memList);

	err = createDocGrammar(&schema, NULL, 0);
	fail_unless (err == ERR_OK, "createDocGrammar returns an error code %d", err);

	err = pushGrammar(&strm.gStack, &schema.docGrammar);
	fail_unless (err == ERR_OK, "pushGrammar returns an error code %d", err);

	strm.context.currNonTermID = 4;
	err = processNextProduction(&strm, &nonTermID_out, &handler, NULL);
	fail_unless (err == INCONSISTENT_PROC_STATE, "processNextProduction does not return the correct error code");

	freeAllocList(&strm.memList);
	freeAllocList(&schema.memList);
}
END_TEST

START_TEST (test_pushGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIGrammarStack* testGrStack = NULL;
	EXIStream strm;
	EXIGrammar testElementGrammar;
	EXIGrammar testElementGrammar1;

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

	err = createBuiltInElementGrammar(&testElementGrammar1, &strm);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &testElementGrammar1);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack != NULL);

	err = createBuiltInElementGrammar(&testElementGrammar, &strm);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &testElementGrammar);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack == NULL);
	fail_if(testGrStack->nextInStack->grammar != &testElementGrammar1);

	freeAllocList(&strm.memList);
}
END_TEST

START_TEST (test_popGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIGrammarStack* testGrStack = NULL;
	EXIGrammar testElementGrammar1;
	EXIStream strm;
	EXIGrammar testElementGrammar;
	EXIGrammar* testGR;

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

	err = createBuiltInElementGrammar(&testElementGrammar1, &strm);
	fail_if(err != ERR_OK);

	err = pushGrammar(&testGrStack, &testElementGrammar1);
	fail_unless (err == ERR_OK, "pushGrammar returns error code %d", err);

	err = createBuiltInElementGrammar(&testElementGrammar, &strm);
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

START_TEST (test_createBuiltInElementGrammar)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIGrammar testElementGrammar;
	EXIStream strm;

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

	err = createBuiltInElementGrammar(&testElementGrammar, &strm);
	fail_unless (err == ERR_OK, "createBuildInElementGrammar returns error code %d", err);

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
	QNameID qname = {0,0};

	rule.pCount = 0;
	rule.prodDim = 1;
	rule.production = prod0Arr;

	tmp_err_code = insertZeroProduction(&rule, EVENT_CH, 5, &qname, FALSE);
	fail_unless (tmp_err_code == ERR_OK, "insertZeroProduction returns an error code %d", tmp_err_code);
	fail_unless (rule.pCount == 1);
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
	  tcase_add_test (tc_gGrammars, test_createBuiltInElementGrammar);
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
