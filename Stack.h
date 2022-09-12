#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CANARY 2398573459345735352L

enum ERROR_LIST
{
    SUCCESS = 0,
    STACK_NULL,
    DATA_NULL,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    HASH_DAMAGED,
    RIGHT_CANARY_DAMAGED, 
    LEFT_CANARY_DAMAGED
};

typedef int stack_data_t;

typedef struct _stack
{
    size_t capacity;
    size_t size;
    stack_data_t* data;
    int hash;
    size_t min_capacity;

} my_stack;

int  StackInit (my_stack* stack, size_t size, size_t min_capacity);

int  StackPush (my_stack* stack, stack_data_t* elem_ptr);

int  StackPop (my_stack* stack, stack_data_t* elem_ptr);

void StackPrint (FILE* file, my_stack* stack, size_t num_of_elem);

void StackDump (my_stack* stack, const char* func_name, int line, int ErrCode);

int  StackResize (my_stack* stack);

int  StackDestroy (my_stack* stack);

#endif