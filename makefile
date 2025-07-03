CC=gcc
CFLAGS=-Wall -Werror -Wpedantic -Wno-parentheses -O3 -ffast-math -std=c11
WEBCFLAGS=-Wall -Werror -Wpedantic -Wno-parentheses -O3 -ffast-math -std=gnu23

.PHONY: magic-gen
.PHONY: de-bruijn-gen

SRCS := src/hyper-active.c \
		src/bitboard-utility.c \
		src/defines.c \
		src/magic-bitboards.c \
		src/move.c \
		src/perft.c \
		src/look-up-tables.c \
		src/test-suite.c \
		src/evaluate.c \
		src/transposition-table.c \
		src/move-ordering.c \
		src/think.c \
		src/make-unmake.c \
		src/evaluate-defines.c \
		src/uci.c \
		src/utils.c
NAME := bin/hyper-active

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)
	./$(NAME)

web:
	emcc $(WEBCFLAGS) $(SRCS) -DWEB -o $(NAME).js \
		-s NO_EXIT_RUNTIME=1 \
		-s ASYNCIFY=1 \
		-s EXPORTED_RUNTIME_METHODS=[ccall,stringToNewUTF8] \
		-s WASM=1

debug:
	$(CC) $(CFLAGS) $(SRCS) -DDEBUG -g -o $(NAME)
	./$(NAME)

magic-gen:
	$(CC) $(CFLAGS) src/magic-gen.c src/bitboard-utility.c src/magic-bitboards.c -o bin/magic-gen
	./bin/magic-gen

de-bruijn-gen:
	$(CC) $(CFLAGS) de-bruijn-gen/de-bruijn-sequence-gen.c -o bin/de-bruijn-sequence-gen
	./bin/de-bruijn-sequence-gen
