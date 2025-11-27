/* Aria_lang/src/stdlib/alt_bases.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define TAG_INTEGER     (0xFFF8000000000000ULL | 4ULL)
#define TAG_STRING      (0xFFF8000000000000ULL | 5ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }

static const char DIGITS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Convert Int to String in Base N
void* base_to_str(void* num_tagged, void* base_tagged) {
    int32_t num = unbox_int((Value)num_tagged);
    int32_t base = unbox_int((Value)base_tagged);
    
    if (base < 2 |

| base > 36) return (void*)box_ptr("", TAG_STRING);
    
    char buf;
    int i = 0;
    uint32_t n = (num < 0)? -num : num;
    
    if (n == 0) {
        char* r = aria_alloc(2);
        r = '0'; r[1] = 0;
        return (void*)box_ptr(r, TAG_STRING);
    }
    
    while (n > 0) {
        buf[i++] = DIGITS[n % base];
        n /= base;
    }
    if (num < 0) buf[i++] = '-';
    buf[i] = '\0';
    
    // Reverse
    char* res = aria_alloc(i + 1);
    for(int j=0; j<i; j++) res[j] = buf[i-j-1];
    res[i] = 0;
    return (void*)box_ptr(res, TAG_STRING);
}

// Convert String Base N to Int
void* base_from_str(void* str_tagged, void* base_tagged) {
    char* s = unbox_str((Value)str_tagged);
    int32_t base = unbox_int((Value)base_tagged);
    if (!s |

| base < 2 |
| base > 36) return (void*)box_int(0);
    
    long val = strtol(s, NULL, base);
    return (void*)box_int((int32_t)val);
}
