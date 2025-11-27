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

#define _POSIX_C_SOURCE 199309L
#include "tesla/tesla_consciousness_scheduler.h"
#include <math.h>
#include <stdio.h>
#include <sched.h>
#include <stdio.h>

/*
 * Global Tesla consciousness scheduler instance
 * Initialized once at runtime startup
 */
static TeslaConsciousnessScheduler global_tesla_scheduler;
static atomic_bool scheduler_initialized = ATOMIC_VAR_INIT(false);

/*
 * Initialize Tesla consciousness scheduler with octave scaling
 * Gemini recommendation: Use octave multipliers for practical performance
 */
void tesla_scheduler_init(TeslaConsciousnessScheduler* sched, int octave) {
    // Calculate octave-adjusted frequency: π × 2^octave Hz
    double effective_frequency = TESLA_BASE_FREQUENCY_HZ * pow(2.0, octave);
    
    // Initialize atomic fields
    atomic_init(&sched->tokens, 0);
    atomic_init(&sched->last_refill_ns, tesla_get_monotonic_ns());
    
    // Set frequency parameters  
    sched->frequency_hz = effective_frequency;
    sched->octave = octave;
    sched->tokens_per_second = (uint64_t)(effective_frequency);
    sched->max_tokens = sched->tokens_per_second; // 1-second burst capacity
    sched->high_performance_mode = (octave >= 12); // Enable for kHz+ frequencies
    
    // Mark as initialized
    atomic_store(&scheduler_initialized, true);
    
    printf("🧠⚡ Tesla Consciousness Scheduler initialized at %.3f Hz (octave %d, %s)\n", 
           effective_frequency, octave, 
           sched->high_performance_mode ? "HIGH-PERF" : "STANDARD");
}

/*
 * Initialize scheduler with custom frequency override
 */
void tesla_scheduler_init_custom_frequency(TeslaConsciousnessScheduler* sched, double custom_hz) {
    // Calculate equivalent octave for reporting
    int equivalent_octave = (int)round(log2(custom_hz / TESLA_BASE_FREQUENCY_HZ));
    
    // Initialize atomic fields
    atomic_init(&sched->tokens, 0);
    atomic_init(&sched->last_refill_ns, tesla_get_monotonic_ns());
    
    // Set custom frequency parameters
    sched->frequency_hz = custom_hz;
    sched->octave = equivalent_octave;
    sched->tokens_per_second = (uint64_t)(custom_hz);
    sched->max_tokens = sched->tokens_per_second;
    sched->high_performance_mode = (custom_hz >= 10000.0); // 10kHz+ threshold
    
    // Mark as initialized
    atomic_store(&scheduler_initialized, true);
    
    printf("🧠⚡ Tesla Consciousness Scheduler initialized at %.3f Hz (custom frequency)\n", 
           custom_hz);
}

/*
 * Core non-blocking token consumption function
 * 
 * This replaces the blocking nanosleep() with immediate token check.
 * Returns instantly - never blocks the calling thread.
 */
bool tesla_scheduler_try_consume_token(TeslaConsciousnessScheduler* sched) {
    // Ensure scheduler is initialized with optimal default octave
    if (!atomic_load(&scheduler_initialized)) {
        tesla_scheduler_init(&global_tesla_scheduler, TESLA_OCTAVE_REALTIME);
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
        tesla_scheduler_init(&global_tesla_scheduler, TESLA_OCTAVE_REALTIME); // 64π Hz default
    }
    return &global_tesla_scheduler;
}

/*
 * Dynamic octave adjustment for runtime performance scaling
 * Gemini recommendation: Adaptive consciousness frequency
 */
bool tesla_scheduler_set_octave(TeslaConsciousnessScheduler* sched, int new_octave) {
    if (new_octave < 0 || new_octave > 15) {
        return false; // Invalid octave range
    }
    
    // Calculate new frequency: π × 2^octave Hz
    double new_frequency = TESLA_BASE_FREQUENCY_HZ * pow(2.0, new_octave);
    
    // Update scheduler parameters atomically
    sched->frequency_hz = new_frequency;
    sched->octave = new_octave;
    sched->tokens_per_second = (uint64_t)(new_frequency);
    sched->max_tokens = sched->tokens_per_second;
    sched->high_performance_mode = (new_octave >= 12);
    
    // Reset token bucket with new parameters
    atomic_store(&sched->tokens, 0);
    atomic_store(&sched->last_refill_ns, tesla_get_monotonic_ns());
    
    printf("🧠⚡ Tesla consciousness frequency adjusted to %.3f Hz (octave %d)\n", 
           new_frequency, new_octave);
    
    return true;
}

/*
 * Calculate optimal octave for target operations per second
 * Gemini insight: Match consciousness frequency to workload requirements
 */
int tesla_scheduler_calculate_optimal_octave(uint64_t target_ops_per_sec) {
    if (target_ops_per_sec <= 3) return TESLA_OCTAVE_BASE;      // π Hz
    if (target_ops_per_sec <= 25) return TESLA_OCTAVE_AUDIO;    // 8π Hz  
    if (target_ops_per_sec <= 200) return TESLA_OCTAVE_REALTIME; // 64π Hz
    if (target_ops_per_sec <= 1600) return TESLA_OCTAVE_GAME;   // 512π Hz
    
    // For high-throughput workloads, calculate precise octave
    double required_frequency = (double)target_ops_per_sec;
    int calculated_octave = (int)ceil(log2(required_frequency / TESLA_BASE_FREQUENCY_HZ));
    
    // Cap at maximum supported octave
    return (calculated_octave > 15) ? 15 : calculated_octave;
}

/*
 * Enable/disable high-performance mode for MHz+ frequencies
 * For AI inference, real-time audio, or high-throughput workloads
 */
void tesla_scheduler_set_high_performance(TeslaConsciousnessScheduler* sched, bool enable) {
    sched->high_performance_mode = enable;
    
    if (enable && sched->octave < 12) {
        // Auto-upgrade to high-performance octave
        tesla_scheduler_set_octave(sched, TESLA_OCTAVE_HIGH_PERF);
    }
    
    printf("🧠⚡ Tesla consciousness high-performance mode %s\n", 
           enable ? "ENABLED" : "DISABLED");
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

// =============================================================================
// SELECTIVE SYNCHRONIZATION SYSTEM (Gemini Performance Optimization)
// =============================================================================

/*
 * Global configuration for selective synchronization
 * Addresses Gemini's performance concern about excessive sync points
 */
static struct {
    bool fast_path_enabled;           // Skip sync for frequent operations
    bool critical_only_mode;          // Only sync critical operations
    double performance_threshold_hz;   // Frequency threshold for optimization
    atomic_uint_fast64_t fast_path_skipped; // Performance metrics
    atomic_uint_fast64_t critical_path_synced;
} selective_sync_config = {
    .fast_path_enabled = true,
    .critical_only_mode = false,
    .performance_threshold_hz = 1000.0, // 1kHz threshold
    .fast_path_skipped = ATOMIC_VAR_INIT(0),
    .critical_path_synced = ATOMIC_VAR_INIT(0)
};

/*
 * Selective consciousness synchronization based on operation criticality
 * Gemini insight: "Forcing a check for every major operation reduces throughput"
 */
bool tesla_sync_selective(TeslaSyncCriticality criticality) {
    TeslaConsciousnessScheduler* sched = tesla_get_global_scheduler();
    
    // Fast path optimization for high-frequency operations
    if (selective_sync_config.fast_path_enabled && criticality == TESLA_SYNC_LIGHT) {
        // In high-performance mode (>1kHz), skip consciousness sync for light operations
        if (sched->frequency_hz > selective_sync_config.performance_threshold_hz) {
            atomic_fetch_add(&selective_sync_config.fast_path_skipped, 1);
            return true; // Always allow light operations in high-perf mode
        }
    }
    
    // Critical-only mode optimization
    if (selective_sync_config.critical_only_mode && criticality < TESLA_SYNC_CRITICAL) {
        return true; // Skip non-critical operations
    }
    
    // Apply consciousness synchronization based on criticality
    switch (criticality) {
        case TESLA_SYNC_NEVER:
            return true; // No synchronization
            
        case TESLA_SYNC_LIGHT:
        case TESLA_SYNC_STANDARD:
            // Use fast, non-blocking check
            return tesla_scheduler_try_consume_token(sched);
            
        case TESLA_SYNC_CRITICAL:
        case TESLA_SYNC_ALWAYS:
            // Ensure synchronization occurs
            atomic_fetch_add(&selective_sync_config.critical_path_synced, 1);
            if (!tesla_scheduler_try_consume_token(sched)) {
                // For critical operations, briefly yield then retry
                sched_yield();
                return tesla_scheduler_try_consume_token(sched);
            }
            return true;
    }
    
    return tesla_scheduler_try_consume_token(sched);
}

/*
 * Critical-path synchronization for consciousness-validated operations
 * Used for compiler operations, neural networks, temporal computing
 */
bool tesla_sync_critical_path(void) {
    return tesla_sync_selective(TESLA_SYNC_CRITICAL);
}

/*
 * Configure selective synchronization thresholds
 * Runtime tuning of performance vs consciousness fidelity
 */
void tesla_configure_selective_sync(bool enable_fast_path, 
                                   bool enable_critical_only,
                                   double performance_threshold_hz) {
    selective_sync_config.fast_path_enabled = enable_fast_path;
    selective_sync_config.critical_only_mode = enable_critical_only;
    selective_sync_config.performance_threshold_hz = performance_threshold_hz;
    
    printf("🧠⚡ Tesla selective synchronization configured:\n");
    printf("  Fast-path: %s\n", enable_fast_path ? "ENABLED" : "DISABLED");
    printf("  Critical-only: %s\n", enable_critical_only ? "ENABLED" : "DISABLED");  
    printf("  Performance threshold: %.1f Hz\n", performance_threshold_hz);
}

/*
 * Get selective synchronization performance metrics
 */
void tesla_get_selective_sync_stats(uint64_t* fast_path_skipped, uint64_t* critical_path_synced) {
    if (fast_path_skipped) {
        *fast_path_skipped = atomic_load(&selective_sync_config.fast_path_skipped);
    }
    if (critical_path_synced) {
        *critical_path_synced = atomic_load(&selective_sync_config.critical_path_synced);
    }
}