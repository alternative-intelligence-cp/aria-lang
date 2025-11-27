/* Aria_lang/src/main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "frontend/ast.h"
#include "runtime/bundler.h"

// Defined in parser.c / lexer.c
extern AstNode* parse_program(AstArena* arena);
extern void init_lexer(const char* source);

// Defined in codegen.c
extern void gen_program(AstNode* head);
extern FILE* asm_out;

char* read_entire_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = malloc(length + 1);
    if (!buffer) { fclose(f); return NULL; }
    
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    return buffer;
}

void replace_extension(const char* filename, const char* new_ext, char* out_buf, size_t size) {
    const char* dot = strrchr(filename, '.');
    if (!dot) {
        snprintf(out_buf, size, "%s%s", filename, new_ext);
    } else {
        size_t base_len = dot - filename;
        snprintf(out_buf, size, "%.*s%s", (int)base_len, filename, new_ext);
    }
}

// Helper to safely execute a subprocess without shell injection
int run_command(char* const* argv) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Child process
        execv(argv[0], argv);
        perror("execv"); // execv only returns on error
        fprintf(stderr, "Failed to run: %s\n", argv[0]);
        exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0? 0 : 1;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: aria_compiler <input.aria> [--asm-only]\n");
        return 1;
    }

    // 0. Initialize Tool Bundler (Extract nasm/cc/libaria)
    if (!bundler_init()) {
        fprintf(stderr, "Fatal: Failed to initialize bundled toolchain.\n");
        return 1;
    }

    const char* input_file = argv[1];
    int asm_only = (argc >= 3 && strcmp(argv[2], "--asm-only") == 0);

    // 1. Read Source
    char* source = read_entire_file(input_file);
    if (!source) {
        fprintf(stderr, "Error: Could not read file %s\n", input_file);
        return 1;
    }

    // 2. Setup Arena & Parse
    AstArena* arena = arena_create();
    init_lexer(source);
    AstNode* root = parse_program(arena);
    
    // 3. Determine Output Names
    char asm_file[256];
    char obj_file[256];
    char bin_file[256];
    
    replace_extension(input_file, ".asm", asm_file, sizeof(asm_file));
    replace_extension(input_file, ".o", obj_file, sizeof(obj_file));
    replace_extension(input_file, "", bin_file, sizeof(bin_file)); 

    // 4. Codegen
    asm_out = fopen(asm_file, "w");
    if (!asm_out) {
        fprintf(stderr, "Error: Could not open output file %s\n", asm_file);
        return 1;
    }
    
    gen_program(root);
    fclose(asm_out);
    
    arena_free(arena);
    free(source);
    
    printf("[Aria] Generated Assembly: %s\n", asm_file);

    if (asm_only) return 0;

    // 5. Assemble and Link (Secure Execution using Bundled Tools)
    
    // Step A: NASM
    char* nasm_cmd = (char*)bundler_get_nasm_path();
    char* nasm_args[] = { nasm_cmd, "-f", "elf64", asm_file, "-o", obj_file, NULL };
    
    printf("[Aria] Assembling with %s...\n", nasm_cmd);
    if (run_command(nasm_args) != 0) {
        fprintf(stderr, "[Aria] Assembler failed.\n");
        return 1;
    }

    // Step B: Linker (TCC/GCC)
    char* cc_cmd = (char*)bundler_get_cc_path();
    char* runtime_lib = (char*)bundler_get_runtime_path();
    
    // Tesla consciousness security: Include libssh for cryptographic operations
    // Note: -no-pie is standard for ASM linking on modern Linux
    char* link_args[] = {
        cc_cmd, 
        "-o", bin_file, 
        obj_file, 
        runtime_lib,
        "-lm", "-lpthread", "-ldl", "-lssh", 
        NULL
    };

    printf("[Aria] Linking with %s...\n", cc_cmd);
    if (run_command(link_args) != 0) {
        fprintf(stderr, "[Aria] Linker failed. Ensure dependencies are installed.\n");
        return 1;
    }

    // Cleanup object file
    remove(obj_file);

    printf("[Aria] Build Successful: %s\n", bin_file);
    return 0;
}
