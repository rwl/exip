#include "EXISerializer.h"
#include "stringManipulate.h"
#include "grammarGenerator.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define OUTPUT_BUFFER_SIZE 2000000
#define MAX_XSD_FILES_COUNT 10 // up to 10 XSD files

const String NS_EMPTY_STR = {NULL, 0};

const String ELEM_CONFIGURATION = {"configuration", 13};
const String ELEM_CAPSWITCH = {"capable-switch", 14};
const String ELEM_RESOURCES = {"resources", 9};
const String ELEM_PORT = {"port", 4};
const String ELEM_RESID = {"resource-id", 11};
const String ELEM_ADMIN_STATE = {"admin-state", 11};
const String ELEM_NORECEIVE = {"no-receive", 10};
const String ELEM_NOFORWARD = {"no-forward", 10};
const String ELEM_NOPACKET = {"no-packet-in", 12};

const String ELEM_LOGSWITCHES = {"logical-switches", 16};
const String ELEM_SWITCH = {"switch", 6};
const String ELEM_ID = {"id", 2};
const String ELEM_DATAPATHID = {"datapath-id", 11};
const String ELEM_ENABLED = {"enabled", 7};
const String ELEM_LOSTCONNBEH = {"lost-connection-behavior", 24};
const String ELEM_CONTROLLERS = {"controllers", 11};
const String ELEM_CONTROLLER = {"controller", 10};
const String ELEM_ROLE = {"role", 4};
const String ELEM_IPADDR = {"ip-address", 10};
const String ELEM_PROTOCOL = {"protocol", 8};
const String ELEM_STATE = {"state", 5};
const String ELEM_CONNSTATE = {"connection-state", 16};
const String ELEM_CURRVER = {"current-version", 18};

const char * PORT_STR = "port";
const char * SWITCH_STR = "switch";
const char * STATE_UP_STR = "up";
const char * DATAPATH_STR = "10:14:56:7C:89:46:7A:";
const char * LOST_CONN_BEHAVIOR_STR = "failSecureMode";
const char * CTRL_STR = "ctrl";
const char * ROLE_STR = "equal";
const char * IPADDR_STR = "10.10.10.";
const char * PROTOCOL_STR = "tcp";
const char * VER_STR = "1.0";

#define SEC2NANO 1000000000

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
        int k;
	struct timespec start;
	struct timespec end;
	long long total = 0;

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

			tmp_err_code = generateSchemaInformedGrammars(buffer, schemaFilesCount, SCHEMA_FORMAT_XSD_EXI, NULL, &schema);
			if(tmp_err_code != ERR_OK)
			{
				printf("\n Grammar Error occured: %d", tmp_err_code);
				return 1;
			}

			schemaPtr = &schema;
			for(i = 0; i < schemaFilesCount; i++)
			{
				free(buffer[i].buf);
			}

		}

		strcpy(sourceFile, argv[argc - 1]);

                for (k = 0; k < 1; k++) {

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
                        int i, j;
                        char strbuffer[32];

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
			testStrm.header.opts.valuePartitionCapacity = INDEX_MAX;
			SET_STRICT(testStrm.header.opts.enumOpt);

			// III: Define an external stream for the output if any
			buffer.ioStrm.readWriteToStream = writeFileOutputStream;
			buffer.ioStrm.stream = outfile;
			//buffer.ioStrm.readWriteToStream = NULL;
			//buffer.ioStrm.stream = NULL;
printf("line:%d: %d\n", __LINE__, tmp_err_code);
//                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
			// IV: Initialize the stream
			tmp_err_code = serialize.initStream(&testStrm, buffer, schemaPtr, SCHEMA_ID_ABSENT, NULL);
			if(tmp_err_code != ERR_OK)
				printError(tmp_err_code, &testStrm, outfile);
printf("line:%d: %d\n", __LINE__, tmp_err_code);
                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

			// V: Start building the stream step by step: header, document, element etc...
			tmp_err_code += serialize.exiHeader(&testStrm);
printf("line:%d: %d\n", __LINE__, tmp_err_code);
			tmp_err_code += serialize.startDocument(&testStrm);
printf("line:%d: %d\n", __LINE__, tmp_err_code);
			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ELEM_CONFIGURATION;
			EXITypeClass typeClass;
			tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ELEM_CAPSWITCH;
			tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ELEM_RESOURCES;
			tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

printf("line:%d: %d\n", __LINE__, tmp_err_code);
                        for (i = 0; i < 100; i++) {
			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_PORT;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_RESID;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

                            sprintf(strbuffer, "%s%d", PORT_STR, i);
			    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
                            tmp_err_code += serialize.endElement(&testStrm);


			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_CONFIGURATION;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_ADMIN_STATE;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString(STATE_UP_STR, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_NORECEIVE;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString("false", &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_NOFORWARD;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString("false", &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_NOPACKET;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString("true", &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);


	                    tmp_err_code += serialize.endElement(&testStrm);

	                    tmp_err_code += serialize.endElement(&testStrm);
                        }

                        tmp_err_code += serialize.endElement(&testStrm);


			qname.uri = &NS_EMPTY_STR;
			qname.localName = &ELEM_LOGSWITCHES;
			tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);


                        for (i = 0; i < 20; i++) {
			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_SWITCH;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_ID;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

                            sprintf(strbuffer, "%s%d", SWITCH_STR, i);
			    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
                            tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_DATAPATHID;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

                            sprintf(strbuffer, "%s%d", DATAPATH_STR, 10 + i);
			    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
                            tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_ENABLED;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString("true", &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_LOSTCONNBEH;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString(LOST_CONN_BEHAVIOR_STR, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);
                            if ( i == 0 ) {
			        qname.uri = &NS_EMPTY_STR;
			        qname.localName = &ELEM_RESOURCES;
			        tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
                           
                                for (j = 0; j < 100; j++) { 
			            qname.uri = &NS_EMPTY_STR;
			            qname.localName = &ELEM_PORT;
			            tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

                                    sprintf(strbuffer, "%s%d", PORT_STR, j);
			            tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
			            tmp_err_code += serialize.stringData(&testStrm, chVal);
                                    tmp_err_code += serialize.endElement(&testStrm);
                                }

                                tmp_err_code += serialize.endElement(&testStrm);
                            }

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_CONTROLLERS;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_CONTROLLER;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_ID;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
                            sprintf(strbuffer, "%s%d", CTRL_STR, i);
			    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
                            tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_ROLE;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString(ROLE_STR, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_IPADDR;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
                            sprintf(strbuffer, "%s%d", IPADDR_STR, i);
			    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_PORT;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
                            sprintf(strbuffer, "%d", 6620 + i);
			    tmp_err_code += asciiToString(strbuffer, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_PROTOCOL;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString(PROTOCOL_STR, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_STATE;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);

			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_CONNSTATE;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
			    tmp_err_code += asciiToString(STATE_UP_STR, &chVal, &testStrm.memList, FALSE);
			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);

printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
			    qname.uri = &NS_EMPTY_STR;
			    qname.localName = &ELEM_CURRVER;
			    tmp_err_code += serialize.startElement(&testStrm, qname, &typeClass);
printf("in loop(%d) line:%d: %d\n", i, __LINE__, tmp_err_code);
			    tmp_err_code += asciiToString(VER_STR, &chVal, &testStrm.memList, FALSE);

			    tmp_err_code += serialize.stringData(&testStrm, chVal);
	                    tmp_err_code += serialize.endElement(&testStrm);


                            tmp_err_code += serialize.endElement(&testStrm);

                            tmp_err_code += serialize.endElement(&testStrm);

                            tmp_err_code += serialize.endElement(&testStrm);
          
                            tmp_err_code += serialize.endElement(&testStrm);
                        }

                        tmp_err_code += serialize.endElement(&testStrm);

			tmp_err_code += serialize.endElement(&testStrm);

			tmp_err_code += serialize.endElement(&testStrm);
			tmp_err_code += serialize.endDocument(&testStrm);

                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

			if(tmp_err_code != ERR_OK)
				printError(tmp_err_code, &testStrm, outfile);

			// VI: Free the memory allocated by the EXI stream object
			tmp_err_code = serialize.closeEXIStream(&testStrm);

//                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
                        total += ((end.tv_sec * SEC2NANO) + end.tv_nsec) - ((start.tv_sec * SEC2NANO) + start.tv_nsec);

                        fclose(outfile);

			if(schemaPtr != NULL)
				destroySchema(schemaPtr);
		}
                }
		total = total / 10000;
		printf("Elapsed time of single run(ms): %f\n", (total / (double)1000000));
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
    printf("  EXIP encoding Demo\n");
    printf("  Usage:   demoe [options] <EXI_FileOut>\n\n");
    printf("           Options :   [-help | -schema <schema_files_in>]\n");
    printf("           -schema :   uses schema defined in <schema_files_in> for encoding\n");
    printf("           -help   :   Prints this help message\n\n");
    printf("  Purpose: This program tests the EXIP encoding functionality\n");
    printf("\n" );
}

static void printError(errorCode err_code, EXIStream* strm, FILE *outfile)
{
	printf("\n This Error occured: %d", err_code);
	serialize.closeEXIStream(strm);
	fclose(outfile);
	exit(1);
}

