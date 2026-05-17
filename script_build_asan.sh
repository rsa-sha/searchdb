#!/bin/bash

set -e  # stop on error

echo "Cleaning up ASAN build dir"
rm -fr build_asan

echo "Generating ASAN build files"
cmake -B build_asan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer -g -fsanitize=address,undefined" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
#-DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer -g"

echo "Building"
ninja -C build_asan

echo "Build successful"
