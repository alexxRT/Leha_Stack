#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define KONOREYKA 2398573459345735352

enum ERROR_LIST
{
    fignya,
    STACK_NULL,
    DATA_NULL,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    HASH_DAMAGED,
    RK_DAMAGED,
    LK_DAMAGED
};

typedef struct _stack
{
    size_t capacity;
    size_t cur_indx;
    void* data;
    int hash;
    size_t base_cap;

} my_stack;

typedef char stack_data_t;

void* DataSafe (void* data, size_t nel, size_t width) 
{
    assert (data != NULL);

    *(long long int*)(data) = KONOREYKA;
    *(long long int*)(data + 8 + nel * width) = KONOREYKA;

    return data + 8;
}

long long int HashFunc (void* data, size_t num_of_bytes)
{
    int hash = 0;

    for (int i = 0; i < num_of_bytes; i ++)
    {
        hash += *(char*)(data + i);
    }

    return hash;
}

int IsOkey (my_stack* stack)
{
    if (stack         == NULL) return STACK_NULL;
    if (stack -> data == NULL) return DATA_NULL;
    if (stack -> cur_indx >= stack -> capacity) return STACK_OVERFLOW;

    return 0;        
} 

void ErrPrint (int ErrCode)
{
    switch (ErrCode)
    {
    case 1:
        printf ("UUUPS... stack has NULL pointer");
        break;
    case 2:
        printf ("UUUPS... data has NULL pointer");
        break;
    case 3:
        printf ("UUUPS... stack overflowed");
        break;
    case 4:
        printf ("UUUPS... stack underflowed");
        break;

    default:

        break;
    }
}

#if defined DBG_MODE

#define STACK_ASSERT( stack_ptr ) \
if ((int ErrCode = IsOkey (stack_ptr)) != 0) ErrPrint (ErrCode); \
return ErrCode; 

#else
#define STACK_ASSERT( stack_ptr ) return 0;
#endif

void HexPrint (void* elem, size_t size)
{
    assert (elem != NULL);

    for (int i = 0; i < size; i ++)
    {
        printf ("%x ", *((unsigned char*)elem + i));
    }
    printf ("\n");
}

void StackPrint (my_stack* stack, size_t nel)
{
    assert (stack != NULL);
    assert (stack -> data != NULL);

    for (int i = 0; i < nel; i ++)
    {
        HexPrint (stack -> data + i, sizeof(stack_data_t));
    }
}

void StackDump (my_stack* stack, char* func, int line, int ErrCode)
{

    printf ("--------------STACK DUMP occurred at ----------------\n");
    printf ("It was called in func %s, and on the line %d\n", func, line);

    if (ErrCode == RK_DAMAGED || ErrCode == LK_DAMAGED || ErrCode == HASH_DAMAGED)
    {
        ErrPrint (ErrCode);
    }

    printf ("The stack address: [%p]\n", stack);

    if (ErrCode == STACK_NULL)
    {
        printf ("Dump cannot be processed further\n");
        ErrPrint (ErrCode);
        printf ("------------- DUMP finished ---------------\n\n");

        return;
    }

    printf ("The data begining address: [%p]\n", stack -> data);

    if (ErrCode == DATA_NULL)
    {
        printf ("Data cannot be revealed\n");
        ErrPrint (ErrCode);
        printf ("------------- DUMP finished ---------------\n\n");

        return;
    }

    if (ErrCode == STACK_OVERFLOW)
    {
        printf ("!!!OVERFLOW!!!\n");
        StackPrint (stack, stack -> capacity);
        printf ("------------- DUMP finished ---------------\n\n");

        return;
    }

    StackPrint (stack, stack -> cur_indx);
    printf ("------------- DUMP finished ---------------\n\n");

    return;
   }

void* ResizeUp (void* data, size_t* cur_size)
{
    assert (data != NULL);
    
    size_t new_size = 16 + 2*(*cur_size);

    data = realloc (data, new_size);
    data = DataSafe (data + 8, new_size - 16, sizeof(stack_data_t));

    *cur_size *= 2;

    return data + 8;
}

void* ResizeDown (void* data, size_t* cur_size)
{
    assert (data != NULL);
    
    size_t new_size = 16 + (*cur_size) / 2;

    data = realloc (data, new_size);
    data = DataSafe (data + 8, new_size - 16, sizeof(stack_data_t));

    *cur_size /= 2;

    return data + 8;
}

my_stack* StackResize (my_stack* stack) 
{
    assert (stack != NULL);

    if (stack -> cur_indx == stack -> capacity)
    {
        stack -> data = ResizeUp (stack -> data - 8, &stack -> capacity);
        return stack;

    }

    if (stack -> cur_indx < (stack -> capacity) / 4 && stack -> cur_indx > stack -> base_cap)
    {
        stack -> data = ResizeDown (stack -> data - 8, &stack -> capacity);
        return stack;
    }

    return stack;
}
