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

#include <setjmp.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

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
    const char* text = "The nuclear weapons disposal facility on Shadow Moses Island in Alaska's Fox Archipelago was attacked and captured by Next Generation Special Forces being lead by members of FOX-HOUND.\n"
                       "They're demanding that the government turn over the remains of Big Boss, and they say that if their demands aren't met within 24 hours, they'll launch a nuclear weapon.\n"
                       "You'll have two mission objectives. First: you're to rescue DARPA Chief Donald Anderson, and the President of Armstech, Kenneth Baker. Both are being held as hostages.\n"
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
    const uint8_t data[] = {
        0xe9, 0x4c, 0x03, 0xef, 0x29, 0x98, 0xf7, 0x35, 0x1d, 0xbd, 0xeb, 0xff, 0xda, 0xf7, 0x20, 0xc7, //
        0x26, 0xfc, 0xaf, 0x4e, 0xa2, 0x51, 0x92, 0xc3, 0xea, 0x16, 0xe9, 0x9d, 0xd1, 0x4a, 0xdc, 0x2e, //
        0x8a, 0xef, 0x64, 0x29, 0xb7, 0x4c, 0xa1, 0x25, 0xe8, 0x13, 0x4e, 0xbc, 0x41, 0xe7, 0x77, 0x47, //
        0xe9, 0xdb, 0x39, 0xe6, 0x74, 0xc0, 0x94, 0xfa, 0x9c, 0x6e, 0x2c, 0xad, 0x3a, 0xaf, 0x97, 0xe8, //
        0x0c, 0xbc, 0xc1, 0x3e, 0x64, 0x42, 0xa3, 0x64, 0x58, 0x80, 0xef, 0x42, 0xfe, 0x4a, 0xba, 0xd0, //
        0xa7, 0x37, 0x8f, 0x3d, 0x4f, 0x29, 0x82, 0xeb, 0xf2, 0x02, 0x4e, 0x9f, 0x3f, 0x1a, 0x31, 0x80, //
        0x72, 0x95, 0xf3, 0xf0, 0x87, 0x35, 0xb7, 0x64, 0x99, 0x04, 0x00, 0x18, 0x3c, 0x36, 0xca, 0xc4, //
        0x09, 0xc3, 0x0f, 0x43, 0x64, 0xb6, 0x87, 0x2a, 0x2c, 0x11, 0x1a, 0x02, 0x07, 0x1c, 0x77, 0xfc, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x64, 0x42, 0xd5, 0x95, 0xc4, 0x32, 0x58, 0x79, 0xd2, 0x64, 0x11, 0xb0, 0xc2, 0x92, 0x43, 0x59, //
        0x64, 0x38, 0x76, 0x00, 0x4d, 0x17, 0x43, 0x39, 0x57, 0xea, 0xf1, 0xcc, 0x7f, 0x3c, 0x2a, 0xc8, //
        0x07, 0x15, 0x1c, 0xcc, 0x7d, 0xd6, 0x67, 0x5d, 0x59, 0xd9, 0x61, 0x3e, 0xff, 0x64, 0xb9, 0xe8, //
        0x96, 0x17, 0xd3, 0xf7, 0x45, 0x64, 0xa6, 0x64, 0xff, 0x64, 0x88, 0x4c, 0xb8, 0x70, 0x9b, 0xf6, //
        0x9d, 0xb7, 0x1f, 0x8e, 0xcc, 0xeb, 0xf7, 0x95, 0x0c, 0x64, 0x8b, 0x8b, 0x71, 0x6a, 0xaf, 0x26, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x64, 0x42, 0xd5, 0x95, 0xc4, 0x32, 0x58, 0x79, 0xd2, 0x64, 0x11, 0xb0, 0xc2, 0x92, 0x43, 0x59, //
        0x64, 0x38, 0x76, 0x00, 0x4d, 0x17, 0x43, 0x39, 0x57, 0xea, 0xf1, 0xcc, 0x7f, 0x3c, 0x2a, 0xc8, //
        0x07, 0x15, 0x1c, 0xcc, 0x7d, 0xd6, 0x67, 0x5d, 0x59, 0xd9, 0x61, 0x3e, 0xff, 0x64, 0xb9, 0xe8, //
        0x96, 0x17, 0xd3, 0xf7, 0x45, 0x64, 0xa6, 0x64, 0xff, 0x64, 0x88, 0x4c, 0xb8, 0x70, 0x9b, 0xf6, //
        0x9d, 0xb7, 0x1f, 0x8e, 0xcc, 0xeb, 0xf7, 0x95, 0x0c, 0x64, 0x8b, 0x8b, 0x71, 0x6a, 0xaf, 0x26, //
        0x83, 0xa3, 0x80, 0x0f, 0xa8, 0x64, 0xac, 0x1a, 0x10, 0x77, 0x6a, 0x4d, 0x35, 0x21, 0x8d, 0x4f, //
        0x74, 0x38, 0x9f, 0x57, 0xff, 0xf8, 0xe9, 0xe1, 0xd9, 0xf4, 0xb5, 0x11, 0x99, 0xeb, 0xd3, 0xae, //
        0x50, 0xc6, 0xe2, 0xe0, 0xae, 0x3b, 0x23, 0xb9, 0x64, 0xa9, 0x8e, 0x13, 0x2d, 0x85, 0xdb, 0xc4, //
        0x3c, 0x61, 0xbb, 0xb7, 0x88, 0xff, 0xdd, 0x3e, 0xdd, 0x01, 0x8d, 0x03, 0xfa, 0x7a, 0xb8, 0x2e, //
        0x13, 0x5a, 0xd4, 0x0a, 0x64, 0x77, 0x34, 0x64, 0x44, 0x31, 0xb4, 0x92, 0xe0, 0xa2, 0xe1, 0x43, //
        0xc1, 0xc3, 0xb0, 0x02, 0x2b, 0xc3, 0x9d, 0xe3, 0x9f, 0xd7, 0x4d, 0x61, 0x30, 0x05, 0xbb, 0x49, //
        0xba, 0x52, 0x77, 0xdd, 0x07, 0xe5, 0x1f, 0xdc, 0x43, 0x03, 0xe4, 0x82, 0x73, 0x9a, 0x51, 0x23, //
        0x6f, 0x33, 0x4a, 0x7c, 0xfc, 0xcf, 0x90, 0xc3, 0x64, 0xea, 0x78, 0xc7, 0xf5, 0x62, 0x81, 0x31, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x63, 0xa2, 0x85, 0x43, 0x54, 0xf7, 0xe7, 0x26, 0x03, 0x64, 0x6d, 0x33, 0x19, 0x84, 0x04, 0xe2, //
        0x64, 0x5b, 0x50, 0xd6, 0xe1, 0xad, 0xaf, 0x83, 0x26, 0x6a, 0x77, 0x72, 0x5e, 0xec, 0x1f, 0x6a, //
        0x54, 0xd2, 0x4f, 0x15, 0xd9, 0x6e, 0x95, 0xd5, 0x57, 0xc8, 0x85, 0xba, 0xd2, 0x64, 0xd4, 0xed, //
        0x75, 0xbe, 0x9e, 0x70, 0x06, 0xe0, 0x06, 0x91, 0xad, 0x1d, 0x1c, 0x23, 0x7c, 0x64, 0xf8, 0xa9, //
        0xa5, 0x4d, 0xfc, 0x9f, 0xf8, 0xeb, 0xe8, 0x27, 0xdd, 0x70, 0x64, 0x17, 0x70, 0x1a, 0x07, 0x4f, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x64, 0x42, 0xd5, 0x95, 0xc4, 0x32, 0x58, 0x79, 0xd2, 0x64, 0x11, 0xb0, 0xc2, 0x92, 0x43, 0x59, //
        0x64, 0x38, 0x76, 0x00, 0x4d, 0x17, 0x43, 0x39, 0x57, 0xea, 0xf1, 0xcc, 0x7f, 0x3c, 0x2a, 0xc8, //
        0x07, 0x15, 0x1c, 0xcc, 0x7d, 0xd6, 0x67, 0x5d, 0x59, 0xd9, 0x61, 0x3e, 0xff, 0x64, 0xb9, 0xe8, //
        0x96, 0x17, 0xd3, 0xf7, 0x45, 0x64, 0xa6, 0x64, 0xff, 0x64, 0x88, 0x4c, 0xb8, 0x70, 0x9b, 0xf6, //
        0x9d, 0xb7, 0x1f, 0x8e, 0xcc, 0xeb, 0xf7, 0x95, 0x0c, 0x64, 0x8b, 0x8b, 0x71, 0x6a, 0xaf, 0x26, //
        0x78, 0x1c, 0x79, 0xd9, 0xa7, 0xa7, 0x54, 0x2a, 0xcd, 0x64, 0x72, 0xa3, 0x12, 0xac, 0xed, 0x10, //
        0x63, 0xa2, 0x85, 0x43, 0x54, 0xf7, 0xe7, 0x26, 0x03, 0x64, 0x6d, 0x33, 0x19, 0x84, 0x04, 0xe2, //
        0x64, 0x5b, 0x50, 0xd6, 0xe1, 0xad, 0xaf, 0x83, 0x26, 0x6a, 0x77, 0x72, 0x5e, 0xec, 0x1f, 0x6a, //
        0x54, 0xd2, 0x4f, 0x15, 0xd9, 0x6e, 0x95, 0xd5, 0x57, 0xc8, 0x85, 0xba, 0xd2, 0x64, 0xd4, 0xed, //
        0x75, 0xbe, 0x9e, 0x70, 0x06, 0xe0, 0x06, 0x91, 0xad, 0x1d, 0x1c, 0x23, 0x7c, 0x64, 0xf8, 0xa9, //
        0xa5, 0x4d, 0xfc, 0x9f, 0xf8, 0xeb, 0xe8, 0x27, 0xdd, 0x70, 0x64, 0x17, 0x70, 0x1a, 0x07, 0x4f, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x64, 0x42, 0xd5, 0x95, 0xc4, 0x32, 0x58, 0x79, 0xd2, 0x64, 0x11, 0xb0, 0xc2, 0x92, 0x43, 0x59, //
        0x64, 0x38, 0x76, 0x00, 0x4d, 0x17, 0x43, 0x39, 0x57, 0xea, 0xf1, 0xcc, 0x7f, 0x3c, 0x2a, 0xc8, //
        0x07, 0x15, 0x1c, 0xcc, 0x7d, 0xd6, 0x67, 0x5d, 0x59, 0xd9, 0x61, 0x3e, 0xff, 0x64, 0xb9, 0xe8, //
        0x96, 0x17, 0xd3, 0xf7, 0x45, 0x64, 0xa6, 0x64, 0xff, 0x64, 0x88, 0x4c, 0xb8, 0x70, 0x9b, 0xf6, //
        0x9d, 0xb7, 0x1f, 0x8e, 0xcc, 0xeb, 0xf7, 0x95, 0x0c, 0x64, 0x8b, 0x8b, 0x71, 0x6a, 0xaf, 0x26, //
        0x78, 0x1c, 0x79, 0xd9, 0xa7, 0xa7, 0x54, 0x2a, 0xcd, 0x64, 0x72, 0xa3, 0x12, 0xac, 0xed, 0x10, //
        0x63, 0xa2, 0x85, 0x43, 0x54, 0xf7, 0xe7, 0x26, 0x03, 0x64, 0x6d, 0x33, 0x19, 0x84, 0x04, 0xe2, //
        0x64, 0x5b, 0x50, 0xd6, 0xe1, 0xad, 0xaf, 0x83, 0x26, 0x6a, 0x77, 0x72, 0x5e, 0xec, 0x1f, 0x6a, //
        0x54, 0xd2, 0x4f, 0x15, 0xd9, 0x6e, 0x95, 0xd5, 0x57, 0xc8, 0x85, 0xba, 0xd2, 0x64, 0xd4, 0xed, //
        0x75, 0xbe, 0x9e, 0x70, 0x06, 0xe0, 0x06, 0x91, 0xad, 0x1d, 0x1c, 0x23, 0x7c, 0x64, 0xf8, 0xa9, //
        0xaf, 0x66, 0x22, 0xce, 0xdd, 0xdd, 0xb5, 0xc0, 0xb5, 0x9c, 0x56, 0x62, 0x32, 0x2d, 0xfb, 0xb8, //
        0x89, 0x9d, 0x66, 0x65, 0xe4, 0xb6, 0x9b, 0x3f, 0x4d, 0x74, 0xda, 0x66, 0xa3, 0xb9, 0xd0, 0xf4, //
    };

    const size_t data_length = sizeof(data);

    uint8_t* compressed_data = NULL;
    size_t compressed_data_length = 0;

    TEST_CHECK(ccrush_compress(data, data_length, 64, 6, &compressed_data, &compressed_data_length) == 0);
    TEST_CHECK(compressed_data_length < data_length);

    uint8_t* decompressed_data = NULL;
    size_t decompressed_data_length = 0;

    TEST_CHECK(0 == ccrush_decompress(compressed_data, compressed_data_length, 64, &decompressed_data, &decompressed_data_length));
    TEST_CHECK(0 == memcmp(data, decompressed_data, data_length));
    TEST_CHECK(data_length == decompressed_data_length);

    free(compressed_data);
    free(decompressed_data);
}

static void ccrush_decompress_wrong_data_fails()
{
    const uint8_t data[] = {
        0xe9, 0x4c, 0x03, 0xef, 0x29, 0x98, 0xf7, 0x35, 0x1d, 0xbd, 0xeb, 0xff, 0xda, 0xf7, 0x20, 0xc7, //
        0x26, 0xfc, 0xaf, 0x4e, 0xa2, 0x51, 0x92, 0xc3, 0xea, 0x16, 0xe9, 0x9d, 0xd1, 0x4a, 0xdc, 0x2e, //
        0x8a, 0xef, 0x64, 0x29, 0xb7, 0x4c, 0xa1, 0x25, 0xe8, 0x13, 0x4e, 0xbc, 0x41, 0xe7, 0x77, 0x47, //
        0xe9, 0xdb, 0x39, 0xe6, 0x74, 0xc0, 0x94, 0xfa, 0x9c, 0x6e, 0x2c, 0xad, 0x3a, 0xaf, 0x97, 0xe8, //
        0x0c, 0xbc, 0xc1, 0x3e, 0x64, 0x42, 0xa3, 0x64, 0x58, 0x80, 0xef, 0x42, 0xfe, 0x4a, 0xba, 0xd0, //
        0xa7, 0x37, 0x8f, 0x3d, 0x4f, 0x29, 0x82, 0xeb, 0xf2, 0x02, 0x4e, 0x9f, 0x3f, 0x1a, 0x31, 0x80, //
        0x72, 0x95, 0xf3, 0xf0, 0x87, 0x35, 0xb7, 0x64, 0x99, 0x04, 0x00, 0x18, 0x3c, 0x36, 0xca, 0xc4, //
        0x09, 0xc3, 0x0f, 0x43, 0x64, 0xb6, 0x87, 0x2a, 0x2c, 0x11, 0x1a, 0x02, 0x07, 0x1c, 0x77, 0xfc, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x64, 0x42, 0xd5, 0x95, 0xc4, 0x32, 0x58, 0x79, 0xd2, 0x64, 0x11, 0xb0, 0xc2, 0x92, 0x43, 0x59, //
        0x64, 0x38, 0x76, 0x00, 0x4d, 0x17, 0x43, 0x39, 0x57, 0xea, 0xf1, 0xcc, 0x7f, 0x3c, 0x2a, 0xc8, //
        0x07, 0x15, 0x1c, 0xcc, 0x7d, 0xd6, 0x67, 0x5d, 0x59, 0xd9, 0x61, 0x3e, 0xff, 0x64, 0xb9, 0xe8, //
        0x96, 0x17, 0xd3, 0xf7, 0x45, 0x64, 0xa6, 0x64, 0xff, 0x64, 0x88, 0x4c, 0xb8, 0x70, 0x9b, 0xf6, //
        0x9d, 0xb7, 0x1f, 0x8e, 0xcc, 0xeb, 0xf7, 0x95, 0x0c, 0x64, 0x8b, 0x8b, 0x71, 0x6a, 0xaf, 0x26, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x64, 0x42, 0xd5, 0x95, 0xc4, 0x32, 0x58, 0x79, 0xd2, 0x64, 0x11, 0xb0, 0xc2, 0x92, 0x43, 0x59, //
        0x64, 0x38, 0x76, 0x00, 0x4d, 0x17, 0x43, 0x39, 0x57, 0xea, 0xf1, 0xcc, 0x7f, 0x3c, 0x2a, 0xc8, //
        0x07, 0x15, 0x1c, 0xcc, 0x7d, 0xd6, 0x67, 0x5d, 0x59, 0xd9, 0x61, 0x3e, 0xff, 0x64, 0xb9, 0xe8, //
        0x96, 0x17, 0xd3, 0xf7, 0x45, 0x64, 0xa6, 0x64, 0xff, 0x64, 0x88, 0x4c, 0xb8, 0x70, 0x9b, 0xf6, //
        0x9d, 0xb7, 0x1f, 0x8e, 0xcc, 0xeb, 0xf7, 0x95, 0x0c, 0x64, 0x8b, 0x8b, 0x71, 0x6a, 0xaf, 0x26, //
        0x83, 0xa3, 0x80, 0x0f, 0xa8, 0x64, 0xac, 0x1a, 0x10, 0x77, 0x6a, 0x4d, 0x35, 0x21, 0x8d, 0x4f, //
        0x74, 0x38, 0x9f, 0x57, 0xff, 0xf8, 0xe9, 0xe1, 0xd9, 0xf4, 0xb5, 0x11, 0x99, 0xeb, 0xd3, 0xae, //
        0x50, 0xc6, 0xe2, 0xe0, 0xae, 0x3b, 0x23, 0xb9, 0x64, 0xa9, 0x8e, 0x13, 0x2d, 0x85, 0xdb, 0xc4, //
        0x3c, 0x61, 0xbb, 0xb7, 0x88, 0xff, 0xdd, 0x3e, 0xdd, 0x01, 0x8d, 0x03, 0xfa, 0x7a, 0xb8, 0x2e, //
        0x13, 0x5a, 0xd4, 0x0a, 0x64, 0x77, 0x34, 0x64, 0x44, 0x31, 0xb4, 0x92, 0xe0, 0xa2, 0xe1, 0x43, //
        0xc1, 0xc3, 0xb0, 0x02, 0x2b, 0xc3, 0x9d, 0xe3, 0x9f, 0xd7, 0x4d, 0x61, 0x30, 0x05, 0xbb, 0x49, //
        0xba, 0x52, 0x77, 0xdd, 0x07, 0xe5, 0x1f, 0xdc, 0x43, 0x03, 0xe4, 0x82, 0x73, 0x9a, 0x51, 0x23, //
        0x6f, 0x33, 0x4a, 0x7c, 0xfc, 0xcf, 0x90, 0xc3, 0x64, 0xea, 0x78, 0xc7, 0xf5, 0x62, 0x81, 0x31, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x63, 0xa2, 0x85, 0x43, 0x54, 0xf7, 0xe7, 0x26, 0x03, 0x64, 0x6d, 0x33, 0x19, 0x84, 0x04, 0xe2, //
        0x64, 0x5b, 0x50, 0xd6, 0xe1, 0xad, 0xaf, 0x83, 0x26, 0x6a, 0x77, 0x72, 0x5e, 0xec, 0x1f, 0x6a, //
        0x54, 0xd2, 0x4f, 0x15, 0xd9, 0x6e, 0x95, 0xd5, 0x57, 0xc8, 0x85, 0xba, 0xd2, 0x64, 0xd4, 0xed, //
        0x75, 0xbe, 0x9e, 0x70, 0x06, 0xe0, 0x06, 0x91, 0xad, 0x1d, 0x1c, 0x23, 0x7c, 0x64, 0xf8, 0xa9, //
        0xa5, 0x4d, 0xfc, 0x9f, 0xf8, 0xeb, 0xe8, 0x27, 0xdd, 0x70, 0x64, 0x17, 0x70, 0x1a, 0x07, 0x4f, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x64, 0x42, 0xd5, 0x95, 0xc4, 0x32, 0x58, 0x79, 0xd2, 0x64, 0x11, 0xb0, 0xc2, 0x92, 0x43, 0x59, //
        0x64, 0x38, 0x76, 0x00, 0x4d, 0x17, 0x43, 0x39, 0x57, 0xea, 0xf1, 0xcc, 0x7f, 0x3c, 0x2a, 0xc8, //
        0x07, 0x15, 0x1c, 0xcc, 0x7d, 0xd6, 0x67, 0x5d, 0x59, 0xd9, 0x61, 0x3e, 0xff, 0x64, 0xb9, 0xe8, //
        0x96, 0x17, 0xd3, 0xf7, 0x45, 0x64, 0xa6, 0x64, 0xff, 0x64, 0x88, 0x4c, 0xb8, 0x70, 0x9b, 0xf6, //
        0x9d, 0xb7, 0x1f, 0x8e, 0xcc, 0xeb, 0xf7, 0x95, 0x0c, 0x64, 0x8b, 0x8b, 0x71, 0x6a, 0xaf, 0x26, //
        0x78, 0x1c, 0x79, 0xd9, 0xa7, 0xa7, 0x54, 0x2a, 0xcd, 0x64, 0x72, 0xa3, 0x12, 0xac, 0xed, 0x10, //
        0x63, 0xa2, 0x85, 0x43, 0x54, 0xf7, 0xe7, 0x26, 0x03, 0x64, 0x6d, 0x33, 0x19, 0x84, 0x04, 0xe2, //
        0x64, 0x5b, 0x50, 0xd6, 0xe1, 0xad, 0xaf, 0x83, 0x26, 0x6a, 0x77, 0x72, 0x5e, 0xec, 0x1f, 0x6a, //
        0x54, 0xd2, 0x4f, 0x15, 0xd9, 0x6e, 0x95, 0xd5, 0x57, 0xc8, 0x85, 0xba, 0xd2, 0x64, 0xd4, 0xed, //
        0x75, 0xbe, 0x9e, 0x70, 0x06, 0xe0, 0x06, 0x91, 0xad, 0x1d, 0x1c, 0x23, 0x7c, 0x64, 0xf8, 0xa9, //
        0xa5, 0x4d, 0xfc, 0x9f, 0xf8, 0xeb, 0xe8, 0x27, 0xdd, 0x70, 0x64, 0x17, 0x70, 0x1a, 0x07, 0x4f, //
        0x85, 0xef, 0x81, 0x3c, 0x4c, 0xa8, 0x9e, 0xee, 0x4e, 0x34, 0xf7, 0xbe, 0x0c, 0xf4, 0xb7, 0x49, //
        0x64, 0x64, 0x42, 0x64, 0x3b, 0x64, 0x82, 0xd4, 0xb4, 0x41, 0x2c, 0xa0, 0x99, 0x94, 0xcc, 0xcd, //
        0x9c, 0xfc, 0xd1, 0xd0, 0xec, 0x34, 0xd2, 0x8f, 0x88, 0x1c, 0x0a, 0x6a, 0xd0, 0x4a, 0x24, 0x1c, //
        0x64, 0x42, 0xd5, 0x95, 0xc4, 0x32, 0x58, 0x79, 0xd2, 0x64, 0x11, 0xb0, 0xc2, 0x92, 0x43, 0x59, //
        0x64, 0x38, 0x76, 0x00, 0x4d, 0x17, 0x43, 0x39, 0x57, 0xea, 0xf1, 0xcc, 0x7f, 0x3c, 0x2a, 0xc8, //
        0x07, 0x15, 0x1c, 0xcc, 0x7d, 0xd6, 0x67, 0x5d, 0x59, 0xd9, 0x61, 0x3e, 0xff, 0x64, 0xb9, 0xe8, //
        0x96, 0x17, 0xd3, 0xf7, 0x45, 0x64, 0xa6, 0x64, 0xff, 0x64, 0x88, 0x4c, 0xb8, 0x70, 0x9b, 0xf6, //
        0x9d, 0xb7, 0x1f, 0x8e, 0xcc, 0xeb, 0xf7, 0x95, 0x0c, 0x64, 0x8b, 0x8b, 0x71, 0x6a, 0xaf, 0x26, //
        0x78, 0x1c, 0x79, 0xd9, 0xa7, 0xa7, 0x54, 0x2a, 0xcd, 0x64, 0x72, 0xa3, 0x12, 0xac, 0xed, 0x10, //
        0x63, 0xa2, 0x85, 0x43, 0x54, 0xf7, 0xe7, 0x26, 0x03, 0x64, 0x6d, 0x33, 0x19, 0x84, 0x04, 0xe2, //
        0x64, 0x5b, 0x50, 0xd6, 0xe1, 0xad, 0xaf, 0x83, 0x26, 0x6a, 0x77, 0x72, 0x5e, 0xec, 0x1f, 0x6a, //
        0x54, 0xd2, 0x4f, 0x15, 0xd9, 0x6e, 0x95, 0xd5, 0x57, 0xc8, 0x85, 0xba, 0xd2, 0x64, 0xd4, 0xed, //
        0x75, 0xbe, 0x9e, 0x70, 0x06, 0xe0, 0x06, 0x91, 0xad, 0x1d, 0x1c, 0x23, 0x7c, 0x64, 0xf8, 0xa9, //
        0xaf, 0x66, 0x22, 0xce, 0xdd, 0xdd, 0xb5, 0xc0, 0xb5, 0x9c, 0x56, 0x62, 0x32, 0x2d, 0xfb, 0xb8, //
        0x89, 0x9d, 0x66, 0x65, 0xe4, 0xb6, 0x9b, 0x3f, 0x4d, 0x74, 0xda, 0x66, 0xa3, 0xb9, 0xd0, 0xf4, //
    };

    const size_t data_length = sizeof(data);

    uint8_t* compressed_data = NULL;
    size_t compressed_data_length = 0;

    TEST_CHECK(ccrush_compress(data, data_length, 64, 6, &compressed_data, &compressed_data_length) == 0);
    TEST_CHECK(compressed_data_length < data_length);

    uint8_t* decompressed_data = NULL;
    size_t decompressed_data_length = 0;

    TEST_CHECK(0 != ccrush_decompress((uint8_t*)"DEFINITIVELY NOT THE RIGHT DATA. C'mon, decompress me motherf*cker!", 67 + 1, 64, &decompressed_data, &decompressed_data_length));
    TEST_CHECK(data_length != decompressed_data_length);
    TEST_CHECK(decompressed_data == NULL);

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
    { "ccrush_compress_bytes_result_is_smaller_and_decompression_succeeds", ccrush_compress_bytes_result_is_smaller_and_decompression_succeeds }, //
    { "ccrush_decompress_wrong_data_fails", ccrush_decompress_wrong_data_fails }, //
    //
    // ----------------------------------------------------------------------------------------------------------
    //
    { NULL, NULL } //
};
