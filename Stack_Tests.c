#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Stack.h"



int main ()
{
    my_stack stack = {};

    StackInit (&stack, 10, 10);

    for (int i = 0; i < 100; i++)
    {
        int err = StackPush (&stack, &i);
        StackDump (&stack, __func__, __LINE__, err);
    }

    for (int i = 0; i < 100; i ++)
    {
        int a = 0;
        int err = StackPop(&stack, &a);
        StackDump (&stack, __func__, __LINE__, err);
    }

    StackDestroy (&stack);

    return 0;
}


