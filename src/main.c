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
                                "Pass the data to compress or decompress into the CLI's stdin (for example with a pipe).\n"
                                "When decompressing, pass the \"-d\" argument to put ccrush into decompression mode.\n\n"
                                "Compression examples:\n\n"
                                "  cat file-to-compress.txt | ccrush > my-compressed-file.txt.zlib\n\n  ---\n  OR\n  ---\n\n"
                                "  echo -n \"Why do we all have to wear these ridiculous ties?!\" | ccrush > my-compressed-file.txt.zlib\n\nn  ---\n  OR\n  ---\n\n"
                                "  ccrush < cat file-to-compress.txt > my-compressed-file.txt.zlib\n\n  ---\n  OR\n  ---\n\n"
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
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            print_help_text();
            return 0;
        }
    }

    if (argc < 2 && feof(stdin))
    {
        print_help_text();
        return 0;
    }

    return 0;
}