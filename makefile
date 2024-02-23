debug:
	gcc hyper-active.c -o hyper-active
all:
	gcc -oFast hyper-active.c -o hyper-active

magic-gen:
	gcc -oFast magic-gen.c bitboard-utility.c -o magic-gen
	./magic-gen