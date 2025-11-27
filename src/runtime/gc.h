/* Aria_lang/src/runtime/gc.h */
#ifndef GC_H
#define GC_H

#include <stddef.h>
#include <stdint.h>

// External variables
extern volatile int32_t gc_suspend_request;

// Global root registration
void aria_register_global_root(void** ptr);

// Thread management  
void gc_register_thread(void* stack_bottom);
void gc_unregister_thread();

// Garbage collection entry points
void gc_enter_safepoint();
void perform_collection();

// Object marking
void mark_object(void* ptr);
void mark_range(void* start, void* end);

// Memory allocation
void* aria_alloc(size_t size);

// Internal bloom filter functions (static inline, defined in gc.c)
// static inline void bloom_add(void* ptr);
// static inline int bloom_check(void* ptr);

#endif