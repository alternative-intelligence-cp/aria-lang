/* Aria_lang/src/stdlib/std_ioe.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

typedef uint64_t Value;
#define PTR_MASK 0x0000FFFFFFFFFFFFULL
#define TAG_STRING (0xFFF8000000000000ULL | 5ULL)

extern void* aria_alloc(size_t size);
static inline Value box_str(char* s) { return TAG_STRING | (uintptr_t)s; }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }

// Standard error: normal standard error
void err_print(void* s_t) {
    char* s = unbox_str((Value)s_t);
    if(s) fprintf(stderr, "%s", s);
}

// Standard dataIn: a separate input path for data (FD 3)
void* data_in() {
    // Attempt to read from file descriptor 3, or stdin fallback
    char buffer[4096];
    ssize_t n = read(3, buffer, 4095);
    if (n < 0) n = 0; // If FD 3 isn't open, just return empty
    buffer[n] = '\0';
    
    char* res = aria_alloc(n + 1);
    memcpy(res, buffer, n + 1);
    return (void*)box_str(res);
}

// Standard dataOut: a separate output path for data (FD 4)
void data_out(void* s_t) {
    char* s = unbox_str((Value)s_t);
    if (!s) return;
    // Attempt write to FD 4
    if (write(4, s, strlen(s)) < 0) {
        // Fallback or ignore if not piped
    }
}

// Standard debug: a separate path for debugging info (stderr usually, or FD 5)
void debug_log(void* s_t) {
    char* s = unbox_str((Value)s_t);
    if (!s) return;
    fprintf(stderr, "[DEBUG] %s\n", s);
}
