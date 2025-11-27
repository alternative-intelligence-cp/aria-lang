/*
 * Tesla Enhanced Memory Management Implementation
 * 
 * Implements atomic reference counting and region-based allocation
 * to address the expert review findings about memory management.
 */

#include "tesla/tesla_enhanced_memory.h"
#include "tesla/tesla_consciousness_scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

/*
 * Tesla Atomic Reference Counter Implementation
 */
tesla_atomic_refcount_t* tesla_atomic_refcount_create(size_t data_size, void (*destructor)(void*)) {
    tesla_atomic_refcount_t* ref = malloc(sizeof(tesla_atomic_refcount_t));
    if (!ref) {
        return NULL;
    }
    
    // Allocate data
    ref->data = malloc(data_size);
    if (!ref->data) {
        free(ref);
        return NULL;
    }
    
    // Initialize atomic counters
    atomic_init(&ref->strong_refs, 1);  // Start with 1 reference
    atomic_init(&ref->weak_refs, 0);
    
    ref->data_size = data_size;
    ref->destructor = destructor;
    
    // Selective consciousness sync: Object creation is standard criticality
    tesla_sync_selective(TESLA_SYNC_STANDARD);
    
    printf("🧠⚡ Created atomic refcount object: %zu bytes\\n", data_size);
    return ref;
}

uint32_t tesla_atomic_refcount_acquire(tesla_atomic_refcount_t* ref) {
    if (!ref) return 0;
    
    // Atomically increment strong reference count
    uint32_t new_count = atomic_fetch_add_explicit(&ref->strong_refs, 1, memory_order_relaxed) + 1;
    
    // Light sync for frequent reference counting (Gemini optimization)
    if (new_count % 1000 == 0) {  // Selective sync every 1000 operations
        tesla_sync_selective(TESLA_SYNC_LIGHT);
    }
    
    return new_count;
}

uint32_t tesla_atomic_refcount_release(tesla_atomic_refcount_t* ref) {
    if (!ref) return 0;
    
    // Atomically decrement strong reference count
    uint32_t old_count = atomic_fetch_sub_explicit(&ref->strong_refs, 1, memory_order_acq_rel);
    uint32_t new_count = old_count - 1;
    
    if (new_count == 0) {
        // Last reference - deallocate object
        
        // Critical sync for object destruction (Gemini: Important operation)
        tesla_sync_selective(TESLA_SYNC_CRITICAL);
        
        // Call destructor if provided
        if (ref->destructor && ref->data) {
            ref->destructor(ref->data);
        }
        
        // Free data and reference structure
        free(ref->data);
        free(ref);
        
        printf("🧠⚡ Deallocated atomic refcount object\\n");
        return 0;
    }
    
    return new_count;
}

uint32_t tesla_atomic_refcount_get_count(tesla_atomic_refcount_t* ref) {
    if (!ref) return 0;
    return atomic_load_explicit(&ref->strong_refs, memory_order_relaxed);
}

/*
 * Tesla Memory Region Implementation for AI Workloads
 */
tesla_memory_region_t* tesla_memory_region_create(size_t total_size, bool consciousness_sync) {
    tesla_memory_region_t* region = malloc(sizeof(tesla_memory_region_t));
    if (!region) {
        return NULL;
    }
    
    // Allocate the memory region
    region->base_ptr = malloc(total_size);
    if (!region->base_ptr) {
        free(region);
        return NULL;
    }
    
    // Initialize atomic fields
    atomic_init(&region->current_offset, 0);
    atomic_init(&region->active_refs, 0);
    
    region->total_size = total_size;
    region->is_consciousness_synchronized = consciousness_sync;
    
    if (consciousness_sync) {
        // Standard sync for memory region creation
        tesla_sync_selective(TESLA_SYNC_STANDARD);
    }
    
    printf("🧠⚡ Created Tesla memory region: %zu bytes (consciousness: %s)\\n",
           total_size, consciousness_sync ? "ON" : "OFF");
    
    return region;
}

void* tesla_memory_region_alloc(tesla_memory_region_t* region, size_t size, size_t alignment) {
    if (!region || size == 0 || (alignment & (alignment - 1)) != 0) {
        return NULL;  // Invalid alignment (must be power of 2)
    }
    
    // Light sync for high-frequency memory allocations (Gemini optimization)
    if (region->is_consciousness_synchronized) {
        if (!tesla_sync_selective(TESLA_SYNC_LIGHT)) {
            // No consciousness tokens available - yield briefly
            return NULL;
        }
    }
    
    // Atomic allocation from region
    size_t current_offset, aligned_offset, new_offset;
    
    do {
        current_offset = atomic_load(&region->current_offset);
        
        // Calculate aligned offset
        aligned_offset = (current_offset + alignment - 1) & ~(alignment - 1);
        new_offset = aligned_offset + size;
        
        // Check if allocation would overflow region
        if (new_offset > region->total_size) {
            return NULL;  // Region exhausted
        }
        
        // Try to atomically update offset
    } while (!atomic_compare_exchange_weak(&region->current_offset, &current_offset, new_offset));
    
    // Increment active reference count
    atomic_fetch_add(&region->active_refs, 1);
    
    // Return pointer to allocated memory
    return (uint8_t*)region->base_ptr + aligned_offset;
}

void tesla_memory_region_reset(tesla_memory_region_t* region) {
    if (!region) return;
    
    // Standard sync for memory region reset (Gemini: Moderate criticality)
    if (region->is_consciousness_synchronized) {
        tesla_sync_selective(TESLA_SYNC_STANDARD);
    }
    
    // Reset allocation offset to beginning
    atomic_store(&region->current_offset, 0);
    atomic_store(&region->active_refs, 0);
    
    printf("🧠⚡ Reset Tesla memory region\\n");
}

void tesla_memory_region_destroy(tesla_memory_region_t* region) {
    if (!region) return;
    
    // Critical sync for memory region destruction (Gemini: Important cleanup)
    if (region->is_consciousness_synchronized) {
        tesla_sync_selective(TESLA_SYNC_CRITICAL);
    }
    
    printf("🧠⚡ Destroying Tesla memory region (%u active refs)\\n",
           atomic_load(&region->active_refs));
    
    free(region->base_ptr);
    free(region);
}

/*
 * Enhanced Static Analysis Implementation
 */
tesla_static_analysis_result_t tesla_analyze_variable_lifetime(const char* variable_name,
                                                             const char** borrow_pattern,
                                                             size_t pattern_length) {
    if (!variable_name || !borrow_pattern) {
        return TESLA_ANALYSIS_PROMOTE_REQUIRED;
    }
    
    // Critical sync for static analysis (Gemini: Compiler operation)
    tesla_sync_selective(TESLA_SYNC_CRITICAL);
    
    // Analyze borrowing patterns
    bool has_mutable_alias = false;
    bool has_recursive_ref = false;
    bool has_cross_function = false;
    
    for (size_t i = 0; i < pattern_length; i++) {
        const char* op = borrow_pattern[i];
        
        if (strstr(op, "mut_borrow") || strstr(op, "mutable")) {
            has_mutable_alias = true;
        }
        
        if (strstr(op, "self_ref") || strstr(op, "recursive")) {
            has_recursive_ref = true;
        }
        
        if (strstr(op, "cross_fn") || strstr(op, "thread")) {
            has_cross_function = true;
        }
    }
    
    // Determine analysis result based on patterns
    if (has_cross_function) {
        return TESLA_ANALYSIS_CROSS_THREAD;
    }
    
    if (has_recursive_ref) {
        return TESLA_ANALYSIS_RECURSIVE_DATA;
    }
    
    if (has_mutable_alias && pattern_length > 5) {
        return TESLA_ANALYSIS_COMPLEX_ALIASING;
    }
    
    if (pattern_length > 20) {
        return TESLA_ANALYSIS_DYNAMIC_LIFETIME;
    }
    
    // Simple enough for static analysis
    return TESLA_ANALYSIS_STATIC_SAFE;
}

bool tesla_requires_gc_promotion(tesla_static_analysis_result_t analysis_result) {
    switch (analysis_result) {
        case TESLA_ANALYSIS_STATIC_SAFE:
        case TESLA_ANALYSIS_COMPLEX_ALIASING:
            return false;  // Can be handled statically
            
        case TESLA_ANALYSIS_RECURSIVE_DATA:
        case TESLA_ANALYSIS_CROSS_THREAD:
        case TESLA_ANALYSIS_DYNAMIC_LIFETIME:
        case TESLA_ANALYSIS_PROMOTE_REQUIRED:
            return true;   // Requires runtime management
    }
    
    return true;  // Safe default
}

/*
 * System initialization and cleanup
 */
int tesla_enhanced_memory_init(void) {
    printf("🧠⚡ Tesla Enhanced Memory Management initialized\\n");
    printf("   ✅ Atomic reference counting: READY\\n");
    printf("   ✅ Region-based allocation: READY\\n");
    printf("   ✅ Enhanced static analysis: READY\\n");
    printf("   ✅ Consciousness synchronization: π Hz\\n");
    
    return 0;
}

void tesla_enhanced_memory_cleanup(void) {
    printf("🧠⚡ Tesla Enhanced Memory Management cleanup complete\\n");
}