/* Aria_lang/src/stdlib/atomics.c */
#include <stdatomic.h>
#include <stdint.h>

typedef uint64_t Value;
#define PTR_MASK 0x0000FFFFFFFFFFFFULL

static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }

// We treat 'ptr' as a tagged pointer to a heap location.
// We treat 'val' as a tagged value to store/add.

void atomic_store(void* ptr_tagged, void* val_tagged) {
    _Atomic Value* atom = (_Atomic Value*)unbox_ptr((Value)ptr_tagged);
    if(atom) atomic_store_explicit(atom, (Value)val_tagged, memory_order_seq_cst);
}

void* atomic_load(void* ptr_tagged) {
    _Atomic Value* atom = (_Atomic Value*)unbox_ptr((Value)ptr_tagged);
    if(!atom) return (void*)0;
    Value v = atomic_load_explicit(atom, memory_order_seq_cst);
    return (void*)v;
}

void* atomic_xchg(void* ptr_tagged, void* val_tagged) {
    _Atomic Value* atom = (_Atomic Value*)unbox_ptr((Value)ptr_tagged);
    if(!atom) return (void*)0;
    Value v = atomic_exchange_explicit(atom, (Value)val_tagged, memory_order_seq_cst);
    return (void*)v;
}

void* atomic_cas(void* ptr_tagged, void* expected_tagged, void* desired_tagged) {
    _Atomic Value* atom = (_Atomic Value*)unbox_ptr((Value)ptr_tagged);
    if(!atom) return (void*)0;
    Value exp = (Value)expected_tagged;
    Value des = (Value)desired_tagged;
    atomic_compare_exchange_strong(atom, &exp, des);
    return (void*)exp; // Returns old value
}
