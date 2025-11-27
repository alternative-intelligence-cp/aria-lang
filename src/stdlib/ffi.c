/* Aria_lang/src/stdlib/ffi.c */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define QNAN_MASK 0x7FF8000000000000ULL
#define TAG_BASE (QNAN_MASK | 0x8000000000000000ULL)
#define TAG_INTEGER (TAG_BASE | 4ULL)
#define TAG_OBJECT (TAG_BASE | 6ULL)
#define PTR_MASK 0x0000FFFFFFFFFFFFULL

static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_ptr(void* ptr) { return TAG_OBJECT | (uintptr_t)ptr; }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }
static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }
static inline int64_t unbox_val(Value v) {
    if ((v & TAG_BASE) == TAG_INTEGER) return (int32_t)(v & 0xFFFFFFFF);
    return (int64_t)(v & PTR_MASK); 
}

void* ffi_open(void* path_tagged) {
    char* path = unbox_str((Value)path_tagged);
    if (!path) return NULL;
    void* h = dlopen(path, RTLD_LAZY);
    if (!h) { fprintf(stderr, "FFI Error: %s\n", dlerror()); return NULL; }
    return (void*)box_ptr(h);
}

void* ffi_sym(void* lib_tagged, void* name_tagged) {
    void* lib = unbox_ptr((Value)lib_tagged);
    char* name = unbox_str((Value)name_tagged);
    if (!lib || !name) return NULL;
    void* sym = dlsym(lib, name);
    if (!sym) { fprintf(stderr, "FFI Error: %s\n", dlerror()); return NULL; }
    return (void*)box_ptr(sym);
}

typedef void* (*f0)();
typedef void* (*f1)(void*);
typedef void* (*f2)(void*, void*);
typedef void* (*f3)(void*, void*, void*);
typedef void* (*f4)(void*, void*, void*, void*);
typedef void* (*f5)(void*, void*, void*, void*, void*);
typedef void* (*f6)(void*, void*, void*, void*, void*, void*);

void* ffi_call(void* func_tagged, void* args_list_tagged, void* count_tagged) {
    void* func = unbox_ptr((Value)func_tagged);
    int count = (int)((Value)count_tagged & 0xFFFFFFFF);
    if (!func) return NULL;

    extern void* list_get(void* list, void* index);
    void* u_args[6];
    for(int i=0; i<count && i<6; i++) {
        Value v_arg = (Value)list_get(args_list_tagged, (void*)box_int(i)); 
        u_args[i] = (void*)unbox_val(v_arg);
    }

    void* res = NULL;
    switch(count) {
        case 0: res = ((f0)func)(); break;
        case 1: res = ((f1)func)(u_args[0]); break;
        case 2: res = ((f2)func)(u_args[0], u_args[1]); break;
        case 3: res = ((f3)func)(u_args[0], u_args[1], u_args[2]); break;
        case 4: res = ((f4)func)(u_args[0], u_args[1], u_args[2], u_args[3]); break;
        case 5: res = ((f5)func)(u_args[0], u_args[1], u_args[2], u_args[3], u_args[4]); break;
        case 6: res = ((f6)func)(u_args[0], u_args[1], u_args[2], u_args[3], u_args[4], u_args[5]); break;
    }
    return (void*)box_ptr(res);
}
