/* Aria_lang/src/backend/codegen.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include "../frontend/ast.h"

#define REG_COUNT 14
static const char* REG_NAMES[REG_COUNT] = {
    "rbx", "r12", "r13", "r14", "r15", 
    "r10", "r11", "r8", "r9",          
    "rsi", "rdi", "rcx", "rdx", "rax"  
};

static const char* ABI_ARG_REGS[6] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

FILE* asm_out = NULL;
static int label_seq = 0;
static int instruction_counter = 0; 
static int max_stack_usage = 0; 
static AstNode* program_root = NULL; 

typedef struct LiveInterval {
    int var_id;         
    int start;          
    int end;            
    int reg_index;      
    int stack_offset;   
    struct LiveInterval* next;
} LiveInterval;

typedef struct {
    LiveInterval* intervals; 
    int count;
    int capacity;
} IntervalMap;

IntervalMap global_intervals;

void gen_function_node(AstNode* func);
void gen_expression(AstNode* node);
void gen_string_literal(const char* str);

void emit(const char* format,...) {
    va_list args;
    va_start(args, format);
    fprintf(asm_out, "    "); 
    vfprintf(asm_out, format, args);
    fprintf(asm_out, "\n");
    va_end(args);
}

void liveness_record_use(int var_id, int instr_idx) {
    if (var_id <= 0) return; 
    for(int i=0; i<global_intervals.count; i++) {
        if (global_intervals.intervals[i].var_id == var_id) {
            if (global_intervals.intervals[i].start == -1) global_intervals.intervals[i].start = instr_idx;
            if (instr_idx > global_intervals.intervals[i].end) global_intervals.intervals[i].end = instr_idx;
            return;
        }
    }
    if (global_intervals.count >= global_intervals.capacity) {
        global_intervals.capacity *= 2;
        global_intervals.intervals = realloc(global_intervals.intervals, sizeof(LiveInterval) * global_intervals.capacity);
    }
    LiveInterval* iv = &global_intervals.intervals[global_intervals.count++];
    iv->var_id = var_id;
    iv->start = instr_idx; 
    iv->end = instr_idx;   
    iv->reg_index = -1;
    iv->stack_offset = 0;
}

void analyze_liveness(AstNode* node) {
    if (!node) return;
    int current_instr = instruction_counter++;
    switch(node->type) {
        case NODE_VAR_DECL: {
            int vid = node->data.var_decl.shadow_stack_offset;
            liveness_record_use(vid, current_instr);
            if (node->data.var_decl.init_expr) analyze_liveness(node->data.var_decl.init_expr);
            break;
        }
        case NODE_VAR_ACCESS: liveness_record_use(node->data.var_access.id, current_instr); break;
        case NODE_BINARY_OP: analyze_liveness(node->data.binary.left); analyze_liveness(node->data.binary.right); break;
        case NODE_BLOCK: {
            AstNode* stmt = node->data.func_decl.body;
            while(stmt) { analyze_liveness(stmt); stmt = stmt->next; }
            break;
        }
        case NODE_WHILE: analyze_liveness(node->data.while_stmt.condition); analyze_liveness(node->data.while_stmt.body); break;
        case NODE_IF: analyze_liveness(node->data.if_stmt.condition); analyze_liveness(node->data.if_stmt.then_branch); 
            if (node->data.if_stmt.else_branch) analyze_liveness(node->data.if_stmt.else_branch); break;
        case NODE_RETURN: if (node->data.return_stmt.expr) analyze_liveness(node->data.return_stmt.expr); break;
        case NODE_CALL: {
            analyze_liveness(node->data.call.callee);
            AstNode* arg = node->data.call.args;
            while(arg) { analyze_liveness(arg); arg = arg->next; }
            break;
        }
        case NODE_ASSIGN: analyze_liveness(node->data.assign.value); liveness_record_use(node->data.assign.id, current_instr); break;
        case NODE_INDEX_SET: analyze_liveness(node->data.index_set.obj); analyze_liveness(node->data.index_set.index); analyze_liveness(node->data.index_set.value); break;
        case NODE_INDEX_GET: analyze_liveness(node->data.index_get.obj); analyze_liveness(node->data.index_get.index); break;
        case NODE_SET: analyze_liveness(node->data.set.obj); analyze_liveness(node->data.set.value); break;
        case NODE_GET: analyze_liveness(node->data.get.obj); break;
        default: break;
    }
}

int compare_intervals(const void* a, const void* b) {
    return ((LiveInterval*)a)->start - ((LiveInterval*)b)->start;
}

void allocate_registers() {
    qsort(global_intervals.intervals, global_intervals.count, sizeof(LiveInterval), compare_intervals);
    int free_regs[REG_COUNT];
    for(int i=0; i<REG_COUNT; i++) free_regs[i] = 1; 
    
    LiveInterval* active[REG_COUNT]; 
    int active_count = 0;
    max_stack_usage = 0; 

    for (int i = 0; i < global_intervals.count; i++) {
        LiveInterval* current = &global_intervals.intervals[i];
        for (int j = 0; j < active_count; j++) {
            if (active[j]->end < current->start) {
                if(active[j]->reg_index!= -1) free_regs[active[j]->reg_index] = 1;
                active[j] = active[--active_count];
                j--; 
            }
        }
        if (active_count < REG_COUNT) {
            for(int r=0; r<REG_COUNT; r++) {
                if (free_regs[r]) {
                    current->reg_index = r;
                    free_regs[r] = 0;
                    active[active_count++] = current;
                    break;
                }
            }
        } else {
            int slot = -8 * (i + 1);
            if ((-slot) > max_stack_usage) max_stack_usage = -slot;
            current->reg_index = -1;
            current->stack_offset = slot;
        }
    }
    if (max_stack_usage % 16!= 0) max_stack_usage += (16 - (max_stack_usage % 16));
    if (max_stack_usage < 32) max_stack_usage = 32; 
}

const char* get_location(int vid) {
    // FIX 3.1: Rotating Buffer Pool - prevents static buffer collision
    static char buffers[4][64]; // Pool of 4 buffers
    static int rotate_idx = 0;
    
    // Select next buffer in rotation
    char* buf = buffers[rotate_idx++ % 4];
    
    for(int i=0; i<global_intervals.count; i++) {
        if (global_intervals.intervals[i].var_id == vid) {
            if (global_intervals.intervals[i].reg_index!= -1) return REG_NAMES[global_intervals.intervals[i].reg_index];
            else { snprintf(buf, 64, "[rbp%d]", global_intervals.intervals[i].stack_offset); return buf; }
        }
    }
    return "rax"; 
}

void gen_safepoint_poll(int lbl) {
    emit("cmp dword [rel gc_suspend_request], 0");
    emit("je.Lsafe_%d", lbl);
    emit("call gc_enter_safepoint"); 
    fprintf(asm_out, ".Lsafe_%d:\n", lbl);
}

void gen_string_literal(const char* str) {
    int lbl_end = label_seq++;
    int lbl_str = label_seq++;
    emit("jmp.Lstr_end_%d", lbl_end);
    fprintf(asm_out, ".Lstr_data_%d: db ", lbl_str);
    const char* p = str;
    int first = 1;
    while(*p) {
        if (!first) fprintf(asm_out, ",");
        fprintf(asm_out, "%d", (unsigned char)*p);
        first = 0; p++;
    }
    if (!first) fprintf(asm_out, ",");
    fprintf(asm_out, "0\n"); 
    fprintf(asm_out, ".Lstr_end_%d:\n", lbl_end);
    emit("mov rax,.Lstr_data_%d", lbl_str);
}

void gen_expression(AstNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_LITERAL: 
            emit("mov rdi, %lld", node->data.int_val);
            emit("call dyn_new_int");
            break;
        case NODE_FLOAT: {
            union { double d; int64_t i; } u; u.d = node->data.double_val;
            emit("mov rdi, %lld", u.i);
            emit("call dyn_new_float");
            break;
        }
        case NODE_BOOL: 
            emit("mov rdi, %d", node->data.int_val);
            emit("call dyn_new_bool");
            break;
        case NODE_NULL: emit("call dyn_new_null"); break;
        case NODE_STRING: 
            gen_string_literal(node->data.string_val);
            emit("mov rdi, rax");
            emit("call dyn_new_str");
            break;
        case NODE_VAR_ACCESS: {
            int vid = node->data.var_access.id;
            if (vid == -2) emit("mov rax, [rel %s]", node->data.var_access.name);
            else if (vid == -1) emit("mov rax, %s", node->data.var_access.name);
            else emit("mov rax, %s", get_location(vid));
            break;
        }
        case NODE_ASSIGN: {
             gen_expression(node->data.assign.value); 
             int vid = node->data.assign.id;
             if (vid == -2) emit("mov [rel %s], rax", node->data.assign.name);
             else emit("mov %s, rax", get_location(vid));
             break;
        }
        case NODE_BINARY_OP: {
            // 1. Handle Unary Operations (Left child is NULL)
            if (node->data.binary.left == NULL) {
                // Generate code for the operand (result in RAX)
                gen_expression(node->data.binary.right);
                
                // Move result to first argument register (RDI)
                emit("mov rdi, rax");
                
                switch(node->data.binary.op) {
                    case TOKEN_MINUS: emit("call dyn_neg"); break;
                    case TOKEN_BANG:  emit("call dyn_not"); break;
                    default: 
                        fprintf(stderr, "Codegen Error: Unknown unary operator token %d\n", node->data.binary.op);
                        exit(1);
                }
            } 
            // 2. Handle Binary Operations
            else {
                gen_expression(node->data.binary.left);
                emit("push rax"); // Save left operand
                
                gen_expression(node->data.binary.right);
                emit("mov rsi, rax"); // Right operand to RSI
                emit("pop rdi");      // Left operand to RDI
                
                switch(node->data.binary.op) {
                    case TOKEN_PLUS: emit("call dyn_add"); break;
                    case TOKEN_MINUS: emit("call dyn_sub"); break;
                    case TOKEN_STAR: emit("call dyn_mul"); break;
                    case TOKEN_SLASH: emit("call dyn_div"); break;
                    case TOKEN_PERCENT: emit("call dyn_mod"); break;
                    case TOKEN_EQEQ: emit("call dyn_eq"); break;
                    case TOKEN_NEQ: emit("call dyn_neq"); break;
                    case TOKEN_LT: emit("call dyn_lt"); break;
                    case TOKEN_GT: emit("call dyn_gt"); break;
                    default: break;
                }
            }
            break;
        }
        case NODE_CALL: {
            AstNode* arg = node->data.call.args;
            AstNode* arg_list[64]; 
            int arg_count = 0;
            while(arg && arg_count < 64) { arg_list[arg_count++] = arg; arg = arg->next; }
            
            int implicit_this = (node->data.call.callee->type == NODE_GET);
            int total_args = arg_count + implicit_this;
            int stack_args = (total_args > 6)? total_args - 6 : 0;
            if (stack_args % 2!= 0) emit("sub rsp, 8"); 

            for (int i = arg_count - 1; i >= 0; i--) {
                gen_expression(arg_list[i]);
                emit("push rax");
            }

            if (implicit_this) {
                gen_expression(node->data.call.callee->data.get.obj); 
                emit("push rax"); 
                gen_string_literal(node->data.call.callee->data.get.name);
                emit("mov rsi, rax"); 
                emit("mov rdi, [rsp]"); 
                emit("call aria_obj_get");
                emit("mov r10, rax"); 
            } else {
                if (node->data.call.callee->type == NODE_VAR_ACCESS && node->data.call.callee->data.var_access.id == -1) {
                } else {
                     gen_expression(node->data.call.callee);
                     emit("mov r10, rax");
                }
            }

            int reg_idx = 0;
            if (implicit_this) {
                emit("pop rdi"); 
                reg_idx++;
            }
            for (int i = 0; i < arg_count; i++) {
                if (reg_idx < 6) emit("pop %s", ABI_ARG_REGS[reg_idx++]);
            }

            if (implicit_this) emit("call r10");
            else {
                if (node->data.call.callee->type == NODE_VAR_ACCESS && node->data.call.callee->data.var_access.id == -1) {
                     emit("call %s", node->data.call.callee->data.var_access.name);
                } else {
                     emit("call r10");
                }
            }
            
            if (stack_args > 0) {
                int total_cleanup = stack_args * 8;
                if (stack_args % 2!= 0) total_cleanup += 8;
                emit("add rsp, %d", total_cleanup);
            }
            break;
        }
        case NODE_ARRAY_LITERAL: {
            emit("call list_new"); 
            AstNode* elem = node->data.array_literal.elements;
            while(elem) {
                emit("push rax"); gen_expression(elem); 
                emit("mov rsi, rax"); emit("pop rdi"); emit("push rdi");     
                emit("call list_push"); emit("pop rax"); elem = elem->next;
            }
            break;
        }
        case NODE_INDEX_GET: {
            gen_expression(node->data.index_get.obj); emit("push rax"); 
            gen_expression(node->data.index_get.index);
            emit("mov rsi, rax"); emit("pop rdi"); emit("call list_get");
            break;
        }
        case NODE_INDEX_SET: {
            gen_expression(node->data.index_set.obj); emit("push rax"); 
            gen_expression(node->data.index_set.index); emit("push rax"); 
            gen_expression(node->data.index_set.value);
            emit("mov rdx, rax"); emit("pop rsi"); emit("pop rdi");      
            emit("call list_set"); 
            // Result is in RAX now
            break;
        }
        case NODE_NEW: {
            emit("call aria_alloc_object"); emit("push rax");
            AstNode* cls = program_root;
            while(cls) {
                if (cls->type == NODE_CLASS_DECL && strcmp(cls->data.class_decl.name, node->data.string_val) == 0) {
                    AstNode* method = cls->data.class_decl.methods;
                    while(method) {
                         gen_string_literal(method->data.func_decl.name); 
                         emit("mov rsi, rax"); 
                         char mangled[256];
                         snprintf(mangled, 256, "%s_%s", cls->data.class_decl.name, method->data.func_decl.name);
                         emit("mov rdx, %s", mangled);
                         emit("mov rdi, [rsp]"); 
                         emit("call aria_obj_set");
                         method = method->next;
                    }
                    break;
                }
                cls = cls->next;
            }
            emit("pop rax");
            break;
        }
        case NODE_GET: {
            gen_expression(node->data.get.obj); emit("push rax"); 
            gen_string_literal(node->data.get.name); 
            emit("mov rsi, rax"); emit("pop rdi"); emit("call aria_obj_get");
            break;
        }
        case NODE_SET: {
            gen_expression(node->data.set.obj); emit("push rax"); 
            gen_expression(node->data.set.value); emit("push rax"); 
            gen_string_literal(node->data.set.name);
            emit("mov rsi, rax"); emit("pop rdx"); emit("pop rdi");      
            emit("call aria_obj_set"); 
            break;
        }
        case NODE_TERNARY: {
            int f = label_seq++, e = label_seq++;
            gen_expression(node->data.ternary.condition);
            emit("mov rdi, rax"); emit("call dyn_truthy"); emit("test rax, rax");
            emit("jz.Ltern_%d", f); gen_expression(node->data.ternary.true_expr); emit("jmp.Ltern_end_%d", e);
            fprintf(asm_out, ".Ltern_%d:\n", f); gen_expression(node->data.ternary.false_expr);
            fprintf(asm_out, ".Ltern_end_%d:\n", e);
            break;
        }
        default: break;
    }
}

void gen_statement(AstNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_VAR_DECL:
            if (node->data.var_decl.init_expr) {
                gen_expression(node->data.var_decl.init_expr);
                emit("mov %s, rax", get_location(node->data.var_decl.shadow_stack_offset));
            }
            break;
        case NODE_WHILE: {
            int start = label_seq++, end = label_seq++;
            fprintf(asm_out, ".Lloop_%d:\n", start);
            gen_safepoint_poll(label_seq++);
            gen_expression(node->data.while_stmt.condition);
            emit("mov rdi, rax"); emit("call dyn_truthy"); emit("test rax, rax");
            emit("jz.Lend_%d", end);
            gen_statement(node->data.while_stmt.body); emit("jmp.Lloop_%d", start);
            fprintf(asm_out, ".Lend_%d:\n", end);
            break;
        }
        case NODE_IF: {
            int el = label_seq++, en = label_seq++;
            gen_expression(node->data.if_stmt.condition);
            emit("mov rdi, rax"); emit("call dyn_truthy"); emit("test rax, rax");
            emit("jz.Lelse_%d", el);
            gen_statement(node->data.if_stmt.then_branch); emit("jmp.Lend_%d", en);
            fprintf(asm_out, ".Lelse_%d:\n", el);
            if (node->data.if_stmt.else_branch) gen_statement(node->data.if_stmt.else_branch);
            fprintf(asm_out, ".Lend_%d:\n", en);
            break;
        }
        case NODE_BLOCK: { AstNode* s = node->data.func_decl.body; while(s) { gen_statement(s); s = s->next; } break; }
        case NODE_RETURN: if (node->data.return_stmt.expr) gen_expression(node->data.return_stmt.expr); emit("leave"); emit("ret"); break;
        case NODE_CALL: case NODE_ASSIGN: case NODE_INDEX_SET: case NODE_SET: gen_expression(node); break;
        default: break;
    }
}

void gen_function_node(AstNode* curr) {
    global_intervals.count = 0; instruction_counter = 0;
    AstNode* p = curr->data.func_decl.params;
    while(p) { liveness_record_use(p->data.var_decl.shadow_stack_offset, 0); p = p->next; }
    analyze_liveness(curr->data.func_decl.body);
    allocate_registers();
    fprintf(asm_out, "%s:\n", curr->data.func_decl.name);
    emit("push rbp"); emit("mov rbp, rsp");
    gen_safepoint_poll(label_seq++); emit("sub rsp, %d", max_stack_usage); 
    p = curr->data.func_decl.params; int param_idx = 0;
    while(p && param_idx < 6) {
        int vid = p->data.var_decl.shadow_stack_offset;
        const char* dst = get_location(vid); const char* src = ABI_ARG_REGS[param_idx];
        if (strcmp(dst, src)!= 0) emit("mov %s, %s", dst, src);
        p = p->next; param_idx++;
    }
    gen_statement(curr->data.func_decl.body);
    emit("leave"); emit("ret");
}

void gen_program(AstNode* head) {
    program_root = head;
    global_intervals.capacity = 128; global_intervals.count = 0;
    global_intervals.intervals = malloc(sizeof(LiveInterval) * 128);
    fprintf(asm_out, "global main\n");
    fprintf(asm_out, "extern gc_suspend_request, gc_enter_safepoint, aria_runtime_init, aria_register_global_root\n");
    fprintf(asm_out, "extern print, println, aria_alloc, exit\n");
    fprintf(asm_out, "extern list_new, list_push, list_get, list_set\n");
    fprintf(asm_out, "extern aria_alloc_object, aria_obj_get, aria_obj_set\n");
    fprintf(asm_out, "extern dyn_new_int, dyn_new_float, dyn_new_str, dyn_new_bool, dyn_new_null\n");
    fprintf(asm_out, "extern dyn_add, dyn_sub, dyn_mul, dyn_div, dyn_mod\n"); // Added dyn_mod
    fprintf(asm_out, "extern dyn_truthy, dyn_eq, dyn_neq, dyn_lt, dyn_gt, dyn_neg, dyn_not\n"); // Added dyn_neg, dyn_not
    
    fprintf(asm_out, "section .data\n");
    AstNode* curr = head;
    while(curr) { if (curr->type == NODE_VAR_DECL) fprintf(asm_out, "%s: dq 0\n", curr->data.var_decl.name); curr = curr->next; }

    fprintf(asm_out, "section .text\n");
    fprintf(asm_out, "main:\n"); emit("push rbp"); emit("mov rbp, rsp"); emit("sub rsp, 32"); 
    curr = head; while(curr) { if (curr->type == NODE_VAR_DECL) { emit("lea rdi, [rel %s]", curr->data.var_decl.name); emit("call aria_register_global_root"); } curr = curr->next; }
    curr = head; while(curr) { if (curr->type == NODE_VAR_DECL && curr->data.var_decl.init_expr) { gen_expression(curr->data.var_decl.init_expr); emit("mov [rel %s], rax", curr->data.var_decl.name); } curr = curr->next; }
    
    // Find and inline the main function
    AstNode* main_func = head;
    while(main_func) { 
        if (main_func->type == NODE_FUNC_DECL && strcmp(main_func->data.func_decl.name, "main") == 0) {
            gen_statement(main_func->data.func_decl.body); 
            break;
        } 
        main_func = main_func->next; 
    }
    
    emit("mov rdi, 0"); emit("call exit");
    
    curr = head; while (curr) { if (curr->type == NODE_FUNC_DECL && strcmp(curr->data.func_decl.name, "main") != 0) gen_function_node(curr); else if (curr->type == NODE_CLASS_DECL) { AstNode* m = curr->data.class_decl.methods; while(m) { gen_function_node(m); m = m->next; } } curr = curr->next; }
    free(global_intervals.intervals);
}
