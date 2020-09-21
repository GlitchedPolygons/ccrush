#!/bin/sh
#
#    BSD 2-Clause License
#
#    Copyright (c) 2020, Raphael Beck
#    All rights reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice, this
#       list of conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above copyright notice,
#       this list of conditions and the following disclaimer in the documentation
#       and/or other materials provided with the distribution.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

if [ "$(whoami)" = "root" ]; then
  echo "  Please don't run as root/using sudo..."
  exit
fi

PREVCC="$CC"
if command -v clang >/dev/null 2>&1
then
    echo "-- Clang found on system, great! Long live LLVM! :D"
    export CC=clang
fi

REPO=$(dirname "$0")
rm -rf "$REPO"/out
rm -rf "$REPO"/build
mkdir -p "$REPO"/build/shared && cd "$REPO"/build || exit
cp -r ../include ./
cd shared || exit
cmake -DBUILD_SHARED_LIBS=On -DCCRUSH_BUILD_DLL=On -DCCRUSH_ENABLE_TESTS=Off -DCMAKE_BUILD_TYPE=Release ../.. || exit
cmake --build . --config Release || exit
cd .. || exit
mkdir static || exit
cd static || exit
cmake -DBUILD_SHARED_LIBS=Off -DCCRUSH_BUILD_DLL=Off -DCCRUSH_ENABLE_TESTS=Off -DCMAKE_BUILD_TYPE=Release ../.. || exit
cmake --build . --config Release || exit
cd .. || exit
VER=$(grep VERSION_STR include/*.h | sed -e "s/^#define CCRUSH_VERSION_STR\ \"//" -e "s/\"$//" | tr -d '\n' | tr -d '\r\n')
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
CPU=$(uname -m)
tar -czvf "ccrush-${VER}-${OS}-${CPU}.tar.gz" $(find . -type f -iname "*.h" -o -iname "*.dll" -o -iname "*.lib" -o -iname "*.exp" -o -iname "*.dylib" -o -iname "*.dylib*" -o -iname "*.so" -o -iname "*.so*" -o -iname "*.a" | sed "s|^\./||" | sed -e "s/^\\.//")
cd "$REPO" || exit
export CC="$PREVCC"
echo "  Done. Exported build into $REPO/build"
echo "  Check out the ccrush.tar.gz file in there! "