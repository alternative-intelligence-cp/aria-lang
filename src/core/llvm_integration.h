#ifndef LLVM_INTEGRATION_H
#define LLVM_INTEGRATION_H

#include <stdint.h>
#include <stdbool.h>

// LLVM-C API includes for static library integration
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Transforms/PassManagerBuilder.h>

#ifdef __cplusplus
extern "C" {
#endif

// LLVM Integration Context
typedef struct {
    LLVMContextRef context;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMTargetMachineRef target_machine;
    LLVMExecutionEngineRef execution_engine;
    bool is_initialized;
    char error_message[1024];
} TeslaLLVMContext;

// LLVM Integration Functions
bool tesla_llvm_init(TeslaLLVMContext* ctx, const char* module_name);
void tesla_llvm_cleanup(TeslaLLVMContext* ctx);

// Code generation functions
LLVMValueRef tesla_llvm_create_function(TeslaLLVMContext* ctx, 
                                       const char* name,
                                       LLVMTypeRef return_type,
                                       LLVMTypeRef* param_types,
                                       unsigned param_count);

LLVMBasicBlockRef tesla_llvm_create_basic_block(TeslaLLVMContext* ctx,
                                               LLVMValueRef function,
                                               const char* name);

// Compilation functions
bool tesla_llvm_compile_to_object(TeslaLLVMContext* ctx, 
                                 const char* output_path);

bool tesla_llvm_compile_to_assembly(TeslaLLVMContext* ctx,
                                   const char* output_path);

// Optimization functions
void tesla_llvm_optimize_module(TeslaLLVMContext* ctx, unsigned opt_level);

// Error handling
const char* tesla_llvm_get_error(TeslaLLVMContext* ctx);
void tesla_llvm_clear_error(TeslaLLVMContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // LLVM_INTEGRATION_H