#!/usr/bin/env bash

function compile_shaders() {
    for filepath in shaders/*.comp; do
        pathname="$(dirname $filepath)"
        basename="$(basename $filepath)"
        filename="${basename%.*}"
        glslc "${pathname}/${filename}.comp" -o "${pathname}/${filename}.spv"
    done
}

function cleanup_shaders() {
    for filepath in shaders/*.spv; do
        rm -v "$filepath"
    done
}

function print_usage() {
    echo "Usage: $0 {shaders | clean | help}"
    echo "  shaders: Compiles all GLSL shader files in the 'shaders' directory."
    echo "  clean: Removes all compiled shader files in the 'shaders' directory."
    echo "  help: Displays this usage information."
}

case $1 in
    shaders)
        compile_shaders;;
    clean)
        cleanup_shaders;;
    help|-h|--help)
        print_usage;;
    *)
        print_usage
        exit 1;;
esac
