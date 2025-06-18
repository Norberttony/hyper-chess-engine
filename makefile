CC=gcc
CFLAGS=-Wall -Wpedantic -Wno-parentheses -O3 -ffast-math -std=c11

.PHONY: magic-gen
.PHONY: de-bruijn-gen

MAIN_SRCS := src/bitboard-utility.c src/defines.c src/magic-bitboards.c src/move.c src/perft.c src/look-up-tables.c src/test-suite.c src/evaluate.c src/transposition-table.c src/move-ordering.c src/think.c src/make-unmake.c src/evaluate-defines.c src/uci.c src/utils.c
MAIN_NAME := bin/hyper-active

all:
	$(CC) $(CFLAGS) src/hyper-active.c $(MAIN_SRCS) -o $(MAIN_NAME)
	./$(MAIN_NAME)

web:
	emcc $(CFLAGS) src/wasm-main.c $(MAIN_SRCS) -DWEB -o bin/hyper-active.js -s NO_EXIT_RUNTIME=1 -s EXPORTED_RUNTIME_METHODS=[ccall]

debug:
	$(CC) $(CFLAGS) -DDEBUG src/hyper-active.c $(MAIN_SRCS) -g -o $(MAIN_NAME)
	./$(MAIN_NAME)

magic-gen:
	$(CC) $(CFLAGS) src/magic-gen.c src/bitboard-utility.c src/magic-bitboards.c -o bin/magic-gen
	./bin/magic-gen

de-bruijn-gen:
	$(CC) $(CFLAGS) de-bruijn-gen/de-bruijn-sequence-gen.c -o bin/de-bruijn-sequence-gen
	./bin/de-bruijn-sequence-gen
