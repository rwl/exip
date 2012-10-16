/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file encodeTestEXI.c
 * @brief Testing the EXI encoder
 *
 * @date Nov 4, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */
#include "EXISerializer.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"
#include <stdio.h>
#include <string.h>

#define OUTPUT_BUFFER_SIZE 200
#define MAX_XSD_FILES_COUNT 10 // up to 10 XSD files

const String NS_STR = {"http://www.ltu.se/EISLAB/schema-test", 36};
const String NS_NESTED_STR = {"http://www.ltu.se/EISLAB/nested-xsd", 35};
const String NS_TYPES_STR = {"http://www.ltu.se/EISLAB/types", 30};
const String NS_EMPTY_STR = {NULL, 0};

const String ELEM_ENCODE_STR = {"EXIPEncoder", 11};
const String ELEM_MULT_TEST_STR = {"MultipleXSDsTest", 16};
const String ELEM_DESCR_STR = {"description", 11};
const String ELEM_TYPE_TEST_STR = {"type-test", 9};
const String ELEM_TEST_SETUP_STR = {"testSetup", 9};
const String ELEM_BOOL_STR = {"bool", 4};
const String ELEM_INT_STR = {"int", 3};
const String ELEM_EXT_TYPES_STR = {"extendedTypeTest", 16};
const String ELEM_BYTE_TYPES_STR = {"byteTest", 8};
const String ELEM_DATE_TYPES_STR = {"dateTimeTest", 12};
const String ELEM_BIN_TYPES_STR = {"binaryTest", 10};
const String ELEM_ENUM_TYPES_STR = {"enumTest", 8};

const String ATTR_BYTE_STR = {"testByte", 8};
const String ATTR_VERSION_STR = {"version", 7};
const String ATTR_GOAL_STR = {"goal", 4};
const String ATTR_ID_STR = {"id", 2};

static char SOME_BINARY_DATA[] = {0x02, 0x6d, 0x2f, 0xa5, 0x20, 0xf2, 0x61, 0x9c, 0xee, 0x0f};
static String SOME_BINARY_DATA_BASE64 = {"i3sd7fatzxad", 12};
//static String ENUM_DATA_1 = {"hello", 5};
//static String ENUM_DATA_2 = {"hi", 2};
//static String ENUM_DATA_3 = {"hey", 3};
static String ENUM_DATA_4 = {"hej", 3};

static void printfHelp();
static void printError(errorCode err_code, EXIStream* strm, FILE *outfile);

size_t writeFileOutputStream(void* buf, size_t readSize, void* stream);

int main(int argc, char *argv[])
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	FILE *outfile;
	char sourceFile[50];
	EXIPSchema schema;
	EXIPSchema* schemaPtr = NULL;

	if(argc > 1)
	{
		if(strcmp(argv[1], "-help") == 0)
		{
			printfHelp();
			return 0;
		}
		else if(strcmp(argv[1], "-schema") == 0)
		{
			FILE *schemaFile;
 			BinaryBuffer buffer[MAX_XSD_FILES_COUNT]; // up to 10 XSD files
			char schemaFileName[50];
			unsigned int schemaFilesCount = 0;
			unsigned int i;

			if(argc <= 3)
			{
				printfHelp();
				return 0;
			}
			else
				schemaFilesCount = argc - 3;

			if(schemaFilesCount > MAX_XSD_FILES_COUNT)
			{
				fprintf(stderr, "Too many xsd files given as an input: %d", schemaFilesCount);
				exit(1);
			}

			for(i = 0; i < schemaFilesCount; i++)
			{
				strcpy(schemaFileName, argv[2 + i]);

				schemaFile = fopen(schemaFileName, "rb" );
				if(!schemaFile)
				{
					fprintf(stderr, "Unable to open file %s", schemaFileName);
					return 1;
				}
				else
				{
					//Get file length
					fseek(schemaFile, 0, SEEK_END);
					buffer[i].bufLen = ftell(schemaFile) + 1;
					fseek(schemaFile, 0, SEEK_SET);

					//Allocate memory
					buffer[i].buf = (char *) malloc(buffer[i].bufLen);
					if (!buffer[i].buf)
					{
						fprintf(stderr, "Memory allocation error!");
						fclose(schemaFile);
						return 1;
					}

					//Read file contents into buffer
					fread(buffer[i].buf, buffer[i].bufLen, 1, schemaFile);
					fclose(schemaFile);

					buffer[i].bufContent = buffer[i].bufLen;
					buffer[i].ioStrm.readWriteToStream = NULL;
					buffer[i].ioStrm.stream = NULL;
				}
			}

			tmp_err_code = generateSchemaInformedGrammars(buffer, schemaFilesCount, SCHEMA_FORMAT_XSD_EXI, &schema);
			if(tmp_err_code != ERR_OK)
			{
				printf("\n Error occured: %d", tmp_err_code);
				return 1;
			}

			schemaPtr = &schema;
			for(i = 0; i < schemaFilesCount; i++)
			{
				free(buffer[i].buf);
			}
		}

		strcpy(sourceFile, argv[argc - 1]);

		outfile = fopen(sourceFile, "wb" );
		if(!outfile)
		{
			fprintf(stderr, "Unable to open file %s", sourceFile);
			return 1;
		}
		else
		{
			EXIStream testStrm;
			String uri;
			String ln;
			QName qname = {&uri, &ln, NULL};
			String chVal;
			char buf[OUTPUT_BUFFER_SIZE];
			BinaryBuffer buffer;

			buffer.buf = buf;
			buffer.bufLen = OUTPUT_BUFFER_SIZE;
			buffer.bufContent = 0;

			// Serialization steps:

			// I: First initialize the header of the stream
			serialize.initHeader(&testStrm);

			// II: Set any options in the header, if different from the defaults
			testStrm.header.has_cookie = TRUE;
			testStrm.header.has_options = TRUE;
			testStrm.header.opts.valueMaxLength = 300;
			testStrm.header.opts.valuePartitionCapacity = 50;
			SET_STRICT(testStrm.header.opts.enumOpt);

			// III: Define an external stream for the output if any
			buffer.ioStrm.readWriteToStream = writeFileOutputStream;
			buffer.ioStrm.stream = outfile;

			// IV: Initialize the stream
			tmp_err_code = serialize.initStream(&testStrm, buffer, schemaPtr, SCHEMA_ID_ABSENT, NULL);
			if(tmp_err_code != ERR_OK)
				printError(tmp_err_code, &testStrm, outfile);

			// V: Start building the stream step by step: header, document, element etc...
			tmp_err_code += serialize.exiHeader(&testStrm);

			tmp_err_code += serialize.startDocument(&testStrm);

			qname.uri = &NS_STR;
			qname.localName = &ELEM_MULT_TEST_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <MultipleXSDsTest>

			qname.uri = &NS_STR;
			qname.localName = &ELEM_ENCODE_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <EXIPEncoder>

			// NOTE: attributes should come lexicographically sorted during serialization

			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ATTR_BYTE_STR;

			if(schemaPtr != NULL)
			{
				// schema mode
				tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_SMALL_INTEGER); // testByte="
				tmp_err_code += serialize.intData(&testStrm, 55);
			}
			else
			{
				// schema-less mode
				tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_STRING); // testByte="
				tmp_err_code += asciiToString("55", &chVal, &testStrm.memList, FALSE);
				tmp_err_code += serialize.stringData(&testStrm, chVal);
			}

			qname.localName = &ATTR_VERSION_STR;
			tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_STRING); // version="

			tmp_err_code += asciiToString("0.2", &chVal, &testStrm.memList, FALSE);
			tmp_err_code += serialize.stringData(&testStrm, chVal);

			tmp_err_code += asciiToString("This is an example of serializing EXI streams using EXIP low level API", &chVal, &testStrm.memList, FALSE);
			tmp_err_code += serialize.stringData(&testStrm, chVal);

			tmp_err_code += serialize.endElement(&testStrm); // </EXIPEncoder>

			qname.uri = &NS_STR;
			qname.localName = &ELEM_DESCR_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <description>

			tmp_err_code += asciiToString("This is a test of processing XML schemes with multiple XSD files", &chVal, &testStrm.memList, FALSE);
			tmp_err_code += serialize.stringData(&testStrm, chVal);

			tmp_err_code += serialize.endElement(&testStrm); // </description>

			qname.uri = &NS_NESTED_STR;
			qname.localName = &ELEM_TEST_SETUP_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <testSetup>

			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ATTR_GOAL_STR;
			tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_STRING); // goal="

			tmp_err_code += asciiToString("Verify that the implementation works!", &chVal, &testStrm.memList, FALSE);
			tmp_err_code += serialize.stringData(&testStrm, chVal);

			tmp_err_code += asciiToString("Simple test element with single attribute", &chVal, &testStrm.memList, FALSE);
			tmp_err_code += serialize.stringData(&testStrm, chVal);

			tmp_err_code += serialize.endElement(&testStrm); // </testSetup>

			qname.uri = &NS_STR;
			qname.localName = &ELEM_TYPE_TEST_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <type-test>

			if(schemaPtr != NULL)
			{
				// schema mode
				qname.uri = &NS_EMPTY_STR;
				qname.localName = &ATTR_ID_STR;
				tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_INTEGER); // id="
				tmp_err_code += serialize.intData(&testStrm, 1001);
			}
			else
			{
				// schema-less mode
				qname.uri = &NS_EMPTY_STR;
				qname.localName = &ATTR_ID_STR;
				tmp_err_code += serialize.attribute(&testStrm, qname, VALUE_TYPE_STRING); // id="
				tmp_err_code += asciiToString("1001", &chVal, &testStrm.memList, FALSE);
				tmp_err_code += serialize.stringData(&testStrm, chVal);
			}

			qname.uri = &NS_NESTED_STR;
			qname.localName = &ELEM_BOOL_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <bool>

			if(schemaPtr != NULL)
			{
				// schema mode
				tmp_err_code += serialize.booleanData(&testStrm, TRUE);
			}
			else
			{
				// schema-less mode
				tmp_err_code += asciiToString("true", &chVal, &testStrm.memList, FALSE);
				tmp_err_code += serialize.stringData(&testStrm, chVal);
			}

			tmp_err_code += serialize.endElement(&testStrm); // </bool>

			tmp_err_code += serialize.endElement(&testStrm); // </type-test>

			qname.uri = &NS_STR;
			qname.localName = &ELEM_EXT_TYPES_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <extendedTypeTest>

			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ELEM_BYTE_TYPES_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <byteTest>

			if(schemaPtr != NULL)
			{
				// schema mode
				tmp_err_code += serialize.intData(&testStrm, 11);
			}
			else
			{
				// schema-less mode
				tmp_err_code += asciiToString("11", &chVal, &testStrm.memList, FALSE);
				tmp_err_code += serialize.stringData(&testStrm, chVal);
			}

			tmp_err_code += serialize.endElement(&testStrm); // </byteTest>

			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ELEM_DATE_TYPES_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <dateTimeTest>

			if(schemaPtr != NULL)
			{
				// schema mode
				EXIPDateTime dt;

				dt.presenceMask = 0;
				dt.presenceMask = dt.presenceMask | YEAR_PRESENCE;
				dt.presenceMask = dt.presenceMask | MON_PRESENCE;
				dt.presenceMask = dt.presenceMask | MDAY_PRESENCE;
				dt.presenceMask = dt.presenceMask | HOUR_PRESENCE;
				dt.presenceMask = dt.presenceMask | MIN_PRESENCE;
				dt.presenceMask = dt.presenceMask | SEC_PRESENCE;
				dt.presenceMask = dt.presenceMask | FRACT_PRESENCE;

				dt.dateTime.tm_year = 112; // 2012
				dt.dateTime.tm_mon = 6;	// July
				dt.dateTime.tm_mday = 31;
				dt.dateTime.tm_hour = 13;
				dt.dateTime.tm_min = 33;
				dt.dateTime.tm_sec = 55;
				dt.fSecs.value = 839;
				dt.fSecs.offset = 5;

				tmp_err_code += serialize.dateTimeData(&testStrm, dt);
			}
			else
			{
				// schema-less mode
				tmp_err_code += asciiToString("2012 Jul 31 13:33", &chVal, &testStrm.memList, FALSE);
				tmp_err_code += serialize.stringData(&testStrm, chVal);
			}

			tmp_err_code += serialize.endElement(&testStrm); // </dateTimeTest>

			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ELEM_BIN_TYPES_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <binaryTest>

			if(schemaPtr != NULL)
			{
				// schema mode
				tmp_err_code += serialize.binaryData(&testStrm, SOME_BINARY_DATA, 10);
			}
			else
			{
				// schema-less mode
				tmp_err_code += serialize.stringData(&testStrm, SOME_BINARY_DATA_BASE64);
			}

			tmp_err_code += serialize.endElement(&testStrm); // </binaryTest>

			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ELEM_ENUM_TYPES_STR;
			tmp_err_code += serialize.startElement(&testStrm, qname); // <enumTest>
			tmp_err_code += serialize.stringData(&testStrm, ENUM_DATA_4);
			tmp_err_code += serialize.endElement(&testStrm); // </enumTest>


			tmp_err_code += serialize.endElement(&testStrm); // </extendedTypeTest>

			tmp_err_code += serialize.endElement(&testStrm); // </MultipleXSDsTest>
			tmp_err_code += serialize.endDocument(&testStrm);

			if(tmp_err_code != ERR_OK)
				printError(tmp_err_code, &testStrm, outfile);

			// VI: Free the memory allocated by the EXI stream object
			tmp_err_code = serialize.closeEXIStream(&testStrm);

			fclose(outfile);

			if(schemaPtr != NULL)
				destroySchema(schemaPtr);
		}
	}
	else
	{
		printfHelp();
		return 1;
	}
	return 0;
}

size_t writeFileOutputStream(void* buf, size_t readSize, void* stream)
{
	FILE *outfile = (FILE*) stream;
	return fwrite(buf, 1, readSize, outfile);
}

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Copyright (c) 2010 - 2012, EISLAB - Luleå University of Technology Version 0.4 \n");
    printf("           Authors: Rumen Kyusakov\n");
    printf("  Usage:   exipe [options] <EXI_FileOut>\n\n");
    printf("           Options: [-help | -schema <schema_files_in>] \n");
    printf("           -schema :   uses schema defined in <schema_files_in> for encoding\n");
    printf("           -help   :   Prints this help message\n\n");
    printf("  Purpose: This program tests the EXIP encoding functionality\n");
    printf("\n" );
}

static void printError(errorCode err_code, EXIStream* strm, FILE *outfile)
{
	printf("\n Error occured: %d", err_code);
	serialize.closeEXIStream(strm);
	fclose(outfile);
	exit(1);
}
