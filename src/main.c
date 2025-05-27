/*

BSD 2-Clause License

Copyright (c) 2025, Raphael Beck
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

/**
 *  @file main.c
 *  @author Raphael Beck
 *  @brief CLI for compressing and decompressing data easily using wrapper functions around Zlib.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ccrush.h>
#include <zlib.h>

#ifdef _WIN32
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#endif

static const char HELP_TEXT[] = "\n"
                                "ccrush v%s\n"
                                "------------- \n"
                                "Compress and decompress data easily using zlib v%s.\n\n"
                                "Usage:\n\n"
                                "Pass the data to compress or decompress into the CLI's stdin (for example with a pipe).\n\n"
                                "When decompressing, pass the \"-d\" argument to put ccrush into decompression mode.\n\n"
                                "Optional parameters are:\n\n"
                                "  -c\n  Sets the compression level to use when deflating the input data.\n  Must be a number between 0 and 9, where 0 means no compression at all and 9 is maximum compression (slowest).\n  Default value: 6\n\n"
                                "  -b\n  Sets the buffer size (in KiB) to use for compressing/decompressing.\n  Must be less than 262144.\n  Default value: 256\n\n"
                                "Compression examples:\n\n"
                                "  cat file-to-compress.txt | ccrush > my-compressed-file.txt.zlib\n\n  ---\n  OR\n  ---\n\n"
                                "  echo -n \"Why do we all have to wear these ridiculous ties?!\" | ccrush > my-compressed-file.txt.zlib\n\nn  ---\n  OR\n  ---\n\n"
                                "  ccrush < cat file-to-compress.txt > my-compressed-file.txt.zlib\n\n  ---\n  OR\n  ---\n\n"
                                "  ccrush -c 8 -b 1024 < cat file-to-compress.txt > my-compressed-file.txt.zlib\n\n"
                                "Decompression examples:\n\n"
                                "  cat my-compressed-file.txt.zlib | ccrush -d > decompressed-file.txt\n\n  ---\n  OR\n  ---\n\n"
                                "  ccrush -d < cat my-compressed-file.txt.zlib\n\n"
                                "This last example would attempt to print out the decompressed result to stdout (which could be the terminal itself)."
                                "\n";

static void print_help_text()
{
    fprintf(stdout, HELP_TEXT, CCRUSH_VERSION_STR, ZLIB_VERSION);
}

int main(const int argc, char* argv[])
{
    int decompress = 0;
    int compression_level = 6;
    int buffer_size_kib = 256;

    for (int i = 1; i < argc; ++i)
    {
        const char* arg = argv[i];

        if (strncmp(arg, "-h", 2) == 0 || strncmp(arg, "--help", 6) == 0)
        {
            print_help_text();
            return EXIT_SUCCESS;
        }

        if (strncmp(arg, "-d", 2) == 0 || strncmp(arg, "--decompress", 12) == 0)
        {
            decompress = 1;
        }

        if (strncmp(arg, "-c", 2) == 0 || strncmp(arg, "--compression-level", 19) == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "Please specify a number between 0 and 9 after the \"-c\" argument.\n");
                return CCRUSH_ERROR_INVALID_ARGS;
            }

            const unsigned long int level = strtoul(argv[i + 1], NULL, 10);

            if (level > 9)
            {
                fprintf(stderr, "Compression level parameter must be a number between 0 and 9.\n");
                return CCRUSH_ERROR_INVALID_ARGS;
            }

            compression_level = (int)level;
        }

        if (strncmp(arg, "-b", 2) == 0 || strncmp(arg, "--buffer-size", 13) == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "Please specify a buffer size in KiB after the \"-b\" argument.\n");
                return CCRUSH_ERROR_INVALID_ARGS;
            }

            const unsigned long int buffer_size = strtoul(argv[i + 1], NULL, 10);

            if (buffer_size > CCRUSH_MAX_BUFFER_SIZE_KiB)
            {
                fprintf(stderr, "Buffer size too big; it must be between [1 KiB; 256 MiB].\n");
                return CCRUSH_ERROR_INVALID_ARGS;
            }

            buffer_size_kib = (int)buffer_size;
        }
    }

    int r = -1;

    if (decompress)
    {
        r = ccrush_decompress_file_raw(stdin, stdout, (uint32_t)buffer_size_kib, 0, 1);
    }
    else
    {
        r = ccrush_compress_file_raw(stdin, stdout, (uint32_t)buffer_size_kib, compression_level, 0, 1);
    }

    switch (r)
    {
        case 0: {
            break;
        }
        case CCRUSH_ERROR_INVALID_ARGS: {
            fprintf(stderr, "Invalid arguments.\n");
            break;
        }
        case CCRUSH_ERROR_FILE_ACCESS_FAILED: {
            fprintf(stderr, "Input and/or output file access failed.\n");
            break;
        }
        case CCRUSH_ERROR_OUT_OF_MEMORY: {
            fprintf(stderr, "Out of memory.\n");
            break;
        }
        case CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE: {
            fprintf(stderr, "Invalid buffer size argument; it must be in the range of [1 KiB; 256 MiB]\n");
            break;
        }
        default: {
            fprintf(stderr, "%s failed; %s returned error code: %d.\n", decompress ? "Decompression" : "Compression", decompress ? "ccrush_decompress_file_raw": "ccrush_compress_file_raw", r);
            break;
        }
    }

    return r;
}