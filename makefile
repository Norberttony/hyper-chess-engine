.PHONY: magic-gen
.PHONY: de-bruijn-gen

debug:
	gcc src/hyper-active.c src/bitboard-utility.c src/defines.c src/magic-bitboards.c src/move.c src/perft.c src/look-up-tables.c -o bin/hyper-active
all:
	gcc -oFast src/hyper-active.c src/bitboard-utility.c src/defines.c src/magic-bitboards.c src/move.c src/perft.c src/look-up-tables.c -o bin/hyper-active

magic-gen:
	gcc -oFast src/magic-gen.c src/bitboard-utility.c src/magic-bitboards.c -o bin/magic-gen
	./bin/magic-gen

de-bruijn-gen:
	gcc -oFast de-bruijn-gen/de-bruijn-sequence-gen.c -o bin/de-bruijn-sequence-gen
	./bin/de-bruijn-sequence-gen