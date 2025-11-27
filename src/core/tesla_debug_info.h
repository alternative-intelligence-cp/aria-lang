#ifndef TESLA_DEBUG_INFO_H
#define TESLA_DEBUG_INFO_H

#include "llvm_integration.h"
#include <llvm-c/DebugInfo.h>

#ifdef __cplusplus
extern "C" {
#endif

// Debug Information Context
typedef struct {
    LLVMDIBuilderRef di_builder;
    LLVMMetadataRef compile_unit;
    LLVMMetadataRef file;
    LLVMMetadataRef current_scope;
    const char* source_filename;
    const char* directory;
    bool is_initialized;
} TeslaDebugContext;

// Debug Information Functions
bool tesla_debug_init(TeslaDebugContext* debug_ctx, 
                     TeslaLLVMContext* llvm_ctx,
                     const char* source_filename,
                     const char* directory,
                     const char* producer);

void tesla_debug_cleanup(TeslaDebugContext* debug_ctx);

// File and compilation unit management
LLVMMetadataRef tesla_debug_create_file(TeslaDebugContext* debug_ctx,
                                       const char* filename,
                                       const char* directory);

LLVMMetadataRef tesla_debug_create_compile_unit(TeslaDebugContext* debug_ctx,
                                               LLVMMetadataRef file,
                                               const char* producer,
                                               bool is_optimized);

// Function debug information
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
                                           LLVMDIFlags flags);

// Variable debug information
LLVMMetadataRef tesla_debug_create_variable(TeslaDebugContext* debug_ctx,
                                          LLVMMetadataRef scope,
                                          const char* name,
                                          LLVMMetadataRef file,
                                          unsigned line,
                                          LLVMMetadataRef type,
                                          bool always_preserve);

// Type debug information
LLVMMetadataRef tesla_debug_create_basic_type(TeslaDebugContext* debug_ctx,
                                             const char* name,
                                             uint64_t size_bits,
                                             LLVMDWARFTypeEncoding encoding);

LLVMMetadataRef tesla_debug_create_pointer_type(TeslaDebugContext* debug_ctx,
                                               LLVMMetadataRef pointee_type,
                                               uint64_t size_bits,
                                               const char* name);

// Location information
LLVMMetadataRef tesla_debug_create_location(TeslaDebugContext* debug_ctx,
                                           unsigned line,
                                           unsigned column,
                                           LLVMMetadataRef scope);

// Finalization
void tesla_debug_finalize(TeslaDebugContext* debug_ctx);

#ifdef __cplusplus
}
#endif

#endif // TESLA_DEBUG_INFO_H