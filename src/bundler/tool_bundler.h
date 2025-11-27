#pragma once

#include <string>
#include <vector>
#include <memory>

// Forward declarations for embedded tools
struct LLVMWrapper;
struct NASMWrapper;
struct TCCWrapper;
struct LLDWrapper;

enum class ToolType {
    LLVM_MC,
    NASM,
    TCC,
    LLD
};

struct CompilationResult {
    bool success = false;
    std::string output_file;
    std::string error_message;
    std::string assembler_name;
    std::string c_compiler_name;
    double tesla_frequency = 3.141592653589793;
    int libraries_count = 0;
    double total_time = 0.0;
    bool consciousness_validated = false;
};

class ToolBundler {
public:
    ToolBundler();
    ~ToolBundler();
    
    // Initialization
    bool initialize(const std::string& include_path);
    void cleanup();
    
    // LLVM Interface
    bool assemble_with_llvm(const std::string& assembly_file, 
                           const std::string& object_file,
                           const std::string& target_triple = "x86_64-unknown-linux-gnu");
    
    // NASM Interface
    bool assemble_with_nasm(const std::string& assembly_file,
                           const std::string& object_file,
                           const std::string& format = "elf64");
    
    // TCC Interface
    bool compile_with_tcc(const std::string& c_file,
                         const std::string& output_file,
                         const std::vector<std::string>& additional_args = {});
    
    // LLD Interface
    bool link_with_lld(const std::vector<std::string>& object_files,
                      const std::string& output_executable,
                      const std::vector<std::string>& libraries = {},
                      const std::string& target_triple = "x86_64-unknown-linux-gnu");
    
    // Linking (unified interface)
    bool link_objects(const std::vector<std::string>& object_files,
                     const std::string& output_executable,
                     const std::vector<std::string>& libraries = {});
    
    // Utility
    bool is_tool_available(ToolType tool);
    std::string get_tool_version(ToolType tool);
    std::string get_temporary_directory() const { return temp_dir; }
    
private:
    std::unique_ptr<LLVMWrapper> llvm;
    std::unique_ptr<NASMWrapper> nasm;
    std::unique_ptr<TCCWrapper> tcc;
    std::unique_ptr<LLDWrapper> lld;
    
    std::string include_path;
    std::string temp_dir;
    bool initialized = false;
    
    // Internal helpers
    bool setup_temporary_directory();
    bool initialize_llvm();
    bool initialize_nasm();
    bool initialize_tcc();
    bool initialize_lld();
    std::string create_temp_file(const std::string& prefix, const std::string& suffix);
};