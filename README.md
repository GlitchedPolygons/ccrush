# CCRUSH
### _Say "C-Crush", "Sick rush", "Seek rush", "CRUSH!!!", or whatever you feel sounds best_

[![Codacy](https://app.codacy.com/project/badge/Grade/f3cf79eea56742b7a5581faab5d3a0cf)](https://www.codacy.com/manual/GlitchedPolygons/ccrush?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=GlitchedPolygons/ccrush&amp;utm_campaign=Badge_Grade)
[![Codecov](https://codecov.io/gh/GlitchedPolygons/ccrush/branch/master/graph/badge.svg)](https://codecov.io/gh/GlitchedPolygons/ccrush)
[![CircleCI](https://circleci.com/gh/GlitchedPolygons/ccrush/tree/master.svg?style=shield)](https://circleci.com/gh/GlitchedPolygons/ccrush/tree/master)
[![License Shield](https://img.shields.io/badge/license-BSD--2--clause-blueviolet)](https://github.com/GlitchedPolygons/ccrush/blob/master/LICENSE)
[![API Docs](https://img.shields.io/badge/api-docs-informational.svg)](https://glitchedpolygons.github.io/ccrush/files.html)

---

This is what you want to use if you _just want to compress some stuff in C_. 
Hence, C-Crush. You're crushing data with C. Especially if you're in a _rush_, 
when you just want to call 1 function and (de)compress stuff.

It's a wrapper around [Zlib](https://github.com/madler/zlib), which implements the [DEFLATE](https://en.m.wikipedia.org/wiki/DEFLATE)
algorithm as defined in the [RFC1951](https://tools.ietf.org/html/rfc1951) standard.

### How to clone
`git clone --recursive https://github.com/GlitchedPolygons/ccrush.git`

### How to use
Just add CCRUSH as a git submodule to your project (e.g. into some `lib/` or `deps/` folder inside your project's repo; `{repo_root}/lib/` is used here in the following example).

```
git submodule add https://github.com/GlitchedPolygons/ccrush.git lib/ccrush
git submodule update --init --recursive
```

If you don't want to use git submodules, you can also start vendoring a specific version of **ccrush** by copying its full repo content into the folder where you keep your project's external libraries/dependencies.

### Linking

If you use [CMake](https://cmake.org) you can just `add_subdirectory(path_to_submodule)` and then `target_link_libraries(your_project PRIVATE ccrush)` inside your `CMakeLists.txt` file.

### Building from sauce

If you don't wanna grab the pre-built binaries from the [GitHub releases page](https://github.com/GlitchedPolygons/ccrush/releases) (and don't want to add ccrush as a subdirectory inside your CMake project), you can also do the following:

#### Build shared library/DLL

```bash
bash build.sh
```
This works on Windows too: just use the [Git Bash for Windows](https://git-scm.com/download/win) CLI!

If the build succeeds, you should have a new _.tar.gz_ file inside the `build/` directory.

**NOTE:** If you use the shared library in your project on Windows, remember to `#define CCRUSH_DLL 1` before including `ccrush.h`! Maybe even set it as a pre-processor definition. Otherwise the header won't have the necessary `__declspec(dllimport)` declarations!

#### Build static library

```bash
mkdir -p build && cd build
cmake -DBUILD_SHARED_LIBS=Off -Dccrush_PACKAGE=On -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

### Examples

#### Compressing

```c
const uint8_t* data = "Your special string or byte array to compress here!";
const size_t data_length = strlen(data);

uint8_t* compressed_data = NULL;
size_t compressed_data_length = 0;

int r = ccrush_compress(data, data_length, 64, 8, &compressed_data, &compressed_data_length);
if (r != 0)
{
    fprintf(stderr, "Compression failed! \"ccrush_compress\" returned: %d", r);
}

ccrush_free(compressed_data);
```

#### Decompressing

```c
const uint8_t* compressed_data = "{{ These should be your compressed bytes written by the ccrush_compress function }}";
const size_t compressed_data_length = 256; // This should be the exact length of the compressed data array as written by the ccrush_compress function! 

uint8_t* data = NULL;
size_t data_length = 0;

int r = ccrush_decompress(compressed_data, compressed_data_length, 64, &data, &data_length);
if (r != 0)
{
    fprintf(stderr, "Decompression failed! \"ccrush_decompress\" returned: %d", r);
}

ccrush_free(data);
```
