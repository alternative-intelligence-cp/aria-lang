#include "tesla_debug_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize debug information context
bool tesla_debug_init(TeslaDebugContext* debug_ctx,
                     TeslaLLVMContext* llvm_ctx,
                     const char* source_filename,
                     const char* directory,
                     const char* producer) {
    if (!debug_ctx || !llvm_ctx || !source_filename || !directory || !producer) {
        return false;
    }
    
    memset(debug_ctx, 0, sizeof(TeslaDebugContext));
    
    // Create DIBuilder
    debug_ctx->di_builder = LLVMCreateDIBuilder(llvm_ctx->module);
    if (!debug_ctx->di_builder) {
        return false;
    }
    
    // Create file metadata
    debug_ctx->file = LLVMDIBuilderCreateFile(
        debug_ctx->di_builder, 
        source_filename, strlen(source_filename),
        directory, strlen(directory)
    );
    
    // Create compile unit
    debug_ctx->compile_unit = LLVMDIBuilderCreateCompileUnit(
        debug_ctx->di_builder,
        LLVMDWARFSourceLanguageC11,  // Tesla uses C11 as base
        debug_ctx->file,
        producer, strlen(producer),
        false,  // is_optimized (set to false for debug builds)
        "", 0,  // flags and runtime_version
        "", 0,  // split_name
        LLVMDWARFEmissionFull,
        0, false, false, "", 0, "", 0
    );
    
    debug_ctx->current_scope = debug_ctx->compile_unit;
    debug_ctx->source_filename = source_filename;
    debug_ctx->directory = directory;
    debug_ctx->is_initialized = true;
    
    return true;
}

// Cleanup debug information context
void tesla_debug_cleanup(TeslaDebugContext* debug_ctx) {
    if (!debug_ctx) return;
    
    if (debug_ctx->di_builder) {
        LLVMDisposeDIBuilder(debug_ctx->di_builder);
        debug_ctx->di_builder = NULL;
    }
    
    debug_ctx->is_initialized = false;
}

// Create file metadata
LLVMMetadataRef tesla_debug_create_file(TeslaDebugContext* debug_ctx,
                                       const char* filename,
                                       const char* directory) {
    if (!debug_ctx || !debug_ctx->is_initialized || !filename || !directory) {
        return NULL;
    }
    
    return LLVMDIBuilderCreateFile(
        debug_ctx->di_builder,
        filename, strlen(filename),
        directory, strlen(directory)
    );
}

// Create compile unit metadata
LLVMMetadataRef tesla_debug_create_compile_unit(TeslaDebugContext* debug_ctx,
                                               LLVMMetadataRef file,
                                               const char* producer,
                                               bool is_optimized) {
    if (!debug_ctx || !debug_ctx->is_initialized || !file || !producer) {
        return NULL;
    }
    
    return LLVMDIBuilderCreateCompileUnit(
        debug_ctx->di_builder,
        LLVMDWARFSourceLanguageC11,
        file,
        producer, strlen(producer),
        is_optimized,
        "", 0,  // flags and runtime_version
        "", 0,  // split_name
        LLVMDWARFEmissionFull,
        0, false, false, "", 0, "", 0
    );
}

// Create function debug information
LLVMMetadataRef tesla_debug_create_function(TeslaDebugContext* debug_ctx,
                                           LLVMMetadataRef scope,
                                           const char* name,
                                           const char* linkage_name,
                                           LLVMMetadataRef file,
                                           unsigned line,
                                           LLVMMetadataRef type,
                                           bool is_local,
                                           bool is_definition,
                                           unsigned scope_line,
                                           LLVMDIFlags flags) {
    if (!debug_ctx || !debug_ctx->is_initialized || !name || !file || !type) {
        return NULL;
    }
    
    const char* actual_linkage_name = linkage_name ? linkage_name : name;
    
    return LLVMDIBuilderCreateFunction(
        debug_ctx->di_builder,
        scope ? scope : debug_ctx->compile_unit,
        name, strlen(name),
        actual_linkage_name, strlen(actual_linkage_name),
        file,
        line,
        type,
        is_local,
        is_definition,
        scope_line,
        flags,
        false  // is_optimized
    );
}

// Create variable debug information
LLVMMetadataRef tesla_debug_create_variable(TeslaDebugContext* debug_ctx,
                                          LLVMMetadataRef scope,
                                          const char* name,
                                          LLVMMetadataRef file,
                                          unsigned line,
                                          LLVMMetadataRef type,
                                          bool always_preserve) {
    if (!debug_ctx || !debug_ctx->is_initialized || !name || !file || !type) {
        return NULL;
    }
    
    return LLVMDIBuilderCreateAutoVariable(
        debug_ctx->di_builder,
        scope ? scope : debug_ctx->current_scope,
        name, strlen(name),
        file,
        line,
        type,
        always_preserve,
        LLVMDIFlagZero
    );
}

// Create basic type debug information
LLVMMetadataRef tesla_debug_create_basic_type(TeslaDebugContext* debug_ctx,
                                             const char* name,
                                             uint64_t size_bits,
                                             LLVMDWARFTypeEncoding encoding) {
    if (!debug_ctx || !debug_ctx->is_initialized || !name) {
        return NULL;
    }
    
    return LLVMDIBuilderCreateBasicType(
        debug_ctx->di_builder,
        name, strlen(name),
        size_bits,
        encoding,
        LLVMDIFlagZero
    );
}

// Create pointer type debug information
LLVMMetadataRef tesla_debug_create_pointer_type(TeslaDebugContext* debug_ctx,
                                               LLVMMetadataRef pointee_type,
                                               uint64_t size_bits,
                                               const char* name) {
    if (!debug_ctx || !debug_ctx->is_initialized || !pointee_type) {
        return NULL;
    }
    
    return LLVMDIBuilderCreatePointerType(
        debug_ctx->di_builder,
        pointee_type,
        size_bits,
        0,  // alignment
        0,  // address_space
        name ? name : "ptr", name ? strlen(name) : 3
    );
}

// Create location debug information
LLVMMetadataRef tesla_debug_create_location(TeslaDebugContext* debug_ctx,
                                           unsigned line,
                                           unsigned column,
                                           LLVMMetadataRef scope) {
    if (!debug_ctx || !debug_ctx->is_initialized) {
        return NULL;
    }
    
    return LLVMDIBuilderCreateDebugLocation(
        LLVMGetGlobalContext(),  // Use global context for locations
        line,
        column,
        scope ? scope : debug_ctx->current_scope,
        NULL  // inlined_at
    );
}

// Finalize debug information
void tesla_debug_finalize(TeslaDebugContext* debug_ctx) {
    if (!debug_ctx || !debug_ctx->is_initialized) {
        return;
    }
    
    LLVMDIBuilderFinalize(debug_ctx->di_builder);
}