
#include <stdlib.h>
#include "stack/stack.hpp"


int main()
{

    Stack_t stack = {};
    STACK_ASSERT(StackCtor(&stack, 10));

    StackElem_t a = 4.455;


    
    for (size_t i = 0; i < 10; i++)
    {
        STACK_ASSERT(StackPush(&stack, a*234.23491 + 3.14*i));
    }


    StackElem_t b = 4.455;

    STACK_ASSERT(StackPop(&stack, &b));
    COLOR_PRINT(RED, "b = '%lf'\n", b);

    STACK_ASSERT(PrintStack(&stack));

    return EXIT_SUCCESS;
}
