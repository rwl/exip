/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file check_strict_grammar.c
 * @brief Tests decoding documents using strict grammar
 *
 * @date Mar 29, 2012
 * @author Ken Bannister
 * @version 0.4
 * @par[Revision] $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "EXISerializer.h"
#include "EXIParser.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"
#include "memManagement.h"

#define INPUT_BUFFER_SIZE 200
#define OUTPUT_BUFFER_SIZE 200
#define MAX_PATH_LEN 200

/* Location for external test data */
static char *dataDir;

struct appData
{
	unsigned int eventCount;
	unsigned short expectAttributeData;
	AllocList allocList;
	String eventCode;
	String uri;
	String localName;
};
typedef struct appData appData;

/* Helper functions */

size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
}

size_t writeFileOutputStream(void* buf, size_t readSize, void* stream)
{
	FILE *outfile = (FILE*) stream;
	return fwrite(buf, 1, readSize, outfile);
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

static errorCode sample_startDocument(void* app_data)
{
	appData* appD = (appData*) app_data;
	asciiToString("SD", &appD->eventCode, &appD->allocList, TRUE);

	return EXIP_ERR_OK;
}

static errorCode sample_endDocument(void* app_data)
{
	appData* appD = (appData*) app_data;
	asciiToString("ED", &appD->eventCode, &appD->allocList, TRUE);

	return EXIP_ERR_OK;
}

static errorCode sample_startElement(QName qname, void* app_data)
{
	appData* appD = (appData*) app_data;
	asciiToString("SE", &appD->eventCode, &appD->allocList, TRUE);
	cloneStringManaged(qname.uri, &appD->uri, &appD->allocList);
	cloneStringManaged(qname.localName, &appD->localName, &appD->allocList);

	return EXIP_ERR_OK;
}

static errorCode sample_endElement(void* app_data)
{
	appData* appD = (appData*) app_data;
	asciiToString("EE", &appD->eventCode, &appD->allocList, TRUE);

	return EXIP_ERR_OK;
}

static errorCode sample_attribute(QName qname, void* app_data)
{
	appData* appD = (appData*) app_data;
	asciiToString("AT", &appD->eventCode, &appD->allocList, TRUE);
	
	/*
	printString(qname.uri);
	printf(" ");
	printString(qname.localName);
	printf("=\"");
	*/
	
	appD->expectAttributeData = 1;

	return EXIP_ERR_OK;
}

static errorCode sample_stringData(const String value, void* app_data)
{
	appData* appD = (appData*) app_data;
	if(appD->expectAttributeData)
	{
		asciiToString("AT", &appD->eventCode, &appD->allocList, TRUE);
		/*
		printString(&value);
		printf("\"\n");
		*/
		appD->expectAttributeData = 0;
	}
	else
	{
		asciiToString("CH", &appD->eventCode, &appD->allocList, TRUE);
		/*
		printString(&value);
		printf("\n");
		*/
	}

	return EXIP_ERR_OK;
}

static errorCode sample_decimalData(Decimal value, void* app_data)
{
	appData* appD = (appData*) app_data;
	if(appD->expectAttributeData)
	{
		asciiToString("AT", &appD->eventCode, &appD->allocList, TRUE);
		/*
		printf("%.1f\"\n", (double) value);
		*/
		appD->expectAttributeData = 0;
	}
	else
	{
		asciiToString("CH", &appD->eventCode, &appD->allocList, TRUE);
	}

	return EXIP_ERR_OK;
}

static errorCode sample_intData(Integer int_val, void* app_data)
{
	appData* appD = (appData*) app_data;
	/* char tmp_buf[30]; */
	if(appD->expectAttributeData)
	{
		asciiToString("AT", &appD->eventCode, &appD->allocList, TRUE);
		/*
		sprintf(tmp_buf, "%lld", int_val);
		printf("%s", tmp_buf);
		printf("\"\n");
		*/
		appD->expectAttributeData = 0;
	}
	else
	{
		asciiToString("CH", &appD->eventCode, &appD->allocList, TRUE);
		/*
		sprintf(tmp_buf, "%lld", int_val);
		printf("%s", tmp_buf);
		printf("\n");
		*/
	}

	return EXIP_ERR_OK;
}

static errorCode sample_floatData(Float fl_val, void* app_data)
{
	appData* appD = (appData*) app_data;
	/* char tmp_buf[30]; */
	if(appD->expectAttributeData)
	{
		asciiToString("AT", &appD->eventCode, &appD->allocList, TRUE);
		/*
		sprintf(tmp_buf, "%lldE%d", fl_val.mantissa, fl_val.exponent);
		printf("%s", tmp_buf);
		printf("\"\n");
		*/
		appD->expectAttributeData = 0;
	}
	else
	{
		asciiToString("CH", &appD->eventCode, &appD->allocList, TRUE);
		/*
		printf("%3d CH ", appD->eventCount);
		sprintf(tmp_buf, "%lldE%d", fl_val.mantissa, fl_val.exponent);
		printf("%s", tmp_buf);
		printf("\n");
		*/
	}

	return EXIP_ERR_OK;
}

/* Tests */

/* Compares document structure, including minOccurs/maxOccurs. */
START_TEST (test_acceptance_for_A_01)
{
	EXIPSchema schema;
	FILE *infile;
	Parser testParser;
	char buf[INPUT_BUFFER_SIZE];
	const char *schemafname = "testStates/acceptance-xsd.exi";
	const char *exifname = "testStates/acceptance_a_01.exi";
	char exipath[MAX_PATH_LEN + strlen(exifname)];
	struct appData parsingData;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	BinaryBuffer buffer;

	buffer.buf = buf;
	buffer.bufContent = 0;
	buffer.bufLen = INPUT_BUFFER_SIZE;

	// Parsing steps:

	// I.A: First, read in the schema
	parseSchema(schemafname, &schema);

	// I.B: Define an external stream for the input to the parser if any
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
	parsingData.expectAttributeData = 0;
	if (EXIP_ERR_OK != initAllocList(&parsingData.allocList))
		fail("Memory allocation error!");


	testParser.handler.fatalError    = sample_fatalError;
	testParser.handler.error         = sample_fatalError;
	testParser.handler.startDocument = sample_startDocument;
	testParser.handler.endDocument   = sample_endDocument;
	testParser.handler.startElement  = sample_startElement;
	testParser.handler.attribute     = sample_attribute;
	testParser.handler.stringData    = sample_stringData;
	testParser.handler.endElement    = sample_endElement;
	testParser.handler.decimalData   = sample_decimalData;
	testParser.handler.intData       = sample_intData;
	testParser.handler.floatData     = sample_floatData;
	
	// IV: Parse the header of the stream
	tmp_err_code = parseHeader(&testParser, FALSE);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "parsing the header returns an error code %d", tmp_err_code);

	tmp_err_code = setSchema(&testParser,  &schema);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "setSchema() returns an error code %d", tmp_err_code);
	// V: Parse the body of the EXI stream
	while(tmp_err_code == EXIP_ERR_OK)
	{
		switch (parsingData.eventCount)
		{
			case 0:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SD"));
				break;
			case 1:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SE"));
				fail_unless(stringEqualToAscii(parsingData.uri, "urn:foo"));
				fail_unless(stringEqualToAscii(parsingData.localName, "A"));
				break;
			case 2:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SE"));
				fail_unless(stringEqualToAscii(parsingData.uri, "urn:foo"));
				fail_unless(stringEqualToAscii(parsingData.localName, "AB"));
				break;
			case 3:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "CH"));
				break;
			case 4:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "EE"));
				break;
			case 5:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SE"));
				fail_unless(stringEqualToAscii(parsingData.uri, "urn:foo"));
				fail_unless(stringEqualToAscii(parsingData.localName, "AC"));
				break;
			case 6:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "CH"));
				break;
			case 7:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "EE"));
				break;
			case 8:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SE"));
				fail_unless(stringEqualToAscii(parsingData.uri, "urn:foo"));
				fail_unless(stringEqualToAscii(parsingData.localName, "AC"));
				break;
			case 9:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "CH"));
				break;
			case 10:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "EE"));
				break;
			case 11:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SE"));
				fail_unless(stringEqualToAscii(parsingData.uri, "urn:foo"));
				fail_unless(stringEqualToAscii(parsingData.localName, "AD"));
				break;
			case 12:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "CH"));
				break;
			case 13:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "EE"));
				break;
			case 14:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SE"));
				fail_unless(stringEqualToAscii(parsingData.uri, "urn:foo"));
				fail_unless(stringEqualToAscii(parsingData.localName, "AE"));
				break;
			case 15:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "CH"));
				break;
			case 16:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "EE"));
				break;
			case 17:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "EE"));
				break;
			case 18:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "ED"));
				break;
			default:
				// Unexpected event count caught below.
				break;
		}
		tmp_err_code = parseNext(&testParser);

		parsingData.eventCount++;
				
	}
	
	fail_unless(stringEqualToAscii(parsingData.eventCode, "ED"));

	fail_unless(parsingData.eventCount == 18,
	            "Unexpected event count: %u", parsingData.eventCount);

	// VI: Free the memory allocated by the parser object
	freeAllocList(&parsingData.allocList);
	destroyParser(&testParser);
	fclose(infile);
	fail_unless (tmp_err_code == EXIP_PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
}
END_TEST

/* Verifies a single global element also used as a reference. */
START_TEST (test_acceptance_for_A_01_exip1)
{
	EXIPSchema schema;
	FILE *infile;
	Parser testParser;
	char buf[INPUT_BUFFER_SIZE];
	const char *schemafname = "testStates/acceptance-xsd.exi";
	const char *exifname = "testStates/acceptance_a_01a.exi";
	char exipath[MAX_PATH_LEN + strlen(exifname)];
	struct appData parsingData;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	BinaryBuffer buffer;

	buffer.buf = buf;
	buffer.bufContent = 0;
	buffer.bufLen = INPUT_BUFFER_SIZE;

	// Parsing steps:

	// I.A: First, read in the schema
	parseSchema(schemafname, &schema);

	// I.B: Define an external stream for the input to the parser if any
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
	parsingData.expectAttributeData = 0;
	if (EXIP_ERR_OK != initAllocList(&parsingData.allocList))
		fail("Memory allocation error!");

	testParser.handler.fatalError    = sample_fatalError;
	testParser.handler.error         = sample_fatalError;
	testParser.handler.startDocument = sample_startDocument;
	testParser.handler.endDocument   = sample_endDocument;
	testParser.handler.startElement  = sample_startElement;
	testParser.handler.attribute     = sample_attribute;
	testParser.handler.stringData    = sample_stringData;
	testParser.handler.endElement    = sample_endElement;
	testParser.handler.decimalData   = sample_decimalData;
	testParser.handler.intData       = sample_intData;
	testParser.handler.floatData     = sample_floatData;
	
	// IV: Parse the header of the stream
	tmp_err_code = parseHeader(&testParser, FALSE);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "parsing the header returns an error code %d", tmp_err_code);

	tmp_err_code = setSchema(&testParser, &schema);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "setSchema() returns an error code %d", tmp_err_code);

	// V: Parse the body of the EXI stream
	while(tmp_err_code == EXIP_ERR_OK)
	{
		switch (parsingData.eventCount)
		{
			case 0:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SD"));
				break;
			case 1:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "SE"));
				fail_unless(stringEqualToAscii(parsingData.uri, "urn:foo"));
				fail_unless(stringEqualToAscii(parsingData.localName, "AB"));
				break;
			case 2:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "CH"));
				break;
			case 3:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "EE"));
				break;
			case 4:
				fail_unless(stringEqualToAscii(parsingData.eventCode, "ED"));
				break;
			default:
				// Unexpected event count caught below.
				break;
		}

		tmp_err_code = parseNext(&testParser);
		parsingData.eventCount++;
	}
	
	fail_unless(stringEqualToAscii(parsingData.eventCode, "ED"));

	fail_unless(parsingData.eventCount == 4,
	            "Unexpected event count: %u", parsingData.eventCount);

	// VI: Free the memory allocated by the parser object
	freeAllocList(&parsingData.allocList);
	destroyParser(&testParser);
	fclose(infile);
	fail_unless (tmp_err_code == EXIP_PARSING_COMPLETE, "Error during parsing of the EXI body %d", tmp_err_code);
}
END_TEST

/* 
 * Verifies error when more elements than schema maxOccurs permits. Attempts 
 * to encode:
 * 
 * <A xmlns='urn:foo'>
 *   <AB/><AC/><AC/><AC/>
 * </A>
 */
START_TEST (test_acceptance_for_A_01b)
{
	EXIPSchema schema;
	EXIStream testStrm;
	String uri;
	String ln;
	QName qname = {&uri, &ln, NULL};
	char buf[OUTPUT_BUFFER_SIZE];
	const char *schemafname = "testStates/acceptance-xsd.exi";
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	EXITypeClass valueType;
	
	const String NS_STR = {"urn:foo", 7};
	const String ELEM_A = {"A", 1};
	const String ELEM_AB = {"AB", 2};
	const String ELEM_AC = {"AC", 2};
	const String CH = {"", 0};
	
	BinaryBuffer buffer;
	buffer.buf = buf;
	buffer.bufLen = OUTPUT_BUFFER_SIZE;
	buffer.bufContent = 0;

	// Serialization steps:

	// I.A: First, read in the schema
	parseSchema(schemafname, &schema);

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_cookie = TRUE;
	testStrm.header.has_options = TRUE;
	testStrm.header.opts.valueMaxLength = 300;
	testStrm.header.opts.valuePartitionCapacity = 50;
	SET_STRICT(testStrm.header.opts.enumOpt);

	// III: Define an external stream for the output if any
	buffer.ioStrm.readWriteToStream = NULL;
	buffer.ioStrm.stream = NULL;

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, &schema);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "initStream returns an error code %d", tmp_err_code);

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code += serialize.exiHeader(&testStrm);

	tmp_err_code += serialize.startDocument(&testStrm);

	qname.uri = &NS_STR;
	qname.localName = &ELEM_A;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "serialization returns an error code %d", tmp_err_code);

	qname.localName = &ELEM_AB;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	tmp_err_code += serialize.stringData(&testStrm, CH);
	tmp_err_code += serialize.endElement(&testStrm);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "serialization returns an error code %d", tmp_err_code);

	qname.localName = &ELEM_AC;
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	tmp_err_code += serialize.stringData(&testStrm, CH);
	tmp_err_code += serialize.endElement(&testStrm);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "serialization returns an error code %d", tmp_err_code);
	
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	tmp_err_code += serialize.stringData(&testStrm, CH);
	tmp_err_code += serialize.endElement(&testStrm);
	fail_unless (tmp_err_code == EXIP_ERR_OK, "serialization returns an error code %d", tmp_err_code);
	
	/* Expect failure when start third AC element */
	tmp_err_code += serialize.startElement(&testStrm, qname, &valueType);
	fail_unless (tmp_err_code == EXIP_INCONSISTENT_PROC_STATE, 
	    "Expected EXIP_INCONSISTENT_PROC_STATE, but returns an error code %d", tmp_err_code);

	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);
}
END_TEST

/* START THE LKAB DEMO SUIT*/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
typedef char error_code; // 0 - OK;  Error otherwise

struct timestamp
{
	unsigned int year;
	unsigned int month;
	unsigned int mday;
	unsigned int hour;
	unsigned int min;
	unsigned int sec;
	unsigned int msec;
};

typedef struct timestamp Timestamp;

enum ValueType
{
	Bool     =0,
	Long     =1
};

typedef enum ValueType ValueType;

enum Quality {
	Good								=0,
	Good_LocalOverride					=1,

	Uncertain							=2,
	Uncertain_LastUsable				=3,
	Uncertain_SensorCal					=4,
	Uncertain_EGUExeeded				=5,
	Uncertain_SubNormal					=6,

	Bad									=7,
	Bad_ConfigError						=8,
	Bad_NotConnected					=9,
	Bad_DeviceFailure					=10,
	Bad_SensorFailure					=11,
	Bad_CommFailureLastKnownAvailable 	=12,
	Bad_CommFailure						=13,
	Bad_OutOfService					=14
};

typedef enum Quality Quality;

struct boolValue
{
	Timestamp ts;
	Quality quality;
	unsigned char val;
};

typedef struct boolValue BoolValue;

struct processValueDesc
{
	char name[50];
	ValueType type;
	unsigned char isReadOnly; // boolean
	char description[150];
};

typedef struct processValueDesc ProcessValueDesc;

struct devDescribtion
{
	char id[50];
	char name[50];
	char type[50];
	char location[100];
	ProcessValueDesc processValue;
};

typedef struct devDescribtion DevDescribtion;

extern const EXISerializer serialize;

const String NS_STR = {"http://imc-aesop.eu/lkab-demo", 29};
const String NS_EMPTY_STR = {NULL, 0};

const String ELEM_BOOL_VAL_STR = {"BoolValue", 9};
const String ELEM_QUAL_STR = {"quality", 7};
const String ELEM_TIMESTAMP_STR = {"timestamp", 9};
const String ELEM_VALUE_STR = {"value", 5};

const String ELEM_DEV_DESC_STR = {"DeviceDescription", 17};
const String ELEM_ID_STR = {"id", 2};
const String ELEM_NAME_STR = {"name", 4};
const String ELEM_TYPE_STR = {"type", 4};

const String ELEM_LOCATION_STR = {"location", 8};
const String ELEM_PROSS_VAL_STR = {"processValues", 13};
const String ELEM_IS_READONLY_STR = {"isReadOnly", 10};
const String ELEM_DESC_STR = {"description", 11};

static String ENUM_DATA_QUALITY[] = {{"Good", 4},
									 {"Good_LocalOverride", 18},
									 {"Uncertain", 9},
									 {"Uncertain_LastUsable", 20},
									 {"Uncertain_SensorCal", 19},
									 {"Uncertain_EGUExeeded", 20},
									 {"Uncertain_SubNormal", 19},
									 {"Bad", 3},
									 {"Bad_ConfigError", 15},
									 {"Bad_NotConnected", 16},
									 {"Bad_DeviceFailure", 17},
									 {"Bad_SensorFailure", 17},
									 {"Bad_CommFailureLastKnownAvailable", 33},
									 {"Bad_CommFailure", 15},
									 {"Bad_OutOfService", 16}};

static String ENUM_DATA_VAL_TYPE[] = {{"Bool", 4},
									  {"Long", 4}};

struct appDataLKAB
{
	BoolValue val;
	DevDescribtion devDesc;
	unsigned int currElementNumber;
};

// Content Handler API
static errorCode lkab_fatalError(const errorCode code, const char* msg, void* app_data);
static errorCode lkab_startElement_io(QName qname, void* app_data);
static errorCode lkab_startElement_desc(QName qname, void* app_data);
static errorCode lkab_endElement(void* app_data);
static errorCode lkab_stringData_io(const String value, void* app_data);
static errorCode lkab_stringData_desc(const String value, void* app_data);
static errorCode lkab_booleanData_io(boolean bool_val, void* app_data);
static errorCode lkab_booleanData_desc(boolean bool_val, void* app_data);
static errorCode lkab_dateTimeData(EXIPDateTime dt_val, void* app_data);

/**
 * @brief Reads the time-stamp and value data from a EXI message
 *
 * @param[in] buf A buffer containing the EXI stream
 * @param[in] buf_size the size of the buffer in bytes
 * @param[out] val a time-stamp and bool (digital I/O) value read from the message
 * @return Error handling code
 */
static error_code parseIOMsg(char* buf, unsigned int buf_size, BoolValue *val);

/**
 * @brief Reads a DevDescribtion EXI message
 *
 * @param[in] buf A buffer containing the EXI stream
 * @param[in] buf_size the size of the buffer in bytes
 * @param[out] devDesc the description of the device to be read
 * @return Error handling code
 */
static error_code parseDevDescMsg(char* buf, unsigned int buf_size, DevDescribtion* devDesc);

/**
 * @brief Serialize an I/O EXI message
 *
 * @param[in, out] buf An empty buffer for storing the EXI stream
 * @param[in] buf_size the size of the buffer in bytes
 * @param[out] msg_size the size of the serialized message in bytes
 * @param[in] val the time-stamp and bool (digital I/O) value to be written to the message
 * @return Error handling code
 */
static error_code serializeIOMsg(char* buf, unsigned int buf_size, unsigned int* msg_size, BoolValue val);

/**
 * @brief Serialize a DevDescribtion EXI message
 *
 * @param[in, out] buf An empty buffer for storing the EXI stream
 * @param[in] buf_size the size of the buffer in bytes
 * @param[out] msg_size the size of the serialized message in bytes
 * @param[in] devDesc the description of the device to be written
 * @return Error handling code
 */
static error_code serializeDevDescMsg(char* buf, unsigned int buf_size, unsigned int* msg_size, DevDescribtion devDesc);


static error_code serializeIOMsg(char* buf, unsigned int buf_size, unsigned int* msg_size, BoolValue val)
{
	EXIStream strm;
	String uri;
	String ln;
	QName qname = {&uri, &ln, NULL};
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	BinaryBuffer buffer;
	EXIPDateTime dt;
	EXIPSchema lkab_schema;
	const char *schemafname = "SchemaStrict/lkab-devices-xsd.exi";
	EXITypeClass valueType;

	buffer.buf = buf;
	buffer.bufLen = buf_size;
	buffer.bufContent = 0;

	// Serialization steps:

	// I.A: First, read in the schema
	parseSchema(schemafname, &lkab_schema);

	// I: First initialize the header of the stream
	serialize.initHeader(&strm);

	// II: Set any options in the header, if different from the defaults
	strm.header.has_options = TRUE;
	SET_STRICT(strm.header.opts.enumOpt);

	// III: Define an external stream for the output if any
	buffer.ioStrm.stream = NULL;
	buffer.ioStrm.readWriteToStream = NULL;

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&strm, buffer, &lkab_schema);
	if(tmp_err_code != EXIP_ERR_OK)
		return tmp_err_code;

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code += serialize.exiHeader(&strm);

	tmp_err_code += serialize.startDocument(&strm);

	qname.uri = &NS_STR;
	qname.localName = &ELEM_BOOL_VAL_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <BoolValue>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_TIMESTAMP_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <timeStamp>

	dt.presenceMask = 0;
	dt.presenceMask = dt.presenceMask | YEAR_PRESENCE;
	dt.presenceMask = dt.presenceMask | MON_PRESENCE;
	dt.presenceMask = dt.presenceMask | MDAY_PRESENCE;
	dt.presenceMask = dt.presenceMask | HOUR_PRESENCE;
	dt.presenceMask = dt.presenceMask | MIN_PRESENCE;
	dt.presenceMask = dt.presenceMask | SEC_PRESENCE;
	dt.presenceMask = dt.presenceMask | FRACT_PRESENCE;

	dt.dateTime.tm_year = val.ts.year;
	dt.dateTime.tm_mon = val.ts.month;
	dt.dateTime.tm_mday = val.ts.mday;
	dt.dateTime.tm_hour = val.ts.hour;
	dt.dateTime.tm_min = val.ts.min;
	dt.dateTime.tm_sec = val.ts.sec;
	dt.fSecs.value = val.ts.msec;
	dt.fSecs.offset = 2;

	tmp_err_code += serialize.dateTimeData(&strm, dt);

	tmp_err_code += serialize.endElement(&strm); // </timeStamp>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_QUAL_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <quality>

	tmp_err_code += serialize.stringData(&strm, ENUM_DATA_QUALITY[val.quality]); // quality

	tmp_err_code += serialize.endElement(&strm); // </quality>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_VALUE_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <value>

	tmp_err_code += serialize.booleanData(&strm, val.val);

	tmp_err_code += serialize.endElement(&strm); // </value>

	tmp_err_code += serialize.endElement(&strm); // </BoolValue>

	tmp_err_code += serialize.endDocument(&strm);

	if(tmp_err_code != EXIP_ERR_OK)
		return tmp_err_code;

	*msg_size = strm.context.bufferIndx + 1;

	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&strm);

	return EXIP_ERR_OK;
}

static error_code serializeDevDescMsg(char* buf, unsigned int buf_size, unsigned int* msg_size, DevDescribtion devDesc)
{
	EXIStream strm;
	String uri;
	String ln;
	String ch;
	QName qname = {&uri, &ln, NULL};
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	BinaryBuffer buffer;
	EXIPSchema lkab_schema;
	const char *schemafname = "SchemaStrict/lkab-devices-xsd.exi";
	EXITypeClass valueType;

	buffer.buf = buf;
	buffer.bufLen = buf_size;
	buffer.bufContent = 0;

	// Serialization steps:

	// I.A: First, read in the schema
	parseSchema(schemafname, &lkab_schema);

	// I: First initialize the header of the stream
	serialize.initHeader(&strm);

	// II: Set any options in the header, if different from the defaults
	strm.header.has_options = TRUE;
	SET_STRICT(strm.header.opts.enumOpt);

	// III: Define an external stream for the output if any
	buffer.ioStrm.stream = NULL;
	buffer.ioStrm.readWriteToStream = NULL;

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&strm, buffer, &lkab_schema);
	if(tmp_err_code != EXIP_ERR_OK)
		return tmp_err_code;

	// V: Start building the stream step by step: header, document, element etc...
	tmp_err_code += serialize.exiHeader(&strm);

	tmp_err_code += serialize.startDocument(&strm);

	qname.uri = &NS_STR;
	qname.localName = &ELEM_DEV_DESC_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <DeviceDescription>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_ID_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <id>

	ch.str = devDesc.id;
	ch.length = strlen(devDesc.id);
	tmp_err_code += serialize.stringData(&strm, ch); // device id

	tmp_err_code += serialize.endElement(&strm); // </id>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_NAME_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <name>

	ch.str = devDesc.name;
	ch.length = strlen(devDesc.name);
	tmp_err_code += serialize.stringData(&strm, ch); // device name

	tmp_err_code += serialize.endElement(&strm); // </name>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_TYPE_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <type>

	ch.str = devDesc.type;
	ch.length = strlen(devDesc.type);
	tmp_err_code += serialize.stringData(&strm, ch); // device type

	tmp_err_code += serialize.endElement(&strm); // </type>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_LOCATION_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <location>

	ch.str = devDesc.location;
	ch.length = strlen(devDesc.location);
	tmp_err_code += serialize.stringData(&strm, ch); // device location

	tmp_err_code += serialize.endElement(&strm); // </location>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_PROSS_VAL_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <processValues>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_NAME_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <name>

	ch.str = devDesc.processValue.name;
	ch.length = strlen(devDesc.processValue.name);
	tmp_err_code += serialize.stringData(&strm, ch); // processValues name

	tmp_err_code += serialize.endElement(&strm); // </name>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_TYPE_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <type>

	tmp_err_code += serialize.stringData(&strm, ENUM_DATA_VAL_TYPE[devDesc.processValue.type]); // processValues type

	tmp_err_code += serialize.endElement(&strm); // </type>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_IS_READONLY_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <isReadOnly>

	tmp_err_code += serialize.booleanData(&strm, 0); // processValues isReadOnly

	tmp_err_code += serialize.endElement(&strm); // </isReadOnly>

	qname.uri = &NS_STR;
	qname.localName = &ELEM_DESC_STR;
	tmp_err_code += serialize.startElement(&strm, qname, &valueType); // <description>

	ch.str = devDesc.processValue.description;
	ch.length = strlen(devDesc.processValue.description);
	tmp_err_code += serialize.stringData(&strm, ch); // processValues description

	tmp_err_code += serialize.endElement(&strm); // </description>

	tmp_err_code += serialize.endElement(&strm); // </processValues>

	tmp_err_code += serialize.endElement(&strm); // </DeviceDescription>

	tmp_err_code += serialize.endDocument(&strm);

	if(tmp_err_code != EXIP_ERR_OK)
		return tmp_err_code;

	*msg_size = strm.context.bufferIndx + 1;

	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&strm);

	return EXIP_ERR_OK;
}

static error_code parseIOMsg(char* buf, unsigned int buf_size, BoolValue *val)
{
	Parser lkabParser;
	BinaryBuffer buffer;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	struct appDataLKAB parsingData;
	EXIPSchema lkab_schema;
	const char *schemafname = "SchemaStrict/lkab-devices-xsd.exi";

	buffer.buf = buf;
	buffer.bufLen = buf_size;
	buffer.bufContent = buf_size;
	// Parsing steps:

	// I.A: First, read in the schema
	parseSchema(schemafname, &lkab_schema);

	// I: First, define an external stream for the input to the parser if any
	buffer.ioStrm.stream = NULL;

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&lkabParser, buffer, &parsingData);
	if(tmp_err_code != EXIP_ERR_OK)
		return tmp_err_code;

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	parsingData.currElementNumber = 0;
	parsingData.val.quality = Good;
	parsingData.val.val = 0;
	parsingData.val.ts.year = 0;
	parsingData.val.ts.month = 0;
	parsingData.val.ts.mday = 0;
	parsingData.val.ts.hour = 0;
	parsingData.val.ts.min = 0;
	parsingData.val.ts.sec = 0;
	parsingData.val.ts.msec = 0;

	lkabParser.handler.fatalError = lkab_fatalError;
	lkabParser.handler.error = lkab_fatalError;
	lkabParser.handler.startElement = lkab_startElement_io;
	lkabParser.handler.stringData = lkab_stringData_io;
	lkabParser.handler.endElement = lkab_endElement;
	lkabParser.handler.booleanData = lkab_booleanData_io;
	lkabParser.handler.dateTimeData = lkab_dateTimeData;

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&lkabParser, FALSE);

	tmp_err_code = setSchema(&lkabParser, &lkab_schema);
	// V: Parse the body of the EXI stream

	while(tmp_err_code == EXIP_ERR_OK)
	{
		tmp_err_code = parseNext(&lkabParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&lkabParser);

	val->ts = parsingData.val.ts;
	val->val = parsingData.val.val;
	val->quality = parsingData.val.quality;

	if(tmp_err_code == EXIP_PARSING_COMPLETE)
		return EXIP_ERR_OK;
	else
		return tmp_err_code;
}

static error_code parseDevDescMsg(char* buf, unsigned int buf_size, DevDescribtion* devDesc)
{
	Parser lkabParser;
	BinaryBuffer buffer;
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	struct appDataLKAB parsingData;
	EXIPSchema lkab_schema;
	const char *schemafname = "SchemaStrict/lkab-devices-xsd.exi";

	buffer.buf = buf;
	buffer.bufLen = buf_size;
	buffer.bufContent = buf_size;

	// Parsing steps:

	// I.A: First, read in the schema
	parseSchema(schemafname, &lkab_schema);

	// I: First, define an external stream for the input to the parser if any
	buffer.ioStrm.stream = NULL;

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&lkabParser, buffer, &parsingData);
	if(tmp_err_code != EXIP_ERR_OK)
		return tmp_err_code;

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	parsingData.currElementNumber = 0;
	parsingData.devDesc.id[0] = '\0';
	parsingData.devDesc.location[0] = '\0';
	parsingData.devDesc.name[0] = '\0';
	parsingData.devDesc.type[0] = '\0';
	parsingData.devDesc.processValue.description[0] = '\0';
	parsingData.devDesc.processValue.name[0] = '\0';
	parsingData.devDesc.processValue.isReadOnly = 0;
	parsingData.devDesc.processValue.type = Bool;

	lkabParser.handler.fatalError = lkab_fatalError;
	lkabParser.handler.error = lkab_fatalError;
	lkabParser.handler.startElement = lkab_startElement_desc;
	lkabParser.handler.stringData = lkab_stringData_desc;
	lkabParser.handler.endElement = lkab_endElement;
	lkabParser.handler.booleanData = lkab_booleanData_desc;

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&lkabParser, FALSE);

	tmp_err_code = setSchema(&lkabParser, &lkab_schema);
	// V: Parse the body of the EXI stream

	while(tmp_err_code == EXIP_ERR_OK)
	{
		tmp_err_code = parseNext(&lkabParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&lkabParser);

	strcpy(devDesc->id, parsingData.devDesc.id);
	strcpy(devDesc->location, parsingData.devDesc.location);
	strcpy(devDesc->name, parsingData.devDesc.name);
	strcpy(devDesc->type, parsingData.devDesc.type);
	strcpy(devDesc->processValue.description, parsingData.devDesc.processValue.description);
	strcpy(devDesc->processValue.name, parsingData.devDesc.processValue.name);
	devDesc->processValue.isReadOnly = parsingData.devDesc.processValue.isReadOnly;
	devDesc->processValue.type = parsingData.devDesc.processValue.type;

	if(tmp_err_code == EXIP_PARSING_COMPLETE)
		return EXIP_ERR_OK;
	else
		return tmp_err_code;
}

static errorCode lkab_fatalError(const errorCode code, const char* msg, void* app_data)
{
	return EXIP_HANDLER_STOP;
}

static errorCode lkab_startElement_io(QName qname, void* app_data)
{
	struct appDataLKAB* appD = (struct appDataLKAB*) app_data;
	QName expectedElem;

	appD->currElementNumber += 1;

	expectedElem.uri = &NS_STR;
	switch(appD->currElementNumber)
	{
		case 1:
			expectedElem.localName = &ELEM_BOOL_VAL_STR;
		break;
		case 2:
			expectedElem.localName = &ELEM_TIMESTAMP_STR;
		break;
		case 3:
			expectedElem.localName = &ELEM_QUAL_STR;
		break;
		case 4:
			expectedElem.localName = &ELEM_VALUE_STR;
		break;
		default:
			return EXIP_HANDLER_STOP;
		break;
	}

	if(!stringEqual(*expectedElem.uri, *(qname.uri)) ||
			!stringEqual(*expectedElem.localName, *(qname.localName)))
	{
		return EXIP_HANDLER_STOP;
	}

	return EXIP_ERR_OK;
}

static errorCode lkab_startElement_desc(QName qname, void* app_data)
{
	struct appDataLKAB* appD = (struct appDataLKAB*) app_data;
	QName expectedElem;

	appD->currElementNumber += 1;

	expectedElem.uri = &NS_STR;
	switch(appD->currElementNumber)
	{
		case 1:
			expectedElem.localName = &ELEM_DEV_DESC_STR;
		break;
		case 2:
			expectedElem.localName = &ELEM_ID_STR;
		break;
		case 3:
			expectedElem.localName = &ELEM_NAME_STR;
		break;
		case 4:
			expectedElem.localName = &ELEM_TYPE_STR;
		break;
		case 5:
			expectedElem.localName = &ELEM_LOCATION_STR;
		break;
		case 6:
			expectedElem.localName = &ELEM_PROSS_VAL_STR;
		break;
		case 7:
			expectedElem.localName = &ELEM_NAME_STR;
		break;
		case 8:
			expectedElem.localName = &ELEM_TYPE_STR;
		break;
		case 9:
			expectedElem.localName = &ELEM_IS_READONLY_STR;
		break;
		case 10:
			expectedElem.localName = &ELEM_DESC_STR;
		break;
		default:
			return EXIP_HANDLER_STOP;
		break;
	}

	if(!stringEqual(*expectedElem.uri, *(qname.uri)) ||
			!stringEqual(*expectedElem.localName, *(qname.localName)))
	{
		return EXIP_HANDLER_STOP;
	}

	return EXIP_ERR_OK;
}

static errorCode lkab_endElement(void* app_data)
{
	return EXIP_ERR_OK;
}

static errorCode lkab_stringData_io(const String value, void* app_data)
{
	struct appDataLKAB* appD = (struct appDataLKAB*) app_data;

	if(appD->currElementNumber != 3)
		return EXIP_HANDLER_STOP;
	else
	{
		int i;
		for(i = 0; i < 15; i++)
		{
			if(stringEqual(ENUM_DATA_QUALITY[i], value))
			{
				appD->val.quality = i;
				return EXIP_ERR_OK;
			}
		}
		return EXIP_HANDLER_STOP;
	}
}

static errorCode lkab_stringData_desc(const String value, void* app_data)
{
	struct appDataLKAB* appD = (struct appDataLKAB*) app_data;

	switch(appD->currElementNumber)
	{
		case 2:
			memcpy(appD->devDesc.id, value.str, sizeof(CharType)*value.length);
			appD->devDesc.id[value.length] = '\0';
		break;
		case 3:
			memcpy(appD->devDesc.name, value.str, sizeof(CharType)*value.length);
			appD->devDesc.name[value.length] = '\0';
		break;
		case 4:
			memcpy(appD->devDesc.type, value.str, sizeof(CharType)*value.length);
			appD->devDesc.type[value.length] = '\0';
		break;
		case 5:
			memcpy(appD->devDesc.location, value.str, sizeof(CharType)*value.length);
			appD->devDesc.location[value.length] = '\0';
		break;
		case 7:
			memcpy(appD->devDesc.processValue.name, value.str, sizeof(CharType)*value.length);
			appD->devDesc.processValue.name[value.length] = '\0';
		break;
		case 8:
			if(stringEqual(value, ENUM_DATA_VAL_TYPE[Bool]))
				appD->devDesc.processValue.type = Bool;
			else if(stringEqual(value, ENUM_DATA_VAL_TYPE[Long]))
				appD->devDesc.processValue.type = Long;
			else
				return EXIP_HANDLER_STOP;
		break;
		case 10:
			memcpy(appD->devDesc.processValue.description, value.str, sizeof(CharType)*value.length);
			appD->devDesc.processValue.description[value.length] = '\0';
		break;
		default:
			return EXIP_HANDLER_STOP;
		break;
	}

	return EXIP_ERR_OK;
}

static errorCode lkab_booleanData_io(boolean bool_val, void* app_data)
{
	struct appDataLKAB* appD = (struct appDataLKAB*) app_data;

	if(appD->currElementNumber != 4)
		return EXIP_HANDLER_STOP;
	else
	{
		appD->val.val = bool_val;
	}

	return EXIP_ERR_OK;
}

static errorCode lkab_booleanData_desc(boolean bool_val, void* app_data)
{
	struct appDataLKAB* appD = (struct appDataLKAB*) app_data;

	if(appD->currElementNumber != 9)
		return EXIP_HANDLER_STOP;
	else
	{
		appD->devDesc.processValue.isReadOnly = bool_val;
	}

	return EXIP_ERR_OK;
}

static errorCode lkab_dateTimeData(EXIPDateTime dt_val, void* app_data)
{
	struct appDataLKAB* appD = (struct appDataLKAB*) app_data;

	appD->val.ts.year = dt_val.dateTime.tm_year;
	appD->val.ts.month = dt_val.dateTime.tm_mon;
	appD->val.ts.mday = dt_val.dateTime.tm_mday;
	appD->val.ts.hour = dt_val.dateTime.tm_hour;
	appD->val.ts.min = dt_val.dateTime.tm_min;
	appD->val.ts.sec = dt_val.dateTime.tm_sec;
	appD->val.ts.msec = dt_val.fSecs.value;

	return EXIP_ERR_OK;
}

#define LKAB_BUFFER_SIZE 1000

START_TEST (test_lkab_demo_suit)
{
	char lkab_buf[LKAB_BUFFER_SIZE];
	unsigned int msg_size;
	BoolValue bVal;
	DevDescribtion devD;
	error_code err;

	memset(lkab_buf, '\0', LKAB_BUFFER_SIZE);

	bVal.ts.hour = 0;
	bVal.ts.mday = 0;
	bVal.ts.min = 10;
	bVal.ts.month = 0;
	bVal.ts.msec = 115;
	bVal.ts.sec = 10;
	bVal.ts.year = 0;
	bVal.quality = Good;
	bVal.val = 1;

	err = serializeIOMsg(lkab_buf, LKAB_BUFFER_SIZE, &msg_size, bVal);

	fail_unless (err == EXIP_ERR_OK, "Error during serialization of IO Msg %d", err);
	fail_if(msg_size == 0, "0 Length message size");

	bVal.ts.hour = 10;
	bVal.ts.mday = 30;
	bVal.ts.min = 120;
	bVal.ts.month = 44;
	bVal.ts.msec = 125;
	bVal.ts.sec = 110;
	bVal.ts.year = 4;
	bVal.val = 0;
	bVal.quality = Bad;

	err = parseIOMsg(lkab_buf, LKAB_BUFFER_SIZE, &bVal);

	fail_unless (err == EXIP_ERR_OK, "Error during parsing of IO Msg %d", err);
	fail_unless (bVal.quality == Good, "quality not correct");
	fail_unless (bVal.val == 1, "value not correct");
	fail_unless (bVal.ts.min == 10, "min not correct");
	fail_unless (bVal.ts.sec == 10, "sec not correct");
	fail_unless (bVal.ts.msec == 115, "msec not correct");

	strcpy(devD.id, "AirPS");
	strcpy(devD.location, "Grate Support Shaft");
	strcpy(devD.name, "Pressure switch");
	strcpy(devD.type, "Air pressure switch");
	strcpy(devD.processValue.description, "This is an Air pressure switch ON/OFF value");
	strcpy(devD.processValue.name, "Air pressure switch ON/OFF");
	devD.processValue.isReadOnly = 0;
	devD.processValue.type = Bool;

	err = serializeDevDescMsg(lkab_buf, LKAB_BUFFER_SIZE, &msg_size, devD);

	fail_unless (err == EXIP_ERR_OK, "Error during serialization of DevDesc Msg %d", err);
	fail_if(msg_size == 0, "0 Length message size");

	strcpy(devD.id, "00000000000");
	strcpy(devD.location, "00000000000");
	strcpy(devD.name, "00000000000");
	strcpy(devD.type, "00000000000");
	strcpy(devD.processValue.description, "00000000000");
	strcpy(devD.processValue.name, "00000000000");
	devD.processValue.isReadOnly = 1;
	devD.processValue.type = Long;

	err = parseDevDescMsg(lkab_buf, LKAB_BUFFER_SIZE, &devD);

	fail_unless (err == EXIP_ERR_OK, "Error during parsing of DevDesc Msg %d", err);
	fail_unless (devD.processValue.isReadOnly == 0, "isReadOnly not correct");
	fail_unless (devD.processValue.type == Bool, "type not correct");
	fail_unless (strcmp(devD.id, "AirPS") == 0, "id not correct");
	fail_unless (strcmp(devD.location, "Grate Support Shaft") == 0, "location not correct");
	fail_unless (strcmp(devD.name, "Pressure switch") == 0, "name not correct");
	fail_unless (strcmp(devD.type, "Air pressure switch") == 0, "type not correct");
	fail_unless (strcmp(devD.processValue.description, "This is an Air pressure switch ON/OFF value") == 0, "description not correct");
	fail_unless (strcmp(devD.processValue.name, "Air pressure switch ON/OFF") == 0, "name not correct");
}
END_TEST
/**********************************************************************/
/**********************************************************************/
/* END THE LKAB DEMO SUIT*/

/* Test suite */

Suite* exip_suite(void)
{
	Suite *s = suite_create("Strict Grammar");

	{
	  TCase *tc_builtin = tcase_create ("Strict Grammar");
	  tcase_add_test (tc_builtin, test_acceptance_for_A_01);
	  tcase_add_test (tc_builtin, test_acceptance_for_A_01_exip1);
	  tcase_add_test (tc_builtin, test_acceptance_for_A_01b);
	  tcase_add_test (tc_builtin, test_lkab_demo_suit);
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

