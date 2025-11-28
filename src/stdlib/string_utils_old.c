/* Aria_lang/src/stdlib/string_utils.c */
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
#include <stdint.h>

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
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

// Returns Tagged Integer
void* str_len(void* s_tagged) {
    char* s = unbox_str((Value)s_tagged);
    if (!s) return (void*)box_int(0);
    return (void*)box_int((int32_t)strlen(s));
}

// Returns Tagged String
void* str_sub(void* s_tagged, void* start_tagged, void* len_tagged) {
    char* s = unbox_str((Value)s_tagged);
    int start = unbox_int((Value)start_tagged);
    int length = unbox_int((Value)len_tagged);

    if (!s) return (void*)box_ptr("", TAG_STRING);
    int total_len = strlen(s);

    if (start < 0) start = 0;
    if (start >= total_len) return (void*)box_ptr("", TAG_STRING);

    if (length < 0) length = 0;
    if (start + length > total_len) length = total_len - start;

    char* sub = (char*)aria_alloc(length + 1);
    memcpy(sub, s + start, length);
    sub[length] = '\0';
    return (void*)box_ptr(sub, TAG_STRING);
}

// Returns Tagged Integer
void* str_to_int(void* s_tagged) {
    char* s = unbox_str((Value)s_tagged);
    if (!s) return (void*)box_int(0);
    return (void*)box_int((int32_t)atoll(s));
}

// Returns Tagged String
void* int_to_str(void* val_tagged) {
    int32_t val = unbox_int((Value)val_tagged);
    char buf[64];
    snprintf(buf, 64, "%d", val);
    size_t len = strlen(buf);
    
    char* res = (char*)aria_alloc(len + 1);
    memcpy(res, buf, len + 1);
    return (void*)box_ptr(res, TAG_STRING);
}

// Returns Tagged String
void* str_concat(void* a_tagged, void* b_tagged) {
    char* a = unbox_str((Value)a_tagged);
    char* b = unbox_str((Value)b_tagged);
    if (!a) a = "";
    if (!b) b = "";
    
    size_t lenA = strlen(a);
    size_t lenB = strlen(b);
    char* res = (char*)aria_alloc(lenA + lenB + 1);
    memcpy(res, a, lenA);
    memcpy(res + lenA, b, lenB);
    res[lenA + lenB] = '\0';
    return (void*)box_ptr(res, TAG_STRING);
}
