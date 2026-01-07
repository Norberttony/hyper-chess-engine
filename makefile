CC := gcc
CFLAGS := -Wall -Werror -Wpedantic -Wno-parentheses -O3 -ffast-math -std=c11 -flto
WEBCC := emcc
WEBCFLAGS := -Wall -Werror -Wpedantic -Wno-parentheses -O3 -ffast-math -std=gnu23
LDLIBS := -lm

WEB_OBJ_DIR := web-obj
OBJ_DIR := obj
SRC_DIR := src

# build a list of src files (SRCS), obj files (OBJS), and the target name.
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) 
WEB_OBJS := $(SRCS:$(SRC_DIR)/%.c=$(WEB_OBJ_DIR)/%.o)
NAME := bin/hyper-active

.PHONY: magic-gen de-bruijn-gen run all debug web clean profile

# default build, a binary executable.
all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDLIBS)

debug: CFLAGS += -DDEBUG -g
debug: all

profile-debug: CFLAGS += -DDEBUG -g
profile-debug: profile

$(OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

profile:
	$(CC) $(CFLAGS) -fprofile-generate $(SRCS) -o $(NAME) $(LDLIBS)
	./$(NAME) < bench.txt
	$(CC) $(CFLAGS) -fprofile-use $(SRCS) -o $(NAME) $(LDLIBS)
	rm -f bin/*.gcda

# web build
web: $(WEB_OBJS)
	$(WEBCC) $(WEBCFLAGS) $(WEB_OBJS) -DWEB -o $(NAME).js \
		-s NO_EXIT_RUNTIME=1 \
		-s ASYNCIFY=1 \
		-s EXPORTED_RUNTIME_METHODS=[ccall,stringToNewUTF8] \
		-s WASM=1

$(WEB_OBJS): $(WEB_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(WEBCC) -c $(WEBCFLAGS) $^ -o $@


clean:
	rm -f $(OBJS)
	rm -f $(WEB_OBJS)
	rm -f $(NAME)
	rm -f $(NAME).js
	rm -f $(NAME).wasm
	rm -f bin/*.gcda

magic-gen:
	$(CC) $(CFLAGS) magic-gen/magic-gen.c $(SRC_DIR)/bitboard-utility.c $(SRC_DIR)/magic-bitboards.c -o bin/magic-gen
	./bin/magic-gen

de-bruijn-gen:
	$(CC) $(CFLAGS) de-bruijn-gen/de-bruijn-sequence-gen.c -o bin/de-bruijn-sequence-gen
	./bin/de-bruijn-sequence-gen
