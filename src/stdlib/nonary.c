/* Aria_lang/src/stdlib/nonary.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_INTEGER     (TAG_BASE | 4ULL)
#define TAG_STRING      (TAG_BASE | 5ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

// Logic Values:
// -4: NEITHER
// -3: DEF_FALSE (Definitely False)
// -2: PROB_FALSE (Probably False)
// -1: MAYBE_FALSE
//  0: UNSURE
//  1: MAYBE_TRUE
//  2: PROB_TRUE (Probably True)
//  3: DEF_TRUE (Definitely True)
//  4: BOTH

static int32_t clamp_nonary(int32_t v) {
    if (v < -4) return -4;
    if (v > 4) return 4;
    return v;
}

void* non_new(void* val_tagged) {
    int32_t v = unbox_int((Value)val_tagged);
    return (void*)box_int(clamp_nonary(v));
}

void* non_and(void* a_tagged, void* b_tagged) {
    int32_t a = unbox_int((Value)a_tagged);
    int32_t b = unbox_int((Value)b_tagged);
    // Min(a, b) logic
    return (void*)box_int((a < b) ? a : b);
}

void* non_or(void* a_tagged, void* b_tagged) {
    int32_t a = unbox_int((Value)a_tagged);
    int32_t b = unbox_int((Value)b_tagged);
    // Max(a, b) logic
    return (void*)box_int((a > b) ? a : b);
}

void* non_not(void* a_tagged) {
    int32_t a = unbox_int((Value)a_tagged);
    // Negation logic
    return (void*)box_int(-a);
}

void* non_consensus(void* list_tagged) {
    // Requires access to the list structure, but assuming this is 
    // passed as a vararg or simple list would require importing list internals.
    // For simplicity in this primitive version, we simply return UNSURE (0).
    // Real implementation would average the values.
    return (void*)box_int(0); 
}

void* non_to_string(void* val_tagged) {
    int32_t v = unbox_int((Value)val_tagged);
    char* s;
    switch(v) {
        case -4: s = "NEITHER"; break;
        case -3: s = "DEF_FALSE"; break;
        case -2: s = "PROB_FALSE"; break;
        case -1: s = "MAYBE_FALSE"; break;
        case  0: s = "UNSURE"; break;
        case  1: s = "MAYBE_TRUE"; break;
        case  2: s = "PROB_TRUE"; break;
        case  3: s = "DEF_TRUE"; break;
        case  4: s = "BOTH"; break;
        default: s = "UNKNOWN"; break;
    }
    
    size_t len = strlen(s);
    char* res = (char*)aria_alloc(len + 1);
    memcpy(res, s, len + 1);
    return (void*)box_ptr(res, TAG_STRING);
}

// Check if value leans True (Threshold > 0)
void* non_is_true(void* val_tagged) {
    int32_t v = unbox_int((Value)val_tagged);
    return (void*)(v > 0 ? (TAG_BASE | 3ULL) : (TAG_BASE | 2ULL)); // Return Boolean
}
