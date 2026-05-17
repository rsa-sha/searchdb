#!/bin/bash

set -e  # stop on error

echo "Cleaning up existing debug build dir"
rm -fr build_debug

echo "Generating debug build files"
cmake -B build_debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer -g"

echo "Building"
ninja -C build_debug

echo "Build successful"
