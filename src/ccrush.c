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

#include "ccrush.h"

#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>
#include <chillbuff.h>

int ccrush_compress(const uint8_t* data, const size_t data_length, const uint32_t buffer_size_kib, const int level, uint8_t** out, size_t* out_length)
{
    if (data == NULL || data_length == 0 || out == NULL || out_length == NULL)
    {
        return CCRUSH_ERROR_INVALID_ARGS;
    }

    if (buffer_size_kib >= CCRUSH_MAX_BUFFER_SIZE_KiB)
    {
        return CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE;
    }

    int r;

    z_stream stream;
    memset(&stream, 0x00, sizeof(stream));

    assert(sizeof(uint8_t) == 1);
    const size_t buffer_size_b = ((size_t)buffer_size_kib) * 1024;
    const unsigned int buffersize = (unsigned int)(buffer_size_b ? buffer_size_b : CCRUSH_DEFAULT_CHUNKSIZE);

    uint8_t* zinbuf = malloc(buffersize);
    uint8_t* zoutbuf = malloc(buffersize);

    chillbuff output_buffer;
    r = chillbuff_init(&output_buffer, ccrush_nextpow2(CCRUSH_MAX(compressBound((unsigned long)data_length), buffersize)), sizeof(uint8_t), CHILLBUFF_GROW_DUPLICATIVE);

    if (r != 0 || zinbuf == NULL || zoutbuf == NULL)
    {
        r = CCRUSH_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    r = deflateInit(&stream, level < 0 || level > 9 ? 6 : level);
    if (r != Z_OK)
    {
        goto exit;
    }

    stream.next_in = zinbuf;
    stream.avail_in = 0;
    stream.next_out = zoutbuf;
    stream.avail_out = buffersize;

    size_t remaining = data_length, consumed = 0;

    for (;;)
    {
        if (stream.avail_in == 0)
        {
            const unsigned int n = (unsigned int)(CCRUSH_MIN((size_t)buffersize, remaining));

            memcpy(zinbuf, data + consumed, n);

            stream.next_in = zinbuf;
            stream.avail_in = n;

            consumed += n;
            remaining -= n;
        }

        r = deflate(&stream, remaining ? Z_NO_FLUSH : Z_FINISH);

        if (r == Z_STREAM_END || stream.avail_out == 0)
        {
            const unsigned int n = buffersize - stream.avail_out;

            chillbuff_push_back(&output_buffer, zoutbuf, n);

            stream.next_out = zoutbuf;
            stream.avail_out = buffersize;
        }

        if (r == Z_STREAM_END)
        {
            break;
        }
        else if (r != 0)
        {
            goto exit;
        }
    }

    *out = malloc(output_buffer.length + 1);
    if (*out == NULL)
    {
        r = CCRUSH_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    r = 0;
    *out_length = output_buffer.length;
    memcpy(*out, output_buffer.array, output_buffer.length);
    (*out)[output_buffer.length] = 0x00;

exit:

    deflateEnd(&stream);
    memset(&stream, 0x00, sizeof(stream));

    if (zinbuf != NULL)
    {
        memset(zinbuf, 0x00, buffersize);
        free(zinbuf);
    }

    if (zoutbuf != NULL)
    {
        memset(zoutbuf, 0x00, buffersize);
        free(zoutbuf);
    }

    chillbuff_free(&output_buffer);

    return (r);
}

int ccrush_compress_file(const char* input_file_path, const char* output_file_path, uint32_t buffer_size_kib, int level)
{
    if (!input_file_path || !output_file_path || input_file_path == output_file_path || strcmp(input_file_path, output_file_path) == 0)
    {
        return CCRUSH_ERROR_INVALID_ARGS;
    }

    if (buffer_size_kib > CCRUSH_MAX_BUFFER_SIZE_KiB)
    {
        return CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE;
    }

    int r;

    z_stream stream;
    memset(&stream, 0x00, sizeof(stream));

    assert(sizeof(uint8_t) == 1);
    const size_t buffer_size_b = ((size_t)buffer_size_kib) * 1024;
    const unsigned int buffersize = (unsigned int)(buffer_size_b ? buffer_size_b : CCRUSH_DEFAULT_CHUNKSIZE);

    uint8_t* input_buffer = malloc(buffersize);
    uint8_t* output_buffer = malloc(buffersize);

    FILE* input_file = fopen(input_file_path, "rb");
    FILE* output_file = fopen(output_file_path, "wb");

    if (input_file == NULL || output_file == NULL)
    {
        r = CCRUSH_ERROR_FILE_ACCESS_FAILED;
        goto exit;
    }

    if (input_buffer == NULL || output_buffer == NULL)
    {
        r = CCRUSH_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    r = deflateInit(&stream, level);
    if (r != Z_OK)
    {
        goto exit;
    }

    int flush;

    do
    {
        stream.avail_in = fread(input_buffer, sizeof(uint8_t), buffersize, input_file);
        if (ferror(input_file))
        {
            r = CCRUSH_ERROR_FILE_ACCESS_FAILED;
            goto exit;
        }

        flush = feof(input_file) ? Z_FINISH : Z_NO_FLUSH;
        stream.next_in = input_buffer;

        do
        {
            stream.avail_out = buffersize;
            stream.next_out = output_buffer;

            r = deflate(&stream, flush);
            if (r == Z_STREAM_ERROR)
            {
                goto exit;
            }

            const unsigned int processed = buffersize - stream.avail_out;

            if (fwrite(output_buffer, sizeof(uint8_t), processed, output_file) != processed || ferror(output_file))
            {
                r = CCRUSH_ERROR_FILE_ACCESS_FAILED;
                goto exit;
            }

        } while (stream.avail_out == 0);

        if (stream.avail_in != 0)
        {
            r = Z_STREAM_ERROR;
            goto exit;
        }

    } while (flush != Z_FINISH);

    if (r != Z_STREAM_END)
    {
        r = Z_STREAM_ERROR;
        goto exit;
    }

    r = 0;

exit:

    deflateEnd(&stream);
    memset(&stream, 0x00, sizeof(stream));

    if (input_buffer != NULL)
    {
        memset(input_buffer, 0x00, buffersize);
        free(input_buffer);
    }

    if (output_buffer != NULL)
    {
        memset(output_buffer, 0x00, buffersize);
        free(output_buffer);
    }

    if (input_file != NULL)
    {
        fclose(input_file);
    }

    if (output_file != NULL)
    {
        fclose(output_file);
    }

    return (r);
}

int ccrush_decompress(const uint8_t* data, const size_t data_length, const uint32_t buffer_size_kib, uint8_t** out, size_t* out_length)
{
    if (data == NULL || data_length == 0 || out == NULL || out_length == NULL)
    {
        return CCRUSH_ERROR_INVALID_ARGS;
    }

    if (buffer_size_kib > CCRUSH_MAX_BUFFER_SIZE_KiB)
    {
        return CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE;
    }

    int r;

    z_stream stream;
    memset(&stream, 0x00, sizeof(stream));

    assert(sizeof(uint8_t) == 1);
    const size_t buffer_size_b = ((size_t)buffer_size_kib) * 1024;
    const unsigned int buffersize = (unsigned int)(buffer_size_b ? buffer_size_b : CCRUSH_DEFAULT_CHUNKSIZE);

    uint8_t* zinbuf = malloc(buffersize);
    uint8_t* zoutbuf = malloc(buffersize);

    stream.next_in = zinbuf;
    stream.avail_in = 0;
    stream.next_out = zoutbuf;
    stream.avail_out = buffersize;

    chillbuff output_buffer;
    r = chillbuff_init(&output_buffer, ccrush_nextpow2((uint64_t)data_length * 2), sizeof(uint8_t), CHILLBUFF_GROW_DUPLICATIVE);

    if (zinbuf == NULL || zoutbuf == NULL || r == CHILLBUFF_OUT_OF_MEM)
    {
        r = CCRUSH_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    r = inflateInit(&stream);
    if (r != 0)
    {
        goto exit;
    }

    size_t remaining = data_length, consumed = 0;

    for (;;)
    {
        if (stream.avail_in == 0)
        {
            const unsigned int n = (unsigned int)(CCRUSH_MIN((size_t)buffersize, remaining));

            memcpy(zinbuf, data + consumed, n);

            stream.next_in = zinbuf;
            stream.avail_in = n;

            consumed += n;
            remaining -= n;
        }

        r = inflate(&stream, Z_SYNC_FLUSH);

        if (r == Z_STREAM_END || stream.avail_out == 0)
        {
            const unsigned int n = buffersize - stream.avail_out;

            chillbuff_push_back(&output_buffer, zoutbuf, n);

            stream.next_out = zoutbuf;
            stream.avail_out = buffersize;
        }

        if (r == Z_STREAM_END)
        {
            break;
        }
        else if (r != 0)
        {
            goto exit;
        }
    }

    *out = malloc(output_buffer.length + 1);
    if (*out == NULL)
    {
        r = CCRUSH_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    r = 0;
    *out_length = output_buffer.length;
    (*out)[output_buffer.length] = '\0';
    memcpy(*out, output_buffer.array, output_buffer.length);

exit:

    inflateEnd(&stream);

    if (zoutbuf != NULL)
    {
        memset(zoutbuf, 0x00, buffersize);
        free(zoutbuf);
    }

    chillbuff_free(&output_buffer);

    return (r);
}

int ccrush_decompress_file(const char* input_file_path, const char* output_file_path, const uint32_t buffer_size_kib)
{
    if (!input_file_path || !output_file_path || input_file_path == output_file_path || strcmp(input_file_path, output_file_path) == 0)
    {
        return CCRUSH_ERROR_INVALID_ARGS;
    }

    if (buffer_size_kib > CCRUSH_MAX_BUFFER_SIZE_KiB)
    {
        return CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE;
    }

    int r;

    z_stream stream;
    memset(&stream, 0x00, sizeof(stream));

    assert(sizeof(uint8_t) == 1);
    const size_t buffer_size_b = ((size_t)buffer_size_kib) * 1024;
    const unsigned int buffersize = (unsigned int)(buffer_size_b ? buffer_size_b : CCRUSH_DEFAULT_CHUNKSIZE);

    uint8_t* input_buffer = malloc(buffersize);
    uint8_t* output_buffer = malloc(buffersize);

    FILE* input_file = fopen(input_file_path, "rb");
    FILE* output_file = fopen(output_file_path, "wb");

    if (input_file == NULL || output_file == NULL)
    {
        r = CCRUSH_ERROR_FILE_ACCESS_FAILED;
        goto exit;
    }

    if (input_buffer == NULL || output_buffer == NULL)
    {
        r = CCRUSH_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    r = inflateInit(&stream);
    if (r != Z_OK)
    {
        goto exit;
    }

    do
    {
        stream.avail_in = fread(input_buffer, sizeof(uint8_t), buffersize, input_file);
        if (ferror(input_file))
        {
            r = CCRUSH_ERROR_FILE_ACCESS_FAILED;
            goto exit;
        }

        if (stream.avail_in == 0)
        {
            break;
        }

        stream.next_in = input_buffer;

        do
        {
            stream.avail_out = buffersize;
            stream.next_out = output_buffer;

            r = inflate(&stream, Z_NO_FLUSH);

            switch (r)
            {
                case Z_NEED_DICT:
                    r = Z_DATA_ERROR; /* Intentional fall-through. */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                case Z_STREAM_ERROR:
                    goto exit;
            }

            const unsigned int processed = buffersize - stream.avail_out;

            if (fwrite(output_buffer, sizeof(uint8_t), processed, output_file) != processed || ferror(output_file))
            {
                r = CCRUSH_ERROR_FILE_ACCESS_FAILED;
                goto exit;
            }

        } while (stream.avail_out == 0);

    } while (r != Z_STREAM_END);

    r = r == Z_STREAM_END ? 0 : Z_DATA_ERROR;

exit:

    inflateEnd(&stream);
    memset(&stream, 0x00, sizeof(stream));

    if (input_buffer != NULL)
    {
        memset(input_buffer, 0x00, buffersize);
        free(input_buffer);
    }

    if (output_buffer != NULL)
    {
        memset(output_buffer, 0x00, buffersize);
        free(output_buffer);
    }

    if (input_file != NULL)
    {
        fclose(input_file);
    }

    if (output_file != NULL)
    {
        fclose(output_file);
    }

    return (r);
}

void ccrush_free(void* mem)
{
    free(mem);
}

uint32_t ccrush_get_version_nr()
{
    return (uint32_t)CCRUSH_VERSION;
}

char* ccrush_get_version_nr_string()
{
    return CCRUSH_VERSION_STR;
}