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

/**
 *  @file ccrush.h
 *  @author Raphael Beck
 *  @brief Compress and decompress byte arrays easily using wrapper functions around miniz.
 */

#ifndef CCRUSH_LIBRARY_H
#define CCRUSH_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/**
 * Default chunksize to use for compression/decompression buffers.
 */
#define CCRUSH_DEFAULT_CHUNKSIZE (1024 * 256)

/**
 * Error code for <c>NULL</c>, invalid, out-of-range or simply just wrong arguments.
 */
#define CCRUSH_ERROR_INVALID_ARGS 1000

/**
 * Error code for exaggerated buffer size arguments...
 */
#define CCRUSH_ERROR_BUFFERSIZE_TOO_LARGE 1001

/**
 * Error code for OOM scenarios. Uh oh...
 */
#define CCRUSH_ERROR_OUT_OF_MEMORY 2000

/**
 * Pick the lower of two numbers.
 */
#define CCRUSH_MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 * Pick the higher of two numbers.
 */
#define CCRUSH_MAX(x, y) (((x) > (y)) ? (x) : (y))

/**
 * Compresses an array of bytes using deflate.
 * @param data The data to compress.
 * @param data_length Length of the \p data array (how many bytes to compress).
 * @param buffer_size_kib The underlying buffer size to use (in KiB). Pass <c>0</c> to use the default value #CCRUSH_DEFAULT_CHUNKSIZE. Especially <c>inflate()</c> profits from a relatively large buffer a.k.a. "chunk" size. A 256KiB buffer works great :)
 * @param level The level of compression <c>[0-9]</c>. Lower means faster, higher level means better compression (but slower). Default is <c>6</c>. If you pass a value that is out of the allowed range of <c>[0-9]</c>, <c>6</c> will be used! <c>0</c> does not compress at all...
 * @param out Pointer to an output buffer. This will be allocated on the heap ONLY on success: if something failed, this is left untouched! Needs to be freed manually by the caller.
 * @param out_length Where to write the output array's length into.
 * @return <c>0</c> on success; non-zero error codes if something fails.
 */
int ccrush_compress(const uint8_t* data, size_t data_length, uint32_t buffer_size_kib, int level, uint8_t** out, size_t* out_length);

/**
 * Decompresses a given set of deflated data using inflate.
 * @param data The compressed bytes to decompress.
 * @param data_length Length of the \p data array.
 * @param buffer_size_kib The underlying buffer size to use (in KiB). If available, a buffer size of 256KiB or more is recommended. Pass <c>0</c> to use the default value #CCRUSH_DEFAULT_CHUNKSIZE.
 * @param out Output buffer pointer: this will be allocated and filled with the decompressed data. In case of a failure it's left alone, so you only need to free it if decompression succeeds!
 * @param out_length Where to write the output array's length into.
 * @return <c>0</c> on success; non-zero error codes if something fails.
 */
int ccrush_decompress(const uint8_t* data, size_t data_length, uint32_t buffer_size_kib, uint8_t** out, size_t* out_length);

/**
 * Calculates a number's next upper power of 2. <p>
 * Source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 * @param n The number to round to the next upper power of 2.
 * @return The upper next power of 2.
 */
static inline uint64_t nextpow2(uint64_t n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return ++n;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CCRUSH_LIBRARY_H
