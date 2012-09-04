/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file sep2.c
 * @brief Testing SEP2 encoding and decoding
 *
 * @date Apr 27, 2012
 * @author Rumen Kyusakov
 * @author Robert Cragie
 * @version 0.4
 * @par[Revision] $Id$
 */
#include "EXIParser.h"
#include "EXISerializer.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"
#include <stdio.h>
#include <string.h>

#define SEP2_SCHEMA sep2v51_schema

extern const EXIPSchema SEP2_SCHEMA;

#define BUFFER_SIZE 200
#define MAX_PREFIXES 10

static void printfHelp();
static void printError(errorCode err_code, EXIStream* strm, FILE *outfile);
static struct element* pop(struct element** stack);
static struct element* createElement(char* name);
static void destroyElement(struct element* el);
static size_t writeFileOutputStream(void* buf, size_t readSize, void* stream);
static size_t readFileInputStream(void* buf, size_t readSize, void* stream);

// Stuff needed for the OUT_XML Output Format
// ******************************************
struct element {
	struct element* next;
	char* name;
};

struct appData
{
	unsigned char expectAttributeData;
	char nameBuf[200];             // needed for the OUT_XML Output Format
	struct element* stack;         // needed for the OUT_XML Output Format
	unsigned char unclosedElement; 	 // needed for the OUT_XML Output Format
	char prefixes[MAX_PREFIXES][200]; // needed for the OUT_XML Output Format
	unsigned char prefixesCount; 	 // needed for the OUT_XML Output Format
};

static void push(struct element** stack, struct element* el);
// returns != 0 if error
static char lookupPrefix(struct appData* aData, String ns, unsigned char* prxHit, unsigned char* prefixIndex);

// Content Handler API
static char sep2_fatalError(const char code, const char* msg, void* app_data);
static char sep2_startDocument(void* app_data);
static char sep2_endDocument(void* app_data);
static char sep2_startElement(QName qname, void* app_data);
static char sep2_endElement(void* app_data);
static char sep2_attribute(QName qname, void* app_data);
static char sep2_stringData(const String value, void* app_data);
static char sep2_decimalData(Decimal value, void* app_data);
static char sep2_intData(Integer int_val, void* app_data);
static char sep2_binaryData(const char* binary_val, Index nbytes, void* app_data);
static char sep2_floatData(Float fl_val, void* app_data);

static errorCode sep2_encodeTime(FILE* outfile);
static errorCode sep2_encodeDcap(FILE* outfile);
static errorCode sep2_encodeCust(FILE* outfile);
static errorCode sep2_decode(FILE* infile);

static uint8_t sep2ExiHdr[] = {0xA0, 0x30, 0x11, 0x4C, 0xC2};
#define SEP2_EXI_HDR_BIT_LEN 39

/* Write a static pre-encoded EXI header to a stream */
static void encodeStaticHeader(EXIStream* strm, uint8_t* header, unsigned int bit_len);

int main(int argc, char *argv[])
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	FILE *infile;
	FILE *outfile;
	int i;
	char *fileNames[3] = {"time_msg.exi", "dcap_msg.exi", "cust_msg.exi"};
	int done[3] = {0,0,0};

	if(argc > 1)
	{
		printfHelp();
		return 0;
	}
	else
	{
		// Perform encoding of the three sample SEP2 messages
		for (i = 0; i < 3; i++)
		{
			outfile = fopen(fileNames[i], "wb" );
			if(!outfile)
			{
				fprintf(stderr, "Unable to open file %s", fileNames[i]);
				return 1;
			}
			else
			{
				switch(i)
				{
					case 0:
						// Encode <Time xmlns="http://zigbee.org/sep">
						tmp_err_code = sep2_encodeTime(outfile);
						done[i] = 1;
						break;
					case 1:
						// Encode <DeviceCapability xmlns="http://zigbee.org/sep" href="/dcap" subscribable="0">
						tmp_err_code = sep2_encodeDcap(outfile);
						done[i] = 1;
						break;
					case 2:
						// Encode <CustomerAccount xmlns="http://zigbee.org/sep" href="/bill/1">
						tmp_err_code = sep2_encodeCust(outfile);
						done[i] = 1;
						break;
				}
			}
			fclose(outfile);
		}

		// Perform decoding of the encoded SEP2 messages

		for (i = 0; i < 3; i++)
		{
			infile = fopen(fileNames[i], "rb" );
			if(!infile)
			{
				fprintf(stderr, "Unable to open file %s", fileNames[i]);
				return 1;
			}
			else
			{
				tmp_err_code = sep2_decode(infile);
			}
			fclose(infile);
		}
	}

	return 0;
}

static errorCode sep2_encodeTime(FILE* outfile)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIStream testStrm;
	String uri;
	String ln;
	String schemaID;
	QName qname= {&uri, &ln, NULL};
	char buf[BUFFER_SIZE];
	BinaryBuffer buffer;

	buffer.buf = buf;
	buffer.bufLen = BUFFER_SIZE;
	buffer.bufContent = 0;

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_cookie = FALSE;
	testStrm.header.has_options = TRUE;
	schemaID.str = "S0";
	schemaID.length = 2;

	// III: Define an external stream for the output if any
	buffer.ioStrm.readWriteToStream = writeFileOutputStream;
	buffer.ioStrm.stream = outfile;

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, (EXIPSchema*)&SEP2_SCHEMA, SCHEMA_ID_SET, &schemaID);
	if(tmp_err_code != ERR_OK)
		printError(tmp_err_code, &testStrm, outfile);

	// V: Start building the stream step by step: header, document, element etc...
	// Pass fixed header - it never changes
	encodeStaticHeader(&testStrm, sep2ExiHdr, SEP2_EXI_HDR_BIT_LEN);

	tmp_err_code += serialize.startDocument(&testStrm);

	/* Hierarchy (v51): */
	/* Time -> Resource */

	/* <Time xmlns="http://zigbee.org/sep"> */
	tmp_err_code += asciiToString("http://zigbee.org/sep", &uri, &testStrm.memList, FALSE);
	tmp_err_code += asciiToString("Time", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname);

	/* <currentTime>1351980000</currentTime> */
	tmp_err_code += asciiToString("currentTime", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname);
	tmp_err_code += serialize.intData(&testStrm, 1351980000);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <dstEndTime>1351994400</dstEndTime> */
	tmp_err_code += asciiToString("dstEndTime", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname);
	tmp_err_code += serialize.intData(&testStrm, 1351994400);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <dstOffset>3600</dstOffset> */
	tmp_err_code += asciiToString("dstOffset", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname);
	tmp_err_code += serialize.intData(&testStrm, 3600);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <dstStartTime>1331431200</dstStartTime> */
	tmp_err_code += asciiToString("dstStartTime", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname);
	tmp_err_code += serialize.intData(&testStrm, 1331431200);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <quality>4</quality> */
	tmp_err_code += asciiToString("quality", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname);
	tmp_err_code += serialize.intData(&testStrm, 4);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <tzOffset>0</tzOffset> */
	tmp_err_code += asciiToString("tzOffset", &ln, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, qname);
	tmp_err_code += serialize.intData(&testStrm, 0);
	tmp_err_code += serialize.endElement(&testStrm);

	/* End first element */
	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += serialize.endDocument(&testStrm);

	if(tmp_err_code != ERR_OK)
		printError(tmp_err_code, &testStrm, outfile);

	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);

	return tmp_err_code;
}

static errorCode sep2_encodeDcap(FILE* outfile)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIStream testStrm;
	String elemUri;
	String elemLn;
	String attrUri;
	String attrLn;
	String chVal;
	QName elemQname= {&elemUri, &elemLn, NULL};
	QName attrQname= {&attrUri, &attrLn, NULL};
	String schemaID;
	char buf[BUFFER_SIZE];
	BinaryBuffer buffer;

	buffer.buf = buf;
	buffer.bufLen = BUFFER_SIZE;
	buffer.bufContent = 0;

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_cookie = FALSE;
	testStrm.header.has_options = TRUE;
	schemaID.str = "S0";
	schemaID.length = 2;

	// III: Define an external stream for the output if any
	buffer.ioStrm.readWriteToStream = writeFileOutputStream;
	buffer.ioStrm.stream = outfile;

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, (EXIPSchema*)&SEP2_SCHEMA, SCHEMA_ID_SET, &schemaID);
	if(tmp_err_code != ERR_OK)
		printError(tmp_err_code, &testStrm, outfile);

	// V: Start building the stream step by step: header, document, element etc...
	// Pass fixed header - it never changes
	encodeStaticHeader(&testStrm, sep2ExiHdr, SEP2_EXI_HDR_BIT_LEN);

	tmp_err_code += serialize.startDocument(&testStrm);

	tmp_err_code += asciiToString("http://zigbee.org/sep", &elemUri, &testStrm.memList, FALSE);
	//tmp_err_code += asciiToString("http://zigbee.org/sep", &attrUri, &testStrm.memList, FALSE);
	getEmptyString(&attrUri);

	/* Hierarchy (v51): */
	/* DeviceCapability -> FunctionSetAssignmentsBase -> Resource */

	/* <DeviceCapability xmlns="http://zigbee.org/sep" href="/dcap" subscribable="0"> */
	tmp_err_code += asciiToString("DeviceCapability", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);

	/* <DemandResponseProgramListLink all="1" href="/sep2/dr"/> */
	tmp_err_code += asciiToString("DemandResponseProgramListLink", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	/* Attributes must be in lexicographical order */
	tmp_err_code += asciiToString("all", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_INTEGER);
	tmp_err_code += serialize.intData(&testStrm, 1); // TODO - need to get the real value
	tmp_err_code += asciiToString("href", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_STRING);
	tmp_err_code += asciiToString("/sep2/dr", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <MessagingProgramListLink all="1" href="/sep2/msg"/> */
	tmp_err_code += asciiToString("MessagingProgramListLink", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	/* Attributes must be in lexicographical order */
	tmp_err_code += asciiToString("all", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_INTEGER);
	tmp_err_code += serialize.intData(&testStrm, 1); // TODO - need to get the real value
	tmp_err_code += asciiToString("href", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_STRING);
	tmp_err_code += asciiToString("/sep2/msg", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <ResponseSetListLink all="0" href="/sep2/rsps"/> */
	tmp_err_code += asciiToString("ResponseSetListLink", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	/* Attributes must be in lexicographical order */
	tmp_err_code += asciiToString("all", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_INTEGER);
	tmp_err_code += serialize.intData(&testStrm, 0); // TODO - need to get the real value
	tmp_err_code += asciiToString("href", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_STRING);
	tmp_err_code += asciiToString("/sep2/rsps", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <TimeLink href="/sep2/tm"/> */
	tmp_err_code += asciiToString("TimeLink", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	tmp_err_code += asciiToString("href", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_STRING);
	tmp_err_code += asciiToString("/sep2/tm", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <UsagePointListLink all="1" href="/sep2/upt"/> */
	tmp_err_code += asciiToString("UsagePointListLink", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	/* Attributes must be in lexicographical order */
	tmp_err_code += asciiToString("all", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_INTEGER);
	tmp_err_code += serialize.intData(&testStrm, 1); // TODO - need to get the real value
	tmp_err_code += asciiToString("href", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_STRING);
	tmp_err_code += asciiToString("/sep2/upt", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* End first element */
	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += serialize.endDocument(&testStrm);

	if(tmp_err_code != ERR_OK)
		printError(tmp_err_code, &testStrm, outfile);

	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);

	return tmp_err_code;
}

static errorCode sep2_encodeCust(FILE* outfile)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIStream testStrm;
	String elemUri;
	String elemLn;
	String attrUri;
	String attrLn;
	String chVal;
	QName elemQname= {&elemUri, &elemLn, NULL};
	QName attrQname= {&attrUri, &attrLn, NULL};
	String schemaID;
	char buf[BUFFER_SIZE];
	BinaryBuffer buffer;

	buffer.buf = buf;
	buffer.bufLen = BUFFER_SIZE;
	buffer.bufContent = 0;

	// Serialization steps:

	// I: First initialize the header of the stream
	serialize.initHeader(&testStrm);

	// II: Set any options in the header, if different from the defaults
	testStrm.header.has_cookie = FALSE;
	testStrm.header.has_options = TRUE;
	schemaID.str = "S0";
	schemaID.length = 2;

	// III: Define an external stream for the output if any
	buffer.ioStrm.readWriteToStream = writeFileOutputStream;
	buffer.ioStrm.stream = outfile;

	// IV: Initialize the stream
	tmp_err_code = serialize.initStream(&testStrm, buffer, (EXIPSchema*)&SEP2_SCHEMA, SCHEMA_ID_SET, &schemaID);
	if(tmp_err_code != ERR_OK)
		printError(tmp_err_code, &testStrm, outfile);

	// V: Start building the stream step by step: header, document, element etc...
	// Pass fixed header - it never changes
	encodeStaticHeader(&testStrm, sep2ExiHdr, SEP2_EXI_HDR_BIT_LEN);

	/* Hierarchy (v51): */
	/* CustomerAccount -> IdentifiedObject -> Resource */

	tmp_err_code += serialize.startDocument(&testStrm);

	tmp_err_code += asciiToString("http://zigbee.org/sep", &elemUri, &testStrm.memList, FALSE);
	//tmp_err_code += asciiToString("http://zigbee.org/sep", &attrUri, &testStrm.memList, FALSE);
	getEmptyString(&attrUri);

	/* <CustomerAccount xmlns="http://zigbee.org/sep" href="/bill/1"> */
	tmp_err_code += asciiToString("CustomerAccount", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	tmp_err_code += asciiToString("href", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_STRING);
	tmp_err_code += asciiToString("/bill/1", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);

	/* <mRID>26D0C9722AB127FB01EDC378</mRID> */
	tmp_err_code += asciiToString("mRID", &elemLn, &testStrm.memList, FALSE); //HexBinary128
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	tmp_err_code += serialize.binaryData(&testStrm, "\x26\xD0\xC9\x72\x2A\xB1\x27\xFB\x01\xED\xC3\x78", 12);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <description>Description</description> */
	tmp_err_code += asciiToString("description", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	tmp_err_code += asciiToString("Description", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <currency>826</currency> */
	tmp_err_code += asciiToString("currency", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	tmp_err_code += serialize.intData(&testStrm, 826);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <CustomerAgreementListLink all="1" href="/bill/1/ca"/> */
	tmp_err_code += asciiToString("CustomerAgreementListLink", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	/* Attributes must be in lexicographical order */
	tmp_err_code += asciiToString("all", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_INTEGER);
	tmp_err_code += serialize.intData(&testStrm, 1); // TODO - need to get the real value
	tmp_err_code += asciiToString("href", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_STRING);
	tmp_err_code += asciiToString("/bill/1/ca", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <customerName>Joe Schmoe</customerName> */
	tmp_err_code += asciiToString("customerName", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	tmp_err_code += asciiToString("Joe Schmoe", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <pricePowerOfTenMultiplier>122</pricePowerOfTenMultiplier> */
	tmp_err_code += asciiToString("pricePowerOfTenMultiplier", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	tmp_err_code += serialize.intData(&testStrm, 122);
	tmp_err_code += serialize.endElement(&testStrm);

	/* <ServiceSupplierLink href="/ss"/> */
	tmp_err_code += asciiToString("ServiceSupplierLink", &elemLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.startElement(&testStrm, elemQname);
	/* Attributes must be in lexicographical order */
	tmp_err_code += asciiToString("href", &attrLn, &testStrm.memList, FALSE);
	tmp_err_code += serialize.attribute(&testStrm, attrQname, VALUE_TYPE_STRING);
	tmp_err_code += asciiToString("/ss", &chVal, &testStrm.memList, FALSE);
	tmp_err_code += serialize.stringData(&testStrm, chVal);
	tmp_err_code += serialize.endElement(&testStrm);

	/* End first element */
	tmp_err_code += serialize.endElement(&testStrm);

	tmp_err_code += serialize.endDocument(&testStrm);

	if(tmp_err_code != ERR_OK)
		printError(tmp_err_code, &testStrm, outfile);

	// VI: Free the memory allocated by the EXI stream object
	tmp_err_code = serialize.closeEXIStream(&testStrm);

	return tmp_err_code;
}

static errorCode sep2_decode(FILE* infile)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	char buf[BUFFER_SIZE];
	BinaryBuffer buffer;
	Parser testParser;
	struct appData parsingData;

	/* Now try and decode it */
	buffer.buf = buf;
	buffer.bufLen = BUFFER_SIZE;
	buffer.bufContent = 0;
	// Parsing steps:

	// I: First, define an external stream for the input to the parser if any
	buffer.ioStrm.readWriteToStream = readFileInputStream;
	buffer.ioStrm.stream = infile;

	// II: Second, initialize the parser object
	tmp_err_code = initParser(&testParser, buffer, (EXIPSchema*)&SEP2_SCHEMA, &parsingData);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// III: Initialize the parsing data and hook the callback handlers to the parser object

	parsingData.expectAttributeData = 0;
	parsingData.stack = NULL;
	parsingData.unclosedElement = 0;
	parsingData.prefixesCount = 0;

	testParser.handler.fatalError = sep2_fatalError;
	testParser.handler.error = sep2_fatalError;
	testParser.handler.startDocument = sep2_startDocument;
	testParser.handler.endDocument = sep2_endDocument;
	testParser.handler.startElement = sep2_startElement;
	testParser.handler.attribute = sep2_attribute;
	testParser.handler.stringData = sep2_stringData;
	testParser.handler.endElement = sep2_endElement;
	testParser.handler.decimalData = sep2_decimalData;
	testParser.handler.intData = sep2_intData;
	testParser.handler.binaryData = sep2_binaryData;
	testParser.handler.floatData = sep2_floatData;

	// IV: Parse the header of the stream

	tmp_err_code = parseHeader(&testParser);

	// V: Parse the body of the EXI stream

	while(tmp_err_code == ERR_OK)
	{
		tmp_err_code = parseNext(&testParser);
	}

	// VI: Free the memory allocated by the parser object

	destroyParser(&testParser);

	if(tmp_err_code == PARSING_COMPLETE)
		return ERR_OK;
	else
	{
		printf("\nError during parsing of the EXI stream: %d", tmp_err_code);
		return 1;
	}
}

static void printfHelp()
{
    printf("\n" );
    printf("  EXIP     Copyright (c) 2010 - 2012, EISLAB - Luleå University of Technology Version 0.4 \n");
    printf("           Authors: Robert Cragie, Rumen Kyusakov\n");
    printf("  Usage:   exipsep2 [options] \n\n");
    printf("           Options: [-help] \n");
    printf("           -help   :   Prints this help message\n\n");
    printf("  Purpose: This program tests the EXIP SEP2 encoding functionality\n");
    printf("\n" );
}

static void printError(errorCode err_code, EXIStream* strm, FILE *outfile)
{
	printf("\n Error occured: %d", err_code);
	serialize.closeEXIStream(strm);
	fclose(outfile);
	exit(1);
}

static char sep2_fatalError(const char code, const char* msg, void* app_data)
{
	printf("\n%d : FATAL ERROR: %s\n", code, msg);
	return EXIP_HANDLER_STOP;
}

static char sep2_startDocument(void* app_data)
{
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	return EXIP_HANDLER_OK;
}

static char sep2_endDocument(void* app_data)
{
	printf("\n");

	return EXIP_HANDLER_OK;
}

static char sep2_startElement(QName qname, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	{
		char error = 0;
		unsigned char prefixIndex = 0;
		unsigned char prxHit = 1;
		int t;

		if(!isStringEmpty(qname.uri))
		{
			error = lookupPrefix(appD, *qname.uri, &prxHit, &prefixIndex);
			if(error != 0)
				return EXIP_HANDLER_STOP;

			sprintf(appD->nameBuf, "p%d:", prefixIndex);
			t = strlen(appD->nameBuf);
			memcpy(appD->nameBuf + t, qname.localName->str, qname.localName->length);
			appD->nameBuf[t + qname.localName->length] = '\0';
		}
		else
		{
			memcpy(appD->nameBuf, qname.localName->str, qname.localName->length);
			appD->nameBuf[qname.localName->length] = '\0';
		}
		push(&(appD->stack), createElement(appD->nameBuf));
		if(appD->unclosedElement)
			printf(">\n");
		printf("<%s", appD->nameBuf);

		if(prxHit == 0)
		{
			sprintf(appD->nameBuf, " xmlns:p%d=\"", prefixIndex);
			printf("%s", appD->nameBuf);
			printString(qname.uri);
		}

		appD->unclosedElement = 1;
	}

	return EXIP_HANDLER_OK;
}

static char sep2_endElement(void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	struct element* el;

	if(appD->unclosedElement)
		printf(">\n");
	appD->unclosedElement = 0;
	el = pop(&(appD->stack));

	printf("</%s>\n", el->name);
	destroyElement(el);

	return EXIP_HANDLER_OK;
}

static char sep2_attribute(QName qname, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	printf(" ");
	if(!isStringEmpty(qname.uri))
	{
		printString(qname.uri);
		printf(":");
	}
	printString(qname.localName);
	printf("=\"");
	appD->expectAttributeData = 1;

	return EXIP_HANDLER_OK;
}

static char sep2_stringData(const String value, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->expectAttributeData)
	{
		printString(&value);
		printf("\"\n");
		appD->expectAttributeData = 0;
	}
	else
	{
		if(appD->unclosedElement)
			printf(">\n");
		appD->unclosedElement = 0;
		printString(&value);
		printf("\n");
	}

	return EXIP_HANDLER_OK;
}

static char sep2_decimalData(Decimal value, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->expectAttributeData)
	{
		printf("%.1f \"\n", (double) value);
		appD->expectAttributeData = 0;
	}
	else
	{
		if(appD->unclosedElement)
			printf(">\n");
		appD->unclosedElement = 0;
		printf("%.1f \n", (double) value);
	}

	return EXIP_HANDLER_OK;
}

static char sep2_intData(Integer int_val, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	if(appD->expectAttributeData)
	{
		printf("%ld\"\n", (long) int_val);
		appD->expectAttributeData = 0;
	}
	else
	{
		if(appD->unclosedElement)
			printf(">\n");
		appD->unclosedElement = 0;
		printf("%ld\n", (long) int_val);
	}

	return EXIP_HANDLER_OK;
}

static char sep2_binaryData(const char* binary_val, Index nbytes, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	Index byteIdx;

	if(appD->expectAttributeData)
	{
		for(byteIdx = 0; byteIdx < nbytes; byteIdx++)
		{
			printf("0x%02x ", (uint8_t)binary_val[byteIdx]);
		}
		printf("\"\n");
		appD->expectAttributeData = 0;
	}
	else
	{
		if(appD->unclosedElement)
			printf(">\n");
		appD->unclosedElement = 0;
		for(byteIdx = 0; byteIdx < nbytes; byteIdx++)
		{
			printf("0x%02x ", (uint8_t)binary_val[byteIdx]);
		}
		printf("\n");
	}
	return EXIP_HANDLER_OK;
}

static char sep2_floatData(Float fl_val, void* app_data)
{
	struct appData* appD = (struct appData*) app_data;
	char tmp_buf[30];
	if(appD->expectAttributeData)
	{
		sprintf(tmp_buf, "%lldE%d", (long long int) fl_val.mantissa, fl_val.exponent);
		printf("%s", tmp_buf);
		printf("\"\n");
		appD->expectAttributeData = 0;
	}
	else
	{
		if(appD->unclosedElement)
			printf(">\n");
		appD->unclosedElement = 0;
		sprintf(tmp_buf, "%lldE%d", (long long int) fl_val.mantissa, fl_val.exponent);
		printf("%s", tmp_buf);
		printf("\n");
	}

	return EXIP_HANDLER_OK;
}

// Stuff needed for the OUT_XML Output Format
// ******************************************
static void push(struct element** stack, struct element* el)
{
	if(*stack == NULL)
		*stack = el;
	else
	{
		el->next = *stack;
		*stack = el;
	}
}

static struct element* pop(struct element** stack)
{
	if(*stack == NULL)
		return NULL;
	else
	{
		struct element* result;
		result = *stack;
		*stack = (*stack)->next;
		return result;
	}
}

static struct element* createElement(char* name)
{
	struct element* el;
	el = malloc(sizeof(struct element));
	if(el == NULL)
		exit(1);
	el->next = NULL;
	el->name = malloc(strlen(name)+1);
	if(el->name == NULL)
		exit(1);
	strcpy(el->name, name);
	return el;
}

static void destroyElement(struct element* el)
{
	free(el->name);
	free(el);
}
// ******************************************

static size_t writeFileOutputStream(void* buf, size_t readSize, void* stream)
{
	FILE *outfile = (FILE*) stream;
	return fwrite(buf, 1, readSize, outfile);
}

static size_t readFileInputStream(void* buf, size_t readSize, void* stream)
{
	FILE *infile = (FILE*) stream;
	return fread(buf, 1, readSize, infile);
}

static char lookupPrefix(struct appData* aData, String ns, unsigned char* prxHit, unsigned char* prefixIndex)
{
	int i;
	for(i = 0; i < aData->prefixesCount; i++)
	{
		if(stringEqualToAscii(ns, aData->prefixes[i]))
		{
			*prefixIndex = i;
			*prxHit = 1;
			return 0;
		}
	}

	if(aData->prefixesCount == MAX_PREFIXES)
		return 1;
	else
	{
		memcpy(aData->prefixes[aData->prefixesCount], ns.str, ns.length);
		aData->prefixes[aData->prefixesCount][ns.length] = '\0';
		*prefixIndex = aData->prefixesCount;
		aData->prefixesCount += 1;
		*prxHit = 0;
		return 0;
	}

}

static void encodeStaticHeader(EXIStream* strm, uint8_t* header, unsigned int bit_len)
{
	memcpy(strm->buffer.buf, header, bit_len / 8 + (bit_len % 8 != 0));
	strm->context.bitPointer = bit_len % 8;
	strm->context.bufferIndx = bit_len / 8;
}
