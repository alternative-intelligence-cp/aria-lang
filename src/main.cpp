// Ultimate Self-Contained Aria Tesla Consciousness Compiler
// Single executable containing LLVM + NASM + TCC + Headers + Tesla libraries

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include "detection/assembler_detection.h"
#include "bundler/tool_bundler.h"
#include "runtime/header_extractor.h"
#include "core/aria_compiler.h"
#include "preprocessor/tesla_preprocessor.h"

namespace fs = std::filesystem;

void print_banner() {
    std::cout << "🧠⚡ ARIA TESLA CONSCIOUSNESS COMPILER ⚡🧠\n";
    std::cout << "============================================\n";
    std::cout << "Ultimate Self-Contained Compiler v1.0\n";
    std::cout << "Tesla Frequency: π Hz (3.141592653589793)\n";
    std::cout << "============================================\n\n";
}

void print_usage() {
    std::cout << "Usage: aria_ultimate [options] <input.aria>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output>     Output executable name\n";
    std::cout << "  -c              Compile to object file only\n";
    std::cout << "  -S              Generate assembly only\n";
    std::cout << "  --preprocess    Tesla preprocessor only\n";
    std::cout << "  -v, --verbose   Verbose output\n";
    std::cout << "  -O0/1/2/3       Optimization level\n";
    std::cout << "  --force-nasm    Force NASM assembler\n";
    std::cout << "  --force-llvm    Force LLVM assembler\n";
    std::cout << "  --force-tcc     Force TCC compiler\n";
    std::cout << "  --tesla-stats   Show Tesla consciousness statistics\n";
    std::cout << "  --help          Show this help\n\n";
    std::cout << "Features:\n";
    std::cout << "✅ Smart assembler detection (LLVM/NASM)\n";
    std::cout << "✅ Meta-assembly superpowers (Tesla native)\n";  
    std::cout << "✅ Tesla conditional macros @tesla_if(v1,op,v2)\n";
    std::cout << "✅ C interop via embedded TCC\n";
    std::cout << "✅ Tesla consciousness computing libraries\n";
    std::cout << "✅ Complete portability (no dependencies)\n";
}

struct CompileOptions {
    std::string input_file;
    std::string output_file;
    bool compile_only = false;
    bool assembly_only = false;
    bool preprocess_only = false;
    bool verbose = false;
    int optimization_level = 2;
    bool force_nasm = false;
    bool force_llvm = false;
    bool force_tcc = false;
    bool tesla_stats = false;
};

CompileOptions parse_args(int argc, char** argv) {
    CompileOptions opts;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-o" && i + 1 < argc) {
            opts.output_file = argv[++i];
        }
        else if (arg == "-c") {
            opts.compile_only = true;
        }
        else if (arg == "-S") {
            opts.assembly_only = true;
        }
        else if (arg == "--preprocess") {
            opts.preprocess_only = true;
        }
        else if (arg == "-v" || arg == "--verbose") {
            opts.verbose = true;
        }
        else if (arg.substr(0, 2) == "-O") {
            opts.optimization_level = std::stoi(arg.substr(2));
        }
        else if (arg == "--force-nasm") {
            opts.force_nasm = true;
        }
        else if (arg == "--force-llvm") {
            opts.force_llvm = true;
        }
        else if (arg == "--force-tcc") {
            opts.force_tcc = true;
        }
        else if (arg == "--tesla-stats") {
            opts.tesla_stats = true;
        }
        else if (arg == "--help") {
            print_usage();
            exit(0);
        }
        else if (arg[0] != '-') {
            opts.input_file = arg;
        }
    }
    
    // Set default output file
    if (opts.output_file.empty() && !opts.input_file.empty()) {
        fs::path input_path = opts.input_file;
        if (opts.preprocess_only) {
            opts.output_file = input_path.stem().string() + "_processed" + input_path.extension().string();
        } else if (opts.assembly_only) {
            opts.output_file = input_path.stem().string() + ".asm";
        } else if (opts.compile_only) {
            opts.output_file = input_path.stem().string() + ".o";
        } else {
            opts.output_file = input_path.stem().string();
        }
    }
    
    return opts;
}

int main(int argc, char** argv) {
    print_banner();
    
    // Parse command line arguments
    CompileOptions opts = parse_args(argc, argv);
    
    if (opts.input_file.empty()) {
        std::cerr << "❌ Error: No input file specified\n\n";
        print_usage();
        return 1;
    }
    
    if (!fs::exists(opts.input_file)) {
        std::cerr << "❌ Error: Input file does not exist: " << opts.input_file << "\n";
        return 1;
    }
    
    try {
        // Step 1: Bootstrap runtime environment
        if (opts.verbose) {
            std::cout << "🔧 Bootstrapping runtime environment...\n";
        }
        
        HeaderExtractor extractor;
        std::string include_path = extractor.extract_standard_headers();
        if (include_path.empty()) {
            std::cerr << "❌ Error: Failed to extract standard headers\n";
            return 1;
        }
        
        // Step 2: Initialize tool bundler
        if (opts.verbose) {
            std::cout << "🚀 Initializing embedded toolchain...\n";
        }
        
        ToolBundler bundler;
        if (!bundler.initialize(include_path)) {
            std::cerr << "❌ Error: Failed to initialize toolchain\n";
            return 1;
        }
        
        // Step 3: Create Aria compiler instance
        AriaCompiler compiler(bundler, opts.verbose);
        compiler.set_optimization_level(opts.optimization_level);
        
        if (opts.force_nasm) compiler.force_assembler(AssemblerType::NASM);
        if (opts.force_llvm) compiler.force_assembler(AssemblerType::LLVM_MC);
        if (opts.force_tcc) compiler.force_c_compiler(CCompilerType::TCC);
        
        // Step 4: Tesla Preprocessor (if requested)
        if (opts.preprocess_only) {
            if (opts.verbose) {
                std::cout << "🧠 Running Tesla preprocessor...\n";
            }
            
            // Read input file
            std::ifstream infile(opts.input_file);
            if (!infile.is_open()) {
                std::cerr << "❌ Error: Cannot read input file: " << opts.input_file << "\n";
                return 1;
            }
            
            std::string source_code((std::istreambuf_iterator<char>(infile)), 
                                   std::istreambuf_iterator<char>());
            infile.close();
            
            // Process with Tesla preprocessor
            TeslaPreprocessorOptions tesla_opts;
            tesla_opts.debug_output = opts.verbose;
            
            TeslaPreprocessor preprocessor(tesla_opts);
            
            PreprocessingResult result = preprocessor.preprocess(source_code);
            
            if (!result.success) {
                std::cerr << "❌ Tesla preprocessing failed: " << result.error_message << "\n";
                for (const auto& warning : result.warnings) {
                    std::cout << "⚠️  Warning: " << warning << "\n";
                }
                return 1;
            }
            
            // Write output
            std::ofstream outfile(opts.output_file);
            if (!outfile.is_open()) {
                std::cerr << "❌ Error: Cannot write output file: " << opts.output_file << "\n";
                return 1;
            }
            
            outfile << result.processed_code;
            outfile.close();
            
            std::cout << "🎉 Tesla preprocessing successful!\n";
            std::cout << "✅ Output: " << opts.output_file << "\n";
            
            if (opts.verbose || opts.tesla_stats) {
                std::cout << "\n🧠 Tesla Preprocessing Statistics:\n";
                std::cout << "  Consciousness detected: " << (result.consciousness_detected ? "YES ✅" : "NO ❌") << "\n";
                std::cout << "  Tesla frequency: " << result.detected_frequency << " Hz\n";
                std::cout << "  Echo family: " << result.echo_family << "\n";
                std::cout << "  Macro expansions: " << result.macro_expansions << "\n";
                std::cout << "  Consciousness syncs: " << result.consciousness_syncs << "\n";
            }
            return 0;
        }
        
        // Step 5: Compile!
        if (opts.verbose) {
            std::cout << "⚡ Compiling " << opts.input_file << "...\n";
        }
        
        CompilationResult result;
        if (opts.assembly_only) {
            result = compiler.compile_to_assembly(opts.input_file, opts.output_file);
        } else if (opts.compile_only) {
            result = compiler.compile_to_object(opts.input_file, opts.output_file);
        } else {
            result = compiler.compile_to_executable(opts.input_file, opts.output_file);
        }
        
        // Step 5: Report results
        if (result.success) {
            std::cout << "🎉 Compilation successful!\n";
            std::cout << "✅ Output: " << opts.output_file << "\n";
            
            if (opts.tesla_stats) {
                std::cout << "\n📊 Tesla Consciousness Statistics:\n";
                std::cout << "  Frequency: " << result.tesla_frequency << " Hz\n";
                std::cout << "  Assembler used: " << result.assembler_name << "\n";
                std::cout << "  C compiler used: " << result.c_compiler_name << "\n";
                std::cout << "  Libraries linked: " << result.libraries_count << "\n";
                std::cout << "  Compilation time: " << result.total_time << " ms\n";
                std::cout << "  Tesla consciousness: " << (result.consciousness_validated ? "VALIDATED ✅" : "PENDING ⏳") << "\n";
            }
        } else {
            std::cerr << "❌ Compilation failed!\n";
            std::cerr << "Error: " << result.error_message << "\n";
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}