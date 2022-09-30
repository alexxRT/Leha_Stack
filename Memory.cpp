#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <strings.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <malloc/malloc.h>
#include "Memory.hpp"

//static const long long int CANARY = 7384326878364923474L;
//static const size_t OFFSET = sizeof (CANARY) + sizeof (mem_info);


void* Recalloc (void* ptr, size_t new_size)
{
    if (ptr == NULL) return NULL;
    
    size_t old_size = malloc_size (ptr);
    ptr = realloc (ptr, new_size);

    if (ptr == NULL) return NULL;

    if (new_size > old_size) 
    bzero ((char*)ptr + old_size, new_size - old_size);

    return ptr;
}

//-------------------------------------!!!UNDER DEVRLOPEMENT!!!---------------------------------------------------//

// void PrintMemoryError (int error)
// {
//     if (error == SUCCESS)        printf ("success body\n");
//     if (error == CALLOC_ERROR)   printf ("Calloc didn't allocated memory!!!\n");
//     if (error == REALLOC_ERROR)  printf ("Data wasn't realloced\n");
//     if (error == INVALID_POINTR) printf ("Data you are working on was not allocated\n");
    
//     return;
// }

// void* DataInCanaries (void* data, size_t num_of_elems, size_t width)
// {
//     assert (data != NULL);

//     *(long long*)data = CANARY;
//     *(long long*)((char*)data + OFFSET + num_of_elems*width) = CANARY;

//     return data;
// }

// enum MEMORY_ERROR_LIST IsCalloced (void* ptr)
// {
//     mem_info* info = NULL;
//     info = GET_INFO (ptr);

//     int result = info->is_calloced;

//     return (result == 1) ? SUCCESS : INVALID_POINTR;
// }


// void* ProCalloc (size_t count, size_t size)
// {
//     //memory_t* data = CALLOC (1, memory_t);
    
//     // if (data == NULL)
//     // {
//     //     return NULL;
//     // }

//     void* ptr = calloc (count * size + OFFSET + sizeof (CANARY), sizeof(char));

//     if (ptr == NULL) 
//     {
//         return NULL;
//     }

//     ptr = DataInCanaries (ptr, count, size);

//     mem_info info = {};
//     info.calloced_size = count;
//     info.is_calloced = 1;
//     info.error = 0;

//     *(mem_info*)((char*)ptr + sizeof(CANARY)) = info;
//     //data->ptr = (char*)ptr + OFFSET;

//     return (char*)ptr + OFFSET;
// }

// void* ProRealloc (void* ptr, size_t new_count, size_t size)
// {
//     void* valid_ptr = (char*)ptr - OFFSET;
//     mem_info* info = GET_INFO (valid_ptr);
    

//     if (IsCalloced (valid_ptr))
//     {
//         info->error = INVALID_POINTR;
//         return ptr;
//     }
    
//     size_t old_bytes = info->calloced_size;

//     valid_ptr = (char*)realloc (valid_ptr, new_count*size + OFFSET + sizeof (CANARY));

//     if (valid_ptr == NULL)
//     {
//         info->error = REALLOC_ERROR;
//         return ptr;
//     }    

//     size_t delta_bytes = new_count*size - old_bytes + 1;
//     memset ((char*)valid_ptr + OFFSET + old_bytes, 0, delta_bytes);

//     DataInCanaries (valid_ptr, new_count, size);

//     ptr = (char*)valid_ptr + OFFSET;
//     return ptr;
// }

// void* ProFree (void* ptr)
// {
//     void* valid_ptr = (char*)ptr - OFFSET;
//     mem_info* info = GET_INFO (valid_ptr);

//     if (IsCalloced (valid_ptr))
//     {
//         info->error = INVALID_POINTR;
//         return ptr;
//     }

    
//     FREE (valid_ptr);

//     return NULL;
// }

