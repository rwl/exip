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
	errorCode err = EXIP_UNEXPECTED_ERROR;

	err = createValueTable(&valueTable);

	fail_unless (err == EXIP_OK, "createValueTable returns error code %d", err);
	fail_unless (valueTable.count == 0,
				"createValueTable populates the valueTable with count: %d", valueTable.count);
	fail_unless (valueTable.dynArray.arrayEntries == DEFAULT_VALUE_ENTRIES_NUMBER,
					"createValueTable creates dynamic array with %d rows", valueTable.dynArray.arrayEntries);
	fail_if(valueTable.value == NULL);

	destroyDynArray(&valueTable.dynArray);
}
END_TEST

START_TEST (test_createPfxTable)
{
	PfxTable* pfxTable;
	errorCode err = EXIP_UNEXPECTED_ERROR;

	err = createPfxTable(&pfxTable);

	fail_unless (err == EXIP_OK, "createPfxTable returns error code %d", err);
	fail_unless (pfxTable->count == 0,
				"createPfxTable populates the pfxTable with count: %d", pfxTable->count);
	fail_if(pfxTable->pfxStr == NULL);

	EXIP_MFREE(pfxTable);
}
END_TEST

START_TEST (test_addUriEntry)
{
	errorCode err = EXIP_UNEXPECTED_ERROR;
	UriTable uriTable;
	SmallIndex entryId = 55;
	String test_uri = {"test_uri_string", 15};

	// Create the URI table
	err = createDynArray(&uriTable.dynArray, sizeof(UriEntry), DEFAULT_URI_ENTRIES_NUMBER);
	fail_if(err != EXIP_OK);

	err = addUriEntry(&uriTable, test_uri, &entryId);

	fail_unless (err == EXIP_OK, "addUriEntry returns error code %d", err);
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

	err = addUriEntry(&uriTable, test_uri, &entryId);

	fail_unless (err == EXIP_OK, "addUriEntry returns error code %d", err);
	fail_unless (uriTable.dynArray.arrayEntries == DEFAULT_URI_ENTRIES_NUMBER*2,
				"addUriEntry did not update the dynArray.arrayEntries properly");
	fail_unless (uriTable.count == DEFAULT_URI_ENTRIES_NUMBER + 1,
					"addUriEntry did not update rowCount properly");
	fail_unless (stringEqual(uriTable.uri[DEFAULT_URI_ENTRIES_NUMBER].uriStr, test_uri) == 1,
						"addUriEntry changed the uriStr");
	fail_unless (entryId == DEFAULT_URI_ENTRIES_NUMBER,
				"addUriEntry returned wrong entryId: %d", entryId);

	fail_if(uriTable.uri[DEFAULT_URI_ENTRIES_NUMBER].lnTable.ln == NULL);

	destroyDynArray(&uriTable.dynArray);
}
END_TEST

START_TEST (test_addLnEntry)
{
	errorCode err = EXIP_UNEXPECTED_ERROR;
	LnTable lnTable;
	Index entryId = 55;
	String test_ln = {"test_ln_string", 14};

	err = createDynArray(&lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER);
	fail_if(err != EXIP_OK);

	err = addLnEntry(&lnTable, test_ln, &entryId);

	fail_unless (err == EXIP_OK, "addLnEntry returns error code %d", err);
	fail_unless (lnTable.dynArray.arrayEntries == DEFAULT_LN_ENTRIES_NUMBER,
				"addLnEntry changed the dynArray.arrayEntries unnecessary");
	fail_unless (lnTable.count == 1,
					"addLnEntry did not update rowCount properly");
	fail_unless (stringEqual(lnTable.ln[0].lnStr, test_ln) == 1,
						"addLnEntry changed the lnStr");
	fail_unless (entryId == 0,
				"addLnEntry returned wrong entryId: %d", entryId);

#if VALUE_CROSSTABLE_USE
	fail_if(lnTable.ln[0].vxTable != NULL);
#endif

	lnTable.count = DEFAULT_LN_ENTRIES_NUMBER;

	err = addLnEntry(&lnTable, test_ln, &entryId);

	fail_unless (err == EXIP_OK, "addLnEntry returns error code %d", err);
	fail_unless (lnTable.dynArray.arrayEntries == DEFAULT_LN_ENTRIES_NUMBER*2,
				"addLnEntry did not update the dynArray.arrayEntries properly");
	fail_unless (lnTable.count == DEFAULT_LN_ENTRIES_NUMBER + 1,
					"addLnEntry did not update count properly");
	fail_unless (stringEqual(lnTable.ln[DEFAULT_LN_ENTRIES_NUMBER].lnStr, test_ln) == 1,
						"addLnEntry changed the lnStr");
	fail_unless (entryId == DEFAULT_LN_ENTRIES_NUMBER,
				"addLnEntry returned wrong entryId: %d", entryId);
#if VALUE_CROSSTABLE_USE
	fail_if(lnTable.ln[DEFAULT_LN_ENTRIES_NUMBER].vxTable != NULL);
#endif
	destroyDynArray(&lnTable.dynArray);
}
END_TEST

START_TEST (test_addValueEntry)
{
	EXIStream testStrm;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	String testStr = {"TEST-007", 8};

	// IV: Initialize the stream
	{
		tmp_err_code = initAllocList(&(testStrm.memList));

		testStrm.context.bitPointer = 0;
		testStrm.buffer.bufLen = 0;
		testStrm.buffer.bufContent = 0;
		tmp_err_code += createValueTable(&testStrm.valueTable);
		testStrm.schema = memManagedAllocate(&testStrm.memList, sizeof(EXIPSchema));
		fail_unless (testStrm.schema != NULL, "Memory alloc error");
		/* Create and initialize initial string table entries */
		tmp_err_code += createDynArray(&testStrm.schema->uriTable.dynArray, sizeof(UriEntry), DEFAULT_URI_ENTRIES_NUMBER);
		tmp_err_code += createUriTableEntries(&testStrm.schema->uriTable, FALSE);
	}
	fail_unless (tmp_err_code == EXIP_OK, "initStream returns an error code %d", tmp_err_code);

	testStrm.gStack->currQNameID.uriId = 1; // http://www.w3.org/XML/1998/namespace
	testStrm.gStack->currQNameID.lnId = 2; // lang

	tmp_err_code = addValueEntry(&testStrm, testStr, testStrm.gStack->currQNameID);

	fail_unless (tmp_err_code == EXIP_OK, "addValueEntry returns an error code %d", tmp_err_code);
#if VALUE_CROSSTABLE_USE
	fail_unless (testStrm.schema->uriTable.uri[testStrm.gStack->currQNameID.uriId].lnTable.ln[testStrm.gStack->currQNameID.lnId].vxTable != NULL, "addValueEntry does not create vxTable");
	fail_unless (testStrm.schema->uriTable.uri[testStrm.gStack->currQNameID.uriId].lnTable.ln[testStrm.gStack->currQNameID.lnId].vxTable->count == 1, "addValueEntry does not create correct vxTable");
#endif
	fail_unless (testStrm.valueTable.count == 1, "addValueEntry does not create global value entry");

	destroyDynArray(&testStrm.valueTable.dynArray);
	destroyDynArray(&testStrm.schema->uriTable.dynArray);
	freeAllocList(&testStrm.memList);
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
