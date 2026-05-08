#!/bin/bash

set -e  # stop on error

echo "Cleaning up existing build dir"
rm -fr build/

echo "Generating debug build files"
cmake -DCMAKE_BUILD_TYPE=Debug -B build -G Ninja

echo "Building"
ninja -C build

echo "Build successful"
