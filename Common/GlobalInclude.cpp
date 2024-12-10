#include <string.h>
#include <assert.h>
#include "GlobalInclude.h"
#include "ColorPrint.h"

//--------------------------------------------------------------------------------------------------------------------------------------

void PrintPlace(const char* const file, const int line, const char* const func)
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
