/**
 * Tesla AST - Abstract Syntax Tree for Tesla Consciousness Computing
 * Foundation for comptime execution and metaprogramming
 */

#ifndef TESLA_AST_H
#define TESLA_AST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Tesla AST Node Types
typedef enum {
    TESLA_AST_PROGRAM = 0,
    TESLA_AST_INTEGER_LITERAL = 1,
    TESLA_AST_FLOAT_LITERAL = 2,
    TESLA_AST_STRING_LITERAL = 3,
    TESLA_AST_BOOLEAN_LITERAL = 4,
    TESLA_AST_IDENTIFIER = 5,
    
    // Comptime specific nodes
    TESLA_AST_COMPTIME_BLOCK = 10,
    TESLA_AST_COMPTIME_FUNCTION = 11,
    TESLA_AST_TYPE_GENERATION = 20,
    TESLA_AST_AI_TENSOR_CREATE = 30,
    TESLA_AST_AI_NEURAL_LAYER = 31,
    
    // Expressions
    TESLA_AST_BINARY_OP = 100,
    TESLA_AST_UNARY_OP = 101,
    TESLA_AST_FUNCTION_CALL = 102,
    
    // Statements
    TESLA_AST_VARIABLE_DECL = 200,
    TESLA_AST_FUNCTION_DECL = 201,
    TESLA_AST_RETURN_STMT = 202,
    TESLA_AST_BLOCK_STMT = 203
} TeslaASTNodeType;

// Forward declaration
struct TeslaAST;

// Tesla AST Node
typedef struct TeslaAST {
    TeslaASTNodeType type;
    
    // Source location
    int line;
    int column;
    const char* filename;
    
    // Node data
    union {
        int64_t integer;
        double float_val;
        char* string;
        int boolean;
        char* identifier;
        
        struct {
            struct TeslaAST* left;
            struct TeslaAST* right;
            int operator_type;
        } binary_op;
        
        struct {
            struct TeslaAST* operand;
            int operator_type;
        } unary_op;
        
        struct {
            char* name;
            struct TeslaAST** args;
            int arg_count;
        } function_call;
        
        struct {
            char* name;
            struct TeslaType* type;
            struct TeslaAST* initializer;
        } variable_decl;
        
        struct {
            char* name;
            struct TeslaAST** params;
            int param_count;
            struct TeslaAST* body;
        } function_decl;
        
        struct {
            struct TeslaAST* value;
        } return_stmt;
        
        struct {
            struct TeslaAST** statements;
            int statement_count;
        } block;
        
        struct {
            struct TeslaAST** statements;
            int statement_count;
        } comptime_block;
        
    } data;
    
    // Parent/child relationships
    struct TeslaAST* parent;
    struct TeslaAST** children;
    int child_count;
    int child_capacity;
    
} TeslaAST;

// AST creation functions
TeslaAST* tesla_ast_create(TeslaASTNodeType type);
TeslaAST* tesla_ast_create_integer(int64_t value);
TeslaAST* tesla_ast_create_float(double value);
TeslaAST* tesla_ast_create_string(const char* value);
TeslaAST* tesla_ast_create_identifier(const char* name);
TeslaAST* tesla_ast_create_comptime_block(void);

// AST manipulation
void tesla_ast_add_child(TeslaAST* parent, TeslaAST* child);
void tesla_ast_free(TeslaAST* ast);

// AST traversal
typedef void (*TeslaASTVisitor)(TeslaAST* node, void* data);
void tesla_ast_traverse(TeslaAST* ast, TeslaASTVisitor visitor, void* data);

#ifdef __cplusplus
}
#endif

#endif // TESLA_AST_H