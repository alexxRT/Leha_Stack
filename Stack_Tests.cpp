#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Stack.hpp"



int main ()
{
    my_stack stack = {};

    StackInit (&stack, 10, 10);

    // for (int i = 0; i < 11; i++)
    // {
    //     int err = StackPush (NULL, &i);
    // }

    // for (int i = 0; i < 11; i ++)
    // {
          int a = 0;
          int err = StackPop(&stack, &a);
    // }
    

    StackDestroy (&stack);

    return 0;
}


