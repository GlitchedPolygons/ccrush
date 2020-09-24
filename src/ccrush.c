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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <miniz.h>
#include <chillbuff.h>

int ccrush_compress(const uint8_t* data, const size_t data_length, const uint32_t buffer_size_kib, const int level, uint8_t** out, size_t* out_length)
{
    if (data == NULL || data_length == 0 || out == NULL || out_length == NULL)
    {
        return CCRUSH_ERROR_INVALID_ARGS;
    }

    const size_t buffer_size_b = ((size_t)buffer_size_kib) * 1024;

    if (buffer_size_b >= UINT32_MAX)
    {
        return CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE;
    }

    int r = -1;

    mz_stream stream;
    memset(&stream, 0x00, sizeof(stream));

    assert(sizeof(uint8_t) == 1);
    const uint32_t buffersize = (uint32_t)(buffer_size_b ? buffer_size_b : CCRUSH_DEFAULT_CHUNKSIZE);
    uint8_t* zinbuf = malloc(buffersize);
    uint8_t* zoutbuf = malloc(buffersize);

    chillbuff output_buffer;
    r = chillbuff_init(&output_buffer, nextpow2(CCRUSH_MAX(mz_compressBound((mz_ulong)data_length), buffersize)), sizeof(uint8_t), CHILLBUFF_GROW_DUPLICATIVE);

    if (r != 0 || zinbuf == NULL || zoutbuf == NULL)
    {
        r = CCRUSH_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    if ((data_length | output_buffer.capacity) > 0xFFFFFFFFU)
    {
        r = MZ_PARAM_ERROR;
        goto exit;
    }

    r = mz_deflateInit(&stream, level < 0 || level > 9 ? 6 : level);
    if (r != MZ_OK)
    {
        goto exit;
    }

    stream.next_in = zinbuf;
    stream.avail_in = 0;
    stream.next_out = zoutbuf;
    stream.avail_out = buffersize;

    size_t remaining = data_length;

    for (;;)
    {
        if (stream.avail_in == 0)
        {
            const uint32_t n = (uint32_t)(CCRUSH_MIN((size_t)buffersize, remaining));

            memcpy(zinbuf, data + stream.total_in, n);

            stream.next_in = zinbuf;
            stream.avail_in = n;

            remaining -= n;
        }

        r = deflate(&stream, remaining ? Z_NO_FLUSH : Z_FINISH);

        if (r == Z_STREAM_END || stream.avail_out == 0)
        {
            const uint32_t n = buffersize - stream.avail_out;

            chillbuff_push_back(&output_buffer, zoutbuf, n);

            stream.next_out = zoutbuf;
            stream.avail_out = buffersize;
        }

        if (r == Z_STREAM_END)
        {
            r = 0;
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

    mz_deflateEnd(&stream);
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

int ccrush_decompress(const uint8_t* data, const size_t data_length, const uint32_t buffer_size_kib, uint8_t** out, size_t* out_length)
{
    if (data == NULL || data_length == 0 || out == NULL || out_length == NULL)
    {
        return CCRUSH_ERROR_INVALID_ARGS;
    }

    const size_t buffer_size_b = ((size_t)buffer_size_kib) * 1024;

    if (buffer_size_b >= UINT32_MAX)
    {
        return CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE;
    }

    int r = -1;

    mz_stream stream;
    memset(&stream, 0x00, sizeof(stream));

    assert(sizeof(uint8_t) == 1);
    const uint32_t buffersize = (uint32_t)(buffer_size_b ? buffer_size_b : CCRUSH_DEFAULT_CHUNKSIZE);
    uint8_t* zinbuf = malloc(buffersize);
    uint8_t* zoutbuf = malloc(buffersize);

    stream.next_in = zinbuf;
    stream.avail_in = 0;
    stream.next_out = zoutbuf;
    stream.avail_out = buffersize;

    chillbuff output_buffer;
    r = chillbuff_init(&output_buffer, nextpow2((uint64_t)data_length * 2), sizeof(uint8_t), CHILLBUFF_GROW_DUPLICATIVE);

    if (zinbuf == NULL || zoutbuf == NULL || r == CHILLBUFF_OUT_OF_MEM)
    {
        r = CCRUSH_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    r = mz_inflateInit(&stream);
    if (r != 0)
    {
        goto exit;
    }

    size_t remaining = data_length;

    for (;;)
    {
        if (stream.avail_in == 0)
        {
            const uint32_t n = (uint32_t)(CCRUSH_MIN((size_t)buffersize, remaining));

            memcpy(zinbuf, data + stream.total_in, n);
            stream.next_in = zinbuf;
            stream.avail_in = n;

            remaining -= n;
        }

        r = mz_inflate(&stream, Z_SYNC_FLUSH);

        if (r == Z_STREAM_END || stream.avail_out == 0)
        {
            const uint32_t n = buffersize - stream.avail_out;

            chillbuff_push_back(&output_buffer, zoutbuf, n);

            stream.next_out = zoutbuf;
            stream.avail_out = buffersize;
        }

        if (r == Z_STREAM_END)
        {
            r = 0;
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

    mz_inflateEnd(&stream);

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
