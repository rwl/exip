/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file check_stringTables.c
 * @brief Tests the EXI String Tables module
 *
 * @date Sep 23, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <check.h>
#include "sTables.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "dynamicArray.h"

/* BEGIN: table tests */

START_TEST (test_createValueTable)
{
	ValueTable valueTable;
	errorCode err = UNEXPECTED_ERROR;
	AllocList memList;

	initAllocList(&memList);
	err = createValueTable(&valueTable, &memList);

	fail_unless (err == ERR_OK, "createValueTable returns error code %d", err);
	fail_unless (valueTable.count == 0,
				"createValueTable populates the valueTable with count: %d", valueTable.count);
	fail_unless (valueTable.dynArray.arrayEntries == DEFAULT_VALUE_ENTRIES_NUMBER,
					"createValueTable creates dynamic array with %d rows", valueTable.dynArray.arrayEntries);
	fail_if(valueTable.value == NULL);
}
END_TEST

START_TEST (test_createPfxTable)
{
	PfxTable* pfxTable;
	errorCode err = UNEXPECTED_ERROR;
	AllocList memList;
	initAllocList(&memList);
	err = createPfxTable(&pfxTable, &memList);

	fail_unless (err == ERR_OK, "createPfxTable returns error code %d", err);
	fail_unless (pfxTable->count == 0,
				"createPfxTable populates the pfxTable with count: %d", pfxTable->count);
	fail_if(pfxTable->pfxStr == NULL);
}
END_TEST

START_TEST (test_addUriEntry)
{
	errorCode err = UNEXPECTED_ERROR;
	UriTable uriTable;
	SmallIndex entryId = 55;
	AllocList memList;
	String test_uri;

	initAllocList(&memList);
	// Create the URI table
	err = createDynArray(&uriTable.dynArray, sizeof(UriEntry), DEFAULT_URI_ENTRIES_NUMBER, &memList);
	fail_if(err != ERR_OK);

	asciiToString("test_uri_string", &test_uri, &memList, FALSE);

	err = addUriEntry(&uriTable, test_uri, &entryId, &memList);

	fail_unless (err == ERR_OK, "addUriEntry returns error code %d", err);
	fail_unless (uriTable.dynArray.arrayEntries == DEFAULT_URI_ENTRIES_NUMBER,
				"addUriEntry changed the dynArray.arrayEntries unnecessary");
	fail_unless (uriTable.count == 1,
					"addUriEntry did not update count properly");
	fail_unless (stringEqual(uriTable.uri[0].uriStr, test_uri) == 1,
						"addUriEntry changed the uriStr");
	fail_unless (entryId == 0,
				"addUriEntry returned wrong entryId: %d", entryId);

	fail_if(uriTable.uri[0].lnTable.ln == NULL);

	uriTable.count = DEFAULT_URI_ENTRIES_NUMBER;

	err = addUriEntry(&uriTable, test_uri, &entryId, &memList);

	fail_unless (err == ERR_OK, "addUriEntry returns error code %d", err);
	fail_unless (uriTable.dynArray.arrayEntries == DEFAULT_URI_ENTRIES_NUMBER*2,
				"addUriEntry did not update the dynArray.arrayEntries properly");
	fail_unless (uriTable.count == DEFAULT_URI_ENTRIES_NUMBER + 1,
					"addUriEntry did not update rowCount properly");
	fail_unless (stringEqual(uriTable.uri[DEFAULT_URI_ENTRIES_NUMBER].uriStr, test_uri) == 1,
						"addUriEntry changed the uriStr");
	fail_unless (entryId == DEFAULT_URI_ENTRIES_NUMBER,
				"addUriEntry returned wrong entryId: %d", entryId);

	fail_if(uriTable.uri[DEFAULT_URI_ENTRIES_NUMBER].lnTable.ln == NULL);
}
END_TEST

START_TEST (test_addLnEntry)
{
	errorCode err = UNEXPECTED_ERROR;
	LnTable lnTable;
	Index entryId = 55;
	AllocList memList;
	String test_ln;

	initAllocList(&memList);
	err = createDynArray(&lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER, &memList);
	fail_if(err != ERR_OK);

	asciiToString("test_ln_string", &test_ln, &memList, FALSE);

	err = addLnEntry(&lnTable, test_ln, &entryId, &memList);

	fail_unless (err == ERR_OK, "addLnEntry returns error code %d", err);
	fail_unless (lnTable.dynArray.arrayEntries == DEFAULT_LN_ENTRIES_NUMBER,
				"addLnEntry changed the dynArray.arrayEntries unnecessary");
	fail_unless (lnTable.count == 1,
					"addLnEntry did not update rowCount properly");
	fail_unless (stringEqual(lnTable.ln[0].lnStr, test_ln) == 1,
						"addLnEntry changed the lnStr");
	fail_unless (entryId == 0,
				"addLnEntry returned wrong entryId: %d", entryId);

	fail_if(lnTable.ln[0].vxTable.vx != NULL);

	lnTable.count = DEFAULT_LN_ENTRIES_NUMBER;

	err = addLnEntry(&lnTable, test_ln, &entryId, &memList);

	fail_unless (err == ERR_OK, "addLnEntry returns error code %d", err);
	fail_unless (lnTable.dynArray.arrayEntries == DEFAULT_LN_ENTRIES_NUMBER*2,
				"addLnEntry did not update the dynArray.arrayEntries properly");
	fail_unless (lnTable.count == DEFAULT_LN_ENTRIES_NUMBER + 1,
					"addLnEntry did not update count properly");
	fail_unless (stringEqual(lnTable.ln[DEFAULT_LN_ENTRIES_NUMBER].lnStr, test_ln) == 1,
						"addLnEntry changed the lnStr");
	fail_unless (entryId == DEFAULT_LN_ENTRIES_NUMBER,
				"addLnEntry returned wrong entryId: %d", entryId);

	fail_if(lnTable.ln[DEFAULT_LN_ENTRIES_NUMBER].vxTable.vx != NULL);
}
END_TEST

START_TEST (test_addValueEntry)
{
	EXIStream testStrm;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	String testStr;

	// IV: Initialize the stream
	{
		tmp_err_code = initAllocList(&(testStrm.memList));

		testStrm.context.bitPointer = 0;
		testStrm.buffer.bufLen = 0;
		testStrm.buffer.bufContent = 0;
		tmp_err_code += createValueTable(&testStrm.valueTable, &testStrm.memList);
		testStrm.schema = memManagedAllocate(&testStrm.memList, sizeof(EXIPSchema));
		fail_unless (testStrm.schema != NULL, "Memory alloc error");
		/* Create and initialize initial string table entries */
		tmp_err_code += createDynArray(&testStrm.schema->uriTable.dynArray, sizeof(UriEntry), DEFAULT_URI_ENTRIES_NUMBER, &testStrm.memList);
		tmp_err_code += createUriTableEntries(&testStrm.schema->uriTable, FALSE, &testStrm.memList);
	}
	fail_unless (tmp_err_code == ERR_OK, "initStream returns an error code %d", tmp_err_code);

	testStrm.context.currElem.uriId = 1; // http://www.w3.org/XML/1998/namespace
	testStrm.context.currElem.lnId = 2; // lang

	asciiToString("TEST-007", &testStr, &testStrm.memList, FALSE);
	tmp_err_code = addValueEntry(&testStrm, testStr, testStrm.context.currElem);

	fail_unless (tmp_err_code == ERR_OK, "addValueEntry returns an error code %d", tmp_err_code);
	fail_unless (testStrm.schema->uriTable.uri[testStrm.context.currElem.uriId].lnTable.ln[testStrm.context.currElem.lnId].vxTable.vx != NULL, "addValueEntry does not create vxTable");
	fail_unless (testStrm.schema->uriTable.uri[testStrm.context.currElem.uriId].lnTable.ln[testStrm.context.currElem.lnId].vxTable.count == 1, "addValueEntry does not create correct vxTable");
	fail_unless (testStrm.valueTable.count == 1, "addValueEntry does not create global value entry");
}
END_TEST

/* END: table tests */

Suite * tables_suite (void)
{
  Suite *s = suite_create ("stringTables");

  {
	  /* Table test case */
	  TCase *tc_tables = tcase_create ("Tables");
	  tcase_add_test (tc_tables, test_createValueTable);
	  tcase_add_test (tc_tables, test_createPfxTable);
	  tcase_add_test (tc_tables, test_addUriEntry);
	  tcase_add_test (tc_tables, test_addLnEntry);
	  tcase_add_test (tc_tables, test_addValueEntry);
	  suite_add_tcase (s, tc_tables);
  }

  return s;
}

int main (void)
{
	int number_failed;
	Suite *s = tables_suite();
	SRunner *sr = srunner_create (s);
#ifdef _MSC_VER
	srunner_set_fork_status(sr, CK_NOFORK);
#endif
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
