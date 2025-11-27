/* Aria_lang/src/stdlib/algorithms.c */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint64_t Value;
// Re-definition of tagging constants for isolation
#define QNAN_MASK       0x7FF8000000000000ULL
#define TAG_BASE        (QNAN_MASK | 0x8000000000000000ULL)
#define TAG_INTEGER     (TAG_BASE | 4ULL)
#define TAG_LIST        (TAG_BASE | 7ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }
static inline double unbox_double(Value v) {
    union { uint64_t u; double d; } cast;
    cast.u = v;
    return cast.d;
}
static inline int is_double(Value v) { return ((v & QNAN_MASK)!= QNAN_MASK); }
static inline int is_int(Value v) { return ((v & TAG_BASE) == TAG_INTEGER); }

// Must match structure in dataStructures.c
typedef struct {
    Value* items; 
    int capacity;
    int count;
    void* lock; 
} AriaList;

static int compare_values(Value a, Value b) {
    double va = is_int(a)? (double)unbox_int(a) : (is_double(a)? unbox_double(a) : 0.0);
    double vb = is_int(b)? (double)unbox_int(b) : (is_double(b)? unbox_double(b) : 0.0);
    
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

static void swap(Value* a, Value* b) {
    Value t = *a; *a = *b; *b = t;
}

static int partition(Value* arr, int low, int high) {
    Value pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (compare_values(arr[j], pivot) < 0) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

static void quick_sort(Value* arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

// API: algo_sort(list)
void* algo_sort(void* list_tagged) {
    AriaList* list = (AriaList*)unbox_ptr((uint64_t)list_tagged);
    if (list && list->count > 1) {
        quick_sort(list->items, 0, list->count - 1);
    }
    return list_tagged;
}

// API: algo_binary_search(list, value) -> index
void* algo_binary_search(void* list_tagged, void* target_tagged) {
    AriaList* list = (AriaList*)unbox_ptr((uint64_t)list_tagged);
    Value target = (Value)target_tagged;
    
    if (!list) return (void*)(TAG_INTEGER | (uint32_t)-1);
    
    int l = 0;
    int r = list->count - 1;
    while (l <= r) {
        int m = l + (r - l) / 2;
        int cmp = compare_values(list->items[m], target);
        if (cmp == 0) return (void*)(TAG_INTEGER | (uint32_t)m);
        if (cmp < 0) l = m + 1;
        else r = m - 1;
    }
    return (void*)(TAG_INTEGER | (uint32_t)-1); 
}
