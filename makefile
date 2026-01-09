CFLAGS := -Wall -Werror -Wpedantic -Wno-parentheses -O3 -ffast-math -flto
LDFLAGS :=
LDLIBS := -lm

OBJ_DIR := obj
SRC_DIR := src
BIN_DIR := bin

NAME := $(BIN_DIR)/hyper-active
DEBUG ?= 0

# Determine the OS this is compiling on
HOST_OS :=
ifeq ($(OS),Windows_NT)
	HOST_OS := windows
else
	HOST_OS := linux
endif

TARGET ?= $(HOST_OS)

# Choose default compiler based on the target
ifeq ($(TARGET),web)
	override CC := emcc
	LDFLAGS += \
		-sNO_EXIT_RUNTIME=1 \
		-sASYNCIFY=1 \
		-sEXPORTED_RUNTIME_METHODS=[ccall,stringToNewUTF8] \
		-sWASM=1
	NAME := $(NAME).js
	OBJ_DIR := $(OBJ_DIR)/web
else ifeq ($(TARGET),windows)
	NAME := $(NAME).exe
else ifeq ($(TARGET),linux)
endif

# EMCC for some reason requires the gnu23 standard when compiling
ifeq ($(TARGET),web)
	CFLAGS += -std=gnu23
else
	CFLAGS += -std=c11
endif

ifneq ($(DEBUG),0)
	CFLAGS += -DDEBUG
endif

# build a list of src files (SRCS), obj files (OBJS), and the target name.
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean profile

all: build_dir $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDLIBS) $(LDFLAGS)

$(OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

profile: build_dir
	$(CC) $(CFLAGS) -fprofile-generate $(SRCS) -o $(NAME) $(LDLIBS) $(LDFLAGS)
	./$(NAME) < bench.txt
	$(CC) $(CFLAGS) -fprofile-use $(SRCS) -o $(NAME) $(LDLIBS) $(LDFLAGS)
	rm -f $(BIN_DIR)/*.gcda

build_dir:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

clean:
	rm -rf $(OBJS) $(OBJ_DIR)/web $(BIN_DIR)/* $(BIN_DIR)/*.gcda
