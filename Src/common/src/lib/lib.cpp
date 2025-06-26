#include <stdio.h>
#include <assert.h>
#include "lib/lib.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void PrintPlace(const char* file, int line , const char* func)
{
    assert(file);
    assert(func);

    COLOR_PRINT(WHITE, "%s:%d\n", file, line);
    printf("in '");
    COLOR_PRINT(GREEN, "%s", func);
    printf("'\n");
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CodePlaceCtor(CodePlace* place, const char* file, int line, const char* func)
{
    assert(place);
    assert(file);
    assert(func);

    place->file = file;
    place->line = line;
    place->func = func;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

