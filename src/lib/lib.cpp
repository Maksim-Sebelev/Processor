#include <stdio.h>
#include <assert.h>
#include "lib/lib.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------

void PrintPlaceStruct(CodePlace* place)
{
    assert(place);
    assert(place->file);
    assert(place->func);

    printf("File [%s]\nLine [%d]\nFunc [%s]\n", place->file, place->line, place->func);
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void PrintPlace(const char* file, const int line , const char* func)
{
    assert(file);
    assert(func);

    printf("File [%s]\nLine [%d]\nFunc [%s]\n", file, line, func);
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CodePlaceCtor(CodePlace* place, const char* const file, const int line, const char* const func)
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
