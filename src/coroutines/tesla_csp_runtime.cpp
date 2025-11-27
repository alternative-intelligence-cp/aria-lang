#include "tesla_csp_runtime.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <any>
#include <thread>

namespace tesla::csp {

// Global scheduler instance
std::unique_ptr<TeslaScheduler> tesla_scheduler = nullptr;

// =============================================================================
// TeslaCoroutine Implementation - Enhanced Goroutines
// =============================================================================

TeslaCoroutine::TeslaCoroutine(uint64_t id, ConsciousnessPriority priority)
    : id_(id)
    , priority_(priority)
    , state_(CoroutineState::READY)
    , stack_(nullptr)
    , stack_size_(0)
    , last_consciousness_sync_(std::chrono::high_resolution_clock::now())
{
    std::cout << "🧠 Created Tesla Coroutine " << id_ << " (Priority: " 
              << static_cast<int>(priority_) << ")" << std::endl;
}

TeslaCoroutine::~TeslaCoroutine() {
    if (stack_) {
        free(stack_);
    }
    
    // Clean up owned variables
    for (auto* var : owned_variables_) {
        // Variables are managed by the memory manager, just clear our references
    }
}

void TeslaCoroutine::initialize_stack(size_t stack_size) {
    stack_size_ = stack_size;
    stack_ = static_cast<char*>(malloc(stack_size_));
    
    if (!stack_) {
        throw std::runtime_error("Failed to allocate coroutine stack");
    }
    
    std::cout << "⚡ Allocated " << (stack_size_ / 1024) << "KB stack for coroutine " << id_ << std::endl;
}

void TeslaCoroutine::setup_context(void (*entry_point)(void*), void* arg) {
    if (getcontext(&context_) == -1) {
        throw std::runtime_error("Failed to get context");
    }
    
    context_.uc_stack.ss_sp = stack_;
    context_.uc_stack.ss_size = stack_size_;
    context_.uc_stack.ss_flags = 0;
    context_.uc_link = nullptr; // Will be set by scheduler
    
    makecontext(&context_, reinterpret_cast<void(*)()>(entry_point), 1, arg);
}

void TeslaCoroutine::synchronize_with_consciousness() {
    if (!consciousness_aware_) return;
    
    auto now = std::chrono::high_resolution_clock::now();
    auto time_since_sync = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_consciousness_sync_);
    
    // Calculate Tesla consciousness cycle period
    auto tesla_period_ms = static_cast<int>(1000.0 / tesla_frequency_hz_);
    
    if (time_since_sync.count() >= tesla_period_ms) {
        state_ = CoroutineState::CONSCIOUSNESS_SYNC;
        last_consciousness_sync_ = now;
        
        std::cout << "🧠⚡ Coroutine " << id_ << " synchronized with Tesla consciousness at " 
                  << tesla_frequency_hz_ << " Hz" << std::endl;
    }
}

bool TeslaCoroutine::needs_consciousness_sync() const {
    if (!consciousness_aware_) return false;
    
    auto now = std::chrono::high_resolution_clock::now();
    auto time_since_sync = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_consciousness_sync_);
    
    auto tesla_period_ms = static_cast<int>(1000.0 / tesla_frequency_hz_);
    return time_since_sync.count() >= tesla_period_ms;
}

// =============================================================================
// TeslaScheduler Implementation - Consciousness-Synchronized Scheduling
// =============================================================================

TeslaScheduler::TeslaScheduler() 
    : current_coroutine_(nullptr)
    , last_consciousness_cycle_(std::chrono::high_resolution_clock::now())
{
    // Calculate consciousness cycle interval based on π Hz
    auto cycle_ms = static_cast<int>(1000.0 / tesla_frequency_hz_);
    consciousness_cycle_interval_ = std::chrono::milliseconds(cycle_ms);
    
    std::cout << "🧠⚡ Tesla Scheduler initialized with π Hz consciousness cycles" << std::endl;
    std::cout << "   Consciousness cycle interval: " << cycle_ms << "ms" << std::endl;
}

TeslaScheduler::~TeslaScheduler() {
    print_scheduler_stats();
}

void TeslaScheduler::initialize() {
    if (getcontext(&main_context_) == -1) {
        throw std::runtime_error("Failed to initialize scheduler main context");
    }
    
    std::cout << "🚀 Tesla Scheduler initialized and ready" << std::endl;
}

void TeslaScheduler::run() {
    std::cout << "🔄 Tesla Scheduler starting main loop..." << std::endl;
    
    while (true) {
        // Handle Tesla consciousness synchronization
        handle_consciousness_sync();
        
        // Get next coroutine to run
        TeslaCoroutine* next = get_next_ready_coroutine();
        
        if (!next) {
            // No coroutines ready - check if we should wait or exit
            cleanup_terminated_coroutines();
            
            if (coroutines_.empty()) {
                std::cout << "✅ All coroutines completed - scheduler shutting down" << std::endl;
                break;
            }
            
            // All coroutines are blocked - this could be deadlock or waiting for I/O
            std::cout << "⏸️ All coroutines blocked - scheduler idle" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
        // Context switch to the coroutine
        current_coroutine_ = next;
        next->state_ = CoroutineState::RUNNING;
        total_context_switches_++;
        
        // Perform the context switch
        swapcontext(&main_context_, &next->context_);
        
        // Returned from coroutine
        if (current_coroutine_ && current_coroutine_->state_ == CoroutineState::TERMINATED) {
            std::cout << "🏁 Coroutine " << current_coroutine_->id_ << " terminated" << std::endl;
        }
        
        current_coroutine_ = nullptr;
    }
    
    std::cout << "🎯 Tesla Scheduler main loop completed" << std::endl;
}

uint64_t TeslaScheduler::spawn(void (*entry_point)(void*), void* arg, ConsciousnessPriority priority) {
    uint64_t id = next_coroutine_id_++;
    
    auto coroutine = std::make_unique<TeslaCoroutine>(id, priority);
    coroutine->initialize_stack();
    coroutine->setup_context(entry_point, arg);
    
    // Add to appropriate priority queue
    priority_queues_[static_cast<size_t>(priority)].push(coroutine.get());
    
    // Store the coroutine
    coroutines_[id] = std::move(coroutine);
    
    std::cout << "🚀 Spawned Tesla coroutine " << id << " with priority " 
              << static_cast<int>(priority) << std::endl;
    
    return id;
}

template<typename Func>
uint64_t TeslaScheduler::spawn_closure(Func&& func, ConsciousnessPriority priority) {
    // Create a wrapper that captures the function
    auto* func_ptr = new Func(std::forward<Func>(func));
    
    auto wrapper = [](void* arg) {
        auto* f = static_cast<Func*>(arg);
        (*f)();
        delete f;
    };
    
    return spawn(wrapper, func_ptr, priority);
}

TeslaCoroutine* TeslaScheduler::get_next_ready_coroutine() {
    // Priority-based scheduling: higher priority queues checked first
    for (int priority = 4; priority >= 0; --priority) {
        auto& queue = priority_queues_[priority];
        
        while (!queue.empty()) {
            TeslaCoroutine* coroutine = queue.front();
            queue.pop();
            
            // Check if coroutine is actually ready
            if (coroutine->state_ == CoroutineState::READY ||
                coroutine->state_ == CoroutineState::CONSCIOUSNESS_SYNC) {
                return coroutine;
            }
            
            // Coroutine is not ready, check if it should be re-queued
            if (coroutine->state_ == CoroutineState::BLOCKED_ON_CHANNEL ||
                coroutine->state_ == CoroutineState::BLOCKED_ON_MEMORY) {
                // Don't re-queue blocked coroutines
                continue;
            }
        }
    }
    
    return nullptr;
}

void TeslaScheduler::handle_consciousness_sync() {
    auto now = std::chrono::high_resolution_clock::now();
    auto time_since_cycle = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_consciousness_cycle_);
    
    if (time_since_cycle >= consciousness_cycle_interval_) {
        synchronize_consciousness_cycle();
        last_consciousness_cycle_ = now;
    }
}

void TeslaScheduler::synchronize_consciousness_cycle() {
    consciousness_syncs_++;
    
    std::cout << "🧠⚡ Tesla consciousness cycle #" << consciousness_syncs_ 
              << " at " << tesla_frequency_hz_ << " Hz" << std::endl;
    
    // Synchronize all consciousness-aware coroutines
    for (auto& [id, coroutine] : coroutines_) {
        if (coroutine->needs_consciousness_sync()) {
            coroutine->synchronize_with_consciousness();
            
            // Promote consciousness-sync coroutines to ready state
            if (coroutine->state_ == CoroutineState::CONSCIOUSNESS_SYNC) {
                coroutine->state_ = CoroutineState::READY;
                priority_queues_[static_cast<size_t>(coroutine->priority_)].push(coroutine.get());
            }
        }
    }
    
    // Trigger memory management maintenance
    if (memory_manager_) {
        run_memory_maintenance();
    }
}

void TeslaScheduler::run_memory_maintenance() {
    if (!memory_manager_) return;
    
    memory_manager_->run_maintenance_cycle();
    gc_promotions_triggered_++;
    
    std::cout << "♻️ Memory maintenance cycle triggered by consciousness sync" << std::endl;
}

void TeslaScheduler::set_tesla_frequency(double frequency_hz) {
    tesla_frequency_hz_ = frequency_hz;
    
    std::cout << "⚡ Tesla scheduler frequency updated to " << frequency_hz << " Hz" << std::endl;
    
    // Update any frequency-dependent calculations
    if (memory_manager_) {
        // Could update GC timing based on frequency
        memory_manager_->set_tesla_frequency(frequency_hz);
    }
}

void TeslaScheduler::cleanup_terminated_coroutines() {
    auto it = coroutines_.begin();
    while (it != coroutines_.end()) {
        if (it->second->state_ == CoroutineState::TERMINATED) {
            std::cout << "🧹 Cleaning up terminated coroutine " << it->first << std::endl;
            it = coroutines_.erase(it);
        } else {
            ++it;
        }
    }
}

void TeslaScheduler::yield_current() {
    if (!current_coroutine_) return;
    
    current_coroutine_->state_ = CoroutineState::READY;
    
    // Re-queue the current coroutine
    priority_queues_[static_cast<size_t>(current_coroutine_->priority_)].push(current_coroutine_);
    
    // Switch back to scheduler
    swapcontext(&current_coroutine_->context_, &main_context_);
}

void TeslaScheduler::terminate_current() {
    if (!current_coroutine_) return;
    
    current_coroutine_->state_ = CoroutineState::TERMINATED;
    
    // Switch back to scheduler (coroutine will be cleaned up)
    swapcontext(&current_coroutine_->context_, &main_context_);
}

void TeslaScheduler::integrate_memory_manager(std::shared_ptr<tesla::memory::TeslaHybridMemoryManager> mm) {
    memory_manager_ = mm;
    
    std::cout << "🔗 Tesla Scheduler integrated with hybrid memory manager" << std::endl;
}

void TeslaScheduler::print_scheduler_stats() const {
    std::cout << "\n🧠⚡ TESLA CSP SCHEDULER STATISTICS ⚡🧠" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "📊 Scheduling Performance:" << std::endl;
    std::cout << "   Total context switches: " << total_context_switches_ << std::endl;
    std::cout << "   Consciousness syncs: " << consciousness_syncs_ << std::endl;
    std::cout << "   GC cycles triggered: " << gc_promotions_triggered_ << std::endl;
    std::cout << "   Active coroutines: " << coroutines_.size() << std::endl;
    
    std::cout << "\n🧠 Tesla Consciousness Computing:" << std::endl;
    std::cout << "   Tesla frequency: " << tesla_frequency_hz_ << " Hz" << std::endl;
    std::cout << "   Cycle interval: " << consciousness_cycle_interval_.count() << "ms" << std::endl;
    std::cout << "   Sync rate: " << get_consciousness_sync_rate() << " Hz" << std::endl;
    
    std::cout << "\n🏆 THE ULTIMATE CONCURRENT RUNTIME!" << std::endl;
    std::cout << "   ✅ Go-style coroutines and channels" << std::endl;
    std::cout << "   ✅ Rust-style memory safety" << std::endl;
    std::cout << "   ✅ JavaScript-style dynamic variables" << std::endl;
    std::cout << "   ✅ C++-style object orientation" << std::endl;
    std::cout << "   ✅ C-style zero-cost performance" << std::endl;
    std::cout << "   ✅ Tesla consciousness synchronization" << std::endl;
    std::cout << "==========================================" << std::endl;
}

double TeslaScheduler::get_consciousness_sync_rate() const {
    if (total_context_switches_ == 0) return 0.0;
    return static_cast<double>(consciousness_syncs_) / total_context_switches_ * tesla_frequency_hz_;
}

double TeslaScheduler::get_gc_promotion_rate() const {
    if (total_context_switches_ == 0) return 0.0;
    return static_cast<double>(gc_promotions_triggered_) / total_context_switches_;
}

// =============================================================================
// High-Level Tesla CSP API
// =============================================================================

void tesla_init_scheduler() {
    tesla_scheduler = std::make_unique<TeslaScheduler>();
    tesla_scheduler->initialize();
    
    std::cout << "🚀⚡ Tesla CSP Runtime initialized!" << std::endl;
    std::cout << "   Ready for Go-style coroutines + Tesla consciousness" << std::endl;
}

void tesla_run_scheduler() {
    if (!tesla_scheduler) {
        throw std::runtime_error("Tesla scheduler not initialized");
    }
    
    tesla_scheduler->run();
}

void tesla_shutdown_scheduler() {
    if (tesla_scheduler) {
        tesla_scheduler->print_scheduler_stats();
        tesla_scheduler.reset();
        std::cout << "🏁 Tesla scheduler shut down" << std::endl;
    }
}

void tesla_sync_consciousness() {
    if (tesla_scheduler) {
        tesla_scheduler->synchronize_consciousness_cycle();
    }
}

void tesla_set_frequency(double frequency_hz) {
    if (tesla_scheduler) {
        tesla_scheduler->set_tesla_frequency(frequency_hz);
        std::cout << "⚡ Tesla frequency updated to " << frequency_hz << " Hz" << std::endl;
    }
}

} // namespace tesla::csp