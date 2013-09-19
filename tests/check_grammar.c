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
	errorCode err = EXIP_UNEXPECTED_ERROR;
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

	fail_unless (err == EXIP_OK, "createDocGrammar returns an error code %d", err);

	freeAllocList(&schema.memList);
}
END_TEST

START_TEST (test_processNextProduction)
{
	errorCode err = EXIP_UNEXPECTED_ERROR;
	EXIStream strm;
	SmallIndex nonTermID_out;
	ContentHandler handler;
	EXIPSchema schema;
	QNameID emptyQnameID = {URI_MAX, LN_MAX};

	initAllocList(&strm.memList);
	initAllocList(&schema.memList);

	err = createDocGrammar(&schema, NULL, 0);
	fail_unless (err == EXIP_OK, "createDocGrammar returns an error code %d", err);

	err = pushGrammar(&strm.gStack, emptyQnameID, &schema.docGrammar);
	fail_unless (err == EXIP_OK, "pushGrammar returns an error code %d", err);

	strm.gStack->currNonTermID = 4;
	err = processNextProduction(&strm, &nonTermID_out, &handler, NULL);
	fail_unless (err == EXIP_INCONSISTENT_PROC_STATE, "processNextProduction does not return the correct error code");

	freeAllocList(&strm.memList);
	freeAllocList(&schema.memList);
}
END_TEST

START_TEST (test_pushGrammar)
{
	errorCode err = EXIP_UNEXPECTED_ERROR;
	EXIGrammarStack* testGrStack = NULL;
	EXIStream strm;
	EXIGrammar testElementGrammar;
	EXIGrammar testElementGrammar1;
	QNameID emptyQnameID = {URI_MAX, LN_MAX};

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

#if BUILD_IN_GRAMMARS_USE
	err = createBuiltInElementGrammar(&testElementGrammar1, &strm);
	fail_if(err != EXIP_OK);

	err = createBuiltInElementGrammar(&testElementGrammar, &strm);
	fail_if(err != EXIP_OK);
#endif

	err = pushGrammar(&testGrStack, emptyQnameID, &testElementGrammar1);
	fail_unless (err == EXIP_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack != NULL);

	err = pushGrammar(&testGrStack, emptyQnameID, &testElementGrammar);
	fail_unless (err == EXIP_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack == NULL);
	fail_if(testGrStack->nextInStack->grammar != &testElementGrammar1);

	freeAllocList(&strm.memList);
}
END_TEST

START_TEST (test_popGrammar)
{
	errorCode err = EXIP_UNEXPECTED_ERROR;
	EXIGrammarStack* testGrStack = NULL;
	EXIGrammar testElementGrammar1;
	EXIStream strm;
	EXIGrammar testElementGrammar;
	QNameID emptyQnameID = {URI_MAX, LN_MAX};

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

#if BUILD_IN_GRAMMARS_USE
	err = createBuiltInElementGrammar(&testElementGrammar1, &strm);
	fail_if(err != EXIP_OK);

	err = createBuiltInElementGrammar(&testElementGrammar, &strm);
	fail_if(err != EXIP_OK);
#endif

	err = pushGrammar(&testGrStack, emptyQnameID, &testElementGrammar1);
	fail_unless (err == EXIP_OK, "pushGrammar returns error code %d", err);

	err = pushGrammar(&testGrStack, emptyQnameID, &testElementGrammar);
	fail_unless (err == EXIP_OK, "pushGrammar returns error code %d", err);
	fail_if(testGrStack->nextInStack == NULL);

	popGrammar(&testGrStack);
	fail_if(testGrStack->nextInStack != NULL);
}
END_TEST

#if BUILD_IN_GRAMMARS_USE
START_TEST (test_createBuiltInElementGrammar)
{
	errorCode err = EXIP_UNEXPECTED_ERROR;
	EXIGrammar testElementGrammar;
	EXIStream strm;

	makeDefaultOpts(&strm.header.opts);
	initAllocList(&strm.memList);

	err = createBuiltInElementGrammar(&testElementGrammar, &strm);
	fail_unless (err == EXIP_OK, "createBuildInElementGrammar returns error code %d", err);

}
END_TEST
#endif

/* END: grammars tests */


/* BEGIN: events tests */

/* END: events tests */


/* BEGIN: rules tests */

#if BUILD_IN_GRAMMARS_USE
START_TEST (test_insertZeroProduction)
{
	DynGrammarRule rule;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	Production prod0Arr[2];
	QNameID qname = {0,0};

	rule.pCount = 0;
	rule.prodDim = 1;
	rule.production = prod0Arr;

	tmp_err_code = insertZeroProduction(&rule, EVENT_CH, 5, &qname, FALSE);
	fail_unless (tmp_err_code == EXIP_OK, "insertZeroProduction returns an error code %d", tmp_err_code);
	fail_unless (rule.pCount == 1);
}
END_TEST
#endif
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
#if BUILD_IN_GRAMMARS_USE
	  tcase_add_test (tc_gGrammars, test_createBuiltInElementGrammar);
#endif
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
#if BUILD_IN_GRAMMARS_USE
	  tcase_add_test (tc_gRules, test_insertZeroProduction);
#endif
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
