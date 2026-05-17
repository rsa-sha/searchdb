#!/bin/bash

set -e

echo "Cleaning TSAN build"
rm -rf build_tsan

echo "Generating TSAN build files"

cmake -B build_tsan -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_TSAN=ON \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"

echo "Building"

ninja -C build_tsan

echo "Build successful"
