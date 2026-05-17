#!/bin/bash

set -e  # stop on error

echo "Generating build files"
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

echo "Building"
ninja -C build

echo "Build successful"
