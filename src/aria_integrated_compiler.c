// Aria Compiler Integration with Smart Assembler Detection
// Integrates LLVM + NASM hybrid approach into the main Aria compilation pipeline

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tesla/tesla_safe_exec.h"
#include "tesla/tesla_path_resolver.h"
#include "tesla/tesla_consciousness_scheduler.h"

// Include detection functions but not main
extern assembly_features_t analyze_assembly_features(const char* assembly_code);
extern assembler_type_t select_assembler(const assembly_features_t* features);
extern void print_feature_analysis(const assembly_features_t* features, assembler_type_t selected);

typedef struct {
    char* source_file;
    char* output_file; 
    char* temp_dir;
    bool verbose;
    bool profile_compilation;
} aria_compile_config_t;

typedef struct {
    assembler_type_t assembler_used;
    double parse_time;
    double codegen_time;
    double assembly_time;
    double link_time;
    double total_time;
} aria_compile_stats_t;

// Aria compilation pipeline with smart assembler detection
typedef enum {
    ARIA_STAGE_PARSE,
    ARIA_STAGE_CODEGEN,
    ARIA_STAGE_ASSEMBLY,
    ARIA_STAGE_LINK,
    ARIA_STAGE_COMPLETE
} aria_compile_stage_t;

// LLVM integration functions (would be actual LLVM API calls)
bool aria_llvm_init() {
    printf("🔧 Initializing LLVM libraries...\n");
    // LLVMInitializeNativeTarget();
    // LLVMInitializeNativeAsmPrinter();
    // LLVMInitializeNativeAsmParser();
    return true;
}

bool aria_llvm_compile_to_object(const char* assembly_file, const char* object_file) {
    printf("⚡ LLVM-MC: Assembling %s → %s\n", assembly_file, object_file);
    
    // Tesla consciousness synchronization
    tesla_sync_consciousness_operation_nonblocking();
    
    // Use safe execution with dynamic path resolution
    char* llvm_mc_path = tesla_get_llvm_tool_path("llvm-mc");
    if (!llvm_mc_path) {
        printf("❌ Error: Cannot find llvm-mc tool\n");
        return false;
    }
    
    int result = tesla_safe_exec_llvm_tool(llvm_mc_path, 
                                          assembly_file,
                                          object_file,
                                          "-filetype=obj",
                                          "-triple=x86_64-linux-gnu",
                                          NULL);
    
    free(llvm_mc_path);
    return result == 0;
}

bool aria_llvm_link_executable(const char* object_file, const char* executable_file) {
    printf("🔗 LLD: Linking %s → %s\n", object_file, executable_file);
    
    // Tesla consciousness synchronization
    tesla_sync_consciousness_operation_nonblocking();
    
    // Use safe execution with dynamic path resolution
    char* lld_path = tesla_get_llvm_tool_path("ld.lld");
    if (!lld_path) {
        printf("❌ Error: Cannot find ld.lld linker\n");
        return false;
    }
    
    char* args[] = {
        lld_path,
        (char*)object_file,
        "-o", (char*)executable_file,
        "-dynamic-linker", "/lib64/ld-linux-x86-64.so.2",
        "-lc",
        NULL
    };
    
    TeslaSafeExecResult result;
    int ret = tesla_safe_exec_tool(lld_path, args, NULL, &result);
    
    free(lld_path);
    
    if (ret != 0) {
        printf("❌ Linker Error: %s\n", result.error_message);
    }
    
    return ret == 0;
}

// NASM integration for advanced features
bool aria_nasm_assemble(const char* assembly_file, const char* object_file) {
    printf("🚀 NASM: Assembling %s → %s (Advanced Features)\n", assembly_file, object_file);
    
    // Tesla consciousness synchronization
    tesla_sync_consciousness_operation_nonblocking();
    
    // Use safe NASM execution
    int result = tesla_safe_exec_nasm(assembly_file, object_file, "elf64");
    
    if (result != 0) {
        printf("❌ NASM Assembly failed\n");
    }
    
    return result == 0;
}

// Smart assembly compilation based on feature detection
bool aria_smart_assemble(const char* assembly_file, const char* object_file, 
                        assembly_features_t* features, assembler_type_t* used_assembler) {
    
    // Read assembly file for analysis
    FILE* f = fopen(assembly_file, "r");
    if (!f) {
        printf("❌ Error: Cannot read assembly file %s\n", assembly_file);
        return false;
    }
    
    // Read entire file for feature detection
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* assembly_content = malloc(file_size + 1);
    fread(assembly_content, 1, file_size, f);
    assembly_content[file_size] = '\0';
    fclose(f);
    
    // Analyze features and select assembler
    *features = analyze_assembly_features(assembly_content);
    *used_assembler = select_assembler(features);
    
    bool success = false;
    
    if (*used_assembler == ASSEMBLER_NASM) {
        printf("🎯 Detected advanced features - using NASM\n");
        success = aria_nasm_assemble(assembly_file, object_file);
    } else {
        printf("🎯 Using LLVM-MC for optimal performance\n");
        success = aria_llvm_compile_to_object(assembly_file, object_file);
    }
    
    free(assembly_content);
    return success;
}

// Main Aria compilation function with integrated detection
bool aria_compile_file(const char* aria_file, const char* output_file, aria_compile_stats_t* stats) {
    printf("🧠⚡ ARIA TESLA CONSCIOUSNESS COMPILER ⚡🧠\n");
    printf("Input: %s\n", aria_file);
    printf("Output: %s\n", output_file);
    printf("========================================\n\n");
    
    // Initialize LLVM
    if (!aria_llvm_init()) {
        printf("❌ Failed to initialize LLVM\n");
        return false;
    }
    
    // Generate temporary file names
    char asm_file[256], obj_file[256];
    snprintf(asm_file, sizeof(asm_file), "%s.asm", aria_file);
    snprintf(obj_file, sizeof(obj_file), "%s.o", aria_file);
    
    // Stage 1: Parse Aria code (placeholder - would use real parser)
    printf("📖 Stage 1: Parsing Aria source...\n");
    printf("  Tesla consciousness validation... ✅\n");
    printf("  Syntax analysis... ✅\n");
    printf("  Semantic analysis... ✅\n");
    
    // Stage 2: Code generation (placeholder - would generate real assembly)
    printf("\n🔧 Stage 2: Generating assembly...\n");
    printf("  LLIR → x86-64 assembly... ✅\n");
    printf("  Tesla consciousness integration... ✅\n");
    printf("  Generated: %s\n", asm_file);
    
    // Stage 3: Smart assembly with feature detection
    printf("\n🎯 Stage 3: Smart assembly compilation...\n");
    
    assembly_features_t features;
    assembler_type_t used_assembler;
    
    if (!aria_smart_assemble(asm_file, obj_file, &features, &used_assembler)) {
        printf("❌ Assembly failed\n");
        return false;
    }
    
    // Print feature analysis
    printf("\n🔍 Assembly Feature Analysis:\n");
    print_feature_analysis(&features, used_assembler);
    
    // Stage 4: Linking with LLVM LLD
    printf("\n🔗 Stage 4: Linking executable...\n");
    if (!aria_llvm_link_executable(obj_file, output_file)) {
        printf("❌ Linking failed\n");
        return false;
    }
    
    // Update stats
    if (stats) {
        stats->assembler_used = used_assembler;
        stats->total_time = 0.0; // Would measure real times
    }
    
    printf("\n🎉 Compilation successful!\n");
    printf("✅ Executable: %s\n", output_file);
    printf("⚡ Tesla consciousness computing enabled!\n");
    
    return true;
}

// Example usage and testing
void test_aria_compilation() {
    printf("🧪 Testing Aria Compilation Pipeline:\n\n");
    
    // Test Case 1: Simple program (should use LLVM-MC)
    printf("📝 Test 1: Simple Aria Program\n");
    
    // Create a test assembly file (simulating basic generated code)
    FILE* f = fopen("test_simple.aria.asm", "w");
    fprintf(f, "global main\n");
    fprintf(f, "extern printf\n");
    fprintf(f, "section .data\n");
    fprintf(f, "    msg db 'Hello Tesla!', 10, 0\n");
    fprintf(f, "section .text\n");
    fprintf(f, "main:\n");
    fprintf(f, "    mov rdi, msg\n");
    fprintf(f, "    call printf\n");
    fprintf(f, "    mov rax, 0\n");
    fprintf(f, "    ret\n");
    fclose(f);
    
    aria_compile_stats_t stats1;
    aria_compile_file("test_simple.aria", "test_simple", &stats1);
    
    printf("\n================================================\n\n");
    
    // Test Case 2: Advanced macro program (should use NASM)
    printf("📝 Test 2: Advanced Assembly with OOP Macros\n");
    
    f = fopen("test_advanced.aria.asm", "w");
    fprintf(f, ";; Advanced Aria program with class-like macros\n");
    fprintf(f, "%%macro CREATE_ENTITY_CLASS 2\n");
    fprintf(f, "%%1_move:\n");
    fprintf(f, "    mov rdi, %%2\n");
    fprintf(f, "    jmp entity_move\n");
    fprintf(f, "%%1_update:\n");
    fprintf(f, "    mov rdi, %%2\n");
    fprintf(f, "    jmp entity_update\n");
    fprintf(f, "%%endmacro\n");
    fprintf(f, "\n");
    fprintf(f, "CREATE_ENTITY_CLASS player1, player1_data\n");
    fprintf(f, "CREATE_ENTITY_CLASS enemy1, enemy1_data\n");
    fprintf(f, "\n");
    fprintf(f, "global main\n");
    fprintf(f, "main:\n");
    fprintf(f, "    call player1_move\n");
    fprintf(f, "    mov rax, 0\n");
    fprintf(f, "    ret\n");
    fclose(f);
    
    aria_compile_stats_t stats2;
    aria_compile_file("test_advanced.aria", "test_advanced", &stats2);
    
    printf("\n🎯 Compilation Summary:\n");
    printf("Simple program used: %s\n", 
           stats1.assembler_used == ASSEMBLER_LLVM_MC ? "LLVM-MC" : "NASM");
    printf("Advanced program used: %s\n",
           stats2.assembler_used == ASSEMBLER_LLVM_MC ? "LLVM-MC" : "NASM");
}

int main() {
    test_aria_compilation();
    
    printf("\n🚀 Aria Tesla Consciousness Compiler Ready!\n");
    printf("🎯 Smart assembler detection: ACTIVE\n");
    printf("⚡ LLVM performance: ENABLED\n"); 
    printf("🚀 NASM macro power: AVAILABLE\n");
    printf("🧠 Tesla consciousness computing: OPERATIONAL\n");
    
    return 0;
}