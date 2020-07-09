/*

BSD 2-Clause License

Copyright (c) 2020, Raphael Beck
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include <ccrush.h>
#include <acutest.h>

/* A test case that does nothing and succeeds. */
static void null_test_success()
{
    TEST_CHECK(1);
}

static void ccrush_compress_invalid_args()
{
    uint8_t* out;
    size_t out_length;

    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_compress(NULL, 256, 256, 8, &out, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_compress((uint8_t*)"TEST STRING TO COMPRESS", 0, 256, 8, &out, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_compress((uint8_t*)"TEST STRING TO COMPRESS", 23 + 1, 256, 8, NULL, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_compress((uint8_t*)"TEST STRING TO COMPRESS", 23 + 1, 256, 8, &out, NULL));
}

static void ccrush_compress_buffersize_too_large()
{
    uint8_t* out;
    size_t out_length;

    TEST_CHECK(CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE == ccrush_compress((uint8_t*)"TEST STRING TO COMPRESS", 23 + 1, 1024 * 1024 * 1024, 8, &out, &out_length));
}

static void ccrush_decompress_invalid_args()
{
    uint8_t* out;
    size_t out_length;

    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_decompress(NULL, 23 + 1, 256, &out, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_decompress((uint8_t*)"TEST DATA TO DECOMPRESS", 0, 256, &out, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_decompress((uint8_t*)"TEST DATA TO DECOMPRESS", 23 + 1, 256, NULL, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_decompress((uint8_t*)"TEST DATA TO DECOMPRESS", 23 + 1, 256, &out, NULL));
}

static void ccrush_decompress_buffersize_too_large()
{
    uint8_t* out;
    size_t out_length;

    TEST_CHECK(CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE == ccrush_decompress((uint8_t*)"TEST DATA TO DECOMPRESS", 23 + 1, 1024 * 1024 * 1024, &out, &out_length));
}

// --------------------------------------------------------------------------------------------------------------

TEST_LIST = {
    //
    { "nulltest", null_test_success }, //
    { "ccrush_compress_invalid_args", ccrush_compress_invalid_args }, //
    { "ccrush_compress_buffersize_too_large", ccrush_compress_buffersize_too_large }, //
    { "ccrush_decompress_invalid_args", ccrush_decompress_invalid_args }, //
    { "ccrush_decompress_buffersize_too_large", ccrush_decompress_buffersize_too_large }, //
    //
    // ----------------------------------------------------------------------------------------------------------
    //
    { NULL, NULL } //
};
