.PHONY: magic-gen
.PHONY: de-bruijn-gen

all:
	gcc -Wall -Wpedantic -Werror -Wno-parentheses -Ofast -std=c11 src/hyper-active.c src/bitboard-utility.c src/defines.c src/magic-bitboards.c src/move.c src/perft.c src/look-up-tables.c src/test-suite.c src/evaluate.c src/transposition-table.c src/move-ordering.c src/think.c src/game-analysis.c src/make-unmake.c src/evaluate-defines.c src/uci.c src/utils.c -o bin/hyper-active
	./bin/hyper-active

web:
	emcc -O3 -ffast-math src/wasm-main.c src/bitboard-utility.c src/defines.c src/magic-bitboards.c src/move.c src/perft.c src/look-up-tables.c src/test-suite.c src/evaluate.c src/transposition-table.c src/move-ordering.c src/think.c src/game-analysis.c src/make-unmake.c src/evaluate-defines.c src/uci.c src/utils.c -DWEB -o bin/hyper-active.js -s NO_EXIT_RUNTIME=1 -s EXPORTED_RUNTIME_METHODS=[ccall]

debug:
	gcc src/hyper-active.c src/bitboard-utility.c src/defines.c src/magic-bitboards.c src/move.c src/perft.c src/look-up-tables.c src/test-suite.c src/evaluate.c src/transposition-table.c src/move-ordering.c src/think.c src/game-analysis.c src/make-unmake.c src/evaluate-defines.c src/uci.c src/utils.c -g -o bin/hyper-active
	./bin/hyper-active

magic-gen:
	gcc -oFast src/magic-gen.c src/bitboard-utility.c src/magic-bitboards.c -o bin/magic-gen
	./bin/magic-gen

de-bruijn-gen:
	gcc -oFast de-bruijn-gen/de-bruijn-sequence-gen.c -o bin/de-bruijn-sequence-gen
	./bin/de-bruijn-sequence-gen