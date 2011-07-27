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
 * @file check_streamIO.c
 * @brief Tests the interface to the EXI stream reader/decoder
 *
 * @date Aug 18, 2010
 * @author Rumen Kyusakov
 * @author Ashok Gowtham
 * @version 0.1
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <check.h>
#include "streamRead.h"
#include "streamWrite.h"
#include "streamDecode.h"
#include "streamEncode.h"
#include "procTypes.h"
#include "errorHandle.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "ioUtil.h"

/* BEGIN: streamRead tests */

START_TEST (test_readNextBit)
{
  EXIStream testStream;

  char buf[2];
  unsigned char bit_val = 0;
  errorCode err = UNEXPECTED_ERROR;

  testStream.context.bitPointer = 0;
  buf[0] = (char) 0xD4; /* 0b11010100 */
  buf[1] = (char) 0x60; /* 0b01100000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.bufContent = 2;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = readNextBit(&testStream, &bit_val);

  fail_unless (bit_val == 1,
	       "The bit 1 from the stream is read as 0");
  fail_unless (err == ERR_OK,
	       "readNextBit returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 1,
    	       "The readNextBit function did not move the bit Pointer of the stream correctly");

  // Set the bit pointer to the first byte boundary
  testStream.context.bitPointer = 7;

  err = readNextBit(&testStream, &bit_val);

  fail_unless (bit_val == 0,
  	       "The bit 0 from the stream is read as 1");
  fail_unless (err == ERR_OK,
  	       "readNextBit returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 0 && testStream.context.bufferIndx == 1,
   	       "The readNextBit function did not move the bit Pointer of the stream correctly");

  // Set the bit pointer to the second byte boundary
  testStream.context.bitPointer = 7;

  err = readNextBit(&testStream, &bit_val);

  fail_unless (err == ERR_OK,
  	       "readNextBit returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 0 && testStream.context.bufferIndx == 2,
   	       "The readNextBit function did not move the bit Pointer of the stream correctly");

  err = readNextBit(&testStream, &bit_val);
  fail_unless (err == BUFFER_END_REACHED, "Incorrect error code");

}
END_TEST

START_TEST (test_readBits)
{
  EXIStream testStream;
  char buf[2];
  unsigned int bits_val = 0;
  errorCode err = UNEXPECTED_ERROR;

  testStream.context.bitPointer = 0;
  buf[0] = (char) 0xD4; /* 0b11010100 */
  buf[1] = (char) 0x60; /* 0b01100000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.bufContent = 2;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = readBits(&testStream, 4, &bits_val);

  fail_unless (bits_val == 13,
	       "The bits 1101 from the stream are read as %d", bits_val);
  fail_unless (err == ERR_OK,
	       "readBits returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 4,
  	       "The readBits function did not move the bit Pointer of the stream correctly");

  // Set the bit pointer to the first byte boundary
  testStream.context.bitPointer = 7;

  err = readBits(&testStream, 5, &bits_val);

  fail_unless (bits_val == 6,
		      "The bits 00110 from the stream are read as %d", bits_val);
  fail_unless (err == ERR_OK,
    	       "readNextBit returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 4 && testStream.context.bufferIndx == 1,
     	       "The readBits function did not move the bit Pointer of the stream correctly");

  err = readBits(&testStream, 5, &bits_val);
  fail_unless (err == BUFFER_END_REACHED, "Incorrect error code");
}
END_TEST

/* END: streamRead tests */

/* BEGIN: streamWrite tests */

START_TEST (test_writeNextBit)
{
  EXIStream testStream;
  char buf[2];
  errorCode err = UNEXPECTED_ERROR;
  int test;

  testStream.context.bitPointer = 0;
  buf[0] = (char) 0x01; /* 0b00000001 */
  buf[1] = (char) 0x00; /* 0b00000000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.bufContent = 2;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = writeNextBit(&testStream, 1);

  test = (buf[0] & 0x80 /* 0b10000000 */ ) != 0;

  fail_unless (test == 1,
	       "The bit 1 was written as 0");
  fail_unless (err == ERR_OK,
	       "writeNextBit returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 1,
    	       "The writeNextBit function did not move the bit Pointer of the stream correctly");

  // Set the bit pointer to the first byte boundary
  testStream.context.bitPointer = 7;

  err = writeNextBit(&testStream, 0);

  test = (buf[0] & 0x01) != 0;

  fail_unless (test == 0,
  	       "The bit 0 was written as 1");
  fail_unless (err == ERR_OK,
  	       "writeNextBit returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 0 && testStream.context.bufferIndx == 1,
   	       "The writeNextBit function did not move the bit Pointer of the stream correctly");

  testStream.context.bufferIndx = 2;
  testStream.context.bitPointer = 0;

  err = writeNextBit(&testStream, 0);
  fail_unless (err == BUFFER_END_REACHED, "Incorrect error code");

}
END_TEST

START_TEST (test_writeBits)
{
  EXIStream testStream;
  char buf[2];
  errorCode err = UNEXPECTED_ERROR;
  int test, test1;

  testStream.context.bitPointer = 0;
  buf[0] = (char) 0x00; /* 0b00000000 */
  buf[1] = (char) 0xE0;	/* 0b11100000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.bufContent = 2;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = writeBits(&testStream, 19);

  test = (buf[0] & 0xF8 /* 0b11111000 */ ) >> 3;

  fail_unless (test == 19,
	       "The number 19 was written as %d", test);
  fail_unless (err == ERR_OK,
	       "writeBits returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 5,
  	       "The writeBits function did not move the bit Pointer of the stream correctly");

  // Set the bit pointer to the first byte boundary
  testStream.context.bitPointer = 7;

  err = writeBits(&testStream, 9);

  test = (buf[0] & 0x01) != 0;

  test1 = (buf[1] & 0xE0 /* 0b11100000 */ ) >> 5;

  fail_unless (test == 1 && test1 == 1,
		      "writeBits function doesn't write correctly");
  fail_unless (err == ERR_OK,
    	       "writeBits returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 3 && testStream.context.bufferIndx == 1,
     	       "The writeBits function did not move the bit Pointer of the stream correctly");

  err = writeBits(&testStream, 32);
  fail_unless (err == BUFFER_END_REACHED, "Incorrect error code");
}
END_TEST

START_TEST (test_writeNBits)
{
  EXIStream testStream;
  char buf[2];
  errorCode err = UNEXPECTED_ERROR;
  int test, test1;

  testStream.context.bitPointer = 0;
  buf[0] = (char) 0xA0; /* 0b10100000 */
  buf[1] = (char) 0xE0; /* 0b11100000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.ioStrm = NULL;
  testStream.bufContent = 2;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = writeNBits(&testStream, 7, 19);

  test = ((unsigned int) buf[0]) >> 1;

  fail_unless (test == 19,
	       "The number 19 was written as %d", test);
  fail_unless (err == ERR_OK,
	       "writeNBits returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 7,
  	       "The writeNBits function did not move the bit Pointer of the stream correctly");

  // Set the bit pointer to the first byte boundary
  testStream.context.bitPointer = 7;

  err = writeNBits(&testStream, 5, 9);

  test = (buf[0] & 0x01 ) != 0;

  test1 = (buf[1] & 0xF0 /* 0b11110000 */ ) >> 4;

  fail_unless (test == 0 && test1 == 9,
		      "writeNBits function doesn't write correctly");
  fail_unless (err == ERR_OK,
    	       "writeNBits returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 4 && testStream.context.bufferIndx == 1,
     	       "The writeNBits function did not move the bit Pointer of the stream correctly");

  err = writeNBits(&testStream, 5, 16);
  fail_unless (err == BUFFER_END_REACHED, "Incorrect error code");
}
END_TEST

/* END: streamWrite tests */

/* BEGIN: streamDecode tests */

START_TEST (test_decodeNBitUnsignedInteger)
{
  EXIStream testStream;
  EXIOptions options;

  char buf[2];
  unsigned int bit_val = 0;
  errorCode err = UNEXPECTED_ERROR;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0xD4; /* 0b11010100 */
  buf[1] = (char) 0x60; /* 0b01100000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.bufContent = 2;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = decodeNBitUnsignedInteger(&testStream, 6, &bit_val);

  fail_unless (bit_val == 53,
	       "The 110101 from the stream is read as %d", bit_val);
  fail_unless (err == ERR_OK,
	       "decodeNBitUnsignedInteger returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 6,
    	       "The decodeNBitUnsignedInteger function did not move the bit Pointer of the stream correctly");

  // TODO: write more extensive tests: in particular handle the situation of non-bit-packed streams

}
END_TEST

START_TEST (test_decodeBoolean)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[2];
  unsigned char bit_val = 0;
  errorCode err = UNEXPECTED_ERROR;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0xD4; /* 0b11010100 */
  buf[1] = (char) 0x60; /* 0b01100000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.bufContent = 2;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = decodeBoolean(&testStream, &bit_val);

  fail_unless (bit_val == 1,
	       "The the bit 1 from the stream is read as %d", bit_val);
  fail_unless (err == ERR_OK,
	       "decodeBoolean returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 1,
    	       "The decodeBoolean function did not move the bit Pointer of the stream correctly");
}
END_TEST

START_TEST (test_decodeUnsignedInteger)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[3];
  unsigned int bit_val = 0;
  errorCode err = UNEXPECTED_ERROR;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0xD4; /* 0b11010100 */
  buf[1] = (char) 0x60; /* 0b01100000 */
  buf[2] = (char) 0x48; /* 0b01001000 */
  testStream.buffer = buf;
  testStream.bufLen = 3;
  testStream.bufContent = 3;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = decodeUnsignedInteger(&testStream, &bit_val);

  fail_unless (bit_val == 12372,
	       "The UnsignedInteger 12372 from the stream is read as %d", bit_val);
  fail_unless (err == ERR_OK,
	       "decodeUnsignedInteger returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 0,
    	       "The decodeUnsignedInteger function did not move the bit Pointer of the stream correctly");
  fail_unless (testStream.context.bufferIndx == 2,
      	       "The decodeUnsignedInteger function did not move the byte Pointer of the stream correctly");

  // TODO: write more extensive tests

}
END_TEST


START_TEST (test_decodeString)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[4];
  StringType bit_val;
  CharType cht[100];
  errorCode err = UNEXPECTED_ERROR;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0x02; /* 0b00000010 */
  buf[1] = (char) 0x65; /* 0b01100101 */ // e - ASCII
  buf[2] = (char) 0x54; /* 0b01010100 */ // T - ASCII
  buf[3] = (char) 0x52; /* 0b01010010 */
  testStream.buffer = buf;
  testStream.bufLen = 4;
  testStream.bufContent = 4;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);
  bit_val.length = 0;
  bit_val.str = cht;

  err = decodeString(&testStream, &bit_val);

  fail_unless (bit_val.length == 2,
	       "The String length of 2 is reported as %d from decodeString", bit_val.length);
  fail_unless (bit_val.str[0] == 'e' && bit_val.str[1] == 'T',
  	       "The String \"eT\" is decoded wrong by decodeString");
  fail_unless (err == ERR_OK,
	       "decodeString returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 0,
    	       "The decodeString function did not move the bit Pointer of the stream correctly");
  fail_unless (testStream.context.bufferIndx == 3,
      	       "The decodeString function did not move the byte Pointer of the stream correctly");

  // TODO: write more extensive tests

}
END_TEST

START_TEST (test_decodeBinary)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[20];
  char testbuf[20];
  int i;
  char* res;
  size_t bytes = 0;
  errorCode err = UNEXPECTED_ERROR;
  int same=1;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char)  0x05; /* 0b00000101 */		//5
  buf[1] = (char)  0xF0; /* 0b11110000 */
  buf[2] = (char)  0xCC; /* 0b11001100 */
  buf[3] = (char)  0xAA; /* 0b10101010 */
  buf[4] = (char)  0x55; /* 0b01010101 */
  buf[5] = (char)  0x33; /* 0b00110011 */
  buf[6] = (char)  0x08; /* 0b00001000 */		//8
  buf[7] = (char)  0x6E; /* 0b01101110 */
  buf[8] = (char)  0xCA; /* 0b11001010 */
  buf[9] = (char)  0x59; /* 0b01011001 */
  buf[10] = (char) 0xD8; /* 0b11011000 */
  buf[11] = (char) 0x59; /* 0b01011001 */
  buf[12] = (char) 0xCA; /* 0b11001010 */
  buf[13] = (char) 0x6C; /* 0b01101100 */
  buf[14] = (char) 0xD8; /* 0b11011000 */
  buf[15] = (char) 0x07; /* 0b00000111 */
  testStream.buffer = buf;
  testStream.bufLen = 20;
  testStream.bufContent = 20;
  testStream.ioStrm = NULL;
  initAllocList(&testStream.memList);
  for(i=0;i<20;i++) testbuf[i]=buf[i];

  testStream.context.bufferIndx = 0;

  //Test1:
  err = decodeBinary(&testStream, &res, &bytes);

  for(i=0;i<5;i++)
  {
    if(res[i]!=testbuf[i+1])
	{
	  same=0;
	  break;
	}
  }
  
  fail_unless (err == ERR_OK,
	       "decodeBinary returns error code %d", err);
  fail_unless (bytes == 5,
	       "The length of the binary content is read as %d (actual : %d)", bytes,5);
  fail_unless (same == 1,
	       "The binary content is read wrongly");
  fail_unless (testStream.context.bitPointer == 0,
    	       "The decodeBinary function did not move the bit Pointer of the stream correctly");
  fail_unless (testStream.context.bufferIndx == 6,
      	       "The decodeBinary function did not move the byte Pointer of the stream correctly");

//Test2:
  bytes=0;
  err = UNEXPECTED_ERROR;

  err = decodeBinary(&testStream, &res, &bytes);

  same = 1;
  for(i=0;i<8;i++)
  {
    if(res[i]!=testbuf[i+7])
	{
	  same=0;
	  break;
	}
  }
  
  fail_unless (err == ERR_OK,
	       "decodeBinary returns error code %d", err);
  fail_unless (bytes == 8,
	       "The length of the binary content is read as %d (actual : %d)", bytes,8);
  fail_unless (same == 1,
	       "The binary content is read wrongly");
  fail_unless (testStream.context.bitPointer == 0,
    	       "The decodeBinary function did not move the bit Pointer of the stream correctly");
  fail_unless (testStream.context.bufferIndx == 15,
      	       "The decodeBinary function did not move the byte Pointer of the stream correctly");

  // TODO: write more extensive tests

}
END_TEST

START_TEST (test_decodeFloat)
{
	EXIStream testStream;
	EXIOptions options;
	char buf[3];
	double dbl_val = 0;
	double res = 500;		// 5 x 10^2
	errorCode err = UNEXPECTED_ERROR;

	makeDefaultOpts(&options);
	testStream.header.opts = &options;

	buf[0] = (char) 0x02; /* 0b00000010 */
	buf[1] = (char) 0x80; /* 0b10000000 */
	buf[2] = (char) 0x92; /* 0b10010010 */
	testStream.buffer = buf;
	testStream.bufLen = 3;
	testStream.bufContent = 3;
	testStream.ioStrm = NULL;
	testStream.context.bufferIndx = 0;
	testStream.context.bitPointer = 0;
	initAllocList(&testStream.memList);

	err = decodeFloatValue(&testStream, &dbl_val);

	fail_unless (err == ERR_OK,
		   "decodeFloat returns error code %d", err);
	fail_unless (dbl_val == res,
		   "The float value is read as %f (actual : %f)", dbl_val, res);
	fail_unless (testStream.context.bitPointer == 2,
			   "The decodeBinary function did not move the bit Pointer of the stream correctly");
	fail_unless (testStream.context.bufferIndx == 2,
			   "The decodeBinary function did not move the byte Pointer of the stream correctly");

  // TODO: write more extensive tests

}
END_TEST

START_TEST (test_decodeIntegerValue)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[3];
  int bit_val = 0;
  errorCode err = UNEXPECTED_ERROR;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0x94; /* 0b10010100 */
  buf[1] = (char) 0x60; /* 0b01100000 */
  buf[2] = (char) 0x48; /* 0b01001000 */
  testStream.buffer = buf;
  testStream.bufLen = 3;
  testStream.bufContent = 3;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = decodeIntegerValue(&testStream, &bit_val);

  fail_unless (bit_val == -40,
	       "The IntegerValue -40 from the stream is read as %d", bit_val);
  fail_unless (err == ERR_OK,
	       "decodeIntegerValue returns error code %d", err);
  fail_unless (testStream.context.bitPointer == 1,
    	       "The decodeIntegerValue function did not move the bit Pointer of the stream correctly");
  fail_unless (testStream.context.bufferIndx == 1,
      	       "The decodeIntegerValue function did not move the byte Pointer of the stream correctly");

  // TODO: write more extensive tests

}
END_TEST

START_TEST (test_decodeDecimalValue)
{
	EXIStream testStream;
	EXIOptions options;
	char buf[3];
	errorCode err = UNEXPECTED_ERROR;
	decimal dec_val = 0;
	decimal res	= 5.001dd;

	makeDefaultOpts(&options);
	testStream.header.opts = &options;

	buf[0] = (char) 0x02; /* 0b00000010 */
	buf[1] = (char) 0xB2; /* 0b10110010 */
	buf[2] = (char) 0x12; /* 0b00010010 */
	testStream.buffer = buf;
	testStream.bufLen = 3;
	testStream.bufContent = 3;
	testStream.ioStrm = NULL;
	testStream.context.bufferIndx = 0;
	testStream.context.bitPointer = 0;
	initAllocList(&testStream.memList);

	err = decodeDecimalValue(&testStream, &dec_val);

	fail_unless (res == dec_val, "The value 5.001 is decoded as %.3f", (double) dec_val);
	fail_unless (err == ERR_OK,
		   "decodeDecimalValue returns error code %d", err);
	fail_unless (testStream.context.bitPointer == 1,
			   "The decodeIntegerValue function did not move the bit Pointer of the stream correctly");
	fail_unless (testStream.context.bufferIndx == 2,
			   "The decodeIntegerValue function did not move the byte Pointer of the stream correctly");

}
END_TEST

/* END: streamDecode tests */

/* BEGIN: streamEncode tests */

START_TEST (test_encodeNBitUnsignedInteger)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[2];
  errorCode err = UNEXPECTED_ERROR;
  unsigned char test, test2;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0xCE; /* 0b11001110 */
  buf[1] = (char) 0xE0; /* 0b11100000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.bufContent = 2;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = encodeNBitUnsignedInteger(&testStream, 9, 412);

  test = buf[0] | 0;
  test2 = (unsigned char) buf[1] >> 7;

  fail_unless (err == ERR_OK,
  	       "encodeNBitUnsignedInteger returns error code %d", err);
  fail_unless (test == 206 && test2 == 0,
	       "encodeNBitUnsignedInteger does not encode correctly");
  fail_unless (testStream.context.bitPointer == 1 && testStream.context.bufferIndx == 1,
    	       "The encodeNBitUnsignedInteger function did not move the bit Pointer of the stream correctly");

  // TODO: write more extensive tests: in particular handle the situation of non-bit-packed streams

}
END_TEST

START_TEST (test_encodeBoolean)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[2];
  errorCode err = UNEXPECTED_ERROR;
  unsigned char bit_val = 0;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0x54; /* 0b01010100 */
  buf[1] = (char) 0x60; /* 0b01100000 */
  testStream.buffer = buf;
  testStream.bufLen = 2;
  testStream.bufContent = 2;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = encodeBoolean(&testStream, 1);

  bit_val = (unsigned char) buf[0] >> 7;

  fail_unless (err == ERR_OK,
	       "encodeBoolean returns error code %d", err);
  fail_unless (bit_val == 1,
	       "encodeBoolean does not write correctly");
  fail_unless (testStream.context.bitPointer == 1,
    	       "The encodeBoolean function did not move the bit Pointer of the stream correctly");

  err = encodeBoolean(&testStream, 0);

  bit_val = (unsigned char) buf[0] >> 6;

  fail_unless (err == ERR_OK,
	   "encodeBoolean returns error code %d", err);
  fail_unless (bit_val == 2,
	   "encodeBoolean does not write correctly");
  fail_unless (testStream.context.bitPointer == 2,
		   "The encodeBoolean function did not move the bit Pointer of the stream correctly");
}
END_TEST

START_TEST (test_encodeUnsignedInteger)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[3];
  errorCode err = UNEXPECTED_ERROR;
  unsigned char test1, test2;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0xD4; /* 0b11010100 */
  buf[1] = (char) 0x00;	/* 0b00000000 */
  buf[2] = (char) 0x00;	/* 0b00000000 */
  testStream.buffer = buf;
  testStream.bufLen = 3;
  testStream.bufContent = 3;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  initAllocList(&testStream.memList);

  err = encodeUnsignedInteger(&testStream, 421);

  test1 = (unsigned char) buf[0];
  test2 = (unsigned char) buf[1];

  fail_unless (err == ERR_OK,
		   "encodeUnsignedInteger returns error code %d", err);
  fail_unless (test1 == 165 && test2 == 3,
	       "The encodeUnsignedInteger function doesn't work correctly");

  fail_unless (testStream.context.bitPointer == 0,
    	       "The encodeUnsignedInteger function did not move the bit Pointer of the stream correctly");
  fail_unless (testStream.context.bufferIndx == 2,
      	       "The encodeUnsignedInteger function did not move the byte Pointer of the stream correctly");

  buf[0] = (char) 0x10;	/* 0b00010000 */
  buf[1] = (char) 0x00;	/* 0b00000000 */
  buf[2] = (char) 0x00;	/* 0b00000000 */ 
  testStream.context.bufferIndx = 0;
  testStream.context.bitPointer = 0;
  err = UNEXPECTED_ERROR;

  err = encodeUnsignedInteger(&testStream, 0);

  test1 = (unsigned char) buf[0];
  test2 = (unsigned char) buf[1];

  fail_unless (err == ERR_OK,
     "encodeUnsignedInteger returns error code %d", err);
  fail_unless (test1 == 0 && test2 == 0,
     "The encodeUnsignedInteger function doesn't work correctly");
  fail_unless (testStream.context.bitPointer == 0,
     "The encodeUnsignedInteger function did not move the bit Pointer of the stream correctly");
  fail_unless (testStream.context.bufferIndx == 1,
     "The encodeUnsignedInteger function did not move the byte Pointer of the stream correctly");
}
END_TEST


START_TEST (test_encodeString)
{
  EXIStream testStream;
  EXIOptions options;
  char buf[50];
  StringType testStr;
  errorCode err = UNEXPECTED_ERROR;
  unsigned char str_len;

  testStream.context.bitPointer = 0;
  makeDefaultOpts(&options);
  testStream.header.opts = &options;

  buf[0] = (char) 0x02; /* 0b00000010 */
  buf[1] = (char) 0x65; /* 0b01100101 */
  buf[2] = (char) 0x64; /* 0b01010100 */
  buf[3] = (char) 0x62; /* 0b01010010 */
  initAllocList(&testStream.memList);
  testStream.buffer = buf;
  testStream.bufLen = 50;
  testStream.bufContent = 50;
  testStream.ioStrm = NULL;
  testStream.context.bufferIndx = 0;
  asciiToString("TEST encodeString()", &testStr, &testStream.memList, FALSE);

  err = encodeString(&testStream, &testStr);

  str_len = buf[0];

  fail_unless (err == ERR_OK,
	       "encodeString returns error code %d", err);
  fail_unless (str_len == 19,
	       "The String length is not encoded correctly");
  fail_unless (buf[1] == 'T' && buf[2] == 'E',
  	       "encodeString doesn't encode correctly");
  fail_unless (testStream.context.bitPointer == 0,
    	       "The encodeString function did not move the bit Pointer of the stream correctly");
  fail_unless (testStream.context.bufferIndx == 20,
      	       "The encodeString function did not move the byte Pointer of the stream correctly");

  // TODO: write more extensive tests

}
END_TEST

START_TEST (test_encodeBinary)
{
	EXIStream testStream;
	EXIOptions options;
	char buf[50];
	char bin_data[50];
	errorCode err = UNEXPECTED_ERROR;

	makeDefaultOpts(&options);
	testStream.header.opts = &options;

	bin_data[0] = (char) 0x22; /* 0b00100010 */
	bin_data[1] = (char) 0x65; /* 0b01100101 */
	bin_data[2] = (char) 0xD4; /* 0b11010100 */
	bin_data[3] = (char) 0x5A; /* 0b01011010 */
	bin_data[4] = (char) 0xD7; /* 0b11010111 */
	initAllocList(&testStream.memList);
	testStream.buffer = buf;
	testStream.bufLen = 50;
	testStream.bufContent = 50;
	testStream.ioStrm = NULL;
	testStream.context.bufferIndx = 0;
	testStream.context.bitPointer = 0;

	err = encodeBinary(&testStream, bin_data, 5);

	fail_unless (err == ERR_OK,
		   "encodeBinary returns error code %d", err);
	fail_unless (testStream.context.bitPointer == 0,
			   "The encodeBinary function did not move the bit Pointer of the stream correctly");
	fail_unless (testStream.context.bufferIndx == 6,
			   "The encodeBinary function did not move the byte Pointer of the stream correctly");

	fail_unless(testStream.buffer[0] == 5, "Incorrect encoding during encodeBinary 1");
	fail_unless(testStream.buffer[1] == (signed char) 0x22, "Incorrect encoding during encodeBinary 2");
	fail_unless(testStream.buffer[2] == (signed char) 0x65, "Incorrect encoding during encodeBinary 3");
	fail_unless(testStream.buffer[3] == (signed char) 0xD4, "Incorrect encoding during encodeBinary 4");
	fail_unless(testStream.buffer[4] == (signed char) 0x5A, "Incorrect encoding during encodeBinary 5");
	fail_unless(testStream.buffer[5] == (signed char) 0xD7, "Incorrect encoding during encodeBinary 6");
}
END_TEST

START_TEST (test_encodeFloatValue)
{
	EXIStream testStream;
	EXIOptions options;
	char buf[3] = {0, 0, 0};
	double test_val = 500;		// 5 x 10^2
	errorCode err = UNEXPECTED_ERROR;

	makeDefaultOpts(&options);
	testStream.header.opts = &options;

	testStream.buffer = buf;
	testStream.bufLen = 3;
	testStream.bufContent = 3;
	testStream.ioStrm = NULL;
	testStream.context.bufferIndx = 0;
	testStream.context.bitPointer = 0;
	initAllocList(&testStream.memList);

	err = encodeFloatValue(&testStream, test_val);

	fail_unless (err == ERR_OK,
		   "encodeFloatValue returns error code %d", err);

	fail_unless(testStream.buffer[0] == (char) 0x02 && 	testStream.buffer[1] == (char) 0x80, "Incorect encoding of float value");

	fail_unless (testStream.context.bitPointer == 2,
			   "The decodeBinary function did not move the bit Pointer of the stream correctly");
	fail_unless (testStream.context.bufferIndx == 2,
			   "The decodeBinary function did not move the byte Pointer of the stream correctly");
}
END_TEST

START_TEST (test_encodeIntegerValue)
{
	fail("Not implemented yet");
}
END_TEST

START_TEST (test_encodeDecimalValue)
{
	fail("Not implemented yet");
}
END_TEST

/* END: streamEncode tests */


/* START: ioUtil tests */

START_TEST (test_moveBitPointer)
{
	EXIStream strm;

	strm.context.bitPointer = 3;
	strm.context.bufferIndx = 0;
	moveBitPointer(&strm, 13);
	fail_unless(strm.context.bitPointer == 0 && strm.context.bufferIndx == 2, "incorrect moving of the BitPointer");

	strm.context.bitPointer = 7;
	strm.context.bufferIndx = 0;
	moveBitPointer(&strm, 1);
	fail_unless(strm.context.bitPointer == 0 && strm.context.bufferIndx == 1, "incorrect moving of the BitPointer");

	strm.context.bitPointer = 3;
	strm.context.bufferIndx = 0;
	moveBitPointer(&strm, 12);
	fail_unless(strm.context.bitPointer == 7 && strm.context.bufferIndx == 1, "incorrect moving of the BitPointer");
}
END_TEST

START_TEST (test_getBitsNumber)
{
	fail_unless(getBitsNumber(99) == 7);
	fail_unless(getBitsNumber(63) == 6);
	fail_unless(getBitsNumber(64) == 7);
	fail_unless(getBitsNumber(4095) == 12);
	fail_unless(getBitsNumber(824) == 10);
	fail_unless(getBitsNumber(16383) == 14);
	fail_unless(getBitsNumber(7234) == 13);
}
END_TEST

START_TEST (test_log2INT)
{
	fail_unless(log2INT(99) == 6);
	fail_unless(log2INT(63) == 5);
	fail_unless(log2INT(64) == 6);
	fail_unless(log2INT(4095) == 11);
	fail_unless(log2INT(824) == 9);
	fail_unless(log2INT(16383) == 13);
	fail_unless(log2INT(7234) == 12);
}
END_TEST

/* END: ioUtil tests */



Suite * streamIO_suite (void)
{
  Suite *s = suite_create ("StreamIO");

  {
	  /* StreamRead test case */
	  TCase *tc_sRead = tcase_create ("StreamRead");
	  tcase_add_test (tc_sRead, test_readNextBit);
	  tcase_add_test (tc_sRead, test_readBits);
	  suite_add_tcase (s, tc_sRead);
  }

  {
	  /* StreamWrite test case */
	  TCase *tc_sWrite = tcase_create ("StreamWrite");
	  tcase_add_test (tc_sWrite, test_writeNextBit);
	  tcase_add_test (tc_sWrite, test_writeBits);
	  tcase_add_test (tc_sWrite, test_writeNBits);
	  suite_add_tcase (s, tc_sWrite);
  }

  {
	  /* StreamDecode test case */
	  TCase *tc_sDecode = tcase_create ("StreamDecode");
	  tcase_add_test (tc_sDecode, test_decodeNBitUnsignedInteger);
	  tcase_add_test (tc_sDecode, test_decodeBoolean);
	  tcase_add_test (tc_sDecode, test_decodeUnsignedInteger);
	  tcase_add_test (tc_sDecode, test_decodeString);
	  tcase_add_test (tc_sDecode, test_decodeBinary);
	  tcase_add_test (tc_sDecode, test_decodeFloat);
	  tcase_add_test (tc_sDecode, test_decodeIntegerValue);
	  tcase_add_test (tc_sDecode, test_decodeDecimalValue);
	  suite_add_tcase (s, tc_sDecode);
  }

  {
	  /* StreamEncode test case */
	  TCase *tc_sEncode = tcase_create ("StreamEncode");
	  tcase_add_test (tc_sEncode, test_encodeNBitUnsignedInteger);
	  tcase_add_test (tc_sEncode, test_encodeBoolean);
	  tcase_add_test (tc_sEncode, test_encodeUnsignedInteger);
	  tcase_add_test (tc_sEncode, test_encodeString);
	  tcase_add_test (tc_sEncode, test_encodeBinary);
	  tcase_add_test (tc_sEncode, test_encodeFloatValue);
	  tcase_add_test (tc_sEncode, test_encodeIntegerValue);
	  tcase_add_test (tc_sEncode, test_encodeDecimalValue);
	  suite_add_tcase (s, tc_sEncode);
  }

  {
	  /* ioUtil test case */
	  TCase *tc_ioUtil = tcase_create ("ioUtil");
	  tcase_add_test (tc_ioUtil, test_moveBitPointer);
	  tcase_add_test (tc_ioUtil, test_getBitsNumber);
	  tcase_add_test (tc_ioUtil, test_log2INT);
	  suite_add_tcase (s, tc_ioUtil);
  }

  return s;
}

int main (void)
{
	int number_failed;
	Suite *s = streamIO_suite();
	SRunner *sr = srunner_create (s);
#ifdef _MSC_VER
	srunner_set_fork_status(sr, CK_NOFORK);
#endif
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
