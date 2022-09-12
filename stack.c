#include "Stack.h"

FILE* log_file;

void PrintErr (int ErrCode, my_stack* stack, const char* func_name, int line);
int  StackCheckInvariants (my_stack* stack);

#ifdef DBG_MODE
#define STACK_ASSERT( stack_ptr )\
do {\
int ErrCode = 0;\
if ((ErrCode = StackCheckInvariants (stack_ptr)) != 0)\
{\
    PrintErr (ErrCode, stack_ptr, __func__, __LINE__);\
    return ErrCode;\
}\
} while (0)\

#else
#define STACK_ASSERT( stack_ptr )
#endif


#ifdef DBG_MODE
#define LIGHT_ASSERT( stack_ptr )\
do {\
int ErrCode = 0;\
if (((ErrCode = StackCheckInvariants (stack_ptr)) != 0) && ErrCode != HASH_DAMAGED)\
{\
    PrintErr (ErrCode, stack_ptr, __func__, __LINE__);\
    return ErrCode;\
}\
} while (0)\

#else
#define LIGHT_ASSERT( stack_ptr )
#endif

//------------------------------------- FUNCTIONS FOR HASH -------------------------------------------//

long long int Hash (void* data, size_t num_of_bytes)
{ // TODO: good hash alternative would be murmur3
    int hash = 0;

    for (int i = 0; i < num_of_bytes; i ++)
    {
        hash += *(char*)(data + i);
    }

    return hash;
}

int StackRehash (my_stack* stack)
{
    LIGHT_ASSERT (stack); 

    int d_hash = Hash (stack -> data, stack -> capacity * sizeof (stack_data_t));
    stack -> hash = 0;

    int s_hash = Hash (stack, sizeof (my_stack));

    stack -> hash = d_hash + s_hash;

    STACK_ASSERT (stack);

    return SUCCESS;
}

int CompHash (my_stack* stack)
{
    int stack_hash = stack -> hash;

    int func_hash = Hash (stack -> data, sizeof (stack_data_t) * stack -> capacity); 
    stack -> hash = 0;
    func_hash += Hash (stack, sizeof (my_stack));

    stack -> hash = stack_hash;

    return (stack_hash == func_hash) ? 1 : 0;    
}

int StackCompHash (my_stack* stack)
{
    if (!CompHash (stack)) return HASH_DAMAGED;

    return SUCCESS;
} 

//-----------------------------------------------------------------------------------------------------//

void* StackDataInCanaries (void* data, size_t num_of_elems, size_t width) 
{
    assert (data != NULL);

    *(long long int*)(data) = CANARY;
    *(long long int*)(data + sizeof(CANARY) + num_of_elems * width) = CANARY;

    return data + sizeof(CANARY);
}

//--------------------------------------- STACK RESIZE FUNCTIONS ---------------------------------------//

int ResizeDown (my_stack* stack)
{
    STACK_ASSERT (stack);
    
    char* data = (char*)stack -> data - sizeof(CANARY);
    
    size_t cur_cap = stack->capacity;
    stack->capacity = !cur_cap + cur_cap / 2;

    data = realloc (data, stack->capacity + 2 * sizeof(CANARY));
    data = StackDataInCanaries (data, stack->capacity, sizeof(stack_data_t));

    stack -> data = (stack_data_t*)data;

    StackRehash (stack);
    STACK_ASSERT (stack);

    return SUCCESS;
}

int ResizeUp (my_stack* stack)
{
    STACK_ASSERT (stack);

    char* data = (char*)stack->data - sizeof(CANARY);
    
    size_t cur_cap = stack->capacity;
    stack->capacity = !cur_cap + 2 * cur_cap;

    data = realloc (data, stack->capacity + 2 * sizeof(CANARY));
    data = StackDataInCanaries (data, stack -> capacity, sizeof(stack_data_t));

    stack -> data = (stack_data_t*)(data);

    StackRehash (stack);
    STACK_ASSERT (stack);

    return SUCCESS;
}

int StackResize (my_stack* stack) 
{
    STACK_ASSERT (stack);
        
    if (stack -> size == stack -> capacity)
    {
        ResizeUp (stack);
    }

    if (stack->size < (stack -> capacity) / 4 && stack->capacity/2 > stack -> min_capacity)
    {
        ResizeDown (stack);
    }

    STACK_ASSERT (stack);

    return SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------//







//---------------------------------------- DEALING WITH ERRORS FUNCTIONS ------------------------------------//

int StackCheckCanary (my_stack* stack)
{
    if (*((long long int*)stack->data - 1) != CANARY) return LEFT_CANARY_DAMAGED;
    if (*(long long int*)(stack->data + stack->capacity) != CANARY) return RIGHT_CANARY_DAMAGED;

    return SUCCESS;
}

int StackBaseInvariants (my_stack* stack)
{
    if (stack         == NULL) return STACK_NULL;
    if (stack->data == NULL) return DATA_NULL; 
    if (stack->size > stack->capacity) return STACK_OVERFLOW;

    return SUCCESS;
}

int StackCheckInvariants (my_stack* stack)
{
    int ErrCode = SUCCESS;

    ErrCode = StackBaseInvariants (stack);
    if (ErrCode) return ErrCode;

    ErrCode = StackCheckCanary (stack);
    if (ErrCode) return ErrCode;

    ErrCode = StackCompHash (stack);
    if (ErrCode) return ErrCode;

    return SUCCESS;  
}

void PrintErr (int ErrCode, my_stack* stack, const char* func_name, int line)
{
    log_file = fopen ("LOG_FILE.txt", "a");

    switch (ErrCode)
    {
    case STACK_NULL:
        fprintf (log_file, "Stack has NULL pointer in func %s ", func_name);
        break;                      
    case DATA_NULL:
        fprintf (log_file, "Data has NULL pointer in func %s ", func_name);
        break;                      
    case STACK_OVERFLOW:
        fprintf (log_file, "Stack overflowed in func %s ", func_name);
        break;                      
    case STACK_UNDERFLOW:
        fprintf (log_file, "Stack underflowed in func %s ", func_name);
        break;                      
    case HASH_DAMAGED:
        fprintf (log_file, "Hash damage in func %s ", func_name);
        break;                     
    case LEFT_CANARY_DAMAGED:
        fprintf (log_file, "Left konoreyka damaged in func %s ", func_name);
        break;
    case RIGHT_CANARY_DAMAGED:
        fprintf (log_file, "Right konoreyka damaged in func %s ", func_name);
        break;

        fprintf (log_file, "and on the line %d\n", line);

    default:
        assert(0 && "Unexpected error code returned"); 
        break;
    }

    fclose (log_file);
    log_file = NULL;

}

// --------------------------------------------------------------------------------------------------------//






// ------------------------------------------ DEBUG FUNCTIONS ---------------------------------------------//

void PrintHexBytes (FILE* file, void* elem, size_t size)
{ 
    for (int i = 0; i < size; i++)
    {
        fprintf (file, "%x ", *((unsigned char*)elem + i));
    }

    fprintf (file, "\n");

    return;
}

 void StackDump (my_stack* stack, const char* func, int line, int ErrCode)
{
    FILE* file = fopen ("DUMP.txt", "a");

    fprintf (file, "--------------STACK DUMP occurred----------------\n");
    fprintf (file, "It was called in func %s, and on the line %d\n", func, line);

    if (ErrCode == RIGHT_CANARY_DAMAGED || ErrCode == LEFT_CANARY_DAMAGED || ErrCode == HASH_DAMAGED)
    {
        fprintf (file, "Your data might be damaged, check the log file\n");
    }

    fprintf (file, "The stack address: [%p]\n", stack);

    if (ErrCode == STACK_NULL)
    {
        fprintf (file, "Dump cannot be processed further becouse stack has NULL pointer\n");
        fprintf (file, "------------- DUMP finished ---------------\n\n");

        return;
    }

    fprintf (file, "The data begining address: [%p]\n", stack->data);

    if (ErrCode == DATA_NULL)
    {
        fprintf (file, "Data cannot be revealed becouse data has NULL pointer\n");
        fprintf (file, "------------- DUMP finished ---------------\n\n");

        return;
    }

    if (ErrCode == STACK_OVERFLOW)
    {
        fprintf (file, "!!!OVERFLOW!!!\n");
        StackPrint (file, stack, stack->capacity);
        fprintf (file, "------------- DUMP finished ---------------\n\n");

        return;
    }
    fprintf (file, "Stack capacity now is [%lu]\n", stack->capacity);
    fprintf (file, "Number of elems in stack now is [%lu]\n", stack->size);

    StackPrint (file, stack, stack->size);
    fprintf (file, "------------- DUMP finished ---------------\n\n");

    fclose (file);
    file = NULL;

    return;
   }

void StackPrint (FILE* file, my_stack* stack, size_t nel)
{
    assert (stack != NULL);
    assert (stack->data != NULL);

    for (int i = 0; i < nel; i ++)
    {
        PrintHexBytes (file, stack->data + i, sizeof(stack_data_t));
   }
}

//-----------------------------------------------------------------------------------------------------------------//







//-------------------------------------------------- INITIAL STACK FUNCTIONS --------------------------------------//

int StackInit (my_stack* stack, size_t size, size_t min_capacity)
{
    assert (stack != NULL);

    stack->capacity = size;
    stack->size = 0;
    stack->hash = 0;

    stack_data_t* data = (stack_data_t*)calloc (2 * sizeof(CANARY) + sizeof(stack_data_t)*size, sizeof(char));

    stack->data = StackDataInCanaries (data, size, sizeof(stack_data_t));

    stack->min_capacity = min_capacity; 
   
    StackRehash (stack);

    STACK_ASSERT (stack);

    return SUCCESS;
}

int StackPop (my_stack* stack, stack_data_t* p_elem)
{
    STACK_ASSERT (stack);

    if (stack->size == 0)
    {
        return STACK_UNDERFLOW;
    }

    stack->size --;
    *p_elem = stack->data [stack->size];

    StackRehash (stack);
    StackResize (stack);

    STACK_ASSERT (stack);

    return SUCCESS;
}

int StackPush (my_stack* stack, stack_data_t* elem)
{
    STACK_ASSERT (stack);

    StackResize (stack);
    
    stack->data [stack->size] = *elem;
    stack->size ++;

    StackRehash (stack);

    STACK_ASSERT (stack);

    return SUCCESS;
}

int StackDestroy (my_stack* stack)
{
    STACK_ASSERT (stack);

    free ((char*)stack->data - sizeof (CANARY));
    stack->capacity     = 0;
    stack->min_capacity = 0;
    stack->hash         = 0;

    STACK_ASSERT (stack);

    return SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------//









