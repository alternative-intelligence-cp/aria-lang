/* Aria_lang/src/stdlib/threads.c */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

extern void* aria_alloc(size_t size);
extern void gc_register_thread(void* stack_bottom);
extern void gc_unregister_thread();

typedef struct {
    void* (*user_func)(void*);
    void* user_arg;
} ThreadWrapperArgs;

void* thread_entry_wrapper(void* ptr) {
    ThreadWrapperArgs* args = (ThreadWrapperArgs*)ptr;
    void* (*func)(void*) = args->user_func;
    void* arg = args->user_arg;
    free(args);

    // FIX: Register this thread's stack with the GC
    // We capture the address of a local variable to approximate stack top/base
    int stack_start_marker;
    // We assume stack grows down, so this is roughly the "bottom" (high address)
    // The GC will scan from current SP up to this address.
    gc_register_thread(&stack_start_marker);

    void* res = func(arg);

    gc_unregister_thread();
    return res;
}

void* thread_create(void* func_ptr, void* arg) {
    pthread_t* thread = (pthread_t*)aria_alloc(sizeof(pthread_t));
    ThreadWrapperArgs* args = malloc(sizeof(ThreadWrapperArgs));
    if (!args) return NULL;
    
    args->user_func = (void*(*)(void*))func_ptr;
    args->user_arg = arg;

    if (pthread_create(thread, NULL, thread_entry_wrapper, args)!= 0) {
        free(args);
        return NULL;
    }
    return (void*)thread;
}

void* thread_join(void* thread_ptr) {
    if (!thread_ptr) return NULL;
    pthread_t* thread = (pthread_t*)thread_ptr;
    void* retval;
    pthread_join(*thread, &retval);
    return retval;
}
