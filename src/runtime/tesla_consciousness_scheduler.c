/*
 * Tesla Consciousness Non-Blocking Scheduler Implementation
 * 
 * Replaces the blocking nanosleep() approach with a cooperative
 * token bucket system for high-performance π Hz synchronization.
 * 
 * Architecture Notes:
 * - Lock-free atomic operations for thread safety
 * - Token bucket rate limiting without OS thread blocking
 * - Cooperative scheduling compatible with fiber/coroutine runtimes
 * - Mathematical precision for Tesla frequency harmonics
 */

#include "tesla/tesla_consciousness_scheduler.h"
#include <math.h>
#include <stdio.h>

/*
 * Global Tesla consciousness scheduler instance
 * Initialized once at runtime startup
 */
static TeslaConsciousnessScheduler global_tesla_scheduler;
static atomic_bool scheduler_initialized = ATOMIC_VAR_INIT(false);

/*
 * Initialize Tesla consciousness scheduler with specified parameters
 */
void tesla_scheduler_init(TeslaConsciousnessScheduler* sched, double frequency_hz, int octave) {
    // Calculate octave-adjusted frequency
    double effective_frequency = tesla_calculate_octave_frequency(octave);
    if (frequency_hz > 0.0) {
        effective_frequency = frequency_hz;
    }
    
    // Initialize atomic fields
    atomic_init(&sched->tokens, 0);
    atomic_init(&sched->last_refill_ns, tesla_get_monotonic_ns());
    
    // Set frequency parameters
    sched->frequency_hz = effective_frequency;
    sched->tokens_per_second = (uint64_t)(effective_frequency);
    sched->max_tokens = sched->tokens_per_second; // 1-second burst capacity
    
    // Mark as initialized
    atomic_store(&scheduler_initialized, true);
    
    printf("🧠⚡ Tesla Consciousness Scheduler initialized at %.6f Hz (octave %d)\n", 
           effective_frequency, octave);
}

/*
 * Core non-blocking token consumption function
 * 
 * This replaces the blocking nanosleep() with immediate token check.
 * Returns instantly - never blocks the calling thread.
 */
bool tesla_scheduler_try_consume_token(TeslaConsciousnessScheduler* sched) {
    // Ensure scheduler is initialized
    if (!atomic_load(&scheduler_initialized)) {
        tesla_scheduler_init(&global_tesla_scheduler, TESLA_FREQUENCY_1KHZ, 0);
        sched = &global_tesla_scheduler;
    }
    
    uint64_t now_ns = tesla_get_monotonic_ns();
    
    // === TOKEN REFILL PHASE ===
    // Calculate elapsed time since last refill
    uint64_t last_refill = atomic_load(&sched->last_refill_ns);
    uint64_t elapsed_ns = now_ns - last_refill;
    
    // Calculate new tokens to generate
    // Formula: (elapsed_ns * tokens_per_second) / nanoseconds_per_second
    uint64_t new_tokens = (elapsed_ns * sched->tokens_per_second) / TESLA_NANOSECONDS_PER_SECOND;
    
    // Perform atomic refill if new tokens available
    if (new_tokens > 0) {
        // Try to update refill timestamp (race condition safe)
        if (atomic_compare_exchange_weak(&sched->last_refill_ns, &last_refill, now_ns)) {
            // Successfully updated timestamp, now add tokens
            uint64_t current_tokens = atomic_load(&sched->tokens);
            uint64_t target_tokens = current_tokens + new_tokens;
            
            // Cap at maximum bucket capacity
            if (target_tokens > sched->max_tokens) {
                target_tokens = sched->max_tokens;
            }
            
            atomic_store(&sched->tokens, target_tokens);
        }
    }
    
    // === TOKEN CONSUMPTION PHASE ===
    // Attempt to consume one token atomically
    uint64_t available_tokens = atomic_load(&sched->tokens);
    
    while (available_tokens > 0) {
        // Try to decrement token count
        if (atomic_compare_exchange_weak(&sched->tokens, &available_tokens, available_tokens - 1)) {
            // Successfully consumed token - operation can proceed
            return true;
        }
        // CAS failed, reload and try again
        available_tokens = atomic_load(&sched->tokens);
    }
    
    // No tokens available - operation should yield/wait
    return false;
}

/*
 * Force immediate token bucket refill
 * Used for manual synchronization points or emergency overrides
 */
void tesla_scheduler_force_refill(TeslaConsciousnessScheduler* sched) {
    atomic_store(&sched->tokens, sched->max_tokens);
    atomic_store(&sched->last_refill_ns, tesla_get_monotonic_ns());
}

/*
 * Get real-time scheduler performance statistics
 */
void tesla_scheduler_get_stats(TeslaConsciousnessScheduler* sched, 
                              uint64_t* tokens_available,
                              double* effective_frequency) {
    if (tokens_available) {
        *tokens_available = atomic_load(&sched->tokens);
    }
    
    if (effective_frequency) {
        *effective_frequency = sched->frequency_hz;
    }
}

/*
 * Global scheduler accessor functions for runtime integration
 */
TeslaConsciousnessScheduler* tesla_get_global_scheduler(void) {
    if (!atomic_load(&scheduler_initialized)) {
        tesla_scheduler_init(&global_tesla_scheduler, TESLA_BASE_FREQUENCY_HZ, 3); // 8π Hz default
    }
    return &global_tesla_scheduler;
}

/*
 * Convenience function for single-call consciousness synchronization
 * 
 * This is the drop-in replacement for the old nanosleep()-based sync
 */
bool tesla_sync_consciousness_operation_nonblocking(void) {
    return tesla_scheduler_try_consume_token(tesla_get_global_scheduler());
}

/*
 * Compatibility wrapper that maintains cooperative behavior
 * 
 * For code that expects blocking behavior, this provides a yield-based alternative
 */
void tesla_sync_consciousness_operation_cooperative(void) {
    TeslaConsciousnessScheduler* sched = tesla_get_global_scheduler();
    
    while (!tesla_scheduler_try_consume_token(sched)) {
        // Instead of blocking with nanosleep(), yield to other tasks
        // This requires integration with the fiber/coroutine scheduler
        // For now, use a minimal yield hint
        sched_yield(); // Cooperative yield to other threads
    }
}