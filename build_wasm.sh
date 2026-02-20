#!/bin/bash

if [ -z "$EMSDK" ]; then
    echo "[INFO] Loading Emscripten SDK"
    source ~/Developer/emsdk/emsdk_env.sh
fi

BUILD_MODE="release"
OUTPUT_DIR="wasm"
OUTPUT_NAME="optionlib"

LIB_SOURCES=$(find src -name "*.c" -type f)

if [ -z "$LIB_SOURCES" ]; then
    echo "[ERROR] No .c files found in the /src directory"
    exit 1
fi

for arg in "$@"; do
    case $arg in
        debug|release)
            BUILD_MODE="$arg"
            ;;
        *)
            echo "Unknown Argument: $arg"
            echo "Usage: $0 [debug|release]"
            exit 1
            ;;
    esac
done

if [ "$BUILD_MODE" = "debug" ]; then
    FLAGS="-g -O0"
else
    FLAGS="-O3 -ffast-math"
fi

mkdir -p $OUTPUT_DIR

EXPORTED_FUNCTIONS='[
    "_black_scholes_call",
    "_black_scholes_put",
    "_black_scholes_call_with_dividend",
    "_black_scholes_put_with_dividend",
    "_binomial_tree_call_european",
    "_binomial_tree_put_european",
    "_binomial_tree_call_european_with_dividend",
    "_binomial_tree_put_european_with_dividend",
    "_binomial_tree_call_american",
    "_binomial_tree_put_american",
    "_binomial_tree_call_american_with_dividend",
    "_binomial_tree_put_american_with_dividend"
]'

emcc $FLAGS \
    -Iinclude \
    $LIB_SOURCES \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS="$EXPORTED_FUNCTIONS" \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="OptionLib" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -o "$OUTPUT_DIR/$OUTPUT_NAME.js"

if [ $? -eq 0 ]; then
    echo "[SUCCESS] WASM build complete!"
    echo "[INFO] $OUTPUT_DIR/$OUTPUT_NAME.js"
    echo "[INFO] $OUTPUT_DIR/$OUTPUT_NAME.wasm"
else
    echo "[ERROR] WASM compilation failed"
    exit 1
fi

