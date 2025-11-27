/**
 * Tesla Type System - Types for Tesla Consciousness Computing
 * Advanced type system supporting comptime type generation
 */

#ifndef TESLA_TYPES_H
#define TESLA_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Tesla Type IDs
typedef enum {
    TESLA_TYPE_VOID = 0,
    TESLA_TYPE_BOOL = 1,
    TESLA_TYPE_I8 = 2,
    TESLA_TYPE_I16 = 3,
    TESLA_TYPE_I32 = 4,
    TESLA_TYPE_I64 = 5,
    TESLA_TYPE_U8 = 6,
    TESLA_TYPE_U16 = 7,
    TESLA_TYPE_U32 = 8,
    TESLA_TYPE_U64 = 9,
    TESLA_TYPE_F32 = 10,
    TESLA_TYPE_F64 = 11,
    TESLA_TYPE_STRING = 12,
    
    // Advanced types
    TESLA_TYPE_POINTER = 100,
    TESLA_TYPE_ARRAY = 101,
    TESLA_TYPE_STRUCT = 102,
    TESLA_TYPE_FUNCTION = 103,
    TESLA_TYPE_GENERIC = 104,
    
    // AI types
    TESLA_TYPE_TENSOR = 200,
    TESLA_TYPE_NEURAL_LAYER = 201,
    TESLA_TYPE_NEURAL_NETWORK = 202,
    
    // Comptime types
    TESLA_TYPE_COMPTIME_INT = 300,
    TESLA_TYPE_COMPTIME_TYPE = 301,
    TESLA_TYPE_COMPTIME_STRING = 302,
    
    // Consciousness types  
    TESLA_TYPE_CONSCIOUSNESS = 400
} TeslaTypeID;

// Forward declarations
struct TeslaType;
struct TeslaSymbol;

// Tesla Type structure
typedef struct TeslaType {
    TeslaTypeID id;
    const char* name;
    size_t size;
    size_t alignment;
    
    union {
        // Pointer type
        struct {
            struct TeslaType* pointed_type;
        } pointer;
        
        // Array type
        struct {
            struct TeslaType* element_type;
            size_t length;
        } array;
        
        // Struct type
        struct {
            struct TeslaSymbol** fields;
            int field_count;
        } struct_type;
        
        // Function type
        struct {
            struct TeslaType* return_type;
            struct TeslaType** param_types;
            int param_count;
        } function;
        
        // Generic type
        struct {
            char* name;
            struct TeslaType** constraints;
            int constraint_count;
        } generic;
        
        // Tensor type (AI)
        struct {
            struct TeslaType* element_type;
            int* dimensions;
            int ndims;
        } tensor;
        
    } data;
    
    // Type flags
    bool is_signed;
    bool is_mutable;
    bool is_comptime;
    bool is_consciousness_sync;
    
} TeslaType;

// Symbol for struct fields, variables, etc.
typedef struct TeslaSymbol {
    char* name;
    TeslaType* type;
    void* value;
    bool is_comptime;
    bool is_mutable;
} TeslaSymbol;

// Tesla Tensor structure for AI operations
typedef struct TeslaTensor {
    void* data;
    TeslaType* element_type;
    int* dimensions;
    int ndims;
    size_t size;
    size_t total_elements;
    bool is_comptime;
    bool consciousness_sync;
} TeslaTensor;

// Symbol table
typedef struct TeslaSymbolTable {
    TeslaSymbol** symbols;
    int count;
    int capacity;
    struct TeslaSymbolTable* parent; // For nested scopes
} TeslaSymbolTable;

// Type creation functions
TeslaType* tesla_type_create_primitive(TeslaTypeID id);
TeslaType* tesla_type_create_pointer(TeslaType* pointed_type);
TeslaType* tesla_type_create_array(TeslaType* element_type, size_t length);
TeslaType* tesla_type_create_struct(const char* name);
TeslaType* tesla_type_create_function(TeslaType* return_type, TeslaType** param_types, int param_count);
TeslaType* tesla_type_create_tensor(TeslaType* element_type, int* dimensions, int ndims);

// Type operations
bool tesla_type_equals(TeslaType* a, TeslaType* b);
size_t tesla_type_sizeof(TeslaType* type);
size_t tesla_type_alignof(TeslaType* type);
bool tesla_type_is_compatible(TeslaType* from, TeslaType* to);

// Symbol table operations
TeslaSymbolTable* tesla_symbol_table_create(TeslaSymbolTable* parent);
void tesla_symbol_table_add(TeslaSymbolTable* table, const char* name, TeslaType* type);
TeslaSymbol* tesla_symbol_table_lookup(TeslaSymbolTable* table, const char* name);
void tesla_symbol_table_free(TeslaSymbolTable* table);

#ifdef __cplusplus
}
#endif

#endif // TESLA_TYPES_H