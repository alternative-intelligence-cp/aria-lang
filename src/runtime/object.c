/* Aria_lang/src/runtime/object.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_OBJECT      (TAG_BASE | 6ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }

typedef struct {
    char* key;
    Value value; 
    int is_occupied; 
} Entry;

typedef struct {
    Entry* entries;
    int capacity;
    int count;
} AriaObject;

#define INITIAL_CAPACITY 8

static uint32_t hash_key(const char* key) {
    uint32_t hash = 2166136261u;
    for (const char* p = key; *p; p++) {
        hash ^= (uint8_t)(*p);
        hash *= 16777619;
    }
    return hash;
}

void* aria_alloc_object() {
    AriaObject* obj = (AriaObject*)aria_alloc(sizeof(AriaObject));
    obj->capacity = INITIAL_CAPACITY;
    obj->count = 0;
    obj->entries = (Entry*)aria_alloc(sizeof(Entry) * obj->capacity);
    return (void*)box_ptr(obj, TAG_OBJECT);
}

static void resize_object(AriaObject* obj) {
    int old_cap = obj->capacity;
    Entry* old_entries = obj->entries;
    int new_cap = old_cap * 2;
    Entry* new_entries = (Entry*)aria_alloc(sizeof(Entry) * new_cap);

    for (int i = 0; i < old_cap; i++) {
        if (old_entries[i].is_occupied == 1) {
            uint32_t hash = hash_key(old_entries[i].key);
            int idx = hash % new_cap;
            while (new_entries[idx].is_occupied) idx = (idx + 1) % new_cap;
            new_entries[idx] = old_entries[i];
        }
    }
    obj->entries = new_entries;
    obj->capacity = new_cap;
}

// Returns TAGGED value
void* aria_obj_set(void* obj_tagged, char* key, void* value_tagged) {
    AriaObject* obj = (AriaObject*)unbox_ptr((Value)obj_tagged);
    Value val = (Value)value_tagged;

    if (!obj) { fprintf(stderr, "Runtime Error: Set on null object.\n"); exit(1); }
    if (!key) return value_tagged;

    if ((obj->count + 1) * 4 > obj->capacity * 3) resize_object(obj);

    uint32_t hash = hash_key(key);
    int idx = hash % obj->capacity;

    while (obj->entries[idx].is_occupied) {
        if (strcmp(obj->entries[idx].key, key) == 0) {
            obj->entries[idx].value = val;
            return value_tagged;
        }
        idx = (idx + 1) % obj->capacity;
    }

    obj->entries[idx].key = key;
    obj->entries[idx].value = val;
    obj->entries[idx].is_occupied = 1;
    obj->count++;
    return value_tagged;
}

void* aria_obj_get(void* obj_tagged, char* key) {
    AriaObject* obj = (AriaObject*)unbox_ptr((Value)obj_tagged);
    if (!obj) { fprintf(stderr, "Runtime Error: Get on null object.\n"); exit(1); }
    if (!key) return (void*)0; 

    uint32_t hash = hash_key(key);
    int idx = hash % obj->capacity;
    int start_idx = idx;
    int looped = 0;

    while (obj->entries[idx].is_occupied) {
        if (strcmp(obj->entries[idx].key, key) == 0) return (void*)obj->entries[idx].value;
        idx = (idx + 1) % obj->capacity;
        if (idx == start_idx || ++looped > obj->capacity) break; 
    }
    return (void*)0; 
}
