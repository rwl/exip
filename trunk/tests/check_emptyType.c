/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file check_emptyType.c
 * @brief Tests the creation of EmptyType grammars and the xsi:nil attribute handling.
 *
 * @date Jul 12, 2012
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <check.h>
#include "procTypes.h"
#include "EXIParser.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"

#define MAX_PATH_LEN 200

/* Location for external test data */
static char *dataDir;
#define EMPTY_TYPE_SCHEMA  "EmptyTypes/emptyTypeSchema-xsd.exi"
#define EMPTY_TYPE_DEFAULT "EmptyTypes/emptyTypeTest-def.exi"
#define EMPTY_TYPE_STRICT  "EmptyTypes/emptyTypeTest-strict.exi"

#define OUTPUT_BUFFER_SIZE 2000

struct appData
{
	unsigned int eventCount;
	unsigned short expectAttributeData;
};
typedef struct appData appData;

size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
}

static void parseSchema(const char* fileName, EXIPSchema* schema)
{
	FILE *schemaFile;
	BinaryBuffer buffer;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	size_t pathlen = strlen(dataDir);
	char exipath[MAX_PATH_LEN + strlen(fileName)];

	memcpy(exipath, dataDir, pathlen);
	exipath[pathlen] = '/';
	memcpy(&exipath[pathlen+1], fileName, strlen(fileName)+1);
	schemaFile = fopen(exipath, "rb" );
	if(!schemaFile)
	{
		fail("Unable to open file %s", exipath);
	}
	else
	{
		//Get file length
		fseek(schemaFile, 0, SEEK_END);
		buffer.bufLen = ftell(schemaFile) + 1;
		fseek(schemaFile, 0, SEEK_SET);

		//Allocate memory
		buffer.buf = (char *)malloc(buffer.bufLen);
		if (!buffer.buf)
		{
			fclose(schemaFile);
			fail("Memory allocation error!");
		}

		//Read file contents into buffer
		fread(buffer.buf, buffer.bufLen, 1, schemaFile);
		fclose(schemaFile);

		buffer.bufContent = buffer.bufLen;
		buffer.ioStrm.readWriteToStream = NULL;
		buffer.ioStrm.stream = NULL;

		tmp_err_code = generateSchemaInformedGrammars(&buffer, 1, SCHEMA_FORMAT_XSD_EXI, NULL, schema, NULL);

		if(tmp_err_code != EXIP_ERR_OK)
		{
			fail("\n Error reading schema: %d", tmp_err_code);
		}

		free(buffer.buf);
	}
}

/* Document callbacks */

static errorCode sample_fatalError(const errorCode code, const char* msg, void* app_data)
{
	printf("\n%3d : FATAL ERROR: %s\n", code, msg);
	return EXIP_HANDLER_STOP;
}

static errorCode sample_startElement(QName qname, void* app_data)
{
	appData* appD = (appData*) app_data;

	if(!stringEqualToAscii(*qname.uri, "http://www.example.org/emptyTypeSchema"))
	{
		printf("\nInvalid namespace!");
		return EXIP_HANDLER_STOP;
	}

	switch(appD->eventCount)
	{
		case 0:
			if(!stringEqualToAscii(*qname.localName, "rootElem"))
			{
				printf("\nInvalid doc: rootElem!");
				return EXIP_HANDLER_STOP;
			}
			appD->eventCount++;
		break;
		case 2:
			if(!stringEqualToAscii(*qname.localName, "nilabElem"))
			{
				printf("\nInvalid doc: nilabElem!");
				return EXIP_HANDLER_STOP;
			}
			appD->eventCount++;
		break;
		default:
			printf("\nInvalid decoding startElement!");
			return EXIP_HANDLER_STOP;
			break;
	}
	return EXIP_ERR_OK;
}

static errorCode sample_attribute(QName qname, void* app_data)
{
	appData* appD = (appData*) app_data;

	switch(appD->eventCount)
	{
		case 1:
			if(!stringEqualToAscii(*qname.localName, "basis"))
			{
				printf("\nInvalid doc: basis!");
				return EXIP_HANDLER_STOP;
			}
			appD->eventCount++;
		break;
		case 3:
			if(!stringEqualToAscii(*qname.uri, "http://www.w3.org/2001/XMLSchema-instance") ||
					!stringEqualToAscii(*qname.localName, "nil"))
			{
				printf("\nInvalid doc: xsi:nill!");
				return EXIP_HANDLER_STOP;
			}
			appD->eventCount++;
		break;
		case 4:
			if(!stringEqualToAscii(*qname.localName, "bass"))
			{
				printf("\nInvalid doc: bass!");
				return EXIP_HANDLER_STOP;
			}
			appD->eventCount++;
		break;
		case 5:
			if(!stringEqualToAscii(*qname.localName, "foo"))
			{
				printf("\nInvalid doc: foo!");
				return EXIP_HANDLER_STOP;
			}
			appD->eventCount++;
		break;
		default:
			printf("\nInvalid decoding: attribute!");
			return EXIP_HANDLER_STOP;
			break;
	}
	return EXIP_ERR_OK;
}

static errorCode sample_stringData(const String value, void* app_data)
{
	appData* appD = (appData*) app_data;

	switch(appD->eventCount)
	{
		case 2:
			if(!stringEqualToAscii(value, "Ahaa"))
			{
				printf("\nInvalid decoding: Ahaa!");
				return EXIP_HANDLER_STOP;
			}
		break;
		case 5:
			if(!stringEqualToAscii(value, "fooBass"))
			{
				printf("\nInvalid decoding: fooBass!");
				return EXIP_HANDLER_STOP;
			}
		break;
		default:
			printf("\nInvalid decoding: stringData!");
			return EXIP_HANDLER_STOP;
			break;
	}
	return EXIP_ERR_OK;
}

static errorCode sample_intData(Integer int_val, void* app_data)
{
	if(int_val != 11)
	{
		printf("\nInvalid decoding: intData!");
		return EXIP_HANDLER_STOP;
	}

	return EXIP_ERR_OK;
}

START_TEST (test_default_options)
{
	EXIPSchema schema;
	Parser testParser;
	char buf[OUTPUT_BUFFER_SIZE];
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	BinaryBuffer buffer;
	char exipath[MAX_PATH_LEN + strlen(EMPTY_TYPE_DEFAULT)];
	size_t pathlen;
	FILE *infile;
	struct appData parsingData;

	buffer.buf = buf;
	buffer.bufContent = 0;
	buffer.bufLen = OUTPUT_BUFFER_SIZE;
	buffer.ioStrm.readWriteToStream = NULL;
	buffer.ioStrm.stream = NULL;

	// Parsing steps:

	// I.A: First, read in the schema
	parseSchema(EMPTY_TYPE_SCHEMA, &schema);

	// I.B: Define an external stream for the input to the parser if any
	pathlen = strlen(dataDir);
	memcpy(exipath, dataDir, pathlen);
	exipath[pathlen] = '/';
	memcpy(&exipath[pathlen+1], EMPTY_TYPE_DEFAULT, strlen(EMPTY_TYPE_DEFAULT)+1);

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
	parsingData.expectAttributeData = 0;

	testParser.handler.fatalError    = sample_fatalError;
	testParser.handler.error         = sample_fatalError;
	testParser.handler.startElement  = sample_startElement;
	testParser.handler.attribute     = sample_attribute;
	testParser.handler.stringData    = sample_stringData;
	testParser.handler.intData       = sample_intData;

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser, TRUE);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "parsing the header returns an error code %d", tmp_err_code);

	tmp_err_code = setSchema(&testParser, &schema);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "setSchema() returns an error code %d", tmp_err_code);
	// V: Parse the body of the EXI stream

	while(tmp_err_code == EXIP_ERR_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&testParser);
	fail_unless (tmp_err_code == EXIP_PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
}
END_TEST

START_TEST (test_strict_option)
{
	EXIPSchema schema;
	Parser testParser;
	char buf[OUTPUT_BUFFER_SIZE];
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	BinaryBuffer buffer;
	char exipath[MAX_PATH_LEN + strlen(EMPTY_TYPE_DEFAULT)];
	size_t pathlen;
	FILE *infile;
	struct appData parsingData;

	buffer.buf = buf;
	buffer.bufContent = 0;
	buffer.bufLen = OUTPUT_BUFFER_SIZE;
	buffer.ioStrm.readWriteToStream = NULL;
	buffer.ioStrm.stream = NULL;

	// Parsing steps:

	// I.A: First, read in the schema
	parseSchema(EMPTY_TYPE_SCHEMA, &schema);

	// I.B: Define an external stream for the input to the parser if any
	pathlen = strlen(dataDir);
	memcpy(exipath, dataDir, pathlen);
	exipath[pathlen] = '/';
	memcpy(&exipath[pathlen+1], EMPTY_TYPE_STRICT, strlen(EMPTY_TYPE_STRICT)+1);

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
	parsingData.expectAttributeData = 0;

	testParser.handler.fatalError    = sample_fatalError;
	testParser.handler.error         = sample_fatalError;
	testParser.handler.startElement  = sample_startElement;
	testParser.handler.attribute     = sample_attribute;
	testParser.handler.stringData    = sample_stringData;
	testParser.handler.intData       = sample_intData;

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser, FALSE);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "parsing the header returns an error code %d", tmp_err_code);

	tmp_err_code = setSchema(&testParser, &schema);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "setSchema() returns an error code %d", tmp_err_code);

	// V: Parse the body of the EXI stream

	while(tmp_err_code == EXIP_ERR_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&testParser);
	fail_unless (tmp_err_code == EXIP_PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
}
END_TEST

/* END: SchemaLess tests */

Suite* exip_suite(void)
{
	Suite *s = suite_create("Empty Type");

	{
	  /* Default options test case */
	  TCase *tc_Def = tcase_create ("Default options");
	  tcase_add_test (tc_Def, test_default_options);
	  suite_add_tcase (s, tc_Def);

	  /* Default options test case */
	  TCase *tc_Strict = tcase_create ("Strict");
	  tcase_add_test (tc_Strict, test_strict_option);
	  suite_add_tcase (s, tc_Strict);
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
