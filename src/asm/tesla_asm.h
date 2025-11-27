#ifndef TESLA_ASM_H
#define TESLA_ASM_H

/**
 * Tesla Assembly Acceleration Layer
 * Ultra-optimized assembly routines for maximum performance
 * 
 * Performance targets:
 * - Context switching: Sub-5ns
 * - Memory operations: REP MOVSB + AVX-512 optimization
 * - String processing: 16x speedup with PCMPISTRI
 * - Timing precision: RDTSC/RDTSCP for π Hz synchronization
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TESLA CONTEXT SWITCHING - Sub-5ns Coroutine Performance
// =============================================================================

/**
 * Tesla Context Structure - Must match assembly layout exactly
 * Represents the complete CPU state for Tesla consciousness computing
 */
typedef struct {
    uint64_t rsp;           // 0  - Stack pointer
    uint64_t rbp;           // 8  - Frame pointer  
    uint64_t rbx;           // 16 - Callee-saved register
    uint64_t r12;           // 24 - Tesla frequency counter
    uint64_t r13;           // 32 - Consciousness cycle ID
    uint64_t r14;           // 40 - Memory manager state
    uint64_t r15;           // 48 - Channel operation state
    uint64_t rip;           // 56 - Instruction pointer (return address)
    double   tesla_freq;    // 64 - π Hz frequency state
    uint64_t cycle_count;   // 72 - Consciousness cycle counter
} TeslaContext;

/**
 * Ultra-fast context switch between Tesla coroutines
 * Achieves sub-5ns switching through direct stack manipulation
 * 
 * @param old_ctx Context to save current state into
 * @param new_ctx Context to restore and switch to
 */
void tesla_swap_context(TeslaContext* old_ctx, TeslaContext* new_ctx);

/**
 * Initialize a new Tesla coroutine context
 * 
 * @param ctx Context structure to initialize
 * @param stack_base Base address of stack memory
 * @param stack_size Size of stack in bytes
 * @param entry_point Function to execute in this context
 * @param arg Argument to pass to entry_point
 */
void tesla_init_context(TeslaContext* ctx, void* stack_base, size_t stack_size, 
                       void (*entry_point)(void*), void* arg);

// =============================================================================
// TESLA MEMORY OPERATIONS - Consciousness-Synchronized Performance
// =============================================================================

/**
 * Tesla consciousness-synchronized memory copy
 * Uses REP MOVSB, AVX-512, and non-temporal stores for optimal performance
 * 
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @param sync_consciousness Whether to synchronize with Tesla frequency
 * @return Number of bytes copied
 */
size_t tesla_consciousness_memcpy(void* dest, const void* src, size_t n, bool sync_consciousness);

/**
 * Tesla consciousness-synchronized memory set
 * Optimized for large memory initialization
 * 
 * @param dest Destination buffer
 * @param value Byte value to set
 * @param n Number of bytes to set
 */
void tesla_consciousness_memset(void* dest, int value, size_t n);

// =============================================================================
// TESLA STRING PROCESSING - SIMD-Accelerated Text Operations
// =============================================================================

/**
 * Ultra-fast string length calculation using PCMPISTRI
 * 16x faster than traditional byte scanning
 * 
 * @param str Null-terminated string
 * @return Length of string in bytes
 */
size_t tesla_strlen(const char* str);

/**
 * SIMD-accelerated character search
 * 
 * @param str String to search in
 * @param ch Character to find
 * @return Pointer to first occurrence or NULL if not found
 */
char* tesla_strchr(const char* str, int ch);

/**
 * High-performance token scanner for language parsing
 * Finds positions of structural characters in text
 * 
 * @param text Text to scan
 * @param tokens String containing characters to find (e.g., "{}[]:,")
 * @param positions Array to store token positions
 * @param max_tokens Maximum number of tokens to find
 * @return Number of tokens found
 */
size_t tesla_find_tokens(const char* text, const char* tokens, 
                        size_t* positions, size_t max_tokens);

/**
 * Optimized string comparison for language keywords
 * 
 * @param s1 First string
 * @param s2 Second string
 * @return < 0 if s1 < s2, 0 if equal, > 0 if s1 > s2
 */
int tesla_strcmp(const char* s1, const char* s2);

// =============================================================================
// TESLA TIMING AND SYNCHRONIZATION - Precision π Hz Computing
// =============================================================================

/**
 * Precise CPU cycle counter reading with serialization
 * Guarantees accurate timing measurements
 * 
 * @return Current CPU cycle count
 */
uint64_t tesla_rdtsc_precise(void);

/**
 * End-of-execution cycle counter reading
 * Uses RDTSCP for completion-synchronized timing
 * 
 * @return CPU cycle count after all instructions complete
 */
uint64_t tesla_rdtscp_end(void);

/**
 * Measure execution time with precise cycle counting
 * 
 * @param start_cycles Pointer to store start cycle count
 * @param end_cycles Pointer to store end cycle count
 */
void tesla_time_execution(uint64_t* start_cycles, uint64_t* end_cycles);

/**
 * Optimized spinlock acquisition with PAUSE instruction
 * Reduces power consumption and pipeline interference
 * 
 * @param lock Pointer to lock variable
 */
void tesla_spinlock_acquire(volatile int* lock);

/**
 * Spinlock release
 * 
 * @param lock Pointer to lock variable
 */
void tesla_spinlock_release(volatile int* lock);

/**
 * Calculate frequency from cycle measurements
 * 
 * @param start_tsc Start cycle count
 * @param end_tsc End cycle count  
 * @param duration_seconds Duration in seconds
 * @return Frequency in Hz
 */
double tesla_calculate_frequency(uint64_t start_tsc, uint64_t end_tsc, double duration_seconds);

/**
 * Check if measured frequency is within tolerance of π Hz
 * 
 * @param measured_freq Measured frequency in Hz
 * @return true if within 0.1% of π Hz
 */
bool tesla_check_pi_frequency(double measured_freq);

/**
 * Yield execution with Tesla consciousness synchronization
 * 
 * @param cycle_count Current consciousness cycle count
 */
void tesla_yield_consciousness(uint64_t cycle_count);

// =============================================================================
// TESLA ASSEMBLY CONSTANTS
// =============================================================================

#define TESLA_PI_HZ 3.141592653589793   // π Hz consciousness frequency
#define TESLA_CYCLE_MS 318              // Milliseconds per π cycle
#define TESLA_CACHE_LINE_SIZE 64        // Modern CPU cache line size
#define TESLA_HUGE_COPY_THRESHOLD 33554432  // 32MB threshold for non-temporal stores

#ifdef __cplusplus
}
#endif

#endif // TESLA_ASM_H