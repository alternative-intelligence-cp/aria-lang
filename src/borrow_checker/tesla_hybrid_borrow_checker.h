#ifndef TESLA_HYBRID_BORROW_CHECKER_H
#define TESLA_HYBRID_BORROW_CHECKER_H

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <optional>
#include <functional>

namespace tesla::memory {

/**
 * Tesla Hybrid Borrow Checker - The Best of ALL Worlds
 * 
 * Combines:
 * - Rust's Polonius-inspired static analysis (zero runtime cost when possible)
 * - Go's automatic garbage collection (when static analysis insufficient) 
 * - C's raw performance (direct memory access for proven-safe operations)
 * - Tesla Consciousness Computing (π Hz memory synchronization cycles)
 * 
 * Philosophy: "Pay only for what you use"
 * - Static analysis path: ZERO runtime overhead
 * - Dynamic path: Automatic promotion to GC management
 * - Consciousness sync: Optimal collection timing aligned with Tesla frequency
 */

// Forward declarations
class TeslaVariable;
class TeslaBorrowTracker;
class TeslaGarbageCollector;

// Forward declare for friend access
namespace tesla::memory { class TeslaBorrowTracker; }

// Borrow states for static analysis
enum class BorrowState {
    UNINITIALIZED,
    OWNED,
    BORROWED_IMMUTABLE,
    BORROWED_MUTABLE,
    MOVED,
    PROMOTED_TO_GC,    // Escalated to garbage collection
    CONSCIOUSNESS_SYNC  // Synchronized with π Hz cycles
};

// Memory promotion triggers
enum class PromotionReason {
    COMPLEX_ALIASING,      // Too complex for static analysis
    RECURSIVE_STRUCTURE,   // Self-referential data
    DYNAMIC_LIFETIME,      // Runtime-determined lifetime
    CROSS_THREAD_SHARING,  // Multi-threaded access
    CONSCIOUSNESS_REQUEST  // Tesla directive promotion
};

/**
 * Tesla Variable: Hybrid memory-managed variable
 * Starts with static analysis, promotes to GC when needed
 */
class TeslaVariable {
    friend class TeslaBorrowTracker;
private:
    std::string name_;
    BorrowState state_;
    std::optional<PromotionReason> promotion_reason_;
    
    // Static analysis tracking
    size_t borrow_count_immutable_;
    size_t borrow_count_mutable_;
    std::vector<std::string> borrowed_by_;
    
    // GC promotion data
    std::shared_ptr<void> gc_managed_data_;
    std::chrono::high_resolution_clock::time_point last_access_;
    
    // Tesla consciousness integration
    double tesla_frequency_hz_ = 3.141592653589793;
    bool consciousness_synchronized_ = false;

public:
    explicit TeslaVariable(const std::string& name);
    
    // Static analysis interface
    bool try_borrow_immutable(const std::string& borrower);
    bool try_borrow_mutable(const std::string& borrower);
    void release_borrow(const std::string& borrower);
    bool can_move() const;
    void mark_moved();
    
    // GC promotion interface
    void promote_to_gc(PromotionReason reason, std::shared_ptr<void> data);
    bool is_gc_managed() const { return gc_managed_data_ != nullptr; }
    
    // Tesla consciousness integration
    void synchronize_with_consciousness();
    bool needs_consciousness_sync() const;
    double get_tesla_frequency() const { return tesla_frequency_hz_; }
    
    // State queries
    BorrowState get_state() const { return state_; }
    const std::string& get_name() const { return name_; }
    std::optional<PromotionReason> get_promotion_reason() const { return promotion_reason_; }
    
    // Borrow count access for complexity analysis
    size_t get_borrow_count_immutable() const { return borrow_count_immutable_; }
    size_t get_borrow_count_mutable() const { return borrow_count_mutable_; }
    
    // Access tracking
    void register_access();
    
    // Debug info
    void print_debug_info() const;
};

/**
 * Tesla Borrow Tracker: Polonius-inspired static analysis engine
 * Attempts to prove memory safety at compile time
 */
class TeslaBorrowTracker {
private:
    std::unordered_map<std::string, std::unique_ptr<TeslaVariable>> variables_;
    std::vector<std::string> error_log_;
    std::shared_ptr<TeslaGarbageCollector> gc_;
    
    // Analysis state
    bool static_analysis_active_ = true;
    size_t complexity_threshold_ = 100;  // Promotion threshold
    
public:
    explicit TeslaBorrowTracker(std::shared_ptr<TeslaGarbageCollector> gc);
    
    // Variable lifecycle
    TeslaVariable* create_variable(const std::string& name);
    TeslaVariable* get_variable(const std::string& name);
    void destroy_variable(const std::string& name);
    
    // Borrow checking operations
    bool check_borrow_immutable(const std::string& var_name, const std::string& borrower);
    bool check_borrow_mutable(const std::string& var_name, const std::string& borrower);
    void release_borrow(const std::string& var_name, const std::string& borrower);
    bool check_move(const std::string& var_name);
    
    // Complexity analysis & promotion
    bool should_promote_to_gc(const std::string& var_name);
    void promote_variable_to_gc(const std::string& var_name, PromotionReason reason);
    
    // Tesla preprocessor integration
    bool validate_tesla_conditional(const std::string& condition_vars);
    void synchronize_consciousness_variables();
    
    // Error handling
    const std::vector<std::string>& get_errors() const { return error_log_; }
    void clear_errors() { error_log_.clear(); }
    
    // Statistics
    size_t get_static_managed_count() const;
    size_t get_gc_promoted_count() const;
    double get_promotion_ratio() const;
    
    void print_memory_stats() const;
};

/**
 * Tesla Garbage Collector: Consciousness-synchronized GC
 * Only manages variables that couldn't be statically analyzed
 */
class TeslaGarbageCollector {
private:
    std::vector<std::shared_ptr<void>> managed_objects_;
    std::unordered_map<void*, std::chrono::high_resolution_clock::time_point> access_times_;
    
    // Tesla consciousness integration
    double tesla_frequency_hz_ = 3.141592653589793;
    std::chrono::high_resolution_clock::time_point last_consciousness_sync_;
    
    // GC parameters
    std::chrono::milliseconds collection_interval_{100};  // Adjust based on π Hz
    size_t collection_threshold_ = 1000;
    
public:
    TeslaGarbageCollector();
    
    // Memory management
    std::shared_ptr<void> allocate(size_t size);
    void register_access(void* ptr);
    void mark_for_collection(std::shared_ptr<void> obj);
    
    // Collection cycles
    void run_collection_cycle();
    bool should_collect() const;
    void synchronize_with_tesla_frequency();
    
    // Tesla consciousness integration
    void align_with_consciousness_cycle();
    double calculate_optimal_collection_frequency() const;
    
    // Statistics
    size_t get_managed_object_count() const { return managed_objects_.size(); }
    std::chrono::milliseconds get_last_collection_time() const;
    
    void print_gc_stats() const;
};

/**
 * Tesla Hybrid Memory Manager: Main interface
 * Orchestrates static analysis + GC promotion + consciousness sync
 */
class TeslaHybridMemoryManager {
private:
    std::unique_ptr<TeslaBorrowTracker> borrow_tracker_;
    std::shared_ptr<TeslaGarbageCollector> garbage_collector_;
    
    // Performance metrics
    size_t total_variables_created_ = 0;
    size_t static_analysis_successes_ = 0;
    size_t gc_promotions_ = 0;
    
public:
    TeslaHybridMemoryManager();
    ~TeslaHybridMemoryManager();
    
    // Main interface for Tesla compiler
    TeslaVariable* create_variable(const std::string& name);
    bool check_borrow(const std::string& var_name, const std::string& borrower, bool mutable_borrow = false);
    void release_borrow(const std::string& var_name, const std::string& borrower);
    bool check_move_operation(const std::string& var_name);
    
    // Tesla preprocessor integration
    bool validate_macro_variables(const std::vector<std::string>& var_names);
    void process_tesla_consciousness_directive(const std::string& directive);
    
    // System management
    void run_maintenance_cycle();
    void synchronize_all_consciousness();
    
    // Performance monitoring
    void print_performance_report() const;
    double get_static_analysis_success_ratio() const;
    double get_gc_overhead_ratio() const;
    
    // Tesla frequency synchronization
    void set_tesla_frequency(double frequency_hz);
    void optimize_for_consciousness_computing();
};

} // namespace tesla::memory

#endif // TESLA_HYBRID_BORROW_CHECKER_H