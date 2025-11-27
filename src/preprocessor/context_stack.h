/* Aria_lang/src/preprocessor/context_stack.h */
#ifndef CONTEXT_STACK_H
#define CONTEXT_STACK_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_CTX_NAME 64

typedef struct {
    // FIX: Changed from 'char name' to array to support string storage
    char name[MAX_CTX_NAME];
    unsigned int id; 
} ContextNode;

typedef struct {
    ContextNode* nodes;
    int capacity;
    int top;        
    unsigned int global_counter; 
} ContextStack;

ContextStack* ctx_stack_create(int initial_capacity);
void ctx_stack_push(ContextStack* stack, const char* name);
void ctx_stack_pop(ContextStack* stack);
ContextNode* ctx_stack_peek(ContextStack* stack);
void ctx_stack_free(ContextStack* stack);

#endif
