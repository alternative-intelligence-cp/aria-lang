# Aria_lang/Makefile

CC = gcc

# Optimization flags are critical for AI tensor operations and HTTP parsing.
# -D_GNU_SOURCE is required for advanced socket options (SO_REUSEPORT) and threading.
# -DARIA_BUNDLE_TOOLS enables the embedding logic in bundler.c
CFLAGS = -Wall -Wextra -Ofast -march=native -g -std=c99 -D_GNU_SOURCE -DARIA_BUNDLE_TOOLS -Isrc/runtime

# Assembler configuration for low-level optimizations if needed
NASM = nasm
ASMFLAGS = -f elf64

# Libraries required for consciousness computing with secure web server
LIBS = -lm -lpthread -ldl -lssh

SRC = src
BIN = bin
LIB = lib
TOOLS = tools

# Default target builds the preprocessor, compiler, and the static runtime library
all: dirs $(BIN)/bin2c $(LIB)/libaria.a gen_blobs $(BIN)/preprocessor $(BIN)/aria_compiler

dirs:
	@mkdir -p $(BIN) $(LIB) $(TOOLS)

# --- Tool Bundling Logic ---

# compile helper tool
$(BIN)/bin2c: utils/bin2c.c
	@mkdir -p utils
	$(CC) utils/bin2c.c -o $(BIN)/bin2c

# Generate C headers from binaries
gen_blobs: $(LIB)/libaria.a
	@echo "Bundling Tools..."
	@# If tools/nasm exists, bundle it, else warn
	@if [ -f $(TOOLS)/nasm ]; then \
		$(BIN)/bin2c $(TOOLS)/nasm nasm_blob > $(SRC)/runtime/nasm_blob.h; \
	else \
		echo "Warning: tools/nasm not found. Bundler will assume system PATH."; \
		echo "const unsigned char nasm_blob[] = {0}; const unsigned int nasm_blob_len = 0;" > $(SRC)/runtime/nasm_blob.h; \
	fi
	@# If tools/tcc exists, bundle it
	@if [ -f $(TOOLS)/tcc ]; then \
		$(BIN)/bin2c $(TOOLS)/tcc tcc_blob > $(SRC)/runtime/tcc_blob.h; \
	else \
		echo "Warning: tools/tcc not found. Bundler will assume system PATH."; \
		echo "const unsigned char tcc_blob[] = {0}; const unsigned int tcc_blob_len = 0;" > $(SRC)/runtime/tcc_blob.h; \
	fi
	@# Bundle the runtime library we just built
	$(BIN)/bin2c $(LIB)/libaria.a libaria_blob > $(SRC)/runtime/libaria_blob.h

# --- Standard Build Steps ---

# Preprocessor build step
PRE_SRC = $(wildcard $(SRC)/preprocessor/*.c)
$(BIN)/preprocessor: $(PRE_SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Compiler build step
# Includes lexer, parser, AST arena, codegen backend, and the new BUNDLER
COMP_SRC = $(SRC)/main.c \
           $(SRC)/frontend/lexer.c \
           $(SRC)/frontend/parser.c \
           $(SRC)/frontend/arena.c \
           $(SRC)/backend/codegen.c \
           $(SRC)/runtime/bundler.c

$(BIN)/aria_compiler: $(COMP_SRC) gen_blobs
	$(CC) $(CFLAGS) -o $@ $(filter %.c, $(COMP_SRC))

# Runtime Library Build Step
RT_SRC = $(wildcard $(SRC)/runtime/*.c) \
         $(SRC)/stdlib/math.c \
         $(SRC)/stdlib/std_io.c \
         $(SRC)/stdlib/string_utils.c \
         $(SRC)/stdlib/clock.c \
         $(SRC)/stdlib/nonary.c \
         $(SRC)/stdlib/quinary.c \
         $(SRC)/stdlib/io.c \
         $(SRC)/stdlib/dynamic.c \
         $(SRC)/stdlib/dataStructures.c \
         $(SRC)/stdlib/algorithms.c \
         $(SRC)/stdlib/threads.c \
         $(SRC)/stdlib/processes.c \
         $(SRC)/stdlib/terminal.c \
         $(SRC)/stdlib/fs.c \
         $(SRC)/stdlib/mouse.c \
         $(SRC)/stdlib/unix.c \
         $(SRC)/stdlib/linux.c \
         $(SRC)/stdlib/2d_drawing.c \
         $(SRC)/stdlib/3d_drawing.c \
         $(SRC)/stdlib/web.c \
         $(SRC)/stdlib/database.c \
         $(SRC)/stdlib/ai.c \
         $(SRC)/stdlib/tui_components.c \
         $(SRC)/stdlib/gui_components.c \
         $(SRC)/stdlib/window.c \
         $(SRC)/stdlib/fetch.c \
         $(SRC)/stdlib/sound.c \
         $(SRC)/stdlib/gamepad.c \
         $(SRC)/stdlib/ffi.c \
         $(SRC)/stdlib/std_ioe.c \
         $(SRC)/stdlib/atomics.c

# Exclude bundler.c from the runtime library (it belongs to the compiler host)
RT_SRC_FILTERED = $(filter-out $(SRC)/runtime/bundler.c, $(RT_SRC))
RT_OBJ = $(RT_SRC_FILTERED:.c=.o)

$(LIB)/libaria.a: $(RT_OBJ)
	ar rcs $@ $^

# Generic rule for compiling C objects
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup artifacts
clean:
	rm -rf $(BIN) $(LIB) $(SRC)/runtime/*.o $(SRC)/stdlib/*.o $(SRC)/runtime/*_blob.h utils/bin2c

.PHONY: all clean dirs gen_blobs
