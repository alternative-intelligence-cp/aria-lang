/* Aria_lang/src/frontend/arena.c */
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* 
 * OPTIMIZATION CONSTANTS
 * ----------------------
 * NODES_PER_PAGE: Defines the granularity of AST node allocation. 
 * Larger pages reduce malloc overhead but increase fragmentation.
 * 
 * STRING_BLOCK_SIZE: 64KB blocks for string data storage. 
 * This fits well within L2 caches of modern CPUs.
 */
#define NODES_PER_PAGE 2048
#define STRING_BLOCK_SIZE (1024 * 64) 

/* 
 * FNV-1a HASH CONSTANTS (32-bit)
 * Source: http://www.isthe.com/chongo/tech/comp/fnv/
 */
#define FNV_PRIME_32 16777619
#define FNV_OFFSET_32 2166136261U
#define INITIAL_INTERN_CAPACITY 1024

// --- Internal Structures ---

struct ArenaPage {
    AstNode nodes[NODES_PER_PAGE];
    struct ArenaPage* next;
    int count;
};

typedef struct StringBlock {
    char data[STRING_BLOCK_SIZE];
    struct StringBlock* next;
    size_t used;
} StringBlock;

/*
 * InternEntry
 * -----------
 * Represents a slot in the open-addressing hash table.
 * storing the full 32-bit hash allows us to skip expensive strcmp calls
 * during collision resolution probing if the hashes don't match.
 */
typedef struct {
    uint32_t hash;
    char* str; // Pointer into a StringBlock, effectively the canonical ID
} InternEntry;

struct AstArena {
    // Node Allocation State
    struct ArenaPage* head;
    struct ArenaPage* current;
    
    // String Allocation State
    StringBlock* str_head;
    StringBlock* str_current;

    // Interning State (Hash Table)
    InternEntry* intern_table;
    size_t intern_capacity; // Must be a power of 2 for fast modulo
    size_t intern_count;
};

// --- FNV-1a Hash Implementation ---
/*
 * Calculates the FNV-1a hash of a string buffer.
 * This function is inline-able and extremely tight.
 */
static uint32_t fnv1a_hash(const char* str, int len) {
    uint32_t hash = FNV_OFFSET_32;
    for (int i = 0; i < len; i++) {
        hash ^= (uint8_t)str[i];
        hash *= FNV_PRIME_32;
    }
    return hash;
}

// --- Allocation Helpers ---

static struct ArenaPage* arena_new_page() {
    struct ArenaPage* page = (struct ArenaPage*)malloc(sizeof(struct ArenaPage));
    if (!page) {
        fprintf(stderr, "Fatal: Out of memory allocating AST page.\n");
        exit(1);
    }
    page->next = NULL;
    page->count = 0;
    return page;
}

static StringBlock* arena_new_str_block() {
    StringBlock* block = (StringBlock*)malloc(sizeof(StringBlock));
    if (!block) {
        fprintf(stderr, "Fatal: Out of memory allocating String block.\n");
        exit(1);
    }
    block->next = NULL;
    block->used = 0;
    return block;
}

// --- Interning Logic ---

/*
 * Resizes the intern hash table when load factor > 0.75.
 * Uses a new capacity of capacity * 2 to maintain power-of-two invariant.
 * This requires re-hashing all existing entries into the new table.
 */
static void arena_resize_intern_table(AstArena* arena) {
    size_t new_cap = arena->intern_capacity * 2;
    InternEntry* new_table = (InternEntry*)calloc(new_cap, sizeof(InternEntry));
    if (!new_table) {
        fprintf(stderr, "Fatal: Out of memory resizing intern table.\n");
        exit(1);
    }

    // Rehash existing entries
    for (size_t i = 0; i < arena->intern_capacity; i++) {
        if (arena->intern_table[i].str!= NULL) {
            InternEntry* entry = &arena->intern_table[i];
            // Fast modulo: hash & (capacity - 1) works because capacity is power of 2
            uint32_t idx = entry->hash & (new_cap - 1);
            
            // Linear probe for empty slot
            while (new_table[idx].str!= NULL) {
                idx = (idx + 1) & (new_cap - 1);
            }
            new_table[idx] = *entry;
        }
    }

    free(arena->intern_table);
    arena->intern_table = new_table;
    arena->intern_capacity = new_cap;
}

// --- Public Interface ---

AstArena* arena_create() {
    AstArena* arena = (AstArena*)malloc(sizeof(AstArena));
    if (!arena) exit(1);
    
    arena->head = arena_new_page();
    arena->current = arena->head;

    arena->str_head = arena_new_str_block();
    arena->str_current = arena->str_head;

    // Initialize Intern Table
    arena->intern_capacity = INITIAL_INTERN_CAPACITY;
    arena->intern_count = 0;
    // calloc ensures all slots are initialized to NULL (0)
    arena->intern_table = (InternEntry*)calloc(arena->intern_capacity, sizeof(InternEntry));
    if (!arena->intern_table) {
        fprintf(stderr, "Fatal: Out of memory initializing intern table.\n");
        exit(1);
    }

    return arena;
}

AstNode* arena_alloc(AstArena* arena) {
    struct ArenaPage* page = arena->current;
    
    if (page->count >= NODES_PER_PAGE) {
        struct ArenaPage* new_page = arena_new_page();
        page->next = new_page;
        arena->current = new_page;
        page = new_page;
    }
    
    AstNode* node = &page->nodes[page->count++];
    // Zero-init is crucial for AST processing safety
    memset(node, 0, sizeof(AstNode));
    return node;
}

/*
 * arena_strndup (INTERNING VERSION)
 * ---------------------------------
 * 1. Computes hash of incoming string.
 * 2. Checks table for existence.
 *    - If found (Hash matches AND strcmp matches), return existing pointer.
 * 3. If not found:
 *    - Check load factor and resize table if necessary.
 *    - Allocate string bytes in the linear StringBlock.
 *    - Insert new entry into the hash table.
 *    - Return new pointer.
 */
char* arena_strndup(AstArena* arena, const char* str, int len) {
    uint32_t hash = fnv1a_hash(str, len);
    uint32_t idx = hash & (arena->intern_capacity - 1);

    // 1. Check for existing string (Lookup Phase)
    while (arena->intern_table[idx].str!= NULL) {
        InternEntry* entry = &arena->intern_table[idx];
        // Optimization: Check cached hash first before touching string memory
        if (entry->hash == hash) {
            // Hash match, verify exact equality to rule out collision
            if (strncmp(entry->str, str, len) == 0 && entry->str[len] == '\0') {
                return entry->str; // Return canonical instance
            }
        }
        // Collision, probe next slot (Linear Probing)
        idx = (idx + 1) & (arena->intern_capacity - 1);
    }

    // 2. Not found, prepare to allocate (Insertion Phase)
    
    // Check Load Factor: count * 4 > capacity * 3 implies > 75% full
    if (arena->intern_count * 4 > arena->intern_capacity * 3) { 
        arena_resize_intern_table(arena);
        // Recalculate index after resize because capacity changed
        idx = hash & (arena->intern_capacity - 1);
        while (arena->intern_table[idx].str!= NULL) {
            idx = (idx + 1) & (arena->intern_capacity - 1);
        }
    }

    // Allocate in StringBlock
    if (arena->str_current->used + len + 1 > STRING_BLOCK_SIZE) {
        // Edge Case: String larger than block size?
        if (len + 1 > STRING_BLOCK_SIZE) {
            fprintf(stderr, "Fatal: String literal too large for arena block (>64KB).\n");
            exit(1);
        }
        StringBlock* new_block = arena_new_str_block();
        arena->str_current->next = new_block;
        arena->str_current = new_block;
    }

    char* ptr = &arena->str_current->data[arena->str_current->used];
    memcpy(ptr, str, len);
    ptr[len] = '\0';
    arena->str_current->used += (len + 1);

    // 3. Insert into Intern Table
    arena->intern_table[idx].hash = hash;
    arena->intern_table[idx].str = ptr;
    arena->intern_count++;

    return ptr;
}

void arena_free(AstArena* arena) {
    // 1. Free Node Pages
    struct ArenaPage* current = arena->head;
    while (current) {
        struct ArenaPage* next = current->next;
        free(current);
        current = next;
    }

    // 2. Free String Blocks
    StringBlock* str_curr = arena->str_head;
    while (str_curr) {
        StringBlock* next = str_curr->next;
        free(str_curr);
        str_curr = next;
    }

    // 3. Free Hash Table
    if (arena->intern_table) {
        free(arena->intern_table);
    }

    free(arena);
}
