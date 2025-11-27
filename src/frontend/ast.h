/* Aria_lang/src/frontend/ast.h */
#ifndef AST_H
#define AST_H

#include "token.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
    NODE_FUNC_DECL, NODE_VAR_DECL, NODE_BLOCK, 
    NODE_BINARY_OP, NODE_LITERAL, NODE_FLOAT, 
    NODE_BOOL, NODE_NULL, NODE_STRING,
    NODE_VAR_ACCESS, NODE_RETURN, NODE_CALL, 
    NODE_CLASS_DECL, NODE_NEW,
    NODE_IF, NODE_WHILE, NODE_BREAK, NODE_CONTINUE,
    NODE_ASSIGN, NODE_GET, NODE_SET,       
    NODE_INDEX_GET, NODE_INDEX_SET, NODE_ARRAY_LITERAL,
    NODE_TERNARY
} AstType;

struct AstNode;

// --- Node Data Structures ---

typedef struct {
    char* name;
    char* type_name;
    struct AstNode* init_expr;
    int is_managed;          
    int shadow_stack_offset; // Unique Variable ID
} VarDeclData;

typedef struct {
    char* name;
    int id; // Resolved Unique Variable ID
} VarAccessData;

typedef struct {
    char* name;
    struct AstNode* params; 
    struct AstNode* body;   
    int is_closure;         
    int upvalue_count;      
} FuncDeclData;

typedef struct {
    TokenType op;
    struct AstNode* left;
    struct AstNode* right;
} BinaryOpData;

typedef struct {
    struct AstNode* condition;
    struct AstNode* then_branch;
    struct AstNode* else_branch;
} IfStmtData;

typedef struct {
    struct AstNode* condition;
    struct AstNode* body;
} WhileStmtData;

typedef struct {
    struct AstNode* expr; 
} ReturnStmtData;

// FIX: Added 'id' to track assignment target resolution
typedef struct {
    char* name;
    int id; 
    struct AstNode* value;
} AssignData;

typedef struct {
    struct AstNode* callee;
    struct AstNode* args;
} CallData;

typedef struct {
    struct AstNode* obj;
    char* name;
    struct AstNode* value; 
} PropAccessData;

typedef struct {
    struct AstNode* obj;
    struct AstNode* index;
    struct AstNode* value; 
} IndexAccessData;

typedef struct {
    struct AstNode* elements;
    int count;
} ArrayLitData;

typedef struct {
    struct AstNode* condition;
    struct AstNode* true_expr;
    struct AstNode* false_expr;
} TernaryData;

typedef struct {
    char* name;
    struct AstNode* methods;
} ClassDeclData;

// --- Unified Node ---

typedef struct AstNode {
    AstType type;
    int line;
    struct AstNode* next; 
    union {
        FuncDeclData func_decl;
        VarDeclData var_decl;
        VarAccessData var_access;
        BinaryOpData binary;
        IfStmtData if_stmt;
        WhileStmtData while_stmt;
        ReturnStmtData return_stmt;
        AssignData assign;
        CallData call;
        PropAccessData get; 
        PropAccessData set;
        IndexAccessData index_get;
        IndexAccessData index_set;
        ArrayLitData array_literal;
        TernaryData ternary;
        ClassDeclData class_decl;
        
        char* string_val;   
        int64_t int_val; 
        double double_val; 
    } data;
} AstNode;

// Opaque Arena Handle
typedef struct AstArena AstArena;

AstArena* arena_create(); 
AstNode* arena_alloc(AstArena* arena);
void arena_free(AstArena* arena);
char* arena_strndup(AstArena* arena, const char* str, int len);

#endif
