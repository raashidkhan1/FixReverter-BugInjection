#!/bin/bash -ex
# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

apt-get update && \
    apt-get install -y \
    make \
    automake \
    autoconf \
    libtool \
    sqlite3 \
    wget \
    libsqlite3-dev

git clone https://github.com/OSGeo/PROJ
cd PROJ
git checkout d00501750b210a73f9fb107ac97a683d4e3d8e7a
./autogen.sh
./configure
make -j $(nproc)

$CXX $CXXFLAGS -std=c++11 -I src test/fuzzers/standard_fuzzer.cpp \
    src/.libs/libproj.a $FUZZER_LIB -o $SRC/fuzzer -lpthread -ldl

cd $SRC
extract-bc $SRC/fuzzer
/usr/local/llvm-12/bin/llvm-dis $SRC/fuzzer.bc
