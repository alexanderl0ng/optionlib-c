CC   			:= clang
EMCC 			:= emcc
SRC_DIR			:= src
TEST_DIR		:= tests
SRCS 			:= $(wildcard $(SRC_DIR)/*.c)
OBJS_DEBUG		:= $(patsubst $(SRC_DIR)/%.c, build/debug/%.o,   $(SRCS))
OBJS_RELEASE	:= $(patsubst $(SRC_DIR)/%.c, build/release/%.o, $(SRCS))

COMMON_FLAGS	:= -Wall -Werror -Wextra -Iinclude
DEBUG_FLAGS		:= $(COMMON_FLAGS) -g -O0 -DDEBUG
RELEASE_FLAGS	:= $(COMMON_FLAGS) -O2 -DNDEBUG
WASM_FLAGS		:= $(COMMON_FLAGS) -O3 -ffast-math					\
				   -s WASM=1										\
				   -s MODULARIZE=1									\
				   -s EXPORT_NAME="OptionLib"						\
				   -s ALLOW_MEMORY_GROWTH=1							\
				   -s EXPORTED_FUNCTIONS='[							\
				       "_black_scholes_call",						\
				       "_black_scholes_put",						\
				       "_black_scholes_call_with_dividend",			\
				       "_black_scholes_put_with_dividend",			\
				       "_binomial_tree_call_european",				\
				       "_binomial_tree_put_european",				\
				       "_binomial_tree_call_european_with_dividend",\
				       "_binomial_tree_put_european_with_dividend",	\
				       "_binomial_tree_call_american",				\
				       "_binomial_tree_put_american",				\
				       "_binomial_tree_call_american_with_dividend",\
				       "_binomial_tree_put_american_with_dividend"	\
				   ]'												\
				   -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'

.PHONY: all debug release wasm test-debug test-release run-test-debug run-test-release clean

all: debug release wasm

debug:   build/debug/liboptionlib.a
release: build/release/liboptionlib.a

build/debug/liboptionlib.a:   $(OBJS_DEBUG)   | build/debug/
	ar rcs $@ $^

build/release/liboptionlib.a: $(OBJS_RELEASE) | build/release/
	ar rcs $@ $^

build/debug/%.o:   $(SRC_DIR)/%.c | build/debug/
	$(CC) $(DEBUG_FLAGS) -c $< -o $@

build/release/%.o: $(SRC_DIR)/%.c | build/release/
	$(CC) $(RELEASE_FLAGS) -c $< -o $@

test-debug:   build/debug/liboptionlib.a | build/debug/
	$(CC) $(DEBUG_FLAGS) $(TEST_DIR)/tests.c -Lbuild/debug -loptionlib -lm -o build/debug/tests

test-release: build/release/liboptionlib.a | build/release/
	$(CC) $(RELEASE_FLAGS) -UNDEBUG $(TEST_DIR)/tests.c -Lbuild/release -loptionlib -lm -o build/release/tests

run-test-debug:   test-debug
	./build/debug/tests

run-test-release: test-release
	./build/release/tests

wasm: | build/wasm/
	source ~/Developer/emsdk/emsdk_env.sh && \
	$(EMCC) $(WASM_FLAGS) $(SRCS) -o build/wasm/optionlib.js

build/debug/:
	mkdir -p $@

build/release/:
	mkdir -p $@

build/wasm/:
	mkdir -p $@

clean:
	rm -rf build
