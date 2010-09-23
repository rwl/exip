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
#include "../include/sTables.h"

/* BEGIN: table tests */

START_TEST (test_createValueTable)
{
	ValueTable* vTable;
	errorCode err = UNEXPECTED_ERROR;
	err = createValueTable(&vTable);

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
	err = createURITable(&uTable);

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
	err = createPrefixTable(&pTable);

	fail_unless (err == ERR_OK, "createPrefixTable returns error code %d", err);
	fail_unless (pTable->rowCount == 0,
				"createPrefixTable populates the pTable with rows: %d", pTable->rowCount);
	fail_unless (pTable->arrayDimension == DEFAULT_PREFIX_ROWS_NUMBER,
					"createPrefixTable creates dynamic array with %d rows", pTable->arrayDimension);
	fail_if(pTable->rows == NULL);
}
END_TEST

START_TEST (test_createLocalNamesTable)
{
	LocalNamesTable* lTable;
	errorCode err = UNEXPECTED_ERROR;
	err = createLocalNamesTable(&lTable);

	fail_unless (err == ERR_OK, "createLocalNamesTable returns error code %d", err);
	fail_unless (lTable->rowCount == 0,
				"createLocalNamesTable populates the vTable with rows: %d", lTable->rowCount);
	fail_unless (lTable->arrayDimension == DEFAULT_LOCALNAMES_ROWS_NUMBER,
					"createLocalNamesTable creates dynamic array with %d rows", lTable->arrayDimension);
	fail_if(lTable->rows == NULL);
}
END_TEST

START_TEST (test_addURIRow)
{
	errorCode err = UNEXPECTED_ERROR;
	URITable* uTable;
	err = createURITable(&uTable);
	fail_if(err != ERR_OK);

	StringType test_uri;
	asciiToString("test_uri_string", test_uri);

	unsigned int rowID = 55;

	err = addURIRow(uTable, test_uri, &rowID);

	fail_unless (err == ERR_OK, "addURIRow returns error code %d", err);
	fail_unless (uTable->arrayDimension == DEFAULT_URI_ROWS_NUMBER,
				"addURIRow changed the arrayDimension unnecessary");
	fail_unless (uTable->rowCount == 1,
					"addURIRow did not update rowCount properly");
	fail_unless (str_equal(uTable->rows[0].string_val, test_uri) == 1,
						"addURIRow changed the uri_string");
	fail_unless (rowID == 0,
				"addURIRow returned wrong rowID: %d", rowID);

	fail_if(uTable->rows[0].lTable == NULL);
	fail_if(uTable->rows[0].pTable == NULL);

	uTable->rowCount = DEFAULT_URI_ROWS_NUMBER;

	err = addURIRow(uTable, test_uri, &rowID);

	fail_unless (err == ERR_OK, "addURIRow returns error code %d", err);
	fail_unless (uTable->arrayDimension == DEFAULT_URI_ROWS_NUMBER*2,
				"addURIRow changed the arrayDimension unnecessary");
	fail_unless (uTable->rowCount == DEFAULT_URI_ROWS_NUMBER + 1,
					"addURIRow did not update rowCount properly");
	fail_unless (str_equal(uTable->rows[DEFAULT_URI_ROWS_NUMBER].string_val, test_uri) == 1,
						"addURIRow changed the uri_string");
	fail_unless (rowID == DEFAULT_URI_ROWS_NUMBER,
				"addURIRow returned wrong rowID: %d", rowID);

	fail_if(uTable->rows[DEFAULT_URI_ROWS_NUMBER].lTable == NULL);
	fail_if(uTable->rows[DEFAULT_URI_ROWS_NUMBER].pTable == NULL);
}
END_TEST

START_TEST (test_createInitialStringTables)
{
	errorCode err = UNEXPECTED_ERROR;
	EXIStream testStream;
	testStream.bufferIndx = 0;
	testStream.bitPointer = 0;
	struct EXIOptions options;
	makeDefaultOpts(&options);
	testStream.opts = &options;

	char buf[2];
	buf[0] = (char) 0b11010100;
	buf[1] = (char) 0b01100000;
	testStream.buffer = buf;


	err = createInitialStringTables(&testStream);

	fail_unless (err == ERR_OK, "createInitialStringTables returns error code %d", err);
	fail_if(testStream.uriTable == NULL);
	fail_if(testStream.vTable == NULL);

	//TODO: extend this test
}
END_TEST

/* END: table tests */

Suite * tables_suite (void)
{
  Suite *s = suite_create ("stringTables");

  /* Table test case */
  TCase *tc_tables = tcase_create ("Tables");
  tcase_add_test (tc_tables, test_createValueTable);
  tcase_add_test (tc_tables, test_createURITable);
  tcase_add_test (tc_tables, test_createPrefixTable);
  tcase_add_test (tc_tables, test_createLocalNamesTable);
  tcase_add_test (tc_tables, test_addURIRow);
  tcase_add_test (tc_tables, test_createInitialStringTables);
  suite_add_tcase (s, tc_tables);
  return s;
}

int main (void)
{
	int number_failed;
	Suite *s = tables_suite();
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
