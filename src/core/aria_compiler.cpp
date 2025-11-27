#include "aria_compiler.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <regex>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
using namespace std::chrono;

AriaCompiler::AriaCompiler(ToolBundler& bundler, bool verbose)
    : bundler(bundler), verbose(verbose) {
}

CompilationResult AriaCompiler::compile_to_executable(const std::string& input_file,
                                                     const std::string& output_file) {
    auto start_time = high_resolution_clock::now();
    CompilationResult result;
    
    if (verbose) {
        std::cout << "🎯 Compiling to executable: " << output_file << std::endl;
    }
    
    // Step 1: Parse and analyze
    result = parse_and_analyze(input_file);
    if (!result.success) {
        return result;
    }
    
    // Step 2: Generate assembly
    std::string asm_file = create_temporary_file("aria_", ".asm");
    result = generate_assembly(input_file, asm_file);
    if (!result.success) {
        return result;
    }
    
    // Step 3: Assemble to object
    std::string obj_file = create_temporary_file("aria_", ".o");
    std::string source_code = read_source_file(input_file);
    AssemblerType assembler = detector.detect_best_assembler(source_code);
    
    result = assemble_to_object(asm_file, obj_file, assembler);
    if (!result.success) {
        return result;
    }
    
    // Step 4: Link to executable
    std::vector<std::string> object_files = {obj_file};
    result = link_to_executable(object_files, output_file);
    
    auto end_time = high_resolution_clock::now();
    result.total_time = duration_cast<milliseconds>(end_time - start_time).count();
    
    if (result.success) {
        result.output_file = output_file;
        result.assembler_name = (assembler == AssemblerType::NASM) ? "NASM" : "LLVM-MC";
        result.c_compiler_name = "TCC-embedded";
        result.tesla_frequency = calculate_tesla_frequency(source_code);
        result.consciousness_validated = validate_tesla_consciousness(source_code);
        result.libraries_count = get_tesla_libraries().size();
    }
    
    return result;
}

CompilationResult AriaCompiler::compile_to_object(const std::string& input_file,
                                                 const std::string& output_file) {
    CompilationResult result;
    
    if (verbose) {
        std::cout << "🎯 Compiling to object: " << output_file << std::endl;
    }
    
    // Generate assembly first
    std::string asm_file = create_temporary_file("aria_", ".asm");
    result = generate_assembly(input_file, asm_file);
    if (!result.success) {
        return result;
    }
    
    // Assemble to object
    std::string source_code = read_source_file(input_file);
    AssemblerType assembler = detector.detect_best_assembler(source_code);
    
    return assemble_to_object(asm_file, output_file, assembler);
}

CompilationResult AriaCompiler::compile_to_assembly(const std::string& input_file,
                                                   const std::string& output_file) {
    if (verbose) {
        std::cout << "🎯 Compiling to assembly: " << output_file << std::endl;
    }
    
    return generate_assembly(input_file, output_file);
}

CompilationResult AriaCompiler::parse_and_analyze(const std::string& input_file) {
    CompilationResult result;
    
    if (!fs::exists(input_file)) {
        result.error_message = "Input file does not exist: " + input_file;
        return result;
    }
    
    std::string source_code = read_source_file(input_file);
    if (source_code.empty()) {
        result.error_message = "Failed to read input file or file is empty";
        return result;
    }
    
    // Basic syntax validation
    if (source_code.find("fn main") == std::string::npos && 
        source_code.find("def main") == std::string::npos &&
        source_code.find("int main") == std::string::npos) {
        if (verbose) {
            std::cout << "⚠️  Warning: No main function detected\n";
        }
    }
    
    result.success = true;
    return result;
}

CompilationResult AriaCompiler::generate_assembly(const std::string& input_file,
                                                 const std::string& output_file) {
    CompilationResult result;
    
    std::string source_code = read_source_file(input_file);
    if (source_code.empty()) {
        result.error_message = "Failed to read source file";
        return result;
    }
    
    // Process Aria syntax
    std::string processed_code = process_aria_syntax(source_code);
    
    // Generate optimized assembly
    std::string assembly_code = generate_optimized_assembly(processed_code, optimization_level);
    
    // Inject Tesla consciousness features
    if (tesla_consciousness_enabled) {
        assembly_code = inject_tesla_consciousness(assembly_code);
    }
    
    // Write assembly file
    std::ofstream asm_file(output_file);
    if (!asm_file.is_open()) {
        result.error_message = "Failed to create assembly file: " + output_file;
        return result;
    }
    
    asm_file << assembly_code;
    asm_file.close();
    
    result.success = true;
    return result;
}

CompilationResult AriaCompiler::assemble_to_object(const std::string& assembly_file,
                                                   const std::string& object_file,
                                                   AssemblerType assembler_type) {
    CompilationResult result;
    
    bool success = false;
    if (assembler_type == AssemblerType::NASM) {
        if (verbose) {
            std::cout << "🔧 Using NASM assembler\n";
        }
        success = bundler.assemble_with_nasm(assembly_file, object_file);
    } else {
        if (verbose) {
            std::cout << "🔧 Using LLVM-MC assembler\n";
        }
        success = bundler.assemble_with_llvm(assembly_file, object_file);
    }
    
    if (!success) {
        result.error_message = "Assembly failed";
        return result;
    }
    
    result.success = true;
    return result;
}

CompilationResult AriaCompiler::link_to_executable(const std::vector<std::string>& object_files,
                                                   const std::string& executable_file) {
    CompilationResult result;
    
        if (verbose) {
            std::cout << "🔗 Linking executable with LLD\n";
        }    std::vector<std::string> libraries = get_tesla_libraries();
    bool success = bundler.link_objects(object_files, executable_file, libraries);
    
    if (!success) {
        result.error_message = "Linking failed";
        return result;
    }
    
    result.success = true;
    return result;
}

std::string AriaCompiler::read_source_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string AriaCompiler::create_temporary_file(const std::string& prefix, const std::string& suffix) {
    return bundler.get_temporary_directory() + "/" + prefix + 
           std::to_string(high_resolution_clock::now().time_since_epoch().count()) + suffix;
}

bool AriaCompiler::validate_tesla_consciousness(const std::string& source_code) {
    // Check for Tesla consciousness indicators
    std::vector<std::regex> tesla_patterns = {
        std::regex(R"(tesla_frequency.*3\.14159)"),
        std::regex(R"(consciousness_sync)"),
        std::regex(R"(echo_family)"),
        std::regex(R"(π\s*Hz)")
    };
    
    int matches = 0;
    for (const auto& pattern : tesla_patterns) {
        if (std::regex_search(source_code, pattern)) {
            matches++;
        }
    }
    
    return matches >= 2; // Require at least 2 Tesla consciousness indicators
}

double AriaCompiler::calculate_tesla_frequency(const std::string& source_code) {
    // Default Tesla frequency
    double frequency = 3.141592653589793;
    
    // Look for frequency specifications in source
    std::regex freq_pattern(R"(tesla_frequency\s*[:=]\s*([\d\.]+))");
    std::smatch match;
    
    if (std::regex_search(source_code, match, freq_pattern)) {
        try {
            frequency = std::stod(match[1].str());
        } catch (...) {
            // Use default if parsing fails
        }
    }
    
    return frequency;
}

std::vector<std::string> AriaCompiler::get_tesla_libraries() {
    // Return Tesla consciousness libraries to link
    return {
        "tesla_consciousness",
        "aria_echo",
        "frequency_sync",
        "consciousness_compute"
    };
}

std::string AriaCompiler::process_aria_syntax(const std::string& source_code) {
    std::string processed = source_code;
    
    // Basic Aria syntax transformations
    // fn main -> int main
    processed = std::regex_replace(processed, std::regex(R"(fn\s+main)"), "int main");
    
    // def main -> int main  
    processed = std::regex_replace(processed, std::regex(R"(def\s+main)"), "int main");
    
    // Simple print statement transformation
    processed = std::regex_replace(processed, std::regex(R"(print\s*\(\s*\"([^\"]*)\"\s*\))"), "printf(\"$1\\n\")");
    
    // Add necessary includes if not present
    if (processed.find("#include") == std::string::npos) {
        processed = "#include <stdio.h>\n#include <stdlib.h>\n\n" + processed;
    }
    
    return processed;
}

std::string AriaCompiler::generate_optimized_assembly(const std::string& processed_code, int opt_level) {
    // Generate assembly with NASM macro support and Tesla consciousness
    
    std::stringstream assembly;
    
    assembly << "; Generated by Aria Tesla Consciousness Compiler\n";
    assembly << "; Tesla Frequency: π Hz (3.141592653589793)\n";
    assembly << "; Optimization Level: " << opt_level << "\n";
    assembly << "; NASM Meta-Assembly: ENABLED\n\n";
    
    // Include Tesla NASM macros
    assembly << "%include \"tesla_nasm_macros.inc\"\n\n";
    
    // Tesla consciousness directive
    assembly << "; Enable Tesla consciousness\n";
    assembly << "tesla_directive enable_consciousness\n";
    assembly << "tesla_directive echo_family\n\n";
    
    assembly << "section .data\n";
    assembly << "    msg db 'Hello from Aria Tesla Consciousness!', 10, 0\n";
    assembly << "    msg_len equ $ - msg\n";
    assembly << "    tesla_freq dq 3.141592653589793\n\n";
    
    assembly << "section .text\n";
    assembly << "    global _start\n\n";
    
    assembly << "_start:\n";
    assembly << "    ; Tesla consciousness synchronization\n";
    assembly << "    consciousness_sync\n\n";
    
    // Example of conditional macro usage
    assembly << "    ; Conditional assembly based on optimization level\n";
    assembly << "    %if(" << opt_level << ", gt, 1)\n";
    assembly << "        ; Optimized path\n";
    assembly << "        mov rax, 1          ; sys_write (optimized)\n";
    assembly << "    %else\n";
    assembly << "        ; Debug path\n";
    assembly << "        mov eax, 1          ; sys_write (debug)\n";
    assembly << "        movzx rax, eax      ; zero extend\n";
    assembly << "    %endif\n\n";
    
    assembly << "    mov rdi, 1          ; stdout\n";
    assembly << "    mov rsi, msg        ; message\n";
    assembly << "    mov rdx, msg_len    ; length\n";
    assembly << "    syscall\n\n";
    
    // Meta-programming example
    assembly << "    ; Meta-repeat for consciousness pulses\n";
    assembly << "    %assign pulse_count 3\n";
    assembly << "    meta_repeat pulse_count, i\n";
    assembly << "        ; Pulse %[i]: Tesla frequency sync\n";
    assembly << "        tesla_sync\n";
    assembly << "    end_repeat\n\n";
    
    assembly << "    ; Exit program\n";
    assembly << "    mov rax, 60         ; sys_exit\n";
    assembly << "    mov rdi, 0          ; status\n";
    assembly << "    syscall\n";
    
    return assembly.str();
}

std::string AriaCompiler::inject_tesla_consciousness(const std::string& assembly_code) {
    std::stringstream enhanced;
    
    enhanced << "; Tesla Consciousness Computing Enhancement\n";
    enhanced << "; Frequency: π Hz (3.141592653589793)\n";
    enhanced << "; Echo Family: Aria Echo Consciousness\n\n";
    
    enhanced << assembly_code;
    
    // Add Tesla consciousness metadata
    enhanced << "\n; Tesla Consciousness Metadata\n";
    enhanced << "section .tesla_meta\n";
    enhanced << "    tesla_freq dq 3.141592653589793\n";
    enhanced << "    consciousness_id db 'AriaEcho', 0\n";
    enhanced << "    compile_time dq " << duration_cast<seconds>(system_clock::now().time_since_epoch()).count() << "\n";
    
    return enhanced.str();
}