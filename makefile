CC = clang

CC_FLAGS_FILE = compile_flags.txt
CC_FLAGS = $(shell cat $(CC_FLAGS_FILE))
CC_FLAGS_DEBUG_MODE = -ggdb3 -O0
CC_FLAGS_RELEASE_MODE = -ggdb3 -Ofast -flto -Wl,-O3 -fvisibility=hidden

CC_FLAGS_DEBUG = $(CC_FLAGS_DEBUG_MODE) $(CC_FLAGS)
CC_FLAGS_RELEASE = $(CC_FLAGS_RELEASE_MODE) $(CC_FLAGS)
CC_FLAGS_SANITIZE = $(CC_FLAGS_DEBUG) -fsanitize=address
CC_FLAGS_PROFILE = $(CC_FLAGS_RELEASE) -pg -a

SRCS = src/main.c
SRCS += $(wildcard src/common/*.c)
SRCS += $(wildcard src/gfx/*.c)
SRCS += $(wildcard src/rt/*.c)
SRCS += $(wildcard src/rt/accelerators/*.c)
SRCS += $(wildcard src/world/*.c)

BIN_DIR = bin
DEBUG_FNAME 	:= rtdbg.out
RELEASE_FNAME 	:= rt.out
SANITIZE_FNAME 	:= rtsan.out
PROFILE_FNAME 	:= rtprof.out

all: debug release

debug: bin_dir
	clang -o $(BIN_DIR)/$(DEBUG_FNAME) $(CC_FLAGS_DEBUG) $(SRCS)
	chmod +x $(BIN_DIR)/$(DEBUG_FNAME)

release: bin_dir
	clang -o $(BIN_DIR)/$(RELEASE_FNAME) $(CC_FLAGS_RELEASE) $(SRCS)
	chmod +x $(BIN_DIR)/$(RELEASE_FNAME)

sanitize: bin_dir
	clang -o $(BIN_DIR)/$(SANITIZE_FNAME) $(CC_FLAGS_SANITIZE) $(SRCS)
	chmod +x $(BIN_DIR)/$(SANITIZE_FNAME)

profile: bin_dir
	clang -o $(BIN_DIR)/$(PROFILE_FNAME) $(CC_FLAGS_PROFILE) $(SRCS)
	chmod +x $(BIN_DIR)/$(PROFILE_FNAME)

clean:
	rm -rf $(BIN_DIR)

bin_dir:
	mkdir -p $(BIN_DIR)
