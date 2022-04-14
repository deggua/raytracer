CC = clang

CC_FLAGS_FILE = compile_flags.txt
CC_FLAGS = $(shell cat $(CC_FLAGS_FILE))
CC_FLAGS_DEBUG_MODE = -ggdb3 -O0
CC_FLAGS_RELEASE_MODE = -ggdb3 -Ofast -flto -Wl,-O3

CC_FLAGS_DEBUG = $(CC_FLAGS_DEBUG_MODE) $(CC_FLAGS)
CC_FLAGS_RELEASE = $(CC_FLAGS_RELEASE_MODE) $(CC_FLAGS)
CC_FLAGS_SANITIZE = $(CC_FLAGS_DEBUG) -fsanitize=address
CC_FLAGS_PROFILE = $(CC_FLAGS_RELEASE) -pg

SRCS = src/main.c
SRCS += src/gfx/camera.c src/gfx/color.c src/gfx/image.c src/gfx/primitives.c src/gfx/random.c src/gfx/renderer.c src/gfx/mesh.c
SRCS += src/object/object.c src/object/surfaces.c src/object/materials.c src/object/scene.c src/object/kdtree.c
SRCS += src/common/memory_arena.c

BIN_DIR = bin
RELEASE_DIR := $(BIN_DIR)/release
DEBUG_DIR := $(BIN_DIR)/debug
SANITIZE_DIR := $(BIN_DIR)/san
PROFILE_DIR := $(BIN_DIR)/profile

ELF_FILENAME = rt.out

DEBUG_OUTPUT := $(DEBUG_DIR)/$(ELF_FILENAME)
RELEASE_OUTPUT := $(RELEASE_DIR)/$(ELF_FILENAME)
SANITIZE_OUTPUT := $(SANITIZE_DIR)/$(ELF_FILENAME)
PROFILE_OUTPUT := $(PROFILE_DIR)/$(ELF_FILENAME)

all: debug release sanitize profile

debug:
	mkdir -p $(DEBUG_DIR)
	clang -o $(DEBUG_OUTPUT) $(CC_FLAGS_DEBUG) $(SRCS)
	chmod +x $(DEBUG_OUTPUT)

release:
	mkdir -p $(RELEASE_DIR)
	clang -o $(RELEASE_OUTPUT) $(CC_FLAGS_RELEASE) $(SRCS)
	chmod +x $(RELEASE_OUTPUT)

sanitize:
	mkdir -p $(SANITIZE_DIR)
	clang -o $(SANITIZE_OUTPUT) $(CC_FLAGS_SANITIZE) $(SRCS)
	chmod +x $(SANITIZE_OUTPUT)

profile:
	mkdir -p $(PROFILE_DIR)
	clang -o $(PROFILE_OUTPUT) $(CC_FLAGS_PROFILE) $(SRCS)
	chmod +x $(PROFILE_OUTPUT)

clean:
	rm -rf $(BIN_DIR)
