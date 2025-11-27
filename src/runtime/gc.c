/* Aria_lang/src/runtime/gc.c */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdint.h>
#include <setjmp.h>
#include "gc.h"

#define HEAP_LIMIT (1024 * 1024 * 64) 
#define BLOOM_SIZE 65536             

// NaN Boxing Constants for Masking
#define TAG_MASK 0xFFFF000000000000ULL
#define PTR_MASK 0x0000FFFFFFFFFFFFULL

volatile int32_t gc_suspend_request = 0; 
static pthread_mutex_t gc_sync_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gc_resume_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t gc_stopped_cond = PTHREAD_COND_INITIALIZER;

static atomic_int active_thread_count = 0;
static atomic_int stopped_thread_count = 0;
static atomic_size_t bytes_allocated = 0;

static uint64_t bloom[BLOOM_SIZE / 64]; 

typedef struct RootEntry {
    void** ptr;
    struct RootEntry* next;
} RootEntry;

static RootEntry* global_roots = NULL;
static pthread_mutex_t roots_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct ThreadDesc {
    pthread_t thread_id;
    void* stack_bottom; 
    void* stack_top;    
    jmp_buf regs;       
    struct ThreadDesc* next;
} ThreadDesc;

static ThreadDesc* threads_head = NULL;
static pthread_mutex_t thread_list_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct ObjHeader {
    struct ObjHeader* next;
    size_t size;
    unsigned char marked;
    char padding;
} ObjHeader;

static ObjHeader* heap_head = NULL;
static pthread_mutex_t alloc_lock = PTHREAD_MUTEX_INITIALIZER;

static inline void bloom_add(void* ptr) {
    uintptr_t h = (uintptr_t)ptr;
    h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9ULL;
    h = (h ^ (h >> 27)) * 0x94d049bb133111ebULL;
    h = h ^ (h >> 31);
    uint32_t idx = h % BLOOM_SIZE;
    bloom[idx / 64] |= (1ULL << (idx % 64));
}

static inline int bloom_check(void* ptr) {
    uintptr_t h = (uintptr_t)ptr;
    h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9ULL;
    h = (h ^ (h >> 27)) * 0x94d049bb133111ebULL;
    h = h ^ (h >> 31);
    uint32_t idx = h % BLOOM_SIZE;
    return (bloom[idx / 64] & (1ULL << (idx % 64))) != 0;
}

void aria_register_global_root(void** ptr) {
    RootEntry* entry = malloc(sizeof(RootEntry));
    entry->ptr = ptr;
    pthread_mutex_lock(&roots_lock);
    entry->next = global_roots;
    global_roots = entry;
    pthread_mutex_unlock(&roots_lock);
}

void gc_register_thread(void* stack_bottom) {
    ThreadDesc* td = malloc(sizeof(ThreadDesc));
    td->thread_id = pthread_self();
    td->stack_bottom = stack_bottom;
    td->next = NULL;
    pthread_mutex_lock(&thread_list_lock);
    td->next = threads_head;
    threads_head = td;
    pthread_mutex_unlock(&thread_list_lock);
    atomic_fetch_add(&active_thread_count, 1);
}

void gc_unregister_thread() {
    pthread_t self = pthread_self();
    pthread_mutex_lock(&thread_list_lock);
    ThreadDesc** curr = &threads_head;
    while (*curr) {
        if (pthread_equal((*curr)->thread_id, self)) {
            ThreadDesc* to_free = *curr;
            *curr = to_free->next;
            free(to_free);
            break;
        }
        curr = &(*curr)->next;
    }
    pthread_mutex_unlock(&thread_list_lock);
    atomic_fetch_sub(&active_thread_count, 1);
}

void gc_enter_safepoint() {
    pthread_mutex_lock(&gc_sync_lock);
    pthread_t self = pthread_self();
    pthread_mutex_lock(&thread_list_lock);
    ThreadDesc* curr = threads_head;
    while (curr) {
        if (pthread_equal(curr->thread_id, self)) {
            setjmp(curr->regs);
            curr->stack_top = __builtin_frame_address(0);
            break;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&thread_list_lock);

    atomic_fetch_add(&stopped_thread_count, 1);
    if (atomic_load(&stopped_thread_count) == atomic_load(&active_thread_count)) {
        pthread_cond_signal(&gc_stopped_cond);
    }
    
    while (atomic_load((_Atomic int32_t*)&gc_suspend_request) != 0) {
        pthread_cond_wait(&gc_resume_cond, &gc_sync_lock);
    }
    
    atomic_fetch_sub(&stopped_thread_count, 1);
    pthread_mutex_unlock(&gc_sync_lock);
}

// FIX: Tag-Aware Marking
void mark_object(void* ptr) {
    if (!ptr) return;

    // 1. Unmask NaN-boxed pointers
    uint64_t val = (uint64_t)ptr;
    // If top bits are set (NaN Tag), mask them out to get the pointer
    if ((val & TAG_MASK) == TAG_MASK) {
        ptr = (void*)(val & PTR_MASK);
    }

    // 2. Validate Pointer
    if (!ptr) return;
    if ((uintptr_t)ptr % 8 != 0) return; // Alignment check
    if (!bloom_check(ptr)) return;

    ObjHeader* curr = heap_head;
    while (curr) {
        void* payload = (void*)(curr + 1);
        if (ptr == payload) {
            if (!curr->marked) {
                curr->marked = 1;
                // Scan payload
                void** fields = (void**)payload;
                size_t cnt = curr->size / sizeof(void*);
                for (size_t i = 0; i < cnt; i++) {
                    mark_object(fields[i]);
                }
            }
            return;
        }
        curr = curr->next;
    }
}

void mark_range(void* start, void* end) {
    void** p = (void**)start;
    void** e = (void**)end;
    if (p > e) { void** t = p; p = e; e = t; } 
    while (p < e) {
        mark_object(*p);
        p++;
    }
}

void perform_collection() {
    pthread_mutex_lock(&gc_sync_lock);
    atomic_store((_Atomic int32_t*)&gc_suspend_request, 1);
    
    int total = atomic_load(&active_thread_count);
    if (total > 1) {
         while (atomic_load(&stopped_thread_count) < total - 1) {
             pthread_cond_wait(&gc_stopped_cond, &gc_sync_lock);
         }
    }
    pthread_mutex_unlock(&gc_sync_lock);
    
    pthread_mutex_lock(&thread_list_lock);
    ThreadDesc* curr = threads_head;
    while (curr) {
        mark_range(curr->stack_top, curr->stack_bottom);
        mark_range(curr->regs, (void*)((uintptr_t)curr->regs + sizeof(jmp_buf)));
        curr = curr->next;
    }
    
    void* self_top = __builtin_frame_address(0);
    curr = threads_head;
    pthread_t self = pthread_self();
    while (curr) {
        if (pthread_equal(curr->thread_id, self)) {
            mark_range(self_top, curr->stack_bottom);
            break;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&thread_list_lock);

    pthread_mutex_lock(&roots_lock);
    RootEntry* root = global_roots;
    while (root) {
        if (root->ptr && *root->ptr) mark_object(*root->ptr);
        root = root->next;
    }
    pthread_mutex_unlock(&roots_lock);
    
    pthread_mutex_lock(&alloc_lock);
    memset(bloom, 0, sizeof(bloom));
    
    ObjHeader** node = &heap_head;
    while (*node) {
        ObjHeader* entry = *node;
        if (!entry->marked) {
            *node = entry->next;
            atomic_fetch_sub(&bytes_allocated, entry->size);
            free(entry);
        } else {
            entry->marked = 0;
            bloom_add((void*)(entry + 1));
            node = &entry->next;
        }
    }
    pthread_mutex_unlock(&alloc_lock);
    
    pthread_mutex_lock(&gc_sync_lock);
    atomic_store((_Atomic int32_t*)&gc_suspend_request, 0);
    pthread_cond_broadcast(&gc_resume_cond);
    pthread_mutex_unlock(&gc_sync_lock);
}

void* aria_alloc(size_t size) {
    if (size % 8 != 0) size = (size + 7) & ~7;
    
    if (atomic_load((_Atomic int32_t*)&gc_suspend_request)) {
        gc_enter_safepoint();
    }

    if (atomic_load(&bytes_allocated) + size > HEAP_LIMIT) {
        perform_collection();
    }
    
    ObjHeader* h = malloc(sizeof(ObjHeader) + size);
    if (!h) {
        perform_collection();
        h = malloc(sizeof(ObjHeader) + size);
        if (!h) { fprintf(stderr, "OOM\n"); exit(1); }
    }
    
    memset(h + 1, 0, size);
    h->size = size;
    h->marked = 0;
    
    pthread_mutex_lock(&alloc_lock);
    h->next = heap_head;
    heap_head = h;
    atomic_fetch_add(&bytes_allocated, size);
    bloom_add(h + 1);
    pthread_mutex_unlock(&alloc_lock);
    
    return (void*)(h + 1);
}

__attribute__((constructor))
void aria_runtime_init() {
    void* stack_bottom = __builtin_frame_address(0); 
    gc_register_thread(stack_bottom);
}
