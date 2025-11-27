/* Aria_lang/src/stdlib/dataStructures.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_LIST        (TAG_BASE | 7ULL) 
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }

typedef struct {
    Value* items; 
    int capacity;
    int count;
    pthread_rwlock_t lock;
} AriaList;

void* list_new() {
    AriaList* list = (AriaList*)aria_alloc(sizeof(AriaList));
    list->capacity = 8;
    list->count = 0;
    list->items = (Value*)aria_alloc(sizeof(Value) * list->capacity);
    pthread_rwlock_init(&list->lock, NULL);
    return (void*)box_ptr(list, TAG_LIST);
}

void list_push(void* list_tagged, void* item_tagged) {
    AriaList* list = (AriaList*)unbox_ptr((Value)list_tagged);
    Value item = (Value)item_tagged;

    if (!list) return;
    pthread_rwlock_wrlock(&list->lock);
    if (list->count >= list->capacity) {
        int new_cap = list->capacity * 2;
        Value* new_items = (Value*)aria_alloc(sizeof(Value) * new_cap);
        memcpy(new_items, list->items, sizeof(Value) * list->count);
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = item;
    pthread_rwlock_unlock(&list->lock);
}

// Correctly accepts Tagged Integer
void* list_get(void* list_tagged, void* index_tagged) {
    AriaList* list = (AriaList*)unbox_ptr((Value)list_tagged);
    int index = (int)((Value)index_tagged & 0xFFFFFFFF);

    if (!list) return (void*)0;
    pthread_rwlock_rdlock(&list->lock);
    if (index < 0 || index >= list->count) {
        pthread_rwlock_unlock(&list->lock);
        fprintf(stderr, "Runtime Error: Index OOB\n");
        exit(1);
    }
    Value val = list->items[index];
    pthread_rwlock_unlock(&list->lock);
    return (void*)val;
}

// Returns the assigned value to satisfy Codegen requirements
void* list_set(void* list_tagged, void* index_tagged, void* val_tagged) {
    AriaList* list = (AriaList*)unbox_ptr((Value)list_tagged);
    int index = (int)((Value)index_tagged & 0xFFFFFFFF);
    Value val = (Value)val_tagged;

    if (!list) return (void*)0;
    pthread_rwlock_wrlock(&list->lock);
    if (index < 0 || index >= list->count) {
        pthread_rwlock_unlock(&list->lock);
        fprintf(stderr, "Runtime Error: Index OOB\n");
        exit(1);
    }
    list->items[index] = val;
    pthread_rwlock_unlock(&list->lock);
    return val_tagged; // Return the value for chaining
}
