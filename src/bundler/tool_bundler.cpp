#include "tool_bundler.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <cstdio>

// Include embedded tool wrappers (simplified for demo)
extern "C" {
    // For demo - these will link to stub implementations
    int llvm_mc_main_stub(int argc, char** argv) { 
        printf("LLVM-MC (demo stub): %s\n", argv[1]); 
        return 0; 
    }
    
    int nasm_main_renamed_stub(int argc, char** argv) { 
        printf("NASM (demo stub): %s\n", argv[1]); 
        return 0; 
    }
    
    // LLD linker interface (demo stub)
    int lld_link_stub(int argc, char** argv) {
        printf("LLD (demo stub): Linking %d objects...\n", argc-1);
        // Simulate successful linking
        return 0;
    }
}

namespace fs = std::filesystem;

// Wrapper structs for embedded tools
struct LLVMWrapper {
    bool available = false;
    std::string version = "18.0.0-embedded";
};

struct NASMWrapper {
    bool available = false;
    std::string version = "2.16-embedded";
};

struct TCCWrapper {
    bool available = false;
    std::string version = "0.9.27-embedded";
    struct TCCState* state = nullptr;
};

struct LLDWrapper {
    bool available = false;
    std::string version = "18.0.0-embedded";
};

ToolBundler::ToolBundler() 
    : llvm(std::make_unique<LLVMWrapper>())
    , nasm(std::make_unique<NASMWrapper>())
    , tcc(std::make_unique<TCCWrapper>())
    , lld(std::make_unique<LLDWrapper>()) {
}

ToolBundler::~ToolBundler() {
    cleanup();
}

bool ToolBundler::initialize(const std::string& include_path) {
    this->include_path = include_path;
    
    if (!setup_temporary_directory()) {
        return false;
    }
    
    bool llvm_ok = initialize_llvm();
    bool nasm_ok = initialize_nasm();
    bool tcc_ok = initialize_tcc();
    bool lld_ok = initialize_lld();
    
    initialized = llvm_ok && nasm_ok && tcc_ok && lld_ok;
    return initialized;
}

void ToolBundler::cleanup() {
    // Clean up temporary directory (simplified for demo)
    if (!temp_dir.empty() && fs::exists(temp_dir)) {
        std::error_code ec;
        fs::remove_all(temp_dir, ec);
    }
    
    initialized = false;
}

bool ToolBundler::assemble_with_llvm(const std::string& assembly_file,
                                    const std::string& object_file,
                                    const std::string& target_triple) {
    if (!llvm->available) {
        return false;
    }
    
    // Prepare arguments for llvm-mc
    std::vector<std::string> args = {
        "llvm-mc",
        "-filetype=obj",
        "-triple=" + target_triple,
        assembly_file,
        "-o", object_file
    };
    
    // Convert to char* array
    std::vector<char*> c_args;
    for (auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);
    
    // Call embedded LLVM (demo stub)
    int result = llvm_mc_main_stub(c_args.size() - 1, c_args.data());
    return result == 0;
}

bool ToolBundler::assemble_with_nasm(const std::string& assembly_file,
                                    const std::string& object_file,
                                    const std::string& format) {
    if (!nasm->available) {
        return false;
    }
    
    // Prepare arguments for NASM
    std::vector<std::string> args = {
        "nasm",
        "-f", format,
        assembly_file,
        "-o", object_file
    };
    
    // Convert to char* array
    std::vector<char*> c_args;
    for (auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);
    
    // Call embedded NASM (demo stub)
    int result = nasm_main_renamed_stub(c_args.size() - 1, c_args.data());
    return result == 0;
}

bool ToolBundler::compile_with_tcc(const std::string& c_file,
                                  const std::string& output_file,
                                  const std::vector<std::string>& additional_args) {
    // Demo stub - use system GCC for now
    std::string cmd = "gcc " + c_file + " -o " + output_file;
    printf("TCC (demo stub): %s\n", cmd.c_str());
    
    int result = std::system(cmd.c_str());
    return result == 0;
}

bool ToolBundler::link_with_lld(const std::vector<std::string>& object_files,
                               const std::string& output_executable,
                               const std::vector<std::string>& libraries,
                               const std::string& target_triple) {
    if (!lld->available) {
        return false;
    }
    
    // Prepare arguments for LLD
    std::vector<std::string> args = {
        "ld.lld",
        "-flavor", "gnu",  // Use GNU LD compatibility mode
        "-o", output_executable
    };
    
    // Add object files
    for (const auto& obj : object_files) {
        args.push_back(obj);
    }
    
    // Add library search paths
    args.push_back("-L/usr/lib/x86_64-linux-gnu");
    args.push_back("-L/lib/x86_64-linux-gnu");
    
    // Add system libraries
    args.push_back("--dynamic-linker=/lib64/ld-linux-x86-64.so.2");
    args.push_back("/usr/lib/x86_64-linux-gnu/crt1.o");
    args.push_back("/usr/lib/x86_64-linux-gnu/crti.o");
    args.push_back("/usr/lib/x86_64-linux-gnu/crtn.o");
    
    // Add requested libraries
    for (const auto& lib : libraries) {
        args.push_back("-l" + lib);
    }
    
    // Add standard C library
    args.push_back("-lc");
    
    // Convert to char* array
    std::vector<char*> c_args;
    for (auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);
    
    // Call embedded LLD
    int result = lld_link_stub(c_args.size() - 1, c_args.data());
    return result == 0;
}

bool ToolBundler::link_objects(const std::vector<std::string>& object_files,
                              const std::string& output_executable,
                              const std::vector<std::string>& libraries) {
    // Prefer LLD if available for self-contained linking
    if (lld->available) {
        return link_with_lld(object_files, output_executable, libraries);
    }
    
    // Fallback to system linker (ld) if LLD not available
    std::ostringstream cmd;
    cmd << "ld";
    
    // Add object files
    for (const auto& obj : object_files) {
        cmd << " " << obj;
    }
    
    // Add libraries
    for (const auto& lib : libraries) {
        cmd << " -l" << lib;
    }
    
    // Add standard library paths
    cmd << " -L/usr/lib/x86_64-linux-gnu";
    cmd << " -L/lib/x86_64-linux-gnu";
    cmd << " --dynamic-linker=/lib64/ld-linux-x86-64.so.2";
    cmd << " /usr/lib/x86_64-linux-gnu/crt1.o";
    cmd << " /usr/lib/x86_64-linux-gnu/crti.o";
    cmd << " /usr/lib/x86_64-linux-gnu/crtn.o";
    cmd << " -lc";
    
    cmd << " -o " << output_executable;
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

bool ToolBundler::is_tool_available(ToolType tool) {
    switch (tool) {
        case ToolType::LLVM_MC: return llvm->available;
        case ToolType::NASM: return nasm->available;
        case ToolType::TCC: return tcc->available;
        case ToolType::LLD: return lld->available;
        default: return false;
    }
}

std::string ToolBundler::get_tool_version(ToolType tool) {
    switch (tool) {
        case ToolType::LLVM_MC: return llvm->version;
        case ToolType::NASM: return nasm->version;
        case ToolType::TCC: return tcc->version;
        case ToolType::LLD: return lld->version;
        default: return "unknown";
    }
}

bool ToolBundler::setup_temporary_directory() {
    temp_dir = fs::temp_directory_path() / "aria_compiler";
    
    std::error_code ec;
    if (fs::exists(temp_dir)) {
        fs::remove_all(temp_dir, ec);
    }
    
    fs::create_directories(temp_dir, ec);
    return !ec;
}

bool ToolBundler::initialize_llvm() {
    // LLVM is statically linked, should always be available
    llvm->available = true;
    return true;
}

bool ToolBundler::initialize_nasm() {
    // NASM is statically linked, should always be available
    nasm->available = true;
    return true;
}

bool ToolBundler::initialize_tcc() {
    // Demo stub - always available
    tcc->available = true;
    return true;
}

bool ToolBundler::initialize_lld() {
    // LLD is statically linked, should always be available
    lld->available = true;
    return true;
}

std::string ToolBundler::create_temp_file(const std::string& prefix, const std::string& suffix) {
    auto now = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    
    return temp_dir + "/" + prefix + "_" + std::to_string(timestamp) + suffix;
}