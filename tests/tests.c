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
    uint8_t* out = NULL;
    size_t out_length = 0;

    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_compress(NULL, 256, 256, 8, &out, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_compress((uint8_t*)"TEST STRING TO COMPRESS", 0, 256, 8, &out, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_compress((uint8_t*)"TEST STRING TO COMPRESS", 23 + 1, 256, 8, NULL, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_compress((uint8_t*)"TEST STRING TO COMPRESS", 23 + 1, 256, 8, &out, NULL));
}

static void ccrush_compress_buffersize_too_large()
{
    uint8_t* out = NULL;
    size_t out_length = 0;

    TEST_CHECK(CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE == ccrush_compress((uint8_t*)"TEST STRING TO COMPRESS", 23 + 1, 1024 * 1024 * 1024, 8, &out, &out_length));
}

static void ccrush_decompress_invalid_args()
{
    uint8_t* out = NULL;
    size_t out_length = 0;

    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_decompress(NULL, 23 + 1, 256, &out, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_decompress((uint8_t*)"TEST DATA TO DECOMPRESS", 0, 256, &out, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_decompress((uint8_t*)"TEST DATA TO DECOMPRESS", 23 + 1, 256, NULL, &out_length));
    TEST_CHECK(CCRUSH_ERROR_INVALID_ARGS == ccrush_decompress((uint8_t*)"TEST DATA TO DECOMPRESS", 23 + 1, 256, &out, NULL));
}

static void ccrush_decompress_buffersize_too_large()
{
    uint8_t* out = NULL;
    size_t out_length = 0;

    TEST_CHECK(CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE == ccrush_decompress((uint8_t*)"TEST DATA TO DECOMPRESS", 23 + 1, 1024 * 1024 * 1024, &out, &out_length));
}

static void ccrush_compress_string_result_is_smaller_and_decompression_succeeds()
{
    const char* text = "The nuclear weapons disposal facility on Shadow Moses Island in Alaska's Fox Archepeligo was attacked and captured by Next Generation Special Forces being lead by members of FOX-HOUND.\n"
                       "They're demanding that the government turn over the remains of Big Boss, and they say that if their demands aren't met within 24 hours, they'll launch a nuclear weapon.\n"
                       "You'll have two mission objectives. First you're to rescue DARPA Chief Donald Anderson, and the President of Armstech, Kennith Baker. Both are being held as hostages.\n"
                       "Secondly, you're to investigate whether or not the terrorists have the ability to make a nuclear strike, and stop them if they do!";

    const size_t text_length = strlen(text);

    uint8_t* compressed_text = NULL;
    size_t compressed_text_length = 0;

    TEST_CHECK(ccrush_compress((uint8_t*)text, text_length, 256, 8, &compressed_text, &compressed_text_length) == 0);
    TEST_CHECK(compressed_text_length < text_length);

    char* decompressed_text = NULL;
    size_t decompressed_text_length = 0;

    TEST_CHECK(0 == ccrush_decompress(compressed_text, compressed_text_length, 256, (uint8_t**)(&decompressed_text), &decompressed_text_length));
    TEST_CHECK(0 == strncmp(text, decompressed_text, text_length));
    TEST_CHECK(text_length == decompressed_text_length);

    free(compressed_text);
    free(decompressed_text);
}

static void ccrush_compress_bytes_result_is_smaller_and_decompression_succeeds()
{
    const uint8_t* data = "The nuclear weapons disposal facility on Shadow Moses Island in Alaska's Fox Archepeligo was attacked and captured by Next Generation Special Forces being lead by members of FOX-HOUND.\n"
                          "They're demanding that the government turn over the remains of Big Boss, and they say that if their demands aren't met within 24 hours, they'll launch a nuclear weapon.\n"
                          "You'll have two mission objectives. First you're to rescue DARPA Chief Donald Anderson, and the President of Armstech, Kennith Baker. Both are being held as hostages.\n"
                          "Secondly, you're to investigate whether or not the terrorists have the ability to make a nuclear strike, and stop them if they do!";

    const size_t data_length = strlen(data);

    uint8_t* compressed_data = NULL;
    size_t compressed_data_length = 0;

    TEST_CHECK(ccrush_compress(data, data_length, 256, 8, &compressed_data, &compressed_data_length) == 0);
    TEST_CHECK(compressed_data_length < data_length);

    uint8_t* decompressed_data = NULL;
    size_t decompressed_data_length = 0;

    TEST_CHECK(0 == ccrush_decompress(compressed_data, compressed_data_length, 256, &decompressed_data, &decompressed_data_length));
    TEST_CHECK(0 == memcmp(data, decompressed_data, data_length));
    TEST_CHECK(data_length == decompressed_data_length);

    free(compressed_data);
    free(decompressed_data);
}

// --------------------------------------------------------------------------------------------------------------

TEST_LIST = {
    //
    { "nulltest", null_test_success }, //
    { "ccrush_compress_invalid_args", ccrush_compress_invalid_args }, //
    { "ccrush_compress_buffersize_too_large", ccrush_compress_buffersize_too_large }, //
    { "ccrush_decompress_invalid_args", ccrush_decompress_invalid_args }, //
    { "ccrush_decompress_buffersize_too_large", ccrush_decompress_buffersize_too_large }, //
    { "ccrush_compress_string_result_is_smaller_and_decompression_succeeds", ccrush_compress_string_result_is_smaller_and_decompression_succeeds }, //
    //
    // ----------------------------------------------------------------------------------------------------------
    //
    { NULL, NULL } //
};
