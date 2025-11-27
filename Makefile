# Aria_lang/Makefile

CC = gcc

# Optimization flags are critical for AI tensor operations and HTTP parsing.
# -march=native allows autovectorization for AVX/SSE instructions, speeding up 
# string manipulation in the web server and tensor math in the AI module.
# -D_GNU_SOURCE is required for advanced socket options (SO_REUSEPORT) and threading.
CFLAGS = -Wall -Wextra -Ofast -march=native -g -std=c99 -D_GNU_SOURCE

# Assembler configuration for low-level optimizations if needed
NASM = nasm
ASMFLAGS = -f elf64

# Libraries required for consciousness computing with secure web server
LIBS = -lm -lpthread -ldl -lssh

SRC = src
BIN = bin
LIB = lib

# Default target builds the preprocessor, compiler, and the static runtime library
all: dirs $(BIN)/preprocessor $(BIN)/aria_compiler $(LIB)/libaria.a

dirs:
	@mkdir -p $(BIN) $(LIB)

# Preprocessor build step
PRE_SRC = $(wildcard $(SRC)/preprocessor/*.c)
$(BIN)/preprocessor: $(PRE_SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Compiler build step
# Includes lexer, parser, AST arena, and codegen backend
COMP_SRC = $(SRC)/main.c \
           $(SRC)/frontend/lexer.c \
           $(SRC)/frontend/parser.c \
           $(SRC)/frontend/arena.c \
           $(SRC)/backend/codegen.c

$(BIN)/aria_compiler: $(COMP_SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Runtime Library Build Step (MINIMAL VERSION)
# Only includes essential runtime components and basic stdlib modules
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
         $(SRC)/stdlib/fetch.c

RT_OBJ = $(RT_SRC:.c=.o)

$(LIB)/libaria.a: $(RT_OBJ)
	ar rcs $@ $^

# Generic rule for compiling C objects
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup artifacts
clean:
	rm -rf $(BIN) $(LIB) $(SRC)/runtime/*.o $(SRC)/stdlib/*.o

.PHONY: all clean dirs
