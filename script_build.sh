#!/bin/bash

set -e  # stop on error

echo "Generating build files"
cmake -B build -G Ninja

echo "Building"
ninja -C build

echo "Build successful"
