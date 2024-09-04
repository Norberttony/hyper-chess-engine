.PHONY: magic-gen
.PHONY: de-bruijn-gen

all:
	gcc -Ofast src/hyper-active.c src/bitboard-utility.c src/defines.c src/magic-bitboards.c src/move.c src/perft.c src/look-up-tables.c src/test-suite.c src/evaluate.c src/transposition-table.c src/move-ordering.c src/think.c src/game-analysis.c -o bin/hyper-active
	./bin/hyper-active

debug:
	gcc src/hyper-active.c src/bitboard-utility.c src/defines.c src/magic-bitboards.c src/move.c src/perft.c src/look-up-tables.c src/test-suite.c src/evaluate.c src/transposition-table.c src/move-ordering.c src/think.c src/game-analysis.c -g -o bin/hyper-active
	./bin/hyper-active

magic-gen:
	gcc -oFast src/magic-gen.c src/bitboard-utility.c src/magic-bitboards.c -o bin/magic-gen
	./bin/magic-gen

de-bruijn-gen:
	gcc -oFast de-bruijn-gen/de-bruijn-sequence-gen.c -o bin/de-bruijn-sequence-gen
	./bin/de-bruijn-sequence-gen