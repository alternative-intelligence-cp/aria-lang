/* Aria_lang/src/stdlib/linux.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);
typedef uint64_t Value;
#define TAG_STRING (0xFFF8000000000000ULL | 5ULL)
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
#define PTR_MASK 0x0000FFFFFFFFFFFFULL

static inline Value box_ptr(void* p, uint64_t t) { return t | (uintptr_t)p; }
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }

void* linux_getenv(void* key_tagged) {
    char* k = unbox_str((Value)key_tagged);
    if (!k) return (void*)0;
    char* v = getenv(k);
    if (!v) return (void*)0;
    char* r = aria_alloc(strlen(v)+1);
    strcpy(r, v);
    return (void*)box_ptr(r, TAG_STRING);
}

void* linux_cpu_count() {
    return (void*)box_int(get_nprocs());
}

void* linux_uptime() {
    struct sysinfo i;
    if(sysinfo(&i)!= 0) return (void*)box_int(-1);
    return (void*)box_int((int32_t)i.uptime);
}
