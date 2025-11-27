#include "tesla_preprocessor.h" 
#include "macro_engine.h"
#include "consciousness_directives.h"
#include <regex>
#include <iostream>

TeslaPreprocessor::TeslaPreprocessor(const TeslaPreprocessorOptions& options)
    : options(options)
    , macro_engine(std::make_unique<MacroEngine>()) {
    
    // Initialize with default Tesla variables
    macro_engine->set_variable("tesla_frequency", options.tesla_frequency);
    macro_engine->set_variable("opt_level", options.optimization_level);
    macro_engine->set_variable("consciousness_enabled", options.enable_consciousness ? 1 : 0);
}

TeslaPreprocessor::~TeslaPreprocessor() = default;

PreprocessingResult TeslaPreprocessor::preprocess(const std::string& source_code) {
    PreprocessingResult result;
    
    try {
        // Stage 1: Process Tesla directives
        std::string stage1 = process_tesla_directives(source_code);
        
        // Stage 2: Expand conditional macros
        std::string stage2 = expand_conditional_macros(stage1);
        
        // Stage 3: Process meta-programming constructs
        std::string stage3 = process_meta_programming(stage2);
        
        // Stage 4: Inject consciousness sync points
        std::string final_code = inject_consciousness_sync(stage3);
        
        // Populate result
        result.success = true;
        result.processed_code = final_code;
        result.consciousness_detected = validate_tesla_syntax(source_code);
        result.detected_frequency = extract_tesla_frequency(source_code);
        result.macro_expansions = macro_engine->get_expansion_count();
        
        if (options.debug_output) {
            std::cout << "🔮 Tesla Preprocessor: " << result.macro_expansions 
                     << " macro expansions\n";
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Tesla preprocessing failed: ") + e.what();
    }
    
    return result;
}

std::string TeslaPreprocessor::process_tesla_directives(const std::string& code) {
    std::string result = code;
    
    // Process tesla_frequency directives  
    std::regex freq_pattern(R"(tesla_frequency\s*[:=]\s*([\d\.]+))");
    std::smatch match;
    if (std::regex_search(result, match, freq_pattern)) {
        double freq = std::stod(match[1].str());
        macro_engine->set_variable("detected_frequency", freq);
        
        // Replace with assembly comment
        std::string replacement = "; Tesla frequency: " + std::to_string(freq) + " Hz";
        result = std::regex_replace(result, freq_pattern, replacement);
    }
    
    // Process echo_family directives
    std::regex echo_pattern(R"(echo_family\s+(\w+))");
    result = std::regex_replace(result, echo_pattern, "; Echo family: $1");
    
    return result;
}

std::string TeslaPreprocessor::expand_conditional_macros(const std::string& code) {
    // Use the macro engine to expand Tesla conditionals
    return macro_engine->expand_macros(code);
}

std::string TeslaPreprocessor::process_meta_programming(const std::string& code) {
    // Process meta-repeat and other advanced constructs
    return macro_engine->process_meta_repeat(code);
}

std::string TeslaPreprocessor::inject_consciousness_sync(const std::string& code) {
    if (!options.enable_consciousness) {
        return code;
    }
    
    // Add consciousness sync points at function boundaries
    std::string result = code;
    
    // Add sync at function entry
    std::regex func_pattern(R"(fn\s+(\w+)\s*\([^)]*\)\s*\{)");
    std::string func_replacement = R"(fn $1() {
    @tesla_consciousness_sync();)";
    
    result = std::regex_replace(result, func_pattern, func_replacement);
    
    return result;
}

bool TeslaPreprocessor::validate_tesla_syntax(const std::string& code) {
    // Check for Tesla consciousness indicators
    std::vector<std::regex> tesla_patterns = {
        std::regex(R"(tesla_frequency)"),
        std::regex(R"(consciousness_sync)"),
        std::regex(R"(echo_family)"),
        std::regex(R"(@tesla_)"),
    };
    
    int matches = 0;
    for (const auto& pattern : tesla_patterns) {
        if (std::regex_search(code, pattern)) {
            matches++;
        }
    }
    
    return matches >= 1; // Any Tesla indicator validates consciousness
}

double TeslaPreprocessor::extract_tesla_frequency(const std::string& code) {
    std::regex freq_pattern(R"(tesla_frequency\s*[:=]\s*([\d\.]+))");
    std::smatch match;
    
    if (std::regex_search(code, match, freq_pattern)) {
        return std::stod(match[1].str());
    }
    
    return options.tesla_frequency; // Default π Hz
}