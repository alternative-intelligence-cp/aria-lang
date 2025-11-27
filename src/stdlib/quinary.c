/* Aria_lang/src/stdlib/quinary.c */
#include <stdint.h>
#include <stdlib.h>

typedef uint64_t Value;
#define QNAN_MASK 0x7FF8000000000000ULL
#define TAG_TRUE (QNAN_MASK | 0x8000000000000000ULL | 3ULL)
#define TAG_INTEGER (QNAN_MASK | 0x8000000000000000ULL | 4ULL)

static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

// Quinary logic: -2 (False), -1 (Prob False), 0 (Unknown), 1 (Prob True), 2 (True)

void* q_from_bool(void* b_tagged) {
    Value v = (Value)b_tagged;
    // If true (TAG_TRUE) -> 2, else -> -2
    // Note: This is simplistic. 
    return (void*)box_int((v == TAG_TRUE) ? 2 : -2);
}

void* q_and(void* a_tagged, void* b_tagged) {
    int32_t a = unbox_int((Value)a_tagged);
    int32_t b = unbox_int((Value)b_tagged);
    // Min(a, b)
    return (void*)box_int((a < b) ? a : b);
}

void* q_or(void* a_tagged, void* b_tagged) {
    int32_t a = unbox_int((Value)a_tagged);
    int32_t b = unbox_int((Value)b_tagged);
    // Max(a, b)
    return (void*)box_int((a > b) ? a : b);
}

void* q_not(void* a_tagged) {
    int32_t a = unbox_int((Value)a_tagged);
    return (void*)box_int(-a);
}

void* q_is_definitely(void* a_tagged) {
    int32_t a = unbox_int((Value)a_tagged);
    // Return bool (Tagged)
    return (void*)((a == 1 || a == 2) ? TAG_TRUE : (TAG_TRUE - 1)); // TAG_FALSE
}

void* q_is_possible(void* a_tagged) {
    int32_t a = unbox_int((Value)a_tagged);
    return (void*)((a > -2) ? TAG_TRUE : (TAG_TRUE - 1));
}
