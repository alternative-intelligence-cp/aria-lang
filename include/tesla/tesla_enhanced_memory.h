/*
 * Tesla Enhanced Memory Management with Atomic Reference Counting
 * 
 * Addresses the expert review findings:
 * - Replaces heuristic promotion with true static analysis foundations
 * - Implements atomic reference counting for thread safety
 * - Adds region-based allocation for AI workloads
 * - Provides deterministic memory management behavior
 */

#ifndef TESLA_ENHANCED_MEMORY_H
#define TESLA_ENHANCED_MEMORY_H

#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Tesla Atomic Reference Counter
 * 
 * Thread-safe reference counting for shared memory management
 * Replaces the unpredictable heuristic promotion system
 */
typedef struct {
    atomic_uint_fast32_t strong_refs;   // Strong references (prevent deallocation)
    atomic_uint_fast32_t weak_refs;     // Weak references (don't prevent deallocation)
    void* data;                         // Actual data pointer
    size_t data_size;                   // Size of allocated data
    void (*destructor)(void*);          // Optional cleanup function
} tesla_atomic_refcount_t;

/*
 * Tesla Memory Region for AI Workloads
 * 
 * High-performance arena allocation for tensor operations and AI data
 * Eliminates fragmentation and GC overhead for bulk allocations
 */
typedef struct {
    void* base_ptr;                     // Base memory region
    atomic_size_t current_offset;       // Current allocation offset (atomic for thread safety)
    size_t total_size;                  // Total region size
    atomic_uint_fast32_t active_refs;   // Number of active allocations
    bool is_consciousness_synchronized; // Aligned with Tesla frequency
} tesla_memory_region_t;

/*
 * Enhanced Static Analysis State
 * 
 * Replaces the simple counter heuristic with proper static analysis
 */
typedef enum {
    TESLA_ANALYSIS_STATIC_SAFE,      // Proven safe by static analysis
    TESLA_ANALYSIS_COMPLEX_ALIASING, // Complex but analyzable aliasing
    TESLA_ANALYSIS_RECURSIVE_DATA,   // Self-referential structures
    TESLA_ANALYSIS_CROSS_THREAD,     // Multi-threaded access detected
    TESLA_ANALYSIS_DYNAMIC_LIFETIME, // Runtime-determined lifetime
    TESLA_ANALYSIS_PROMOTE_REQUIRED  // Must promote to GC/region
} tesla_static_analysis_result_t;

/*
 * Create atomic reference-counted object
 * 
 * @param data_size Size of data to allocate
 * @param destructor Optional cleanup function (can be NULL)
 * @return Pointer to tesla_atomic_refcount_t or NULL on failure
 */
tesla_atomic_refcount_t* tesla_atomic_refcount_create(size_t data_size, void (*destructor)(void*));

/*
 * Atomically increment strong reference count
 * 
 * @param ref Reference counter object
 * @return New reference count, or 0 if object was already deallocated
 */
uint32_t tesla_atomic_refcount_acquire(tesla_atomic_refcount_t* ref);

/*
 * Atomically decrement strong reference count
 * 
 * @param ref Reference counter object
 * @return New reference count (object deallocated if reaches 0)
 */
uint32_t tesla_atomic_refcount_release(tesla_atomic_refcount_t* ref);

/*
 * Get current strong reference count (for debugging)
 * 
 * @param ref Reference counter object
 * @return Current strong reference count
 */
uint32_t tesla_atomic_refcount_get_count(tesla_atomic_refcount_t* ref);

/*
 * Create memory region for AI workloads
 * 
 * @param total_size Size of memory region to allocate
 * @param consciousness_sync Whether to align with Tesla frequency
 * @return Pointer to tesla_memory_region_t or NULL on failure
 */
tesla_memory_region_t* tesla_memory_region_create(size_t total_size, bool consciousness_sync);

/*
 * Allocate from memory region (fast, no fragmentation)
 * 
 * @param region Memory region
 * @param size Size to allocate
 * @param alignment Memory alignment (must be power of 2)
 * @return Pointer to allocated memory or NULL if region full
 */
void* tesla_memory_region_alloc(tesla_memory_region_t* region, size_t size, size_t alignment);

/*
 * Reset entire memory region (deallocate all objects at once)
 * 
 * @param region Memory region to reset
 */
void tesla_memory_region_reset(tesla_memory_region_t* region);

/*
 * Destroy memory region and all allocated objects
 * 
 * @param region Memory region to destroy
 */
void tesla_memory_region_destroy(tesla_memory_region_t* region);

/*
 * Enhanced static analysis for variable lifetime
 * 
 * Performs true static analysis instead of simple heuristics
 * 
 * @param variable_name Name of variable to analyze
 * @param borrow_pattern Array of borrowing operations
 * @param pattern_length Number of operations in pattern
 * @return Analysis result indicating memory management strategy
 */
tesla_static_analysis_result_t tesla_analyze_variable_lifetime(const char* variable_name,
                                                             const char** borrow_pattern,
                                                             size_t pattern_length);

/*
 * Check if variable requires GC promotion based on static analysis
 * 
 * @param analysis_result Result from static analysis
 * @return true if GC promotion required, false if static management sufficient
 */
bool tesla_requires_gc_promotion(tesla_static_analysis_result_t analysis_result);

/*
 * Initialize enhanced memory management system
 * 
 * @return 0 on success, -1 on failure
 */
int tesla_enhanced_memory_init(void);

/*
 * Cleanup enhanced memory management system
 */
void tesla_enhanced_memory_cleanup(void);

#endif /* TESLA_ENHANCED_MEMORY_H */