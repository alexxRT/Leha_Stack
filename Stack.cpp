#include "Stack.hpp"
#include "Memory.hpp"
#include <malloc/malloc.h>
#include <string.h>

void PrintErr (int ErrCode, my_stack* stack, const char* func_name, int line);
int  StackCheckInvariants (my_stack* stack);
void StackDump (my_stack* stack, const char* func, int line, int ErrCode);

#define DBG_MODE

#ifdef DBG_MODE

#define STACK_ASSERT( stack_ptr )                                           \
do {                                                                        \
int ErrCode = 0;                                                            \
if ((ErrCode = StackCheckInvariants (stack_ptr)) != 0)                      \
{                                                                           \
    PrintErr (ErrCode, stack_ptr, __func__, __LINE__);                      \
    StackDump (stack_ptr, __func__, __LINE__, ErrCode);                     \
    return ErrCode;                                                         \
}                                                                           \
}                                                                           \
while (0)

#else
#define STACK_ASSERT( stack_ptr ) (void*)0

#endif


#ifdef DBG_MODE
#define STACK_NO_HASH_ASSERT( stack_ptr )\
do {\
int ErrCode = 0;\
if (((ErrCode = StackCheckInvariants (stack_ptr)) != 0) && ErrCode != HASH_DAMAGED) \
{                                                                                   \
    PrintErr (ErrCode, stack_ptr, __func__, __LINE__);                              \
    StackDump (stack_ptr, __func__, __LINE__, ErrCode);                             \
    return ErrCode;                                                                 \
}                                                                                   \
} while (0)

#else
#define STACK_NO_HASH_ASSERT( stack_ptr ) (void*)0
#endif


#define     SIZE( size ) 2*sizeof(CANARY) + sizeof(stack_data_t)*size
#define NEW_SIZE( num_of_elems ) num_of_elems*sizeof (stack_data_t) + 2*sizeof (CANARY)

#define CHECK_DOWN( stack_ptr ) stack_ptr->size < (stack_ptr->capacity) / (2*RESIZE_PARAM) && \
stack_ptr->capacity / RESIZE_PARAM > stack_ptr->min_capacity

#define   CHECK_UP( stack_ptr ) stack_ptr->size == stack_ptr->capacity

//GLOBAL VARIABLES FOR DEBUG ONLY
static FILE* log_file;
static const long long int CANARY = 2398573459345735352L;


//ERROR LIST FOR DEBUG
enum ERROR_LIST  
{
    SUCCESS = 0,
    STACK_NULL,
    STACK_DEAD,
    DATA_NULL,
    REALLOC_FAILED,
    STACK_OVERFLOW,
    INIT_FAILED,
    DESTROY_FAILED, 
    STACK_UNDERFLOW,
    HASH_DAMAGED,
    RIGHT_CANARY_DAMAGED, 
    LEFT_CANARY_DAMAGED
};

//------------------------------------- FUNCTIONS FOR HASH -------------------------------------------//

long long int Hash (void* data, size_t num_of_bytes)
{
    int hash = 0;

    for (int i = 0; i < num_of_bytes; i ++)
    {
        hash += *((char*)data + i);
    }

    return hash;
}

int StackRehash (my_stack* stack)
{
    STACK_NO_HASH_ASSERT (stack); 

    int data_hash = Hash (stack->data, stack->capacity * sizeof (stack_data_t));
    stack->hash = 0;

    int stack_hash = Hash (stack, sizeof (my_stack));
    stack->hash = data_hash + stack_hash;
    
    STACK_ASSERT (stack);

    return SUCCESS;
}

int StackCompHash (my_stack* stack)
{
    int stack_hash = stack -> hash;

    int func_hash = Hash (stack -> data, sizeof (stack_data_t) * stack -> capacity); 
    stack -> hash = 0;
    func_hash += Hash (stack, sizeof (my_stack));

    stack -> hash = stack_hash;

    return (stack_hash == func_hash) ? SUCCESS : HASH_DAMAGED;
}

//-----------------------------------------------------------------------------------------------------//


void* PlaceDataInCanaries (void* data, size_t num_of_elems, size_t width)
{
    assert (data != NULL);

    *(long long *)data = CANARY;
    *(long long *)((char*)data + sizeof(CANARY) + num_of_elems * width) = CANARY;

    return (char*)data + sizeof(CANARY);
}

//--------------------------------------- STACK RESIZE FUNCTIONS ---------------------------------------//

int ResizeDown (my_stack* stack)
{
    STACK_ASSERT (stack);

    #ifdef DBG_MODE
        char* data_start = (char*)stack->data - sizeof (CANARY);
    #else
        char* data_start = (char*)stack->data;
    #endif

    size_t cur_cap = stack->capacity;
    stack->capacity = !cur_cap + cur_cap / 2;

    data_start = RECALLOC (data_start, NEW_SIZE (stack->capacity), char);

    if (data_start == NULL)
    {
        PrintErr (REALLOC_FAILED, stack, __func__, __LINE__);                      
        StackDump (stack, __func__, __LINE__, REALLOC_FAILED);
        return REALLOC_FAILED;
    }

    #ifdef DBG_MODE
    {
        stack->data = (stack_data_t*)PlaceDataInCanaries (data_start, stack->capacity, sizeof (stack_data_t));
        StackRehash (stack);
    }
    #else
    {
        stack->data = (stack_data_t*)data_start;
    }
    #endif

    STACK_ASSERT (stack);

    return SUCCESS;
}

int ResizeUp (my_stack* stack)
{
    STACK_ASSERT (stack);

    #ifdef DBG_MODE
        char* data_start = (char*)stack->data - sizeof(CANARY);
    #else 
        char* data_start = (char*)stack->data;
    #endif

    size_t cur_cap = stack->capacity;
    stack->capacity = !cur_cap + 2 * cur_cap;

    data_start =  RECALLOC (data_start, NEW_SIZE (stack->capacity), char);

    if (data_start == NULL)
    {
        PrintErr (REALLOC_FAILED, stack, __func__, __LINE__);                      
        StackDump (stack, __func__, __LINE__, REALLOC_FAILED);
        return REALLOC_FAILED;
    }

    #ifdef DBG_MODE
    {
        stack->data = (stack_data_t*)PlaceDataInCanaries (data_start, stack->capacity, sizeof(stack_data_t));
        StackRehash (stack);
    }
    #else
    {
        stack->data = (stack_data_t*)data_start;
    }
    #endif

    STACK_ASSERT (stack);

    return SUCCESS;
}

int StackResize (my_stack* stack) 
{
    STACK_ASSERT (stack);
        
    if (CHECK_UP (stack))
    {
        ResizeUp (stack);
    }

    if (CHECK_DOWN (stack))
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
    if (stack       ==  NULL)            return STACK_NULL    ;
    if (!stack->alive_status)            return STACK_DEAD    ;
    if (stack->data ==  NULL)            return DATA_NULL     ;
    if (stack->size  >  stack->capacity) return STACK_OVERFLOW;

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
    assert (log_file != NULL);

    switch (ErrCode)
    {
    case STACK_NULL:
        fprintf (log_file, "Stack has NULL pointer in func %s ", func_name);
        break;  
    case STACK_DEAD:
        fprintf (log_file, "Your stack was already destroyed in func %s ", func_name);
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
    case REALLOC_FAILED:
        fprintf (log_file, "Stack capacity didn't changed in func %s ", func_name);
        break;
    case DESTROY_FAILED:
        fprintf (log_file, "Your aka alive stack is dead in func %s ", func_name);
        break;
    case INIT_FAILED:
        fprintf (log_file, "Your aka dead stack is alive in func %s ", func_name);
    case HASH_DAMAGED:
        fprintf (log_file, "Hash damage in func %s ", func_name);
        break;                     
    case LEFT_CANARY_DAMAGED:
        fprintf (log_file, "Left konoreyka damaged in func %s ", func_name);
        break;
    case RIGHT_CANARY_DAMAGED:
        fprintf (log_file, "Right konoreyka damaged in func %s ", func_name);
        break;

    default:
        assert(0 && "Unexpected error code returned"); 
        break;
    }

    fprintf (log_file, "and on the line %d\n", line);
    fclose (log_file);
    log_file = NULL;

}

// --------------------------------------------------------------------------------------------------------//






// ------------------------------------------ DEBUG FUNCTIONS ---------------------------------------------//

void PrintHexBytes (FILE* file, void* elem, size_t size)
{ 
    assert (file != NULL);
    assert (elem != NULL);

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
    assert (file != NULL);

    printf ("i am here from function %s\n", func);
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

int StackPrint (FILE* file, my_stack* stack, size_t nel)
{
    for (int i = 0; i < nel; i ++)
    {
        PrintHexBytes (file, stack->data + i, sizeof(stack_data_t));
    }
    return SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------//







//-------------------------------------------------- INITIAL STACK FUNCTIONS --------------------------------------//

int StackInit (my_stack* stack, size_t size, size_t min_capacity)
{
    if (stack == NULL) return STACK_NULL;

    stack->capacity = size;
    stack->size = 0;
    stack->hash = 0;
    stack->alive_status = 1;

    #ifdef DBG_MODE
    {
        char* data = CALLOC (SIZE (size), char);
        stack->data = (stack_data_t*)PlaceDataInCanaries (data, size, sizeof(stack_data_t));
    }
    #else
    {
        stack->data = CALLOC (size, stack_data_t);
    }
    #endif

    stack->min_capacity = min_capacity;
    #ifdef DBG_MODE
        StackRehash (stack);
    #endif

    STACK_ASSERT (stack);

    #ifdef DBG_MODE
        if (!stack->alive_status)
        {   
            PrintErr (INIT_FAILED, stack, __func__, __LINE__);                      
            StackDump (stack, __func__, __LINE__, INIT_FAILED);
            return INIT_FAILED;
        }
    #endif

    return SUCCESS;
}

int StackPop (my_stack* stack, stack_data_t* p_elem)
{
    STACK_ASSERT (stack);

    if (stack->size == 0)
    {
        PrintErr (STACK_UNDERFLOW, stack, __func__, __LINE__);                      
        StackDump (stack, __func__, __LINE__, STACK_UNDERFLOW);

        return STACK_UNDERFLOW;
    }

    stack->size --;
    *p_elem = stack->data [stack->size];

    #ifdef DBG_MODE
        StackRehash (stack);
    #endif

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

    #ifdef DBG_MODE
        StackRehash (stack);
    #endif

    STACK_ASSERT (stack);

    return SUCCESS;
}

int StackDestroy (my_stack* stack)
{
    STACK_ASSERT (stack);

    #ifdef DBG_MODE
        void* ptr_to_free = (char*)stack->data - sizeof (CANARY);
    #else
        void* ptr_to_free = stack->data;
    #endif

    FREE (ptr_to_free);

    stack->alive_status = 0;
    stack->capacity     = 0;
    stack->min_capacity = 0;
    stack->hash         = 0;

    #ifdef DBG_MODE
        if (stack->alive_status)
        {
            PrintErr  (DESTROY_FAILED, stack, __func__, __LINE__);                      
            StackDump (stack, __func__, __LINE__, DESTROY_FAILED); 
            return DESTROY_FAILED;
        }
    #endif

    return SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------//
