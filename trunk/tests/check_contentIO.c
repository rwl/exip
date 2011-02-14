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
 * @file check_ContentIO.c
 * @brief Tests the Content IO module
 *
 * @date Sep 28, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include <stdlib.h>
#include <check.h>
#include "headerDecode.h"
#include "bodyDecode.h"

/* BEGIN: header tests */

START_TEST (test_decodeHeader)
{
	EXIStream testStream;  // Default options, no EXI cookie
	struct EXIOptions options;
	char buf[3];
	errorCode err = UNEXPECTED_ERROR;
	EXIStream testStream2;  // Default options, with EXI cookie
	struct EXIOptions options2;
	char buf2[7];

	testStream.header.opts = &options;
	testStream.bitPointer = 0;
	buf[0] = (char) 0x80;
	buf[1] = (char) 0x60;
	buf[2] = (char) 0x7C;
	testStream.buffer = buf;
	testStream.bufferIndx = 0;
	testStream.bufLen = 3;

	err = decodeHeader(&testStream);
	fail_unless (err == ERR_OK, "decodeHeader returns error code %d", err);
	fail_if(testStream.header.opts == NULL);
	fail_unless (testStream.header.has_cookie == 0,
				"decodeHeader founds EXI cookie");
	fail_unless (testStream.header.has_options == 0,
					"decodeHeader founds options");
	fail_unless (testStream.header.is_preview_version == 0,
					"decodeHeader founds preview version");
	fail_unless (testStream.header.version_number == 1,
					"decodeHeader does not recognize version 1 of the stream");

	testStream2.header.opts = &options2;
	testStream2.bitPointer = 0;

	buf2[0] = (char) 36;
	buf2[1] = (char) 69;
	buf2[2] = (char) 88;
	buf2[3] = (char) 73;
	buf2[4] = (char) 0x80;
	buf2[5] = (char) 0x60;
	buf2[6] = (char) 0x7C;

	testStream2.buffer = buf2;
	testStream2.bufferIndx = 0;
	testStream2.bufLen = 7;

	err = decodeHeader(&testStream2);
	fail_unless (err == ERR_OK, "decodeHeader returns error code %d", err);
	fail_if(testStream2.header.opts == NULL);
	fail_unless (testStream2.header.has_cookie == 1,
				"decodeHeader does not found EXI cookie");
	fail_unless (testStream2.header.has_options == 0,
					"decodeHeader founds options");
	fail_unless (testStream2.header.is_preview_version == 0,
					"decodeHeader founds preview version");
	fail_unless (testStream2.header.version_number == 1,
					"decodeHeader does not recognize version 1 of the stream");
}
END_TEST

/* END: header tests */

Suite * contentio_suite (void)
{
  Suite *s = suite_create ("ContentIO");

  /* Header test case */
  TCase *tc_header = tcase_create ("EXI Header");
  tcase_add_test (tc_header, test_decodeHeader);
  suite_add_tcase (s, tc_header);
  return s;
}

int main (void)
{
	int number_failed;
	Suite *s = contentio_suite();
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
