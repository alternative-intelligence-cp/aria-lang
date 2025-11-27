/* Aria_lang/src/stdlib/std_io.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint64_t Value;
#define TAG_STRING (0xFFF8000000000000ULL | 5ULL)
#define PTR_MASK 0x0000FFFFFFFFFFFFULL
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }

void io_print(void* s_t) {
    char* s = unbox_str((Value)s_t);
    if(s) printf("%s", s);
}

void io_flush() { fflush(stdout); }
