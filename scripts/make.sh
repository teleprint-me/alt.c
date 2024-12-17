#!/usr/bin/env bash

function compile_alt() {
    cmake -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --config Debug -j "$(nproc)"
}

function compile_shaders() {
    for filepath in "$(pwd)"/shaders/*.comp; do
        pathname="$(dirname $filepath)"
        basename="$(basename $filepath)"
        filename="${basename%.*}"
        glslang -V "${pathname}/${filename}.comp" -o "${pathname}/${filename}.spv"
    done
}

function cleanup_alt() {
    rm -rf build build-x64-linux-clang-debug CMakeFiles
}

function cleanup_shaders() {
    for filepath in shaders/*.spv; do
        rm -v "$filepath"
    done
}

function print_usage() {
    echo "Usage: $0 {all | alt | shaders | clean | help}"
    echo "  all: Builds the Alt library, examples, and compiles all GLSL shader files."
    echo "  alt: Builds the Alt library."
    echo "  shaders: Compiles all GLSL shader files in the 'shaders' directory."
    echo "  clean: Removes all compiled shader files in the 'shaders' directory."
    echo "  help: Displays this usage information."
}

case $1 in
    all)
        compile_shaders
        compile_alt;;
    alt)
        compile_alt;;
    shaders)
        compile_shaders;;
    clean)
        cleanup_shaders
        cleanup_alt;;
    help|-h|--help)
        print_usage;;
    *)
        print_usage
        exit 1;;
esac
