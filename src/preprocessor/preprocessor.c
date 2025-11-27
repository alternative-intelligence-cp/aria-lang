/* Aria_lang/src/preprocessor/preprocessor.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "context_stack.h"

#define HASH_TABLE_SIZE 1024
#define MAX_RECURSION_DEPTH 64
#define MAX_INCLUDE_DEPTH 32

// --- Dynamic String Builder ---
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} StringBuilder;

void sb_init(StringBuilder* sb) {
    sb->capacity = 128;
    sb->length = 0;
    sb->data = malloc(sb->capacity);
    if (!sb->data) exit(1);
    sb->data[0] = '\0';
}

void sb_free(StringBuilder* sb) {
    free(sb->data);
}

void sb_append_char(StringBuilder* sb, char c) {
    if (sb->length + 2 >= sb->capacity) {
        sb->capacity *= 2;
        sb->data = realloc(sb->data, sb->capacity);
        if (!sb->data) exit(1);
    }
    sb->data[sb->length++] = c;
    sb->data[sb->length] = '\0';
}

void sb_append_str(StringBuilder* sb, const char* str) {
    if (!str) return;
    size_t len = strlen(str);
    if (sb->length + len + 1 >= sb->capacity) {
        while (sb->length + len + 1 >= sb->capacity) {
            sb->capacity *= 2;
        }
        sb->data = realloc(sb->data, sb->capacity);
        if (!sb->data) exit(1);
    }
    memcpy(sb->data + sb->length, str, len);
    sb->length += len;
    sb->data[sb->length] = '\0';
}

void sb_reset(StringBuilder* sb) {
    sb->length = 0;
    sb->data[0] = '\0';
}

// --- Data Structures ---

typedef struct {
    char* name;
    char** lines;
    int line_count;
    int arg_count; 
    char** args; 
} Macro;

typedef struct MacroNode {
    Macro data;
    struct MacroNode* next;
} MacroNode;

MacroNode* macro_table[HASH_TABLE_SIZE];
ContextStack* ctx_stack;

// --- UTILS ---

char* safe_strdup(const char* s) {
    if (!s) return NULL;
    return strdup(s);
}

unsigned int hash(const char* key) {
    unsigned int hash = 0;
    while (*key) {
        hash += *key;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        key++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash % HASH_TABLE_SIZE;
}

void add_macro(char* name, char** lines, int count, int arg_count) {
    if (!name) return; 
    unsigned int idx = hash(name);
    MacroNode* node = (MacroNode*)malloc(sizeof(MacroNode));
    node->data.name = safe_strdup(name); 
    node->data.lines = lines;
    node->data.line_count = count;
    node->data.arg_count = arg_count;
    node->next = macro_table[idx];
    macro_table[idx] = node;
}

Macro* find_macro(const char* name) {
    if (!name) return NULL;
    unsigned int idx = hash(name);
    MacroNode* current = macro_table[idx];
    while (current) {
        if (strcmp(current->data.name, name) == 0) {
            return &current->data;
        }
        current = current->next;
    }
    return NULL;
}

void cleanup_macros() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        MacroNode* current = macro_table[i];
        while (current) {
            MacroNode* next = current->next;
            free(current->data.name);
            for(int j=0; j<current->data.line_count; j++) {
                free(current->data.lines[j]);
            }
            free(current->data.lines);
            free(current);
            current = next;
        }
    }
}

// Expands %$ and $1, $2
void expand_line(StringBuilder* sb, const char* src, int arg_count, char** args, int recursion_depth) {
    if (recursion_depth > MAX_RECURSION_DEPTH) {
        fprintf(stderr, "Error: Max macro recursion depth exceeded.\n");
        exit(1);
    }

    const char* r = src;
    ContextNode* ctx = ctx_stack_peek(ctx_stack);
    char temp_buf[256]; 

    while (*r) {
        // Context expansion %$
        if (*r == '%' && *(r + 1) == '$') {
            if (!ctx) {
                fprintf(stderr, "Error: %%$ used without active context\n");
                exit(1);
            }
            r += 2;
            snprintf(temp_buf, sizeof(temp_buf), "_ctx_%s_%u_", ctx->name, ctx->id);
            sb_append_str(sb, temp_buf);
        }
        // Argument expansion $1, $2, etc.
        else if (*r == '$' && isdigit(*(r + 1))) {
            r++;
            int arg_idx = *r - '1'; // $1 -> index 0
            r++;
            if (arg_idx >= 0 && arg_idx < arg_count) {
                sb_append_str(sb, args[arg_idx]);
            } else {
                fprintf(stderr, "Error: Macro argument $%d out of bounds\n", arg_idx + 1);
                exit(1);
            }
        }
        else {
            sb_append_char(sb, *r++);
        }
    }
}

void process_file(const char* filename, FILE* output, int include_depth) {
    if (include_depth > MAX_INCLUDE_DEPTH) {
        fprintf(stderr, "Error: Maximum include depth exceeded (circular dependency?)\n");
        exit(1);
    }

    FILE* input = fopen(filename, "r");
    if (!input) {
        fprintf(stderr, "Error: Failed to open file: %s\n", filename);
        exit(1);
    }

    char* line = NULL;
    size_t line_cap = 0;
    ssize_t line_len;

    int recording_macro = 0;
    char* current_macro_name = NULL;
    char** current_macro_lines = NULL;
    int current_macro_line_count = 0;
    int current_macro_capacity = 0;

    StringBuilder sb;
    sb_init(&sb);

    while ((line_len = getline(&line, &line_cap, input)) != -1) {
        if (line_len > 0 && line[line_len-1] == '\n') {
            line[line_len-1] = '\0';
        }

        char* token_scratch = strdup(line);
        if (!token_scratch) exit(1);

        // --- Handle %include "file" ---
        if (strncmp(line, "%include", 8) == 0) {
            char* ptr = line + 8;
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr == '"') {
                ptr++;
                char* end = strchr(ptr, '"');
                if (end) {
                    *end = '\0';
                    // Recursively process the included file
                    fprintf(output, "; --- BEGIN INCLUDE: %s ---\n", ptr);
                    process_file(ptr, output, include_depth + 1);
                    fprintf(output, "; --- END INCLUDE: %s ---\n", ptr);
                }
            }
            free(token_scratch);
            continue;
        }

        if (strncmp(line, "%macro", 6) == 0) {
            recording_macro = 1;
            char* token = strtok(token_scratch, " "); 
            token = strtok(NULL, " ");      
            if (token) {
                current_macro_name = safe_strdup(token); 
            }
            current_macro_capacity = 16;
            current_macro_lines = (char**)malloc(sizeof(char*) * current_macro_capacity);
            current_macro_line_count = 0;
            free(token_scratch);
            continue;
        }

        if (strncmp(line, "%endmacro", 9) == 0) {
            recording_macro = 0;
            if (current_macro_name) {
                add_macro(current_macro_name, current_macro_lines, current_macro_line_count, 0);
                free(current_macro_name);
                current_macro_name = NULL;
            }
            current_macro_lines = NULL; 
            free(token_scratch);
            continue;
        }

        if (recording_macro) {
            if (current_macro_name) {
                if (current_macro_line_count >= current_macro_capacity) {
                    current_macro_capacity *= 2;
                    current_macro_lines = realloc(current_macro_lines, sizeof(char*) * current_macro_capacity);
                }
                current_macro_lines[current_macro_line_count++] = safe_strdup(line);
            }
            free(token_scratch);
            continue;
        }

        if (strncmp(line, "%push", 5) == 0) {
            char* token = strtok(token_scratch, " ");
            token = strtok(NULL, " ");
            if (token) ctx_stack_push(ctx_stack, token);
            free(token_scratch);
            continue; 
        }

        if (strncmp(line, "%pop", 4) == 0) {
            ctx_stack_pop(ctx_stack);
            free(token_scratch);
            continue;
        }

        // Check for macro invocation
        char first_word[MAX_CTX_NAME];
        if (sscanf(line, "%63s", first_word) == 1) {
            Macro* m = find_macro(first_word);
            if (m) {
                char* args[10];
                int arg_count = 0;
                
                char* token = strtok(token_scratch, " "); // skip macro name
                while ((token = strtok(NULL, " ")) != NULL && arg_count < 10) {
                    args[arg_count++] = token;
                }

                for (int i = 0; i < m->line_count; i++) {
                    sb_reset(&sb);
                    expand_line(&sb, m->lines[i], arg_count, args, 0);
                    fprintf(output, "%s\n", sb.data);
                }
                free(token_scratch);
                continue;
            }
        }

        sb_reset(&sb);
        expand_line(&sb, line, 0, NULL, 0);
        fprintf(output, "%s\n", sb.data);
        free(token_scratch);
    }
    
    sb_free(&sb);
    free(line);
    fclose(input);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: preprocessor <input.aria> <output.aria.pp>\n");
        return 1;
    }
    
    FILE* out = fopen(argv[2], "w");
    if (!out) { perror("Failed to open output"); return 1; }
    
    ctx_stack = ctx_stack_create(16);
    
    process_file(argv[1], out, 0);
    
    cleanup_macros();
    ctx_stack_free(ctx_stack);
    fclose(out);
    return 0;
}
