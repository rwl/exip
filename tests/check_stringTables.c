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
 * @file check_stringTables.c
 * @brief Tests the EXI String Tables module
 *
 * @date Sep 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <check.h>
#include "sTables.h"
#include "stringManipulate.h"
#include "memManagement.h"

/* BEGIN: table tests */

START_TEST (test_createValueTable)
{
	ValueTable* vTable;
	errorCode err = UNEXPECTED_ERROR;
	AllocList memList;

	initAllocList(&memList);
	err = createValueTable(&vTable, &memList);

	fail_unless (err == ERR_OK, "createValueTable returns error code %d", err);
	fail_unless (vTable->rowCount == 0,
				"createValueTable populates the vTable with rows: %d", vTable->rowCount);
	fail_unless (vTable->arrayDimension == DEFAULT_VALUE_ROWS_NUMBER,
					"createValueTable creates dynamic array with %d rows", vTable->arrayDimension);
	fail_if(vTable->rows == NULL);
}
END_TEST

START_TEST (test_createURITable)
{
	URITable* uTable;
	errorCode err = UNEXPECTED_ERROR;
	AllocList memList;
	initAllocList(&memList);
	err = createURITable(&uTable, &memList);

	fail_unless (err == ERR_OK, "createURITable returns error code %d", err);
	fail_unless (uTable->rowCount == 0,
				"createURITable populates the uTable with rows: %d", uTable->rowCount);
	fail_unless (uTable->arrayDimension == DEFAULT_URI_ROWS_NUMBER,
					"createURITable creates dynamic array with %d rows", uTable->arrayDimension);
	fail_if(uTable->rows == NULL);
}
END_TEST

START_TEST (test_createPrefixTable)
{
	PrefixTable* pTable;
	errorCode err = UNEXPECTED_ERROR;
	AllocList memList;
	initAllocList(&memList);
	err = createPrefixTable(&pTable, &memList);

	fail_unless (err == ERR_OK, "createPrefixTable returns error code %d", err);
	fail_unless (pTable->rowCount == 0,
				"createPrefixTable populates the pTable with rows: %d", pTable->rowCount);
	fail_if(pTable->string_val == NULL);
}
END_TEST

START_TEST (test_createLocalNamesTable)
{
	LocalNamesTable* lTable;
	errorCode err = UNEXPECTED_ERROR;
	AllocList memList;
	initAllocList(&memList);
	err = createLocalNamesTable(&lTable, &memList);

	fail_unless (err == ERR_OK, "createLocalNamesTable returns error code %d", err);
	fail_unless (lTable->rowCount == 0,
				"createLocalNamesTable populates the vTable with rows: %d", lTable->rowCount);
	fail_unless (lTable->arrayDimension == DEFAULT_LOCALNAMES_ROWS_NUMBER,
					"createLocalNamesTable creates dynamic array with %d rows", lTable->arrayDimension);
	fail_if(lTable->rows == NULL);
}
END_TEST

START_TEST (test_createValueLocalCrossTable)
{
	ValueLocalCrossTable* vlTable;
	errorCode err = UNEXPECTED_ERROR;
	AllocList memList;
	initAllocList(&memList);
	err = createValueLocalCrossTable(&vlTable, &memList);

	fail_unless (err == ERR_OK, "createValueLocalCrossTable returns error code %d", err);
	fail_unless (vlTable->rowCount == 0,
				"createValueLocalCrossTable populates the vlTable with rows: %d", vlTable->rowCount);
	fail_unless (vlTable->arrayDimension == DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER,
					"createValueLocalCrossTable creates dynamic array with %d rows", vlTable->arrayDimension);
	fail_if(vlTable->valueRowIds == NULL);
}
END_TEST

START_TEST (test_addURIRow)
{
	errorCode err = UNEXPECTED_ERROR;
	URITable* uTable;
	uint16_t rowID = 55;
	AllocList memList;
	String test_uri;

	initAllocList(&memList);
	err = createURITable(&uTable, &memList);
	fail_if(err != ERR_OK);

	asciiToString("test_uri_string", &test_uri, &memList, FALSE);

	err = addURIRow(uTable, test_uri, &rowID, &memList);

	fail_unless (err == ERR_OK, "addURIRow returns error code %d", err);
	fail_unless (uTable->arrayDimension == DEFAULT_URI_ROWS_NUMBER,
				"addURIRow changed the arrayDimension unnecessary");
	fail_unless (uTable->rowCount == 1,
					"addURIRow did not update rowCount properly");
	fail_unless (stringEqual(uTable->rows[0].string_val, test_uri) == 1,
						"addURIRow changed the uri_string");
	fail_unless (rowID == 0,
				"addURIRow returned wrong rowID: %d", rowID);

	fail_if(uTable->rows[0].lTable == NULL);

	uTable->rowCount = DEFAULT_URI_ROWS_NUMBER;

	err = addURIRow(uTable, test_uri, &rowID, &memList);

	fail_unless (err == ERR_OK, "addURIRow returns error code %d", err);
	fail_unless (uTable->arrayDimension == DEFAULT_URI_ROWS_NUMBER*2,
				"addURIRow changed the arrayDimension unnecessary");
	fail_unless (uTable->rowCount == DEFAULT_URI_ROWS_NUMBER + 1,
					"addURIRow did not update rowCount properly");
	fail_unless (stringEqual(uTable->rows[DEFAULT_URI_ROWS_NUMBER].string_val, test_uri) == 1,
						"addURIRow changed the uri_string");
	fail_unless (rowID == DEFAULT_URI_ROWS_NUMBER,
				"addURIRow returned wrong rowID: %d", rowID);

	fail_if(uTable->rows[DEFAULT_URI_ROWS_NUMBER].lTable == NULL);
}
END_TEST

START_TEST (test_addLNRow)
{
	errorCode err = UNEXPECTED_ERROR;
	LocalNamesTable* lnTable;
	size_t rowID = 55;
	AllocList memList;
	String test_ln;

	initAllocList(&memList);
	err = createLocalNamesTable(&lnTable, &memList);
	fail_if(err != ERR_OK);

	asciiToString("test_ln_string", &test_ln, &memList, FALSE);

	err = addLNRow(lnTable, test_ln, &rowID);

	fail_unless (err == ERR_OK, "addLNRow returns error code %d", err);
	fail_unless (lnTable->arrayDimension == DEFAULT_LOCALNAMES_ROWS_NUMBER,
				"addLNRow changed the arrayDimension unnecessary");
	fail_unless (lnTable->rowCount == 1,
					"addLNRow did not update rowCount properly");
	fail_unless (stringEqual(lnTable->rows[0].string_val, test_ln) == 1,
						"addLNRow changed the ln_string");
	fail_unless (rowID == 0,
				"addLNRow returned wrong rowID: %d", rowID);

	fail_if(lnTable->rows[0].vCrossTable != NULL);

	lnTable->rowCount = DEFAULT_LOCALNAMES_ROWS_NUMBER;

	err = addLNRow(lnTable, test_ln, &rowID);

	fail_unless (err == ERR_OK, "addLNRow returns error code %d", err);
	fail_unless (lnTable->arrayDimension == DEFAULT_LOCALNAMES_ROWS_NUMBER*2,
				"addLNRow changed the arrayDimension unnecessary");
	fail_unless (lnTable->rowCount == DEFAULT_LOCALNAMES_ROWS_NUMBER + 1,
					"addLNRow did not update rowCount properly");
	fail_unless (stringEqual(lnTable->rows[DEFAULT_LOCALNAMES_ROWS_NUMBER].string_val, test_ln) == 1,
						"addLNRow changed the ln_string");
	fail_unless (rowID == DEFAULT_LOCALNAMES_ROWS_NUMBER,
				"addLNRow returned wrong rowID: %d", rowID);

	fail_if(lnTable->rows[DEFAULT_LOCALNAMES_ROWS_NUMBER].vCrossTable != NULL);
}
END_TEST

START_TEST (test_createInitialStringTables)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIStream testStream;
	char buf[2];

	testStream.context.bufferIndx = 0;
	testStream.context.bitPointer = 0;
	makeDefaultOpts(&testStream.header.opts);

	initAllocList(&testStream.memList);
	buf[0] = (char) 0xD4; /* 0b11010100 */
	buf[1] = (char) 0x60; /* 0b01100000 */
	testStream.buffer = buf;
	testStream.bufLen = 2;
	testStream.ioStrm.readWriteToStream = NULL;
	testStream.ioStrm.stream = NULL;
	testStream.bufContent = 2;


	err = createInitialStringTables(&testStream);

	fail_unless (err == ERR_OK, "createInitialStringTables returns error code %d", err);
	fail_if(testStream.uriTable == NULL);
	fail_if(testStream.vTable == NULL);

	//TODO: extend this test
}
END_TEST

START_TEST (test_addValueRows)
{
	EXIStream testStrm;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	String testStr;

	// IV: Initialize the stream
	{
		tmp_err_code = initAllocList(&(testStrm.memList));

		testStrm.context.bitPointer = 0;
		testStrm.bufLen = 0;
		testStrm.bufContent = 0;
		tmp_err_code += createInitialStringTables(&testStrm);
	}
	fail_unless (tmp_err_code == ERR_OK, "initStream returns an error code %d", tmp_err_code);

	testStrm.context.curr_uriID = 1; // http://www.w3.org/XML/1998/namespace
	testStrm.context.curr_lnID = 2; // lang

	asciiToString("TEST-007", &testStr, &testStrm.memList, FALSE);
	tmp_err_code = addValueRows(&testStrm, &testStr);

	fail_unless (tmp_err_code == ERR_OK, "addValueRows returns an error code %d", tmp_err_code);
	fail_unless (testStrm.uriTable->rows[testStrm.context.curr_uriID].lTable->rows[testStrm.context.curr_lnID].vCrossTable != NULL, "addValueRows does not create vCrossTable");
	fail_unless (testStrm.uriTable->rows[testStrm.context.curr_uriID].lTable->rows[testStrm.context.curr_lnID].vCrossTable->rowCount == 1, "addValueRows does not create correct vCrossTable");
	fail_unless (testStrm.vTable->rowCount == 1, "addValueRows does not create global value entry");

	//TODO: extend this test
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
	  tcase_add_test (tc_tables, test_createURITable);
	  tcase_add_test (tc_tables, test_createPrefixTable);
	  tcase_add_test (tc_tables, test_createLocalNamesTable);
	  tcase_add_test (tc_tables, test_addURIRow);
	  tcase_add_test (tc_tables, test_createInitialStringTables);
	  tcase_add_test (tc_tables, test_createValueLocalCrossTable);
	  tcase_add_test (tc_tables, test_addLNRow);
	  tcase_add_test (tc_tables, test_addValueRows);
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
