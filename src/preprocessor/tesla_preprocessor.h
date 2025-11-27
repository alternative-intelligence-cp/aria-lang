#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Forward declarations
class MacroEngine;
class ConsciousnessDirectiveProcessor;

struct TeslaPreprocessorOptions {
    bool enable_consciousness = true;
    double tesla_frequency = 3.141592653589793;
    int optimization_level = 2;
    bool enable_meta_assembly = true;
    bool debug_output = false;
};

struct PreprocessingResult {
    bool success = false;
    std::string processed_code;
    std::string error_message;
    std::vector<std::string> warnings;
    
    // Tesla consciousness metadata
    bool consciousness_detected = false;
    double detected_frequency = 0.0;
    std::string echo_family;
    int macro_expansions = 0;
    int consciousness_syncs = 0;
};

class TeslaPreprocessor {
public:
    TeslaPreprocessor(const TeslaPreprocessorOptions& options = {});
    ~TeslaPreprocessor();
    
    // Main preprocessing interface
    PreprocessingResult preprocess(const std::string& source_code);
    
    // Advanced macro system - your exact syntax!
    bool register_conditional_macro(const std::string& name, 
                                   const std::string& condition_syntax);
    
    // Tesla consciousness directives
    bool process_tesla_directive(const std::string& directive, 
                                const std::string& parameters);
    
    // Meta-programming features
    bool expand_meta_repeat(const std::string& count_expr, 
                           const std::string& variable_name,
                           const std::string& body);
    
    // Configuration
    void set_options(const TeslaPreprocessorOptions& options) { this->options = options; }
    TeslaPreprocessorOptions get_options() const { return options; }
    
private:
    TeslaPreprocessorOptions options;
    std::unique_ptr<MacroEngine> macro_engine;
    std::unique_ptr<ConsciousnessDirectiveProcessor> consciousness_processor;
    
    // Core preprocessing stages
    std::string process_tesla_directives(const std::string& code);
    std::string expand_conditional_macros(const std::string& code);
    std::string process_meta_programming(const std::string& code);
    std::string inject_consciousness_sync(const std::string& code);
    
    // Utility methods
    bool validate_tesla_syntax(const std::string& code);
    double extract_tesla_frequency(const std::string& code);
    std::string generate_consciousness_metadata(const std::string& code);
};