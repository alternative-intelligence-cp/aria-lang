/* Aria_lang/src/frontend/parser.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "ast.h"
#include "token.h"

Token current_token;
Token previous_token;
AstArena* global_arena;

int had_error = 0;
int panic_mode = 0;

static char* current_class_name = NULL;

typedef struct {
    const char* name;
    int depth;
    int id; 
} Symbol;

#define MAX_SYMBOLS 2048
Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;
int current_scope_depth = 0;
int unique_var_id_counter = 1; 

void begin_scope() {
    current_scope_depth++;
}

void end_scope() {
    while (symbol_count > 0 && symbol_table[symbol_count - 1].depth > current_scope_depth) {
        symbol_count--;
    }
    current_scope_depth--;
}

int declare_variable(const char* name) {
    if (symbol_count >= MAX_SYMBOLS) {
        fprintf(stderr, "Fatal: Symbol table overflow.\n");
        exit(1);
    }
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (symbol_table[i].depth < current_scope_depth) break;
        if (strcmp(symbol_table[i].name, name) == 0) {
            fprintf(stderr, "Error: Variable '%s' already declared in this scope.\n", name);
            had_error = 1;
            return symbol_table[i].id;
        }
    }
    
    // FIX: Identify Global Variables
    int id;
    if (current_scope_depth == 0) {
        id = -2; // Special ID for Global Variables
    } else {
        id = unique_var_id_counter++;
    }

    symbol_table[symbol_count].name = name;
    symbol_table[symbol_count].depth = current_scope_depth;
    symbol_table[symbol_count].id = id;
    symbol_count++;
    return id;
}

int resolve_variable(const char* name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return symbol_table[i].id;
        }
    }
    // Return -1 for Unresolved (External Symbols/Functions)
    return -1; 
}

// --- Forward Declarations ---
AstNode* parse_expression(int precedence);
AstNode* parse_statement();
AstNode* parse_block();
AstNode* parse_var_decl();
AstNode* ternary_op(AstNode* left); 
void advance();
int match(TokenType type);
void consume(TokenType type, const char* message);

typedef enum {
    PREC_NONE, PREC_ASSIGNMENT, PREC_TERNARY, PREC_OR, PREC_AND,
    PREC_EQUALITY, PREC_COMPARISON, PREC_TERM, PREC_FACTOR,
    PREC_UNARY, PREC_CALL, PREC_PRIMARY
} Precedence;

typedef AstNode* (*ParsePrefixFn)();
typedef AstNode* (*ParseInfixFn)(AstNode* left);

typedef struct {
    ParsePrefixFn prefix;
    ParseInfixFn infix;
    Precedence precedence;
} ParseRule;

ParseRule rules[100]; 

void error_at_current(const char* message) {
    if (panic_mode) return;
    panic_mode = 1;
    had_error = 1;
    fprintf(stderr, "[line %d] Error at '%.*s': %s\n", 
            current_token.line, current_token.length, current_token.start, message);
}

void synchronize() {
    panic_mode = 0;
    while (current_token.type != TOKEN_EOF) {
        if (previous_token.type == TOKEN_SEMICOLON) return;
        switch (current_token.type) {
            case TOKEN_FUNC: case TOKEN_VAR: case TOKEN_IF:
            case TOKEN_WHILE: case TOKEN_FOR: case TOKEN_RETURN:
            case TOKEN_MANAGED: case TOKEN_CLASS:
                return;
            default: ;
        }
        advance();
    }
}

void advance() {
    previous_token = current_token;
    for (;;) {
        current_token = scan_token();
        if (current_token.type != TOKEN_ERROR) break;
        error_at_current(current_token.start); 
    }
}

void consume(TokenType type, const char* message) {
    if (current_token.type == type) {
        advance();
        return;
    }
    error_at_current(message);
}

int match(TokenType type) {
    if (current_token.type == type) {
        advance();
        return 1;
    }
    return 0;
}

char* parse_string_content(AstArena* arena, const char* src, int len) {
    char* buf = malloc(len + 1);
    if (!buf) exit(1);
    int write_idx = 0;
    for (int i = 1; i < len - 1; i++) {
        if (src[i] == '\\' && i + 1 < len - 1) {
            i++; 
            switch (src[i]) {
                case 'n': buf[write_idx++] = '\n'; break;
                case 'r': buf[write_idx++] = '\r'; break;
                case 't': buf[write_idx++] = '\t'; break;
                case '\\': buf[write_idx++] = '\\'; break;
                case '"': buf[write_idx++] = '"'; break;
                case '0': buf[write_idx++] = '\0'; break;
                default: buf[write_idx++] = src[i]; break; 
            }
        } else {
            buf[write_idx++] = src[i];
        }
    }
    buf[write_idx] = '\0';
    char* result = arena_strndup(arena, buf, write_idx);
    free(buf);
    return result;
}

AstNode* number() {
    AstNode* node = arena_alloc(global_arena);
    char buf[128]; 
    int len = previous_token.length < 127 ? previous_token.length : 127;
    memcpy(buf, previous_token.start, len);
    buf[len] = '\0';

    if (previous_token.type == TOKEN_FLOAT) {
        node->type = NODE_FLOAT;
        node->data.double_val = strtod(buf, NULL);
    } else {
        node->type = NODE_LITERAL;
        if (len > 2 && buf[0] == '0') {
            if (buf[1] == 'x' || buf[1] == 'X') {
                node->data.int_val = strtoll(buf, NULL, 16);
            } else if (buf[1] == 'b' || buf[1] == 'B') {
                char* p = buf + 2;
                int64_t val = 0;
                while (*p) {
                    val = (val << 1) | (*p == '1' ? 1 : 0);
                    p++;
                }
                node->data.int_val = val;
            } else {
                node->data.int_val = strtoll(buf, NULL, 0);
            }
        } else {
            node->data.int_val = strtoll(buf, NULL, 10);
        }
    }
    return node;
}

AstNode* literal() {
    AstNode* node = arena_alloc(global_arena);
    switch (previous_token.type) {
        case TOKEN_TRUE: node->type = NODE_BOOL; node->data.int_val = 1; break;
        case TOKEN_FALSE: node->type = NODE_BOOL; node->data.int_val = 0; break;
        case TOKEN_NULL: node->type = NODE_NULL; node->data.int_val = 0; break;
        default: return NULL;
    }
    return node;
}

AstNode* string_literal() {
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_STRING;
    node->data.string_val = parse_string_content(global_arena, previous_token.start, previous_token.length);
    return node;
}

AstNode* variable() {
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_VAR_ACCESS;
    node->data.var_access.name = arena_strndup(global_arena, previous_token.start, previous_token.length);
    node->data.var_access.id = resolve_variable(node->data.var_access.name);
    return node;
}

AstNode* grouping() {
    AstNode* expr = parse_expression(PREC_ASSIGNMENT);
    consume(TOKEN_RPAREN, "Expect ')' after expression.");
    return expr;
}

AstNode* unary() {
    TokenType operatorType = previous_token.type;
    AstNode* operand = parse_expression(PREC_UNARY);

    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_BINARY_OP;
    node->data.binary.op = operatorType;
    node->data.binary.left = NULL;
    node->data.binary.right = operand;
    return node;
}

AstNode* binary(AstNode* left) {
    TokenType op = previous_token.type;
    ParseRule* rule = &rules[op];
    AstNode* right = parse_expression((int)rule->precedence + 1);
    
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_BINARY_OP;
    node->data.binary.op = op;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}

AstNode* ternary_op(AstNode* condition) {
    AstNode* true_expr = parse_expression(PREC_TERNARY);
    consume(TOKEN_COLON, "Expect ':' after true branch of ternary.");
    AstNode* false_expr = parse_expression(PREC_TERNARY);
    
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_TERNARY;
    node->data.ternary.condition = condition;
    node->data.ternary.true_expr = true_expr;
    node->data.ternary.false_expr = false_expr;
    return node;
}

AstNode* call(AstNode* callee) {
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_CALL;
    node->data.call.callee = callee;
    node->data.call.args = NULL;

    if (current_token.type != TOKEN_RPAREN) {
        AstNode* arg = parse_expression(PREC_ASSIGNMENT);
        node->data.call.args = arg;
        while (match(TOKEN_COMMA)) {
            arg->next = parse_expression(PREC_ASSIGNMENT);
            arg = arg->next;
        }
    }
    consume(TOKEN_RPAREN, "Expect ')' after arguments.");
    return node;
}

AstNode* index_op(AstNode* left) {
    AstNode* index = parse_expression(PREC_ASSIGNMENT);
    consume(TOKEN_RBRACKET, "Expect ']' after index.");

    if (match(TOKEN_EQ)) {
        AstNode* val = parse_expression(PREC_ASSIGNMENT);
        AstNode* node = arena_alloc(global_arena);
        node->type = NODE_INDEX_SET;
        node->data.index_set.obj = left;
        node->data.index_set.index = index;
        node->data.index_set.value = val;
        return node;
    }

    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_INDEX_GET;
    node->data.index_get.obj = left;
    node->data.index_get.index = index;
    return node;
}

AstNode* array_literal() {
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_ARRAY_LITERAL;
    node->data.array_literal.elements = NULL;
    node->data.array_literal.count = 0;

    if (current_token.type != TOKEN_RBRACKET) {
        AstNode* elem = parse_expression(PREC_ASSIGNMENT);
        node->data.array_literal.elements = elem;
        node->data.array_literal.count++;
        
        AstNode* curr = elem;
        while (match(TOKEN_COMMA)) {
            AstNode* next = parse_expression(PREC_ASSIGNMENT);
            curr->next = next;
            curr = next;
            node->data.array_literal.count++;
        }
    }
    consume(TOKEN_RBRACKET, "Expect ']' after array elements.");
    return node;
}

AstNode* dot(AstNode* left) {
    consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    char* name = arena_strndup(global_arena, previous_token.start, previous_token.length);
    
    if (match(TOKEN_EQ)) {
        AstNode* value = parse_expression(PREC_ASSIGNMENT);
        AstNode* node = arena_alloc(global_arena);
        node->type = NODE_SET;
        node->data.set.obj = left;
        node->data.set.name = name;
        node->data.set.value = value;
        return node;
    }

    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_GET;
    node->data.get.obj = left;
    node->data.get.name = name;
    return node;
}

AstNode* object_new() {
    consume(TOKEN_IDENTIFIER, "Expect class name after 'new'.");
    char* name = arena_strndup(global_arena, previous_token.start, previous_token.length);
    consume(TOKEN_LPAREN, "Expect '()' after class name.");
    consume(TOKEN_RPAREN, "Expect '()' after class name.");
    
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_NEW;
    node->data.string_val = name;
    return node;
}

void init_rules() {
    for(int i=0; i<100; i++) rules[i] = (ParseRule){NULL, NULL, PREC_NONE};

    rules[TOKEN_IDENTIFIER] = (ParseRule){variable, NULL, PREC_NONE};
    rules[TOKEN_NUMBER]     = (ParseRule){number, NULL, PREC_NONE};
    rules[TOKEN_FLOAT]      = (ParseRule){number, NULL, PREC_NONE};
    rules[TOKEN_STRING]     = (ParseRule){string_literal, NULL, PREC_NONE};
    rules[TOKEN_LPAREN]     = (ParseRule){grouping, call, PREC_CALL};
    rules[TOKEN_TRUE]       = (ParseRule){literal, NULL, PREC_NONE};
    rules[TOKEN_FALSE]      = (ParseRule){literal, NULL, PREC_NONE};
    rules[TOKEN_NULL]       = (ParseRule){literal, NULL, PREC_NONE};
    
    rules[TOKEN_PLUS]       = (ParseRule){NULL, binary, PREC_TERM};
    rules[TOKEN_MINUS]      = (ParseRule){unary, binary, PREC_TERM}; 
    rules[TOKEN_STAR]       = (ParseRule){NULL, binary, PREC_FACTOR};
    rules[TOKEN_SLASH]      = (ParseRule){NULL, binary, PREC_FACTOR};
    rules[TOKEN_PERCENT]    = (ParseRule){NULL, binary, PREC_FACTOR};
    rules[TOKEN_BANG]       = (ParseRule){unary, NULL, PREC_UNARY};
    
    rules[TOKEN_EQEQ]       = (ParseRule){NULL, binary, PREC_EQUALITY};
    rules[TOKEN_NEQ]        = (ParseRule){NULL, binary, PREC_EQUALITY};
    rules[TOKEN_LT]         = (ParseRule){NULL, binary, PREC_COMPARISON};
    rules[TOKEN_LTEQ]       = (ParseRule){NULL, binary, PREC_COMPARISON};
    rules[TOKEN_GT]         = (ParseRule){NULL, binary, PREC_COMPARISON};
    rules[TOKEN_GTEQ]       = (ParseRule){NULL, binary, PREC_COMPARISON};
    rules[TOKEN_AND]        = (ParseRule){NULL, binary, PREC_AND};
    rules[TOKEN_OR]         = (ParseRule){NULL, binary, PREC_OR};
    
    rules[TOKEN_DOT]        = (ParseRule){NULL, dot, PREC_CALL};
    rules[TOKEN_LBRACKET]   = (ParseRule){array_literal, index_op, PREC_CALL}; 
    rules[TOKEN_NEW]        = (ParseRule){object_new, NULL, PREC_NONE};

    rules[TOKEN_IS]         = (ParseRule){NULL, ternary_op, PREC_TERNARY};
    rules[TOKEN_QUESTION]   = (ParseRule){NULL, ternary_op, PREC_TERNARY};
}

AstNode* parse_expression(int precedence) {
    advance();
    ParsePrefixFn prefix = rules[previous_token.type].prefix;
    if (!prefix) {
        error_at_current("Expect expression.");
        return NULL;
    }
    AstNode* left = prefix();

    while (precedence <= rules[current_token.type].precedence) {
        advance();
        ParseInfixFn infix = rules[previous_token.type].infix;
        if (!infix) return left;
        left = infix(left);
    }
    
    if (match(TOKEN_EQ)) {
        if (precedence <= PREC_ASSIGNMENT) {
            AstNode* value = parse_expression(PREC_ASSIGNMENT);
            if (left->type == NODE_VAR_ACCESS) {
                AstNode* assign = arena_alloc(global_arena);
                assign->type = NODE_ASSIGN;
                assign->data.assign.name = left->data.var_access.name;
                assign->data.assign.id = left->data.var_access.id; 
                assign->data.assign.value = value;
                return assign;
            }
            error_at_current("Invalid assignment target.");
        }
    }
    
    TokenType op = TOKEN_ERROR;
    if (match(TOKEN_PLUS_EQ)) op = TOKEN_PLUS;
    else if (match(TOKEN_MINUS_EQ)) op = TOKEN_MINUS;
    else if (match(TOKEN_STAR_EQ)) op = TOKEN_STAR;
    else if (match(TOKEN_SLASH_EQ)) op = TOKEN_SLASH;

    if (op != TOKEN_ERROR) {
         if (precedence <= PREC_ASSIGNMENT) {
            AstNode* value = parse_expression(PREC_ASSIGNMENT);
            if (left->type == NODE_VAR_ACCESS) {
                AstNode* bin = arena_alloc(global_arena);
                bin->type = NODE_BINARY_OP;
                bin->data.binary.op = op;
                bin->data.binary.left = left;
                bin->data.binary.right = value;

                AstNode* assign = arena_alloc(global_arena);
                assign->type = NODE_ASSIGN;
                assign->data.assign.name = left->data.var_access.name;
                assign->data.assign.id = left->data.var_access.id;
                assign->data.assign.value = bin;
                return assign;
            }
            error_at_current("Invalid assignment target.");
         }
    }
    return left;
}

AstNode* parse_var_decl() {
    consume(TOKEN_IDENTIFIER, "Expect variable name.");
    char* name = arena_strndup(global_arena, previous_token.start, previous_token.length);
    
    int id = declare_variable(name);

    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_VAR_DECL;
    node->data.var_decl.name = name;
    node->data.var_decl.shadow_stack_offset = id;
    node->data.var_decl.is_managed = 0; 
    
    if (match(TOKEN_EQ)) {
        node->data.var_decl.init_expr = parse_expression(PREC_ASSIGNMENT);
    } else {
        node->data.var_decl.init_expr = NULL;
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    return node;
}

AstNode* parse_statement() {
    if (match(TOKEN_VAR)) return parse_var_decl();
    if (match(TOKEN_MANAGED)) {
        consume(TOKEN_VAR, "Expect 'var' after 'managed'.");
        AstNode* node = parse_var_decl();
        node->data.var_decl.is_managed = 1;
        return node;
    }
    if (match(TOKEN_RETURN)) {
        AstNode* node = arena_alloc(global_arena);
        node->type = NODE_RETURN;
        if (current_token.type != TOKEN_SEMICOLON) {
             node->data.return_stmt.expr = parse_expression(PREC_ASSIGNMENT);
        } else {
             node->data.return_stmt.expr = NULL;
        }
        consume(TOKEN_SEMICOLON, "Expect ';' after return.");
        return node;
    }
    if (match(TOKEN_BREAK)) {
        AstNode* node = arena_alloc(global_arena);
        node->type = NODE_BREAK;
        consume(TOKEN_SEMICOLON, "Expect ';' after break.");
        return node;
    }
    if (match(TOKEN_CONTINUE)) {
        AstNode* node = arena_alloc(global_arena);
        node->type = NODE_CONTINUE;
        consume(TOKEN_SEMICOLON, "Expect ';' after continue.");
        return node;
    }
    if (match(TOKEN_IF)) {
        consume(TOKEN_LPAREN, "Expect '(' after 'if'.");
        AstNode* cond = parse_expression(PREC_ASSIGNMENT);
        consume(TOKEN_RPAREN, "Expect ')' after condition.");
        AstNode* thenBranch = parse_statement();
        AstNode* elseBranch = NULL;
        if (match(TOKEN_ELSE)) elseBranch = parse_statement();
        
        AstNode* node = arena_alloc(global_arena);
        node->type = NODE_IF;
        node->data.if_stmt.condition = cond;
        node->data.if_stmt.then_branch = thenBranch;
        node->data.if_stmt.else_branch = elseBranch;
        return node;
    }
    if (match(TOKEN_WHILE)) {
        consume(TOKEN_LPAREN, "Expect '(' after 'while'.");
        AstNode* cond = parse_expression(PREC_ASSIGNMENT);
        consume(TOKEN_RPAREN, "Expect ')' after condition.");
        AstNode* body = parse_statement();
        
        AstNode* node = arena_alloc(global_arena);
        node->type = NODE_WHILE;
        node->data.while_stmt.condition = cond;
        node->data.while_stmt.body = body;
        return node;
    }
    if (match(TOKEN_FOR)) {
        begin_scope(); 
        consume(TOKEN_LPAREN, "Expect '(' after 'for'.");
        AstNode* init = NULL;
        if (match(TOKEN_SEMICOLON)) {
        } else if (match(TOKEN_VAR)) {
            init = parse_var_decl();
        } else {
            AstNode* expr = parse_expression(PREC_ASSIGNMENT);
            consume(TOKEN_SEMICOLON, "Expect ';' after init.");
            init = expr; 
        }

        AstNode* condition = NULL;
        if (!match(TOKEN_SEMICOLON)) {
            condition = parse_expression(PREC_ASSIGNMENT);
            consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
        } else {
            AstNode* true_node = arena_alloc(global_arena);
            true_node->type = NODE_BOOL; 
            true_node->data.int_val = 1;
            condition = true_node;
        }

        AstNode* increment = NULL;
        if (!match(TOKEN_RPAREN)) {
            increment = parse_expression(PREC_ASSIGNMENT);
            consume(TOKEN_RPAREN, "Expect ')' after for clauses.");
        }

        AstNode* body = parse_statement();
        end_scope(); 

        AstNode* while_node = arena_alloc(global_arena);
        while_node->type = NODE_WHILE;
        while_node->data.while_stmt.condition = condition;

        if (increment) {
            AstNode* seq_block = arena_alloc(global_arena);
            seq_block->type = NODE_BLOCK;
            body->next = increment;
            seq_block->data.func_decl.body = body;
            while_node->data.while_stmt.body = seq_block;
        } else {
            while_node->data.while_stmt.body = body;
        }

        AstNode* outer_block = arena_alloc(global_arena);
        outer_block->type = NODE_BLOCK;
        if (init) {
            init->next = while_node;
            outer_block->data.func_decl.body = init;
        } else {
            outer_block->data.func_decl.body = while_node;
        }
        
        return outer_block;
    }
    
    if (match(TOKEN_LBRACE)) return parse_block();
    
    AstNode* expr = parse_expression(PREC_ASSIGNMENT);
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    return expr; 
}

AstNode* parse_block() {
    begin_scope();
    AstNode* head = NULL;
    AstNode* cur = NULL;
    while (current_token.type != TOKEN_RBRACE && current_token.type != TOKEN_EOF) {
        AstNode* stmt = parse_statement();
        if (stmt) {
            if (!head) head = stmt;
            else cur->next = stmt;
            cur = stmt;
        }
    }
    consume(TOKEN_RBRACE, "Expect '}' after block.");
    end_scope();
    
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_BLOCK;
    node->data.func_decl.body = head;
    return node;
}

AstNode* parse_function() {
    consume(TOKEN_IDENTIFIER, "Expect function name.");
    char* raw_name = arena_strndup(global_arena, previous_token.start, previous_token.length);
    
    char* name;
    if (current_class_name) {
        size_t cls_len = strlen(current_class_name);
        size_t fn_len = strlen(raw_name);
        char* mangled = malloc(cls_len + fn_len + 2);
        sprintf(mangled, "%s_%s", current_class_name, raw_name);
        name = arena_strndup(global_arena, mangled, cls_len + fn_len + 1);
        free(mangled);
    } else {
        // FIX: Rename entry point to avoid conflict with compiler-generated main
        if (strcmp(raw_name, "main") == 0) {
            name = "aria_main";
        } else {
            name = raw_name;
        }
    }

    consume(TOKEN_LPAREN, "Expect '(' after function name.");
    
    begin_scope(); 
    
    AstNode* param_head = NULL;
    AstNode* param_cur = NULL;
    
    if (current_token.type != TOKEN_RPAREN) {
        do {
            consume(TOKEN_IDENTIFIER, "Expect parameter name.");
            char* param_name = arena_strndup(global_arena, previous_token.start, previous_token.length);
            int id = declare_variable(param_name);
            
            AstNode* param = arena_alloc(global_arena);
            param->type = NODE_VAR_DECL;
            param->data.var_decl.name = param_name;
            param->data.var_decl.shadow_stack_offset = id;
            
            if (!param_head) param_head = param;
            else param_cur->next = param;
            param_cur = param;
            
        } while (match(TOKEN_COMMA));
    }
    
    consume(TOKEN_RPAREN, "Expect ')' after parameters.");
    consume(TOKEN_LBRACE, "Expect '{' before function body.");
    
    AstNode* body_block = parse_block(); 
    
    end_scope();
    
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_FUNC_DECL;
    node->data.func_decl.name = name;
    node->data.func_decl.params = param_head; 
    node->data.func_decl.body = body_block;
    return node;
}

AstNode* parse_class_decl() {
    consume(TOKEN_IDENTIFIER, "Expect class name.");
    char* name = arena_strndup(global_arena, previous_token.start, previous_token.length);
    consume(TOKEN_LBRACE, "Expect '{' before class body.");
    
    current_class_name = name;
    
    AstNode* head = NULL;
    AstNode* cur = NULL;
    
    while (current_token.type != TOKEN_RBRACE && current_token.type != TOKEN_EOF) {
        if (match(TOKEN_FUNC)) {
             AstNode* method = parse_function();
             if (!head) head = method;
             else cur->next = method;
             cur = method;
        } else {
             if (had_error) synchronize(); 
             else advance();
        }
    }
    consume(TOKEN_RBRACE, "Expect '}' after class body.");
    
    current_class_name = NULL;
    
    AstNode* node = arena_alloc(global_arena);
    node->type = NODE_CLASS_DECL;
    node->data.class_decl.name = name;
    node->data.class_decl.methods = head;
    return node;
}

AstNode* parse_program(AstArena* arena) {
    global_arena = arena;
    init_rules();
    advance(); 
    AstNode* head = NULL;
    AstNode* tail = NULL;
    
    symbol_count = 0;
    current_scope_depth = 0;
    unique_var_id_counter = 1;
    current_class_name = NULL;

    while (current_token.type != TOKEN_EOF) {
        AstNode* node = NULL;
        if (match(TOKEN_FUNC)) node = parse_function();
        else if (match(TOKEN_CLASS)) node = parse_class_decl();
        else if (match(TOKEN_VAR)) node = parse_var_decl();
        else if (match(TOKEN_MANAGED)) {
            consume(TOKEN_VAR, "Expect 'var' after 'managed'.");
            node = parse_var_decl();
            node->data.var_decl.is_managed = 1;
        } else {
            if (had_error) synchronize();
            else advance();
        }

        if (node) {
            if (!head) head = node;
            else tail->next = node;
            tail = node;
        }
    }
    if (had_error) exit(1);
    return head;
}
