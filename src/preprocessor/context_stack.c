/* Aria_lang/src/preprocessor/context_stack.c */
#include "context_stack.h"

ContextStack* ctx_stack_create(int initial_capacity) {
    ContextStack* stack = (ContextStack*)malloc(sizeof(ContextStack));
    if (!stack) {
        fprintf(stderr, "Fatal: Out of memory allocating context stack.\n");
        exit(1);
    }
    stack->capacity = initial_capacity;
    stack->nodes = (ContextNode*)malloc(sizeof(ContextNode) * initial_capacity);
    if (!stack->nodes) {
        free(stack);
        fprintf(stderr, "Fatal: Out of memory allocating context nodes.\n");
        exit(1);
    }
    stack->top = -1;
    stack->global_counter = 0;
    return stack;
}

void ctx_stack_push(ContextStack* stack, const char* name) {
    // Resize if necessary
    if (stack->top >= stack->capacity - 1) {
        stack->capacity *= 2;
        ContextNode* new_nodes = (ContextNode*)realloc(stack->nodes, sizeof(ContextNode) * stack->capacity);
        if (!new_nodes) {
            fprintf(stderr, "Fatal: Out of memory expanding context stack.\n");
            exit(1);
        }
        stack->nodes = new_nodes;
    }
    
    stack->top++;
    // Safe string copy with explicit null termination
    strncpy(stack->nodes[stack->top].name, name, MAX_CTX_NAME - 1);
    stack->nodes[stack->top].name[MAX_CTX_NAME - 1] = '\0';
    
    // Assign unique ID: This is the core hygiene mechanism
    stack->nodes[stack->top].id = ++stack->global_counter;
}

void ctx_stack_pop(ContextStack* stack) {
    if (stack->top < 0) {
        fprintf(stderr, "Preprocessor Error: Context stack underflow (%%pop without matching %%push)\n");
        exit(1);
    }
    stack->top--;
}

ContextNode* ctx_stack_peek(ContextStack* stack) {
    if (stack->top < 0) return NULL;
    return &stack->nodes[stack->top];
}

void ctx_stack_free(ContextStack* stack) {
    free(stack->nodes);
    free(stack);
}
