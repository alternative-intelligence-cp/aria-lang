/*
 * Aria Binary-Safe String Implementation
 * 
 * Replaces null-terminated C strings with length-prefixed structures.
 * This ensures O(1) length access and support for binary data (0x00) 
 * required for WebSockets, Images, and Encryption.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);

// --- Tagging System (NaN Boxing) ---
typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_INTEGER     (TAG_BASE | 4ULL)
#define TAG_STRING      (TAG_BASE | 5ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

// --- String Structure ---
typedef struct {
    uint32_t length;
    char data[]; // Flexible array member
} AriaString;

// --- Boxing/Unboxing Helpers ---
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_str(AriaString* s) { return TAG_STRING | (uintptr_t)s; }
static inline AriaString* unbox_str(Value v) { return (AriaString*)(v & PTR_MASK); }

// --- String Operations ---

/* 
 * FIX 3.3: Binary Safe Constructor
 * Allocates string based on explicit length, not strlen.
 */
Value str_from_buffer(const char* buf, uint32_t len) {
    AriaString* s = (AriaString*)aria_alloc(sizeof(AriaString) + len + 1);
    s->length = len;
    if (buf) {
        memcpy(s->data, buf, len);
    }
    s->data[len] = '\0'; // Sentinel for legacy C-interop safety
    return box_str(s);
}

// Construct from C-string (Legacy support)
Value str_new(const char* c_str) {
    if (!c_str) return str_from_buffer("", 0);
    return str_from_buffer(c_str, (uint32_t)strlen(c_str));
}

// Get Length (O(1) operation)
Value str_len(Value s_tagged) {
    AriaString* s = unbox_str(s_tagged);
    if (!s) return box_int(0);
    return box_int(s->length);
}

// Concatenation (Binary Safe)
Value str_concat(Value a_tagged, Value b_tagged) {
    AriaString* a = unbox_str(a_tagged);
    AriaString* b = unbox_str(b_tagged);
    
    uint32_t len_a = a ? a->length : 0;
    uint32_t len_b = b ? b->length : 0;
    uint32_t new_len = len_a + len_b;
    
    AriaString* res = (AriaString*)aria_alloc(sizeof(AriaString) + new_len + 1);
    res->length = new_len;
    
    if (len_a > 0) memcpy(res->data, a->data, len_a);
    if (len_b > 0) memcpy(res->data + len_a, b->data, len_b);
    
    res->data[new_len] = '\0';
    return box_str(res);
}

// Substring (Binary Safe)
Value str_sub(Value s_tagged, Value start_t, Value len_t) {
    AriaString* s = unbox_str(s_tagged);
    int32_t start = (int32_t)(start_t & 0xFFFFFFFF);
    int32_t len = (int32_t)(len_t & 0xFFFFFFFF);
    
    if (!s) return str_new("");
    
    // Bounds clamping
    if (start < 0) start = 0;
    if (start > (int32_t)s->length) start = s->length;
    if (start + len > (int32_t)s->length) len = s->length - start;
    if (len < 0) len = 0;
    
    return str_from_buffer(s->data + start, (uint32_t)len);
}

// Comparison (Binary Safe using memcmp)
Value str_equals(Value a_tagged, Value b_tagged) {
    AriaString* a = unbox_str(a_tagged);
    AriaString* b = unbox_str(b_tagged);
    
    if (a == b) return box_int(1); // Same pointer or both null
    if (!a || !b) return box_int(0); // One is null
    
    if (a->length != b->length) return box_int(0);
    
    return box_int(memcmp(a->data, b->data, a->length) == 0);
}