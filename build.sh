#!/bin/bash

COMPILER="clang"
BUILD_MODE="debug"
BUILD_TYPE="test"

LIB_SOURCES=$(find src -name "*.c" -type f)

if [ -z "$LIB_SOURCES" ]; then
    echo "[ERROR] No .c files found in the /src directory"
    exit 1
fi

for arg in "$@"; do
    case $arg in
        gcc|clang)
            COMPILER="$arg"
            ;;
        debug|release)
            BUILD_MODE="$arg"
            ;;
        test|lib)
            BUILD_TYPE="$arg"
            ;;
        target=*)
            BUILD_TYPE="target"
            TARGET="${arg#*=}"
            ;;
        *)
            echo "Unknown Argument: $arg"
            echo "Usage: $0 [gcc|clang] [debug|release] [test|lib|target=<filename>]"
            exit 1
            ;;
    esac
done

if [ "$BUILD_MODE" = "debug" ]; then
    FLAGS="-g -O0 -DDEBUG"
else
    FLAGS="-O2 -DNDEBUG"
fi

COMMON_FLAGS="-Wall -Werror -Wextra -Iinclude"
LINK_FLAGS="-lm"

mkdir -p build
mkdir -p tests

case $BUILD_TYPE in
    test)
        $COMPILER $FLAGS $COMMON_FLAGS -o "build/test_optionlib" $LIB_SOURCES "tests/test_main.c" $LINK_FLAGS
        if [ $? -eq 0 ]; then
            echo "[SUCCESS] Test executable created: build/test_optionlib"
            echo "[INFO] Run with: ./build/test_optionlib"
        else
            echo "[ERROR] Failed to compile tests"
            exit 1
        fi
        ;;
    lib)
        echo "[INFO] Source files: $LIB_SOURCES"
        for src in $LIB_SOURCES; do
            obj="build/$(basename ${src%.c}.o)"
            $COMPILER $FLAGS $COMMON_FLAGS -c "$src" -o "$obj"
            if [ $? -ne 0 ]; then
                echo "[ERROR] Failed to compile $src"
                exit 1
            fi
        done

        ar rcs build/liboptionlib.a build/*.o
        if [ $? -eq 0 ]; then
            echo "[SUCCESS] Static library created: build/liboptionlib.a"
        else
            echo "[ERROR] Failed to create static library"
            exit 1
        fi
        ;;
    target)
        $COMPILER $FLAGS $COMMON_FLAGS -o "build/$TARGET" $LIB_SOURCES "src/$TARGET.c" $LINK_FLAGS

        if [ $? -eq 0 ]; then
            echo "[SUCCESS] Executable created in: build/$TARGET"
        else
            echo "[ERROR] Compilation failed"
            exit 1
        fi
        ;;
esac

