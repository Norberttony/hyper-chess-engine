CFLAGS := -Wall -Werror -Wpedantic -Wno-parentheses -O3 -ffast-math -flto
LDFLAGS :=
LDLIBS := -lm

OBJ_DIR := obj
SRC_DIR := src
BIN_DIR := bin

MODULE_DIRS := \
	$(SRC_DIR)/debug \
	$(SRC_DIR)/eval \
	$(SRC_DIR)/movegen \
	$(SRC_DIR)/search \
	$(SRC_DIR)/uci \
	$(SRC_DIR)/app
PLATFORM_DIR := $(SRC_DIR)/platform

ADDITIONAL_SRCS :=

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

# Special flags for each target
ifeq ($(TARGET),web)
	override CC := emcc
	LDFLAGS += \
		-sNO_EXIT_RUNTIME=1 \
		-sASYNCIFY=1 \
		-sEXPORTED_RUNTIME_METHODS=[ccall,stringToNewUTF8] \
		-sWASM=1
	NAME := $(NAME).js
	OBJ_DIR := $(OBJ_DIR)/web
	ADDITIONAL_SRCS += $(PLATFORM_DIR)/platform_web.c
else ifeq ($(TARGET),windows)
	NAME := $(NAME).exe
	ADDITIONAL_SRCS += $(PLATFORM_DIR)/platform_win.c
else ifeq ($(TARGET),linux)
	ADDITIONAL_SRCS += $(PLATFORM_DIR)/platform_linux.c
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

SRCS := $(foreach dir,$(MODULE_DIRS),$(wildcard $(dir)/*.c)) $(ADDITIONAL_SRCS)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean profile

all: build_dir $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDLIBS) $(LDFLAGS)

$(OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
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
