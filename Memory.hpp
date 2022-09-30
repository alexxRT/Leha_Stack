#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <strings.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <malloc/malloc.h>




static const void* POISNED_PTR = (const void*)0x78;
static const char POISNED_ELEM = 0x77;

void* Recalloc (void* ptr, size_t new_size);

#define CALLOC( count, type )            (type*)calloc(count, sizeof(type))
#define RECALLOC( ptr, new_size, type )  (type*)Recalloc(ptr, new_size)
#define FREE( ptr ) { memset (ptr, POISNED_ELEM, malloc_size (ptr)); free (ptr); }

#endif

//#define GET_INFO( ptr ) (mem_info*)((char*)ptr + sizeof(CANARY));
// enum MEMORY_ERROR_LIST
// {
//     SUCCESS = 0,
//     CALLOC_ERROR,
//     REALLOC_ERROR,
//     INVALID_POINTR,
// };

// typedef struct _mem_info
// {
//     size_t calloced_size;
//     int    is_calloced;
//     int error;

// }mem_info;

// void PrintMemoryError (int error);

// void* ProCalloc (size_t count, size_t size);

// void* ProRealloc (void* data, size_t new_count, size_t size);

// void* ProFree (void* data);

