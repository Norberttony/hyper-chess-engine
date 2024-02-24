debug:
	gcc hyper-active.c -o bin/hyper-active
all:
	gcc -oFast hyper-active.c -o bin/hyper-active

magic-gen:
	gcc -oFast magic-gen.c bitboard-utility.c -o bin/magic-gen
	./bin/magic-gen

de-bruijn-gen:
	gcc -oFast de-bruijn-sequence-gen.c -o bin/de-bruijn-sequence-gen
	./bin/de-bruijn-sequence-gen