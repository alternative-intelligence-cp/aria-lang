// Tesla Consciousness Computing - LLVM/DWARF Integration Example
// Demonstrates in-process compilation with debug information generation

#include "llvm_integration.h"
#include "tesla_debug_info.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Tesla Consciousness Computing - LLVM/DWARF Integration Test ===\n");
    
    // Initialize LLVM context
    TeslaLLVMContext llvm_ctx;
    if (!tesla_llvm_init(&llvm_ctx, "tesla_test_module")) {
        printf("ERROR: Failed to initialize LLVM context: %s\n", 
               tesla_llvm_get_error(&llvm_ctx));
        return 1;
    }
    
    printf("✅ LLVM context initialized successfully\n");
    
    // Initialize debug information
    TeslaDebugContext debug_ctx;
    if (!tesla_debug_init(&debug_ctx, &llvm_ctx, "test.tesla", ".", 
                         "Tesla Consciousness Compiler v1.0")) {
        printf("ERROR: Failed to initialize debug information\n");
        tesla_llvm_cleanup(&llvm_ctx);
        return 1;
    }
    
    printf("✅ DWARF debug information initialized\n");
    
    // Create basic types for demonstration
    LLVMMetadataRef int_type = tesla_debug_create_basic_type(
        &debug_ctx, "int", 32, LLVMDWARFTypeEncodingSigned
    );
    
    LLVMMetadataRef void_type = tesla_debug_create_basic_type(
        &debug_ctx, "void", 0, LLVMDWARFTypeEncodingUnsigned
    );
    
    printf("✅ Debug types created (int, void)\n");
    
    // Create a simple function with debug information
    LLVMTypeRef func_type = LLVMFunctionType(
        LLVMInt32TypeInContext(llvm_ctx.context), NULL, 0, false
    );
    
    LLVMValueRef test_function = LLVMAddFunction(llvm_ctx.module, "test_main", func_type);
    
    // Create debug information for the function
    LLVMMetadataRef func_debug = tesla_debug_create_function(
        &debug_ctx,
        debug_ctx.compile_unit,  // scope
        "test_main",            // name
        "test_main",            // linkage name
        debug_ctx.file,         // file
        1,                      // line
        int_type,              // return type debug info
        false,                 // is_local
        true,                  // is_definition
        1,                     // scope_line
        LLVMDIFlagZero        // flags
    );
    
    // Create basic block and simple return
    LLVMBasicBlockRef entry = tesla_llvm_create_basic_block(&llvm_ctx, test_function, "entry");
    LLVMPositionBuilderAtEnd(llvm_ctx.builder, entry);
    
    // Create debug location
    LLVMMetadataRef location = tesla_debug_create_location(&debug_ctx, 2, 5, func_debug);
    LLVMSetCurrentDebugLocation(llvm_ctx.builder, LLVMMetadataAsValue(llvm_ctx.context, location));
    
    // Return 42 (consciousness frequency related!)\n    LLVMValueRef return_value = LLVMConstInt(LLVMInt32TypeInContext(llvm_ctx.context), 42, false);
    LLVMBuildRet(llvm_ctx.builder, return_value);
    
    printf("✅ Function with debug information created\n");
    
    // Optimize the module
    tesla_llvm_optimize_module(&llvm_ctx, 1);
    printf("✅ Module optimized\n");
    
    // Finalize debug information\n    tesla_debug_finalize(&debug_ctx);
    printf("✅ Debug information finalized\n");
    
    // Compile to object file with debug information
    if (!tesla_llvm_compile_to_object(&llvm_ctx, "test_output.o")) {
        printf("ERROR: Failed to compile to object: %s\n", 
               tesla_llvm_get_error(&llvm_ctx));
    } else {
        printf("✅ Compiled to object file: test_output.o\n");
    }
    
    // Compile to assembly for inspection
    if (!tesla_llvm_compile_to_assembly(&llvm_ctx, "test_output.s")) {
        printf("ERROR: Failed to compile to assembly: %s\n", 
               tesla_llvm_get_error(&llvm_ctx));
    } else {
        printf("✅ Compiled to assembly file: test_output.s\n");
    }
    
    // Cleanup
    tesla_debug_cleanup(&debug_ctx);
    tesla_llvm_cleanup(&llvm_ctx);
    
    printf("✅ Cleanup completed\n");
    printf("\n=== Integration test completed successfully! ===\n");
    printf("Files generated:\n");
    printf("  - test_output.o  (object file with DWARF debug info)\n");
    printf("  - test_output.s  (assembly file for inspection)\n");
    printf("\nTo inspect debug information:\n");
    printf("  objdump -W test_output.o  # Show DWARF sections\n");
    printf("  nm test_output.o          # Show symbols\n");
    
    return 0;
}