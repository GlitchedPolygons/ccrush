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

### Examples

// TODO
