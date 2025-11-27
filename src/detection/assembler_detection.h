#pragma once

#include <string>
#include <vector>

enum class AssemblerType {
    LLVM_MC,
    NASM,
    AUTO_DETECT
};

enum class CCompilerType {
    TCC,
    AUTO_DETECT
};

struct AssemblyFeatures {
    bool has_inline_asm = false;
    bool has_macros = false;
    bool has_meta_programming = false;
    bool has_context_stack = false;
    bool has_tesla_directives = false;
    bool needs_c_interop = false;
};

class AssemblerDetector {
public:
    AssemblerDetector() = default;
    
    // Main detection method
    AssemblerType detect_best_assembler(const std::string& source_code);
    CCompilerType detect_best_c_compiler(const std::string& source_code);
    
    // Feature analysis
    AssemblyFeatures analyze_features(const std::string& source_code);
    
    // Force specific assembler/compiler
    void force_assembler(AssemblerType type) { forced_assembler = type; }
    void force_c_compiler(CCompilerType type) { forced_c_compiler = type; }
    
private:
    AssemblerType forced_assembler = AssemblerType::AUTO_DETECT;
    CCompilerType forced_c_compiler = CCompilerType::AUTO_DETECT;
    
    // Feature detection helpers
    bool detect_meta_assembly_features(const std::string& code);
    bool detect_tesla_consciousness_features(const std::string& code);
    bool detect_c_interop_needs(const std::string& code);
    bool detect_context_stack_usage(const std::string& code);
    
    // Scoring system for assembler selection
    int score_for_llvm(const AssemblyFeatures& features);
    int score_for_nasm(const AssemblyFeatures& features);
};