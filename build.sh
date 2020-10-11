#!/bin/bash

mkdir -p build/
cd build/
conan install .. --build missing -s compiler=clang -s compiler.version=3.8 -s compiler.libcxx=libstdc++11 -s build_type=Release
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
make install -j4
make test