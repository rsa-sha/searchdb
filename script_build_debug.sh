#!/bin/bash

set -e  # stop on error

echo "Cleaning up existing build dir"
rm -fr build/

echo "Generating debug build files"
echo "Generating debug build files"
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer -g"

echo "Building"
ninja -C build

echo "Build successful"
