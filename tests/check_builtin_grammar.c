/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file check_builtin_grammar.c
 * @brief Tests decoding documents using built-in grammar
 *
 * @date Mar 29, 2012
 * @author Ken Bannister
 * @version 0.4
 * @par[Revision] $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "procTypes.h"
#include "EXISerializer.h"
#include "EXIParser.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"

#define INPUT_BUFFER_SIZE 200
#define MAX_PATH_LEN 200

/* Location for external test data */
static char *dataDir;

struct appData
{
	unsigned int eventCount;
	unsigned int attributeCount;
};


/* Helper functions */

size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
}


/* Document callbacks */

static errorCode sample_fatalError(const errorCode code, const char* msg, void* app_data)
{
	printf("\n%d : FATAL ERROR: %s\n", code, msg);
	return EXIP_HANDLER_STOP;
}

static errorCode sample_attribute(QName qname, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	appD->attributeCount++;

	return EXIP_ERR_OK;
}


/* Tests */

/* Basic count of document events and attributes. */
START_TEST (test_decode_ant_example01)
{
	FILE *infile;
	Parser testParser;
	char buf[INPUT_BUFFER_SIZE];
	const char *exifname = "Ant/build-build.bitPacked";
	char exipath[MAX_PATH_LEN + strlen(exifname)];
	struct appData parsingData;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	BinaryBuffer buffer;

	buffer.buf = buf;
	buffer.bufContent = 0;
	buffer.bufLen = INPUT_BUFFER_SIZE;

	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any
	size_t pathlen = strlen(dataDir);
	memcpy(exipath, dataDir, pathlen);
	exipath[pathlen] = '/';
	memcpy(&exipath[pathlen+1], exifname, strlen(exifname)+1);
	
	infile = fopen(exipath, "rb" );
	if(!infile)
		fail("Unable to open file %s", exipath);
	
	buffer.ioStrm.readWriteToStream = readFileInputStream;
	buffer.ioStrm.stream = infile;

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buffer, &parsingData);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "initParser returns an error code %d", tmp_err_code);

	// III: Initialize the parsing data and hook the callback handlers to the parser object
	parsingData.eventCount = 0;
	parsingData.attributeCount = 0;

	testParser.handler.fatalError = sample_fatalError;
	testParser.handler.error = sample_fatalError;
	testParser.handler.attribute = sample_attribute;

	// IV: Parse the header of the stream
	tmp_err_code = parseHeader(&testParser, TRUE);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "parsing the header returns an error code %d", tmp_err_code);
	parsingData.eventCount++; // SD event is implicit

	tmp_err_code = setSchema(&testParser, NULL);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "setSchema() returns an error code %d", tmp_err_code);

	// V: Parse the body of the EXI stream
	while(tmp_err_code == EXIP_ERR_OK)
	{
		tmp_err_code = parseNext(&testParser);
		parsingData.eventCount++;
	}
	
	fail_unless(parsingData.eventCount == 401, 
	            "Unexpected event count: %u", parsingData.eventCount);
	fail_unless(parsingData.attributeCount == 171, 
	            "Unexpected attribute count: %u", parsingData.attributeCount);

	// VI: Free the memory allocated by the parser object
	destroyParser(&testParser);
	fclose(infile);
	fail_unless (tmp_err_code == EXIP_PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
}
END_TEST


/* Test suite */

Suite* exip_suite(void)
{
	Suite *s = suite_create("Built-in Grammar");

	{
	  /* Schema-less test case */
	  TCase *tc_builtin = tcase_create ("Built-in Grammar");
	  tcase_add_test (tc_builtin, test_decode_ant_example01);
	  suite_add_tcase (s, tc_builtin);
	}

	return s;
}

int main (int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("ERR: Expected test data directory\n");
		exit(1);
	}
	if (strlen(argv[1]) > MAX_PATH_LEN)
	{
		printf("ERR: Test data pathname too long: %u", (unsigned int) strlen(argv[1]));
		exit(1);
	}
	dataDir = argv[1];
	
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

