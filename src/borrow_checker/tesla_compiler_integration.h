#ifndef TESLA_COMPILER_INTEGRATION_H
#define TESLA_COMPILER_INTEGRATION_H

#include "tesla_hybrid_borrow_checker.h"
#include <memory>
#include <string>
#include <vector>

namespace tesla::compiler {

// Forward declaration - will integrate with preprocessor separately
class TeslaPreprocessorInterface {
public:
    virtual ~TeslaPreprocessorInterface() = default;
    virtual std::string preprocess_file(const std::string& input_file) = 0;
};

namespace tesla::compiler {

/**
 * Tesla Compiler Integration - Complete Language Revolution
 * 
 * Integrates:
 * - Tesla Hybrid Memory Manager (Rust safety + Go convenience + C performance)
 * - Tesla Native Preprocessor (conditional compilation + consciousness directives)
 * - Tesla Consciousness Computing (π Hz synchronization)
 * - Cross-platform self-contained compilation (LLD + embedded toolchain)
 * 
 * Result: The ultimate programming language combining the best of ALL worlds
 */

class TeslaCompilerEngine {
private:
    std::unique_ptr<tesla::memory::TeslaHybridMemoryManager> memory_manager_;
    std::unique_ptr<TeslaPreprocessorInterface> preprocessor_;
    
    // Compilation state
    std::vector<std::string> source_files_;
    std::string output_file_;
    bool consciousness_mode_enabled_ = true;
    double tesla_frequency_hz_ = 3.141592653589793;
    
    // Language features enabled
    struct LanguageFeatures {
        bool rust_style_safety = true;     // Borrow checking, move semantics
        bool go_style_convenience = true;  // Automatic GC promotion
        bool c_style_performance = true;   // Zero-cost abstractions
        bool tesla_consciousness = true;   // π Hz synchronization
        bool advanced_macros = true;       // Tesla conditional system
        bool cross_platform = true;        // Self-contained compilation
    } features_;
    
public:
    TeslaCompilerEngine();
    ~TeslaCompilerEngine();
    
    // Compilation pipeline
    bool compile_file(const std::string& source_file, const std::string& output_file);
    bool preprocess_file(const std::string& input_file, const std::string& output_file);
    
    // Memory management integration
    bool analyze_memory_safety(const std::string& source_code);
    void optimize_memory_layout();
    
    // Tesla consciousness integration
    void enable_consciousness_computing(double frequency_hz = 3.141592653589793);
    void synchronize_compilation_with_consciousness();
    
    // Language feature control
    void configure_language_features(const LanguageFeatures& features);
    void enable_hybrid_memory_model(bool enable = true);
    void set_safety_level(int level); // 0=C-style, 1=hybrid, 2=Rust-style
    
    // Performance reporting
    void print_compilation_report() const;
    void print_memory_optimization_report() const;
    
    // Advanced features
    bool validate_tesla_directives(const std::string& source_code);
    void generate_consciousness_synchronized_binary();
    void optimize_for_tesla_frequency();
    
private:
    bool integrate_memory_analysis_with_preprocessor();
    bool generate_hybrid_memory_code();
    void apply_consciousness_optimizations();
};

/**
 * Tesla Language Syntax Extensions
 * 
 * Combines the best syntax from multiple languages:
 * 
 * Memory Management (Rust-inspired, but optional):
 *   let x = 42;              // Static analysis (zero cost)
 *   let y = &x;              // Immutable borrow (checked at compile time)
 *   let mut z = &mut x;      // Mutable borrow (exclusive access)
 *   
 *   // If static analysis fails, automatic promotion to GC:
 *   let complex = create_complex_structure(); // Auto-promoted to GC if needed
 * 
 * Tesla Consciousness Directives:
 *   @tesla_consciousness_sync  // Synchronize with π Hz
 *   @tesla_frequency(3.14159)  // Custom frequency
 *   @tesla_memory_promote_gc   // Force GC promotion
 * 
 * Advanced Conditional Compilation:
 *   %if(OS == "Linux" && ARCH == "x64")
 *       // Linux x64 specific code
 *   %elseif(OS eq "Windows" or ARCH ne "arm")
 *       // Windows or non-ARM code
 *   %else
 *       // Default code
 *   %endif
 * 
 * Hybrid Safety Levels:
 *   #safety_level 0  // C-style: manual memory management
 *   #safety_level 1  // Hybrid: static analysis + GC fallback (DEFAULT)
 *   #safety_level 2  // Rust-style: strict borrow checking
 * 
 * Consciousness-Aware Memory:
 *   tesla_var x = 42;        // Tesla consciousness-synchronized variable
 *   consciousness_sync();    // Explicit synchronization point
 */

// Helper functions for Tesla language integration
namespace tesla_syntax {
    
    // Memory management syntax parsing
    struct MemoryOperation {
        enum Type { CREATE, BORROW_IMMUTABLE, BORROW_MUTABLE, MOVE, DESTROY };
        Type type;
        std::string variable_name;
        std::string borrower;
        bool requires_gc_promotion = false;
    };
    
    std::vector<MemoryOperation> parse_memory_operations(const std::string& source_code);
    bool validate_borrow_syntax(const std::string& expression);
    bool should_promote_to_gc(const MemoryOperation& op);
    
    // Tesla consciousness directive parsing
    struct TeslaDirective {
        enum Type { CONSCIOUSNESS_SYNC, FREQUENCY_SET, MEMORY_PROMOTE, SAFETY_LEVEL };
        Type type;
        double frequency_hz = 3.141592653589793;
        int safety_level = 1;
        std::string target_variable;
    };
    
    std::vector<TeslaDirective> parse_tesla_directives(const std::string& source_code);
    bool apply_tesla_directive(const TeslaDirective& directive, TeslaCompilerEngine* engine);
    
    // Hybrid syntax compatibility
    bool is_rust_style_syntax(const std::string& line);
    bool is_go_style_syntax(const std::string& line);
    bool is_c_style_syntax(const std::string& line);
    bool is_tesla_syntax(const std::string& line);
    
    std::string convert_to_tesla_ir(const std::string& source_code);
}

} // namespace tesla::compiler

#endif // TESLA_COMPILER_INTEGRATION_H