#!/bin/bash
set -e


BUILD_DIR="build_windows"
TOOLCHAIN_FILE="cmake/mingw-toolchain.cmake"

cmake -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DSYSTEM_SDL=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build "$BUILD_DIR" --parallel $(sysctl -n hw.logicalcpu)
