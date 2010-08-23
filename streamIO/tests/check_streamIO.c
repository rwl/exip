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
 * @version 0.1
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <check.h>
#include "../include/streamRead.h"
#include "procTypes.h"
#include "errorHandle.h"

START_TEST (test_readNextBit)
{
  EXIStream testStream;
  testStream.bitPointer = 0;

  char buf[2];
  buf[0] = (char) 0b11010100;
  buf[1] = (char) 0b01100000;
  testStream.buffer = buf;

  testStream.bufferIndx = 0;
  unsigned char bit_val = 0;
  errorCode err = UNEXPECTED_ERROR;

  err = readNextBit(&testStream, &bit_val);

  fail_unless (bit_val == 1,
	       "The bit 1 from the stream is read as 0");
  fail_unless (err == ERR_OK,
	       "readNextBit returns error code %d", err);
  fail_unless (testStream.bitPointer == 1,
    	       "The readNextBit function did not move the bit Pointer of the stream correctly");

  // Set the bit pointer to the first byte boundary
  testStream.bitPointer = 7;

  err = readNextBit(&testStream, &bit_val);

  fail_unless (bit_val == 0,
  	       "The bit 0 from the stream is read as 1");
  fail_unless (err == ERR_OK,
  	       "readNextBit returns error code %d", err);
  fail_unless (testStream.bitPointer == 0 && testStream.bufferIndx == 1,
   	       "The readNextBit function did not move the bit Pointer of the stream correctly");

}
END_TEST

START_TEST (test_readBits)
{
  EXIStream testStream;
  testStream.bitPointer = 0;

  char buf[2];
  buf[0] = (char) 0b11010100;
  buf[1] = (char) 0b01100000;
  testStream.buffer = buf;

  testStream.bufferIndx = 0;
  unsigned int bits_val = 0;
  errorCode err = UNEXPECTED_ERROR;

  err = readBits(&testStream, 4, &bits_val);

  fail_unless (bits_val == 13,
	       "The bits 1101 from the stream are read as %d", bits_val);
  fail_unless (err == ERR_OK,
	       "readBits returns error code %d", err);
  fail_unless (testStream.bitPointer == 4,
  	       "The readBits function did not move the bit Pointer of the stream correctly");

  // Set the bit pointer to the first byte boundary
  testStream.bitPointer = 7;

  err = readBits(&testStream, 5, &bits_val);

  fail_unless (bits_val == 6,
		      "The bits 00110 from the stream are read as %d", bits_val);
  fail_unless (err == ERR_OK,
    	       "readNextBit returns error code %d", err);
  fail_unless (testStream.bitPointer == 4 && testStream.bufferIndx == 1,
     	       "The readBits function did not move the bit Pointer of the stream correctly");

}
END_TEST


Suite * streamIO_suite (void)
{
  Suite *s = suite_create ("StreamIO");

  /* StreamRead test case */
  TCase *tc_sRead = tcase_create ("StreamRead");
  tcase_add_test (tc_sRead, test_readNextBit);
  tcase_add_test (tc_sRead, test_readBits);
  suite_add_tcase (s, tc_sRead);

  /* StreamDecode test case */
  TCase *tc_sDecode = tcase_create ("StreamDecode");
//  tcase_add_test (tc_sDecode, test_);
  suite_add_tcase (s, tc_sDecode);

  return s;
}

int main (void)
{
	int number_failed;
	Suite *s = streamIO_suite();
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
