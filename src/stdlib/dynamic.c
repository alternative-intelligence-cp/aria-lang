/* Aria_lang/src/stdlib/dynamic.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;

#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)

#define TAG_NULL        (TAG_BASE | 1ULL)
#define TAG_FALSE       (TAG_BASE | 2ULL)
#define TAG_TRUE        (TAG_BASE | 3ULL)
#define TAG_INTEGER     (TAG_BASE | 4ULL) 
#define TAG_STRING      (TAG_BASE | 5ULL) 
#define TAG_OBJECT      (TAG_BASE | 6ULL) 
#define TAG_LIST        (TAG_BASE | 7ULL) 

#define IS_DOUBLE(v)    (((v) & QNAN_MASK)!= QNAN_MASK)
#define IS_TAGGED(v)    (((v) & TAG_BASE) == TAG_BASE)

#define VAL_NULL        TAG_NULL
#define VAL_FALSE       TAG_FALSE
#define VAL_TRUE        TAG_TRUE

static inline Value box_double(double d) {
    union { double d; uint64_t u; } cast;
    cast.d = d;
    if ((cast.u & QNAN_MASK) == QNAN_MASK) return QNAN_MASK;
    return cast.u;
}

static inline Value box_int(int32_t i) {
    return TAG_INTEGER | (uint32_t)i;
}

static inline Value box_ptr(void* ptr, uint64_t tag) {
    return tag | (uintptr_t)ptr;
}

static inline double unbox_double(Value v) {
    union { uint64_t u; double d; } cast;
    cast.u = v;
    return cast.d;
}

static inline int32_t unbox_int(Value v) {
    return (int32_t)(v & 0xFFFFFFFF);
}

static inline void* unbox_ptr(Value v) {
    return (void*)(v & 0x0000FFFFFFFFFFFFULL);
}

// Constructors
void* dyn_new_int(long long val) { return (void*)box_int((int32_t)val); }
void* dyn_new_float(long long bits) {
    union { long long i; double d; } u; u.i = bits;
    return (void*)box_double(u.d);
}
void* dyn_new_str(char* val) { return (void*)box_ptr(val, TAG_STRING); }
void* dyn_new_bool(long long val) { return (void*)(val? VAL_TRUE : VAL_FALSE); }
void* dyn_new_null() { return (void*)VAL_NULL; }

// Type check helper
int dyn_get_type(void* handle) {
    Value v = (Value)handle;
    if (IS_DOUBLE(v)) return 1;
    if (v == VAL_NULL) return 0;
    if (v == VAL_TRUE || v == VAL_FALSE) return 4;
    uint64_t tag = v & 0xFFFF000000000000ULL;
    if (tag == TAG_INTEGER) return 2;
    if (tag == TAG_STRING) return 3;
    return -1;
}

// Truthy check for IF/WHILE
int64_t dyn_truthy(void* ptr) {
    Value v = (Value)ptr;
    if (v == VAL_FALSE || v == VAL_NULL) return 0;
    if (v == VAL_TRUE) return 1;
    if (IS_DOUBLE(v)) return unbox_double(v)!= 0.0;
    if ((v & TAG_BASE) == TAG_INTEGER) return unbox_int(v)!= 0;
    return 1; // Objects/Strings are true
}

// Binary Operations
void* dyn_add(void* a_ptr, void* b_ptr) {
    Value a = (Value)a_ptr;
    Value b = (Value)b_ptr;
    
    bool a_int = ((a & TAG_BASE) == TAG_INTEGER);
    bool b_int = ((b & TAG_BASE) == TAG_INTEGER);
    
    if (a_int && b_int) return (void*)box_int(unbox_int(a) + unbox_int(b));
    
    double da = a_int? unbox_int(a) : (IS_DOUBLE(a)? unbox_double(a) : 0);
    double db = b_int? unbox_int(b) : (IS_DOUBLE(b)? unbox_double(b) : 0);
    
    if (IS_DOUBLE(a) || IS_DOUBLE(b)) return (void*)box_double(da + db);
    
    // String Concat
    if ((a & TAG_BASE) == TAG_STRING && (b & TAG_BASE) == TAG_STRING) {
        char* s1 = (char*)unbox_ptr(a);
        char* s2 = (char*)unbox_ptr(b);
        size_t l1 = strlen(s1), l2 = strlen(s2);
        char* r = aria_alloc(l1 + l2 + 1);
        strcpy(r, s1); strcat(r, s2);
        return (void*)box_ptr(r, TAG_STRING);
    }
    return (void*)VAL_NULL;
}

void* dyn_sub(void* a_ptr, void* b_ptr) {
    Value a = (Value)a_ptr;
    Value b = (Value)b_ptr;
    
    if (((a & TAG_BASE) == TAG_INTEGER) && ((b & TAG_BASE) == TAG_INTEGER)) 
        return (void*)box_int(unbox_int(a) - unbox_int(b));
        
    double da = IS_DOUBLE(a)? unbox_double(a) : unbox_int(a);
    double db = IS_DOUBLE(b)? unbox_double(b) : unbox_int(b);
    return (void*)box_double(da - db);
}

void* dyn_mul(void* a_ptr, void* b_ptr) {
    Value a = (Value)a_ptr;
    Value b = (Value)b_ptr;
    
    if (((a & TAG_BASE) == TAG_INTEGER) && ((b & TAG_BASE) == TAG_INTEGER)) 
        return (void*)box_int(unbox_int(a) * unbox_int(b));
        
    double da = IS_DOUBLE(a)? unbox_double(a) : unbox_int(a);
    double db = IS_DOUBLE(b)? unbox_double(b) : unbox_int(b);
    return (void*)box_double(da * db);
}

void* dyn_div(void* a_ptr, void* b_ptr) {
    Value a = (Value)a_ptr;
    Value b = (Value)b_ptr;
    double da = IS_DOUBLE(a)? unbox_double(a) : unbox_int(a);
    double db = IS_DOUBLE(b)? unbox_double(b) : unbox_int(b);
    return (void*)box_double(da / db);
}

// Modulo Operation: Supports Int % Int and Double % Double
void* dyn_mod(void* a_ptr, void* b_ptr) {
    Value a = (Value)a_ptr;
    Value b = (Value)b_ptr;
    
    // Fast path for integers
    if (((a & TAG_BASE) == TAG_INTEGER) && ((b & TAG_BASE) == TAG_INTEGER)) {
        int32_t va = unbox_int(a);
        int32_t vb = unbox_int(b);
        if (vb == 0) {
            fprintf(stderr, "Runtime Error: Division by zero (mod).\n");
            exit(1);
        }
        return (void*)box_int(va % vb);
    }
    
    // Fallback to floating point modulo
    double da = IS_DOUBLE(a)? unbox_double(a) : (double)unbox_int(a);
    double db = IS_DOUBLE(b)? unbox_double(b) : (double)unbox_int(b);
    return (void*)box_double(fmod(da, db));
}

// Unary Negation (-x)
void* dyn_neg(void* a_ptr) {
    Value a = (Value)a_ptr;
    
    if ((a & TAG_BASE) == TAG_INTEGER) {
        return (void*)box_int(-unbox_int(a));
    }
    
    if (IS_DOUBLE(a)) {
        return (void*)box_double(-unbox_double(a));
    }
    
    return (void*)VAL_NULL; 
}

// Logical NOT (!x)
void* dyn_not(void* a_ptr) {
    int64_t truth = dyn_truthy(a_ptr);
    return (void*)(truth? VAL_FALSE : VAL_TRUE);
}

// Comparisons
void* dyn_eq(void* a, void* b) { return (void*)(a == b? VAL_TRUE : VAL_FALSE); }
void* dyn_neq(void* a, void* b) { return (void*)(a!= b? VAL_TRUE : VAL_FALSE); }

void* dyn_lt(void* a_ptr, void* b_ptr) {
    Value a = (Value)a_ptr, b = (Value)b_ptr;
    double da = IS_DOUBLE(a)? unbox_double(a) : unbox_int(a);
    double db = IS_DOUBLE(b)? unbox_double(b) : unbox_int(b);
    return (void*)(da < db? VAL_TRUE : VAL_FALSE);
}

void* dyn_gt(void* a_ptr, void* b_ptr) {
    Value a = (Value)a_ptr, b = (Value)b_ptr;
    double da = IS_DOUBLE(a)? unbox_double(a) : unbox_int(a);
    double db = IS_DOUBLE(b)? unbox_double(b) : unbox_int(b);
    return (void*)(da > db? VAL_TRUE : VAL_FALSE);
}

void dyn_print(void* ptr) {
    Value v = (Value)ptr;
    if (IS_DOUBLE(v)) printf("%f", unbox_double(v));
    else if (v == VAL_TRUE) printf("true");
    else if (v == VAL_FALSE) printf("false");
    else if (v == VAL_NULL) printf("null");
    else if ((v & TAG_BASE) == TAG_INTEGER) printf("%d", unbox_int(v));
    else if ((v & TAG_BASE) == TAG_STRING) printf("%s", (char*)unbox_ptr(v));
    else printf("<object>");
}
