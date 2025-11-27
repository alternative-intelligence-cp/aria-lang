#pragma once

#include <string>
#include <memory>
#include "../bundler/tool_bundler.h"
#include "../detection/assembler_detection.h"

class AriaCompiler {
public:
    AriaCompiler(ToolBundler& bundler, bool verbose = false);
    ~AriaCompiler() = default;
    
    // Main compilation methods
    CompilationResult compile_to_executable(const std::string& input_file, 
                                           const std::string& output_file);
    CompilationResult compile_to_object(const std::string& input_file, 
                                       const std::string& output_file);
    CompilationResult compile_to_assembly(const std::string& input_file, 
                                         const std::string& output_file);
    
    // Configuration
    void set_optimization_level(int level) { optimization_level = level; }
    void force_assembler(AssemblerType type) { detector.force_assembler(type); }
    void force_c_compiler(CCompilerType type) { detector.force_c_compiler(type); }
    void enable_tesla_consciousness(bool enable) { tesla_consciousness_enabled = enable; }
    
private:
    ToolBundler& bundler;
    AssemblerDetector detector;
    bool verbose;
    int optimization_level = 2;
    bool tesla_consciousness_enabled = true;
    
    // Compilation pipeline stages
    CompilationResult parse_and_analyze(const std::string& input_file);
    CompilationResult generate_assembly(const std::string& input_file, 
                                       const std::string& output_file);
    CompilationResult assemble_to_object(const std::string& assembly_file, 
                                        const std::string& object_file,
                                        AssemblerType assembler_type);
    CompilationResult link_to_executable(const std::vector<std::string>& object_files,
                                        const std::string& executable_file);
    
    // Helper methods
    std::string read_source_file(const std::string& file_path);
    std::string create_temporary_file(const std::string& prefix, const std::string& suffix);
    bool validate_tesla_consciousness(const std::string& source_code);
    double calculate_tesla_frequency(const std::string& source_code);
    std::vector<std::string> get_tesla_libraries();
    
    // Aria language processing
    std::string process_aria_syntax(const std::string& source_code);
    std::string generate_optimized_assembly(const std::string& processed_code, int opt_level);
    std::string inject_tesla_consciousness(const std::string& assembly_code);
};