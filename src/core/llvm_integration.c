#include "llvm_integration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize LLVM context and components
bool tesla_llvm_init(TeslaLLVMContext* ctx, const char* module_name) {
    if (!ctx || !module_name) {
        return false;
    }
    
    memset(ctx, 0, sizeof(TeslaLLVMContext));
    
    // Initialize LLVM targets
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();
    
    // Create LLVM context and module
    ctx->context = LLVMContextCreate();
    if (!ctx->context) {
        snprintf(ctx->error_message, sizeof(ctx->error_message),
                "Failed to create LLVM context");
        return false;
    }
    
    ctx->module = LLVMModuleCreateWithNameInContext(module_name, ctx->context);
    if (!ctx->module) {
        snprintf(ctx->error_message, sizeof(ctx->error_message),
                "Failed to create LLVM module");
        LLVMContextDispose(ctx->context);
        return false;
    }
    
    // Create IR builder
    ctx->builder = LLVMCreateBuilderInContext(ctx->context);
    if (!ctx->builder) {
        snprintf(ctx->error_message, sizeof(ctx->error_message),
                "Failed to create LLVM builder");
        LLVMDisposeModule(ctx->module);
        LLVMContextDispose(ctx->context);
        return false;
    }
    
    // Initialize target machine for current platform
    char* target_triple = LLVMGetDefaultTargetTriple();
    LLVMTargetRef target;
    char* error_msg = NULL;
    
    if (LLVMGetTargetFromTriple(target_triple, &target, &error_msg)) {
        snprintf(ctx->error_message, sizeof(ctx->error_message),
                "Failed to get target: %s", error_msg ? error_msg : "unknown error");
        LLVMDisposeMessage(error_msg);
        LLVMDisposeMessage(target_triple);
        tesla_llvm_cleanup(ctx);
        return false;
    }
    
    ctx->target_machine = LLVMCreateTargetMachine(
        target, target_triple, "generic", "",
        LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault
    );
    
    LLVMDisposeMessage(target_triple);
    
    if (!ctx->target_machine) {
        snprintf(ctx->error_message, sizeof(ctx->error_message),
                "Failed to create target machine");
        tesla_llvm_cleanup(ctx);
        return false;
    }
    
    ctx->is_initialized = true;
    return true;
}

// Cleanup LLVM context and resources
void tesla_llvm_cleanup(TeslaLLVMContext* ctx) {
    if (!ctx) return;
    
    if (ctx->execution_engine) {
        LLVMDisposeExecutionEngine(ctx->execution_engine);
        ctx->execution_engine = NULL;
    }
    
    if (ctx->target_machine) {
        LLVMDisposeTargetMachine(ctx->target_machine);
        ctx->target_machine = NULL;
    }
    
    if (ctx->builder) {
        LLVMDisposeBuilder(ctx->builder);
        ctx->builder = NULL;
    }
    
    if (ctx->module) {
        LLVMDisposeModule(ctx->module);
        ctx->module = NULL;
    }
    
    if (ctx->context) {
        LLVMContextDispose(ctx->context);
        ctx->context = NULL;
    }
    
    ctx->is_initialized = false;
}

// Create LLVM function
LLVMValueRef tesla_llvm_create_function(TeslaLLVMContext* ctx,
                                       const char* name,
                                       LLVMTypeRef return_type,
                                       LLVMTypeRef* param_types,
                                       unsigned param_count) {
    if (!ctx || !ctx->is_initialized || !name || !return_type) {
        return NULL;
    }
    
    LLVMTypeRef function_type = LLVMFunctionType(
        return_type, param_types, param_count, false
    );
    
    LLVMValueRef function = LLVMAddFunction(ctx->module, name, function_type);
    LLVMSetLinkage(function, LLVMExternalLinkage);
    
    return function;
}

// Create basic block
LLVMBasicBlockRef tesla_llvm_create_basic_block(TeslaLLVMContext* ctx,
                                               LLVMValueRef function,
                                               const char* name) {
    if (!ctx || !ctx->is_initialized || !function || !name) {
        return NULL;
    }
    
    return LLVMAppendBasicBlockInContext(ctx->context, function, name);
}

// Compile module to object file
bool tesla_llvm_compile_to_object(TeslaLLVMContext* ctx, const char* output_path) {
    if (!ctx || !ctx->is_initialized || !output_path) {
        return false;
    }
    
    char* error_msg = NULL;
    LLVMCodeGenFileType file_type = LLVMObjectFile;
    
    if (LLVMTargetMachineEmitToFile(ctx->target_machine, ctx->module,
                                   (char*)output_path, file_type, &error_msg)) {
        snprintf(ctx->error_message, sizeof(ctx->error_message),
                "Failed to emit object file: %s", error_msg ? error_msg : "unknown error");
        if (error_msg) LLVMDisposeMessage(error_msg);
        return false;
    }
    
    return true;
}

// Compile module to assembly
bool tesla_llvm_compile_to_assembly(TeslaLLVMContext* ctx, const char* output_path) {
    if (!ctx || !ctx->is_initialized || !output_path) {
        return false;
    }
    
    char* error_msg = NULL;
    LLVMCodeGenFileType file_type = LLVMAssemblyFile;
    
    if (LLVMTargetMachineEmitToFile(ctx->target_machine, ctx->module,
                                   (char*)output_path, file_type, &error_msg)) {
        snprintf(ctx->error_message, sizeof(ctx->error_message),
                "Failed to emit assembly file: %s", error_msg ? error_msg : "unknown error");
        if (error_msg) LLVMDisposeMessage(error_msg);
        return false;
    }
    
    return true;
}

// Optimize module
void tesla_llvm_optimize_module(TeslaLLVMContext* ctx, unsigned opt_level) {
    if (!ctx || !ctx->is_initialized) {
        return;
    }
    
    // Create pass manager
    LLVMPassManagerRef pass_manager = LLVMCreatePassManager();
    
    // Add basic optimizations based on opt_level
    if (opt_level >= 1) {
        LLVMAddInstructionCombiningPass(pass_manager);
        LLVMAddReassociatePass(pass_manager);
        LLVMAddGVNPass(pass_manager);
        LLVMAddCFGSimplificationPass(pass_manager);
    }
    
    if (opt_level >= 2) {
        LLVMAddFunctionInliningPass(pass_manager);
        LLVMAddGlobalDCEPass(pass_manager);
        LLVMAddGlobalOptimizerPass(pass_manager);
    }
    
    if (opt_level >= 3) {
        LLVMAddAggressiveDCEPass(pass_manager);
        LLVMAddScalarReplAggregatesPass(pass_manager);
        LLVMAddLoopUnrollPass(pass_manager);
    }
    
    // Run optimization passes
    LLVMRunPassManager(pass_manager, ctx->module);
    
    // Cleanup
    LLVMDisposePassManager(pass_manager);
}

// Get error message
const char* tesla_llvm_get_error(TeslaLLVMContext* ctx) {
    if (!ctx) return "Invalid context";
    return ctx->error_message;
}

// Clear error message
void tesla_llvm_clear_error(TeslaLLVMContext* ctx) {
    if (ctx) {
        ctx->error_message[0] = '\0';
    }
}