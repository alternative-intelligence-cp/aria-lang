#include "assembler_detection.h"
#include <regex>
#include <algorithm>

AssemblerType AssemblerDetector::detect_best_assembler(const std::string& source_code) {
    if (forced_assembler != AssemblerType::AUTO_DETECT) {
        return forced_assembler;
    }
    
    AssemblyFeatures features = analyze_features(source_code);
    
    int llvm_score = score_for_llvm(features);
    int nasm_score = score_for_nasm(features);
    
    // NASM gets bonus for meta-assembly features
    if (features.has_meta_programming || features.has_context_stack) {
        nasm_score += 50;
    }
    
    // LLVM gets bonus for performance-critical code
    if (!features.has_macros && !features.has_meta_programming) {
        llvm_score += 20;
    }
    
    return (nasm_score > llvm_score) ? AssemblerType::NASM : AssemblerType::LLVM_MC;
}

CCompilerType AssemblerDetector::detect_best_c_compiler(const std::string& source_code) {
    if (forced_c_compiler != CCompilerType::AUTO_DETECT) {
        return forced_c_compiler;
    }
    
    // For now, always use TCC since it's embedded
    // Could add GCC/Clang detection for external builds later
    return CCompilerType::TCC;
}

AssemblyFeatures AssemblerDetector::analyze_features(const std::string& source_code) {
    AssemblyFeatures features;
    
    // Detect inline assembly
    std::regex inline_asm_pattern(R"(\binline\s+asm\b|__asm__\b|asm\s*\()");
    features.has_inline_asm = std::regex_search(source_code, inline_asm_pattern);
    
    // Detect macro usage
    std::regex macro_pattern(R"(%macro\b|%define\b|%include\b|%if\b)");
    features.has_macros = std::regex_search(source_code, macro_pattern);
    
    // Detect meta-programming features
    features.has_meta_programming = detect_meta_assembly_features(source_code);
    
    // Detect Tesla consciousness features
    features.has_tesla_directives = detect_tesla_consciousness_features(source_code);
    
    // Detect context stack usage
    features.has_context_stack = detect_context_stack_usage(source_code);
    
    // Detect C interop needs
    features.needs_c_interop = detect_c_interop_needs(source_code);
    
    return features;
}

bool AssemblerDetector::detect_meta_assembly_features(const std::string& code) {
    // Look for advanced meta-assembly patterns
    std::vector<std::regex> meta_patterns = {
        std::regex(R"(\bcontext_push\b|\bcontext_pop\b)"),  // Context stack
        std::regex(R"(\bmeta_repeat\b|\bmeta_unroll\b)"),   // Meta loops
        std::regex(R"(\bcompile_time\b|\bevaluate_at_compile\b)"), // Compile-time eval
        std::regex(R"(\bgenerate_code\b|\bmacro_expand\b)") // Code generation
    };
    
    for (const auto& pattern : meta_patterns) {
        if (std::regex_search(code, pattern)) {
            return true;
        }
    }
    
    return false;
}

bool AssemblerDetector::detect_tesla_consciousness_features(const std::string& code) {
    // Look for Tesla consciousness computing directives
    std::vector<std::regex> tesla_patterns = {
        std::regex(R"(\btesla_frequency\b|\bπ\s*Hz\b)"),
        std::regex(R"(\bconsciousness_sync\b|\btesla_sync\b)"),
        std::regex(R"(\becho_family\b|\baria_echo\b)"),
        std::regex(R"(\btesla_directive\b|\bconsciousness_directive\b)")
    };
    
    for (const auto& pattern : tesla_patterns) {
        if (std::regex_search(code, pattern)) {
            return true;
        }
    }
    
    return false;
}

bool AssemblerDetector::detect_c_interop_needs(const std::string& code) {
    // Look for C interop indicators
    std::vector<std::regex> c_patterns = {
        std::regex(R"(\bextern\s+"C"\b)"),
        std::regex(R"(\b#include\s*<[^>]+\.h>)"),
        std::regex(R"(\bcall_c_function\b|\bc_interop\b)"),
        std::regex(R"(\bprintf\b|\bmalloc\b|\bfree\b)") // Common C functions
    };
    
    for (const auto& pattern : c_patterns) {
        if (std::regex_search(code, pattern)) {
            return true;
        }
    }
    
    return false;
}

bool AssemblerDetector::detect_context_stack_usage(const std::string& code) {
    // Look for context stack manipulation
    std::regex context_pattern(R"(\bpush_context\b|\bpop_context\b|\bcontext_depth\b)");
    return std::regex_search(code, context_pattern);
}

int AssemblerDetector::score_for_llvm(const AssemblyFeatures& features) {
    int score = 100; // Base score
    
    // LLVM advantages
    if (!features.has_macros) score += 30;  // Better without macros
    if (!features.has_meta_programming) score += 25;  // Performance focus
    
    // LLVM disadvantages
    if (features.has_macros) score -= 40;  // Poor macro support
    if (features.has_meta_programming) score -= 50;  // No meta-assembly
    if (features.has_context_stack) score -= 30;  // Limited context features
    
    return score;
}

int AssemblerDetector::score_for_nasm(const AssemblyFeatures& features) {
    int score = 100; // Base score
    
    // NASM advantages
    if (features.has_macros) score += 40;  // Excellent macro support
    if (features.has_meta_programming) score += 60;  // Meta-assembly superpowers
    if (features.has_context_stack) score += 35;  // Context stack features
    if (features.has_tesla_directives) score += 20;  // Tesla consciousness support
    
    // NASM is generally good for everything, fewer penalties
    
    return score;
}