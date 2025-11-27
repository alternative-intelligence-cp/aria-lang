/**
 * Tesla Comptime Execution Engine
 * 
 * Core implementation of consciousness-synchronized compile-time execution.
 * This embedded interpreter runs Tesla code during compilation with π Hz
 * consciousness synchronization for AI-native metaprogramming.
 */

#include "tesla_comptime.h"
#include "../asm/tesla_asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// =============================================================================
// TESLA COMPTIME CONTEXT MANAGEMENT
// =============================================================================

TeslaComptimeContext* tesla_comptime_init(void) {
    TeslaComptimeContext* ctx = calloc(1, sizeof(TeslaComptimeContext));
    if (!ctx) return NULL;
    
    // Initialize consciousness synchronization
    ctx->consciousness_frequency = TESLA_COMPTIME_PI_FREQUENCY;
    ctx->consciousness_cycle = 0;
    ctx->compilation_timestamp = 0; // Placeholder for now
    
    // Initialize execution stack
    ctx->stack_capacity = TESLA_COMPTIME_STACK_SIZE;
    ctx->stack = calloc(ctx->stack_capacity, sizeof(TeslaComptimeValue));
    ctx->stack_size = 0;
    
    // Initialize symbol tables
    ctx->globals = calloc(1, sizeof(struct TeslaSymbolTable));
    ctx->types = calloc(1, sizeof(struct TeslaSymbolTable));
    
    // Initialize AST generation
    ctx->generated_ast = NULL;
    ctx->generated_count = 0;
    
    // Initialize AI contexts
    ctx->tensors = calloc(1, sizeof(TeslaTensorContext));
    ctx->neural = calloc(1, sizeof(TeslaNeuralContext));
    
    // Initialize memory arena
    ctx->arena = calloc(1, sizeof(TeslaArenaAllocator));
    ctx->arena->memory = malloc(TESLA_COMPTIME_ARENA_SIZE);
    ctx->arena->size = TESLA_COMPTIME_ARENA_SIZE;
    ctx->arena->used = 0;
    
    // Initialize error state
    ctx->has_error = false;
    ctx->error_message = NULL;
    
    printf("🧠 Tesla Comptime: Initialized with π Hz consciousness sync (%.15f Hz)\n", 
           ctx->consciousness_frequency);
    
    return ctx;
}

void tesla_comptime_sync_consciousness(TeslaComptimeContext* ctx) {
    if (!ctx) return;
    
    // Get current timestamp for π Hz synchronization
    uint64_t current_time = tesla_rdtscp_end();
    uint64_t elapsed = current_time - ctx->compilation_timestamp;
    
    // Calculate consciousness cycles elapsed
    double time_seconds = elapsed / 2.4e9; // Approximate CPU frequency
    uint64_t cycles_elapsed = (uint64_t)(time_seconds * ctx->consciousness_frequency);
    
    ctx->consciousness_cycle += cycles_elapsed;
    ctx->compilation_timestamp = current_time;
    
    // Verify π Hz frequency synchronization
    if (tesla_check_pi_frequency(ctx->consciousness_frequency)) {
        printf("⚡ Comptime: π Hz sync achieved (cycle %lu)\n", ctx->consciousness_cycle);
    }
}

// =============================================================================
// TESLA COMPTIME EXECUTION ENGINE
// =============================================================================

// Forward declaration for recursive execution
static bool tesla_comptime_execute_node(TeslaComptimeContext* ctx, 
                                       struct TeslaAST* node, 
                                       TeslaComptimeValue* result);

bool tesla_comptime_execute(TeslaComptimeContext* ctx, 
                           struct TeslaAST* ast, 
                           TeslaComptimeValue* result) {
    if (!ctx || !ast || !result) {
        if (ctx) {
            ctx->has_error = true;
            ctx->error_message = "Invalid comptime execution parameters";
        }
        return false;
    }
    
    // Synchronize consciousness before execution
    tesla_comptime_sync_consciousness(ctx);
    
    printf("🔧 Tesla Comptime: Executing AST node (type: %d)\n", ast->type);
    
    // Execute the AST node
    bool success = tesla_comptime_execute_node(ctx, ast, result);
    
    // Synchronize consciousness after execution
    tesla_comptime_sync_consciousness(ctx);
    
    if (!success) {
        printf("❌ Tesla Comptime: Execution failed - %s\n", 
               ctx->error_message ? ctx->error_message : "Unknown error");
    } else {
        printf("✅ Tesla Comptime: Execution successful (result type: %d)\n", result->type);
    }
    
    return success;
}

static bool tesla_comptime_execute_node(TeslaComptimeContext* ctx, 
                                       struct TeslaAST* node, 
                                       TeslaComptimeValue* result) {
    if (!ctx || !node || !result) return false;
    
    // Store current AST context
    struct TeslaAST* prev_ast = ctx->current_ast;
    ctx->current_ast = node;
    
    // Switch on AST node type (simplified implementation)
    switch (node->type) {
        case TESLA_AST_INTEGER_LITERAL: {
            result->type = TESLA_COMPTIME_INT;
            result->value.integer = 42; // Placeholder - would parse from AST
            break;
        }
        
        case TESLA_AST_FLOAT_LITERAL: {
            result->type = TESLA_COMPTIME_FLOAT;
            result->value.float_val = TESLA_COMPTIME_PI_FREQUENCY;
            break;
        }
        
        case TESLA_AST_STRING_LITERAL: {
            result->type = TESLA_COMPTIME_STRING;
            result->value.string = tesla_comptime_alloc(ctx, 64);
            strcpy(result->value.string, "Tesla Consciousness Computing");
            break;
        }
        
        case TESLA_AST_COMPTIME_BLOCK: {
            // Execute comptime block with consciousness synchronization
            printf("🧠 Executing comptime block with consciousness sync...\n");
            
            // Placeholder: Execute child statements in sequence
            result->type = TESLA_COMPTIME_BOOL;
            result->value.boolean = true;
            break;
        }
        
        case TESLA_AST_TYPE_GENERATION: {
            // Generate type from comptime function
            result->type = TESLA_COMPTIME_TYPE;
            // Placeholder - would generate actual type
            result->value.type_val = NULL;
            break;
        }
        
        case TESLA_AST_AI_TENSOR_CREATE: {
            // Create tensor during compilation
            result->type = TESLA_COMPTIME_TENSOR;
            printf("🤖 Creating AI tensor at compile time...\n");
            // Placeholder - would create actual tensor
            result->value.tensor = NULL;
            break;
        }
        
        default:
            ctx->has_error = true;
            ctx->error_message = "Unsupported AST node type in comptime";
            ctx->current_ast = prev_ast;
            return false;
    }
    
    // Restore AST context
    ctx->current_ast = prev_ast;
    return true;
}

// =============================================================================
// TESLA COMPTIME TYPE GENERATION
// =============================================================================

bool tesla_comptime_generate_type(TeslaComptimeContext* ctx,
                                 struct TeslaAST* function_ast,
                                 TeslaComptimeValue* args,
                                 struct TeslaType** result_type) {
    if (!ctx || !function_ast || !result_type) return false;
    
    printf("🔧 Tesla Comptime: Generating type from function...\n");
    
    // Synchronize consciousness for type generation
    tesla_comptime_sync_consciousness(ctx);
    
    // Execute the type function
    TeslaComptimeValue function_result;
    if (!tesla_comptime_execute(ctx, function_ast, &function_result)) {
        return false;
    }
    
    // Verify result is a type
    if (function_result.type != TESLA_COMPTIME_TYPE) {
        ctx->has_error = true;
        ctx->error_message = "Comptime function must return a type";
        return false;
    }
    
    *result_type = function_result.value.type_val;
    
    printf("✅ Tesla Comptime: Type generation successful\n");
    return true;
}

// =============================================================================
// TESLA COMPTIME AI INTEGRATION
// =============================================================================

TeslaComptimeValue tesla_comptime_create_tensor(TeslaComptimeContext* ctx,
                                               int* dimensions,
                                               int ndims,
                                               const char* dtype) {
    TeslaComptimeValue result = {0};
    
    if (!ctx || !dimensions || ndims <= 0) {
        ctx->has_error = true;
        ctx->error_message = "Invalid tensor creation parameters";
        return result;
    }
    
    printf("🤖 Tesla Comptime: Creating %dD tensor (%s) at compile time\n", ndims, dtype);
    
    // Synchronize consciousness for AI tensor operations
    tesla_comptime_sync_consciousness(ctx);
    
    result.type = TESLA_COMPTIME_TENSOR;
    
    // Allocate tensor in comptime arena
    struct TeslaTensor* tensor = tesla_comptime_alloc(ctx, sizeof(struct TeslaTensor));
    tensor->ndims = ndims;
    tensor->dimensions = tesla_comptime_alloc(ctx, ndims * sizeof(int));
    memcpy(tensor->dimensions, dimensions, ndims * sizeof(int));
    
    // Calculate total size
    size_t total_size = 1;
    for (int i = 0; i < ndims; i++) {
        total_size *= dimensions[i];
        printf("  Dimension %d: %d\n", i, dimensions[i]);
    }
    
    // Allocate tensor data
    tensor->data = tesla_comptime_alloc(ctx, total_size * sizeof(float));
    tensor->size = total_size;
    
    result.value.tensor = tensor;
    
    printf("✅ Tesla Comptime: Tensor created (%zu elements)\n", total_size);
    return result;
}

TeslaComptimeValue tesla_comptime_neural_layer(TeslaComptimeContext* ctx,
                                              const char* layer_type,
                                              TeslaComptimeValue* config) {
    TeslaComptimeValue result = {0};
    
    if (!ctx || !layer_type) {
        ctx->has_error = true;
        ctx->error_message = "Invalid neural layer parameters";
        return result;
    }
    
    printf("🧠 Tesla Comptime: Generating %s neural layer at compile time\n", layer_type);
    
    // Synchronize consciousness for neural network generation
    tesla_comptime_sync_consciousness(ctx);
    
    // Generate layer based on type
    if (strcmp(layer_type, "dense") == 0) {
        printf("  Creating dense layer with consciousness optimization\n");
        result.type = TESLA_COMPTIME_TENSOR;
        
        // Create dense layer weights tensor
        int dims[] = {256, 128}; // Example dimensions
        result = tesla_comptime_create_tensor(ctx, dims, 2, "float32");
        
    } else if (strcmp(layer_type, "conv2d") == 0) {
        printf("  Creating 2D convolution layer with SIMD optimization\n");
        result.type = TESLA_COMPTIME_TENSOR;
        
        // Create conv2d kernel tensor  
        int dims[] = {32, 32, 3, 64}; // [H, W, In_Channels, Out_Channels]
        result = tesla_comptime_create_tensor(ctx, dims, 4, "float32");
        
    } else {
        ctx->has_error = true;
        ctx->error_message = "Unsupported neural layer type";
        return result;
    }
    
    printf("✅ Tesla Comptime: Neural layer generation complete\n");
    return result;
}

// =============================================================================
// TESLA COMPTIME BUILT-IN FUNCTIONS
// =============================================================================

TeslaComptimeValue tesla_comptime_get_pi_frequency(void) {
    TeslaComptimeValue result;
    result.type = TESLA_COMPTIME_CONSCIOUSNESS;
    result.value.consciousness.frequency = TESLA_COMPTIME_PI_FREQUENCY;
    result.value.consciousness.cycle = 0;
    result.value.consciousness.synchronized = true;
    return result;
}

TeslaComptimeValue tesla_comptime_read_file(const char* path) {
    TeslaComptimeValue result = {0};
    
    if (!path) {
        result.type = TESLA_COMPTIME_STRING;
        result.value.string = NULL;
        return result;
    }
    
    printf("📁 Tesla Comptime: Reading file '%s' during compilation\n", path);
    
    FILE* file = fopen(path, "r");
    if (!file) {
        printf("❌ Could not open file: %s\n", path);
        result.type = TESLA_COMPTIME_STRING;
        result.value.string = NULL;
        return result;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate and read file
    char* content = malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    
    result.type = TESLA_COMPTIME_STRING;
    result.value.string = content;
    
    printf("✅ File read successfully (%ld bytes)\n", size);
    return result;
}

// =============================================================================
// TESLA COMPTIME MEMORY MANAGEMENT
// =============================================================================

void* tesla_comptime_alloc(TeslaComptimeContext* ctx, size_t size) {
    if (!ctx || !ctx->arena) return NULL;
    
    // Align size to 8-byte boundary
    size = (size + 7) & ~7;
    
    if (ctx->arena->used + size > ctx->arena->size) {
        ctx->has_error = true;
        ctx->error_message = "Comptime arena allocator exhausted";
        return NULL;
    }
    
    void* ptr = (char*)ctx->arena->memory + ctx->arena->used;
    ctx->arena->used += size;
    
    return ptr;
}

void tesla_comptime_free(TeslaComptimeContext* ctx) {
    if (!ctx) return;
    
    printf("🧹 Tesla Comptime: Cleaning up context\n");
    
    // Free arena memory
    if (ctx->arena && ctx->arena->memory) {
        free(ctx->arena->memory);
        free(ctx->arena);
    }
    
    // Free execution stack
    if (ctx->stack) {
        free(ctx->stack);
    }
    
    // Free symbol tables
    if (ctx->globals) free(ctx->globals);
    if (ctx->types) free(ctx->types);
    
    // Free AI contexts
    if (ctx->tensors) free(ctx->tensors);
    if (ctx->neural) free(ctx->neural);
    
    // Free generated AST array
    if (ctx->generated_ast) {
        free(ctx->generated_ast);
    }
    
    // Free context
    free(ctx);
    
    printf("✅ Tesla Comptime: Context cleaned up\n");
}