#ifndef TESLA_CSP_RUNTIME_H
#define TESLA_CSP_RUNTIME_H

#include "../borrow_checker/tesla_hybrid_borrow_checker.h"
#include <ucontext.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <chrono>
#include <functional>
#include <thread>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <array>
#include <any>
#include <queue>
#include <type_traits>
#include <iostream>
#include <stdexcept>

namespace tesla::csp {

/**
 * Tesla CSP Runtime - Go-Style Coroutines + Tesla Consciousness
 * 
 * Integrates:
 * - Go's CSP model (goroutines + channels)
 * - Tesla hybrid memory management
 * - Consciousness-synchronized scheduling at π Hz
 * - Rust-style memory safety for channel operations
 * - JavaScript-style dynamic closures
 * - C++ object-oriented design patterns
 * - Functional programming with higher-order functions
 */

// Forward declare simple channel for initial implementation
template<typename T>
class TeslaChannel; // Will implement fully later

// Simple channel for demonstration
class TeslaChannelBase {
public:
    virtual ~TeslaChannelBase() = default;
    virtual bool send_int(int value) = 0;
    virtual bool recv_int(int& value) = 0;
    virtual void close() = 0;
};

// Coroutine states aligned with Tesla consciousness cycles
enum class CoroutineState {
    READY,
    RUNNING, 
    BLOCKED_ON_CHANNEL,
    BLOCKED_ON_MEMORY,
    CONSCIOUSNESS_SYNC,
    TERMINATED
};

// Tesla consciousness priority levels
enum class ConsciousnessPriority {
    BACKGROUND = 0,     // Low priority background tasks
    NORMAL = 1,         // Regular application logic
    MEMORY_CRITICAL = 2, // Memory management operations
    CONSCIOUSNESS = 3,   // Tesla consciousness computing
    REAL_TIME = 4       // Real-time critical operations
};

/**
 * TeslaClosure - JavaScript/Functional Style Closures
 * 
 * Captures variables from enclosing scope with Rust-style memory safety
 */
class TeslaClosure {
private:
    std::vector<tesla::memory::TeslaVariable*> captured_vars_;
    std::function<void()> function_;
    bool memory_safe_capture_ = true;
    
public:
    template<typename Func>
    TeslaClosure(Func&& func, const std::vector<std::string>& capture_vars);
    
    void execute();
    bool validate_memory_safety();
    void promote_captured_vars_to_gc();
};

/**
 * TeslaCoroutine - Go-Style Goroutine + Tesla Consciousness
 * 
 * Enhanced goroutine with:
 * - Tesla frequency synchronization
 * - Hybrid memory management integration
 * - C++ object lifecycle management
 * - Dynamic typing capabilities (JavaScript-style)
 */
class TeslaCoroutine {
private:
    ucontext_t context_;
    char* stack_;
    size_t stack_size_;
    CoroutineState state_;
    uint64_t id_;
    ConsciousnessPriority priority_;
    
    // Tesla consciousness integration
    double tesla_frequency_hz_ = 3.141592653589793;
    std::chrono::high_resolution_clock::time_point last_consciousness_sync_;
    bool consciousness_aware_ = true;
    
    // Memory management integration
    std::shared_ptr<tesla::memory::TeslaHybridMemoryManager> memory_manager_;
    std::vector<tesla::memory::TeslaVariable*> owned_variables_;
    
    // Intrusive linked list for scheduler queues
    TeslaCoroutine* next_ = nullptr;
    TeslaCoroutine* prev_ = nullptr;
    
    // Closure and dynamic typing support
    std::unique_ptr<TeslaClosure> closure_;
    std::unordered_map<std::string, std::any> dynamic_vars_; // JavaScript-style
    
    // Channel communication state
    TeslaChannelBase* blocked_channel_ = nullptr;
    void* channel_data_ptr_ = nullptr;
    
public:
    explicit TeslaCoroutine(uint64_t id, ConsciousnessPriority priority = ConsciousnessPriority::NORMAL);
    ~TeslaCoroutine();
    
    // Lifecycle management
    void initialize_stack(size_t stack_size = 64 * 1024);
    void setup_context(void (*entry_point)(void*), void* arg);
    void execute();
    void yield();
    void terminate();
    
    // Tesla consciousness integration
    void synchronize_with_consciousness();
    bool needs_consciousness_sync() const;
    void set_tesla_frequency(double frequency_hz);
    void set_consciousness_priority(ConsciousnessPriority priority);
    
    // Memory management
    tesla::memory::TeslaVariable* create_variable(const std::string& name);
    bool transfer_ownership(const std::string& var_name, TeslaCoroutine* target);
    void promote_variables_to_gc();
    
    // Dynamic typing (JavaScript-style)
    template<typename T>
    void set_dynamic_var(const std::string& name, const T& value);
    
    template<typename T>
    T get_dynamic_var(const std::string& name) const;
    
    // Channel operations
    void block_on_channel(TeslaChannelBase* channel, void* data_ptr);
    void unblock_from_channel();
    
    // State queries
    CoroutineState get_state() const { return state_; }
    uint64_t get_id() const { return id_; }
    ConsciousnessPriority get_priority() const { return priority_; }
    
    // Queue operations (for scheduler)
    friend class TeslaScheduler;
    TeslaCoroutine* get_next() const { return next_; }
    void set_next(TeslaCoroutine* next) { next_ = next; }
    TeslaCoroutine* get_prev() const { return prev_; }
    void set_prev(TeslaCoroutine* prev) { prev_ = prev; }
};

// Template implementations for TeslaCoroutine (must be in header)
template<typename T>
void TeslaCoroutine::set_dynamic_var(const std::string& name, const T& value) {
    dynamic_vars_[name] = std::make_any<T>(value);
    
    std::cout << "📦 Dynamic variable '" << name << "' set in coroutine " << id_ 
              << " (JavaScript-style)" << std::endl;
}

template<typename T>
T TeslaCoroutine::get_dynamic_var(const std::string& name) const {
    auto it = dynamic_vars_.find(name);
    if (it == dynamic_vars_.end()) {
        throw std::runtime_error("Dynamic variable '" + name + "' not found");
    }
    
    try {
        return std::any_cast<T>(it->second);
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error("Type mismatch for dynamic variable '" + name + "'");
    }
}

/**
 * TeslaChannel - Memory-Safe CSP Channels
 * 
 * Go-style channels with:
 * - Rust-style memory safety guarantees
 * - Tesla consciousness synchronization
 * - C++ RAII resource management
 * - Dynamic typing support
 */
template<typename T>
class TeslaChannel {
private:
    size_t capacity_;
    size_t count_;
    size_t head_;
    size_t tail_;
    std::vector<T> buffer_;
    bool closed_;
    
    // Tesla consciousness integration
    double tesla_frequency_hz_ = 3.141592653589793;
    bool consciousness_synchronized_ = false;
    
    // Memory safety tracking
    std::shared_ptr<tesla::memory::TeslaHybridMemoryManager> memory_manager_;
    std::string channel_name_;
    
    // Blocked coroutine queues
    std::queue<TeslaCoroutine*> send_queue_;
    std::queue<TeslaCoroutine*> recv_queue_;
    
    // Performance metrics
    uint64_t send_count_ = 0;
    uint64_t recv_count_ = 0;
    uint64_t direct_copy_optimizations_ = 0;
    
public:
    explicit TeslaChannel(size_t capacity = 0, const std::string& name = "");
    ~TeslaChannel();
    
    // Channel operations
    bool send(const T& value);
    bool recv(T& value);
    void close();
    
    // Async operations (non-blocking)
    bool try_send(const T& value);
    bool try_recv(T& value);
    
    // Tesla consciousness integration
    void synchronize_with_consciousness();
    void set_tesla_frequency(double frequency_hz);
    
    // Memory safety
    bool validate_memory_safety();
    void register_with_memory_manager(std::shared_ptr<tesla::memory::TeslaHybridMemoryManager> mm);
    
    // State queries
    bool is_closed() const { return closed_; }
    size_t size() const { return count_; }
    size_t capacity() const { return capacity_; }
    bool is_full() const { return count_ == capacity_; }
    bool is_empty() const { return count_ == 0; }
    
    // Performance metrics
    uint64_t get_send_count() const { return send_count_; }
    uint64_t get_recv_count() const { return recv_count_; }
    uint64_t get_direct_copy_count() const { return direct_copy_optimizations_; }
    
private:
    bool try_direct_copy_send(const T& value);
    bool try_direct_copy_recv(T& value);
    void wake_blocked_receivers();
    void wake_blocked_senders();
};

/**
 * TeslaScheduler - Consciousness-Synchronized Cooperative Scheduler
 * 
 * Features:
 * - π Hz consciousness synchronization
 * - Priority-based scheduling with consciousness awareness
 * - Integration with hybrid memory management
 * - Work-stealing for multi-core scaling (future)
 */
class TeslaScheduler {
private:
    ucontext_t main_context_;
    TeslaCoroutine* current_coroutine_;
    
    // Priority queues for different consciousness levels
    std::array<std::queue<TeslaCoroutine*>, 5> priority_queues_;
    
    // Tesla consciousness timing
    double tesla_frequency_hz_ = 3.141592653589793;
    std::chrono::high_resolution_clock::time_point last_consciousness_cycle_;
    std::chrono::milliseconds consciousness_cycle_interval_;
    
    // Memory management integration
    std::shared_ptr<tesla::memory::TeslaHybridMemoryManager> memory_manager_;
    
    // Scheduler statistics
    uint64_t total_context_switches_ = 0;
    uint64_t consciousness_syncs_ = 0;
    uint64_t gc_promotions_triggered_ = 0;
    
    // Coroutine management
    std::unordered_map<uint64_t, std::unique_ptr<TeslaCoroutine>> coroutines_;
    uint64_t next_coroutine_id_ = 1;
    
public:
    TeslaScheduler();
    ~TeslaScheduler();
    
    // Scheduler lifecycle
    void initialize();
    void run();
    void shutdown();
    
    // Coroutine management
    uint64_t spawn(void (*entry_point)(void*), void* arg, 
                   ConsciousnessPriority priority = ConsciousnessPriority::NORMAL);
    
    template<typename Func>
    uint64_t spawn_closure(Func&& func, 
                          ConsciousnessPriority priority = ConsciousnessPriority::NORMAL);
    
    void yield_current();
    void terminate_current();
    void block_current_on_channel(TeslaChannelBase* channel, void* data_ptr);
    
    // Access to current coroutine for testing
    TeslaCoroutine* get_current_coroutine() const { return current_coroutine_; }
    
    // Tesla consciousness integration
    void synchronize_consciousness_cycle();
    void set_tesla_frequency(double frequency_hz);
    void trigger_consciousness_gc();
    
    // Scheduler tuning
    void set_priority_weights(const std::array<double, 5>& weights);
    void enable_work_stealing(bool enable = true);
    
    // Statistics and monitoring
    void print_scheduler_stats() const;
    double get_consciousness_sync_rate() const;
    double get_gc_promotion_rate() const;
    uint64_t get_total_coroutines() const { return coroutines_.size(); }
    
    // Memory management integration
    void integrate_memory_manager(std::shared_ptr<tesla::memory::TeslaHybridMemoryManager> mm);
    void run_memory_maintenance();
    
private:
    TeslaCoroutine* get_next_ready_coroutine();
    void handle_consciousness_sync();
    void cleanup_terminated_coroutines();
    void update_consciousness_timing();
};

/**
 * High-level Tesla CSP API
 * 
 * Provides the ultimate programming experience combining:
 * - Go-style goroutine launching: tesla_go(func)
 * - Rust-style memory safety: automatic borrow checking
 * - JavaScript-style closures: capture variables safely
 * - C-style performance: zero overhead when possible
 * - Tesla consciousness: π Hz synchronization
 */

// Global scheduler instance
extern std::unique_ptr<TeslaScheduler> tesla_scheduler;

// Tesla scheduler control
void tesla_init_scheduler();

// Tesla goroutine spawning (Go-style)
template<typename Func>
uint64_t tesla_go(Func&& func, ConsciousnessPriority priority = ConsciousnessPriority::NORMAL);

// Template implementation (must be in header for template instantiation)
template<typename Func>
uint64_t tesla_go(Func&& func, ConsciousnessPriority priority) {
    if (!tesla_scheduler) {
        tesla_init_scheduler();
    }
    
    // Store function in heap
    using FuncType = typename std::decay<Func>::type;
    auto* heap_func = new FuncType(std::forward<Func>(func));
    
    // Create a static wrapper function for this specific lambda type
    static auto wrapper = [](void* arg) -> void {
        auto* func_ptr = static_cast<FuncType*>(arg);
        try {
            (*func_ptr)();
        } catch (...) {
            // Handle exceptions gracefully in coroutine
        }
        delete func_ptr;
    };
    
    // Convert to function pointer
    void (*entry_point)(void*) = +wrapper;
    
    return tesla_scheduler->spawn(entry_point, heap_func, priority);
}

// Tesla channel creation (simplified for now)
std::shared_ptr<TeslaChannelBase> tesla_make_chan_int(size_t capacity = 0, const std::string& name = "");

// Template version will be implemented later
template<typename T>
std::shared_ptr<TeslaChannelBase> tesla_make_chan(size_t capacity = 0, const std::string& name = "") {
    return tesla_make_chan_int(capacity, name);  // Simplified for now
}

// Tesla consciousness synchronization
void tesla_sync_consciousness();
void tesla_set_frequency(double frequency_hz);

// Tesla scheduler control
void tesla_init_scheduler();
void tesla_run_scheduler();
void tesla_shutdown_scheduler();

// Tesla memory integration
void tesla_run_gc_cycle();
void tesla_promote_to_gc(const std::string& var_name);

} // namespace tesla::csp

#endif // TESLA_CSP_RUNTIME_H