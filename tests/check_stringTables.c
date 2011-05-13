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
	fail_unless (pTable->arrayDimension == DEFAULT_PREFIX_ROWS_NUMBER,
					"createPrefixTable creates dynamic array with %d rows", pTable->arrayDimension);
	fail_if(pTable->rows == NULL);
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
	StringType test_uri;

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
	fail_unless (str_equal(uTable->rows[0].string_val, test_uri) == 1,
						"addURIRow changed the uri_string");
	fail_unless (rowID == 0,
				"addURIRow returned wrong rowID: %d", rowID);

	fail_if(uTable->rows[0].lTable == NULL);
	fail_if(uTable->rows[0].pTable == NULL);

	uTable->rowCount = DEFAULT_URI_ROWS_NUMBER;

	err = addURIRow(uTable, test_uri, &rowID, &memList);

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

START_TEST (test_addLNRow)
{
	errorCode err = UNEXPECTED_ERROR;
	LocalNamesTable* lnTable;
	size_t rowID = 55;
	AllocList memList;
	StringType test_ln;

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
	fail_unless (str_equal(lnTable->rows[0].string_val, test_ln) == 1,
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
	fail_unless (str_equal(lnTable->rows[DEFAULT_LOCALNAMES_ROWS_NUMBER].string_val, test_ln) == 1,
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
	EXIOptions options;
	char buf[2];

	testStream.bufferIndx = 0;
	testStream.bitPointer = 0;
	makeDefaultOpts(&options);
	testStream.header.opts = &options;

	initAllocList(&testStream.memList);
	buf[0] = (char) 0xD4; /* 0b11010100 */
	buf[1] = (char) 0x60; /* 0b01100000 */
	testStream.buffer = buf;
	testStream.bufLen = 2;
	testStream.ioStrm = NULL;
	testStream.bufContent = 2;


	err = createInitialStringTables(&testStream);

	fail_unless (err == ERR_OK, "createInitialStringTables returns error code %d", err);
	fail_if(testStream.uriTable == NULL);
	fail_if(testStream.vTable == NULL);

	//TODO: extend this test
}
END_TEST

START_TEST (test_addGVRow)
{
	errorCode err = UNEXPECTED_ERROR;
	ValueTable* vTable;
	size_t rowID = 55;
	StringType test_val;

	AllocList memList;
	initAllocList(&memList);
	err = createValueTable(&vTable, &memList);
	fail_if(err != ERR_OK);

	asciiToString("test_val_string", &test_val, &memList, FALSE);



	err = addGVRow(vTable, test_val, &rowID);

	fail_unless (err == ERR_OK, "addGVRow returns error code %d", err);
	fail_unless (vTable->arrayDimension == DEFAULT_VALUE_ROWS_NUMBER,
				"addGVRow changed the arrayDimension unnecessary");
	fail_unless (vTable->rowCount == 1,
					"addGVRow did not update rowCount properly");
	fail_unless (str_equal(vTable->rows[0].string_val, test_val) == 1,
						"addGVRow changed the val_string");
	fail_unless (rowID == 0,
				"addGVRow returned wrong rowID: %d", rowID);

	vTable->rowCount = DEFAULT_VALUE_ROWS_NUMBER;

	err = addGVRow(vTable, test_val, &rowID);

	fail_unless (err == ERR_OK, "addGVRow returns error code %d", err);
	fail_unless (vTable->arrayDimension == DEFAULT_VALUE_ROWS_NUMBER*2,
				"addGVRow changed the arrayDimension unnecessary");
	fail_unless (vTable->rowCount == DEFAULT_VALUE_ROWS_NUMBER + 1,
					"addGVRow did not update rowCount properly");
	fail_unless (str_equal(vTable->rows[DEFAULT_VALUE_ROWS_NUMBER].string_val, test_val) == 1,
						"addGVRow changed the val_string");
	fail_unless (rowID == DEFAULT_VALUE_ROWS_NUMBER,
				"addGVRow returned wrong rowID: %d", rowID);
}
END_TEST

START_TEST (test_addLVRow)
{
	errorCode err = UNEXPECTED_ERROR;
	LocalNamesTable* lnTable;
	size_t rowID = 55;
	unsigned int globalValueRowID = 101;
	AllocList memList;
	StringType test_ln;

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

	fail_unless (str_equal(lnTable->rows[0].string_val, test_ln) == 1,
						"addLNRow changed the ln_string");
	fail_unless (rowID == 0,
				"addLNRow returned wrong rowID: %d", rowID);
	fail_if(lnTable->rows[0].vCrossTable != NULL);

	err = addLVRow(&(lnTable->rows[0]), globalValueRowID, &memList);
	fail_unless (err == ERR_OK, "addLVRow returns error code %d", err);
	fail_if(lnTable->rows[0].vCrossTable == NULL);
	fail_if(lnTable->rows[0].vCrossTable->valueRowIds == NULL);
	fail_unless (lnTable->rows[0].vCrossTable->rowCount == 1,
				 "addLVRow did not update vCrossTable.rowCount properly");
	fail_unless (lnTable->rows[0].vCrossTable->arrayDimension == DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER,
					 "addLVRow did not update the arrayDimension");
	fail_unless (lnTable->rows[0].vCrossTable->valueRowIds[0] == 101,
						 "addLVRow did not set the valueRowIds properly");
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
	  tcase_add_test (tc_tables, test_addGVRow);
	  tcase_add_test (tc_tables, test_addLVRow);
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
