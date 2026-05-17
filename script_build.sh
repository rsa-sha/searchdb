#!/bin/bash

set -e  # stop on error

echo "Cleaning release build"
rm -rf build_release

echo "Generating release build files"
cmake -B build_release -G Ninja -DCMAKE_BUILD_TYPE=Release

echo "Building"
ninja -C build_release

echo "Build successful"
