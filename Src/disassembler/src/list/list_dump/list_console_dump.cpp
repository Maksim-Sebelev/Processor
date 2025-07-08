
#include <assert.h>
#include "list/list.hpp"
#include "lib/lib.hpp"
#include "list/list_dump/list_console_dump.hpp"

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void ListPrint(const List_t* list)
{   
    assert(list);
    assert(list->data);

    COLOR_PRINT(VIOLET, "list:\n");

          size_t data_i = GetHead(list);
    const size_t size   = GetDataSize(list);

    printf(GREEN);

    for (size_t i = 0; i < size; i++)
    {
        printf("%d ", GetDataElemValue(list, data_i));
        data_i = GetNextIndex(list, data_i);
    }

    printf(RESET);

    COLOR_PRINT(VIOLET, "\nlist end.\n");

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void ListConsoleDump(const List_t* list, const char* file, unsigned int line, const char* func)
{
    assert(list);
    assert(file);
    assert(func);
    assert(list->data);

    COLOR_PRINT(GREEN, "\nDUMP BEGIN\n\n");

    COLOR_PRINT(WHITE, "Dump made in:\n");
    PrintPlace(file, line, func);
    printf("\n\n");

    COLOR_PRINT(VIOLET, "capcity = %lu\n"   , GetCapacity(list));
    COLOR_PRINT(VIOLET, "size    = %lu\n"   , GetDataSize(list));
    COLOR_PRINT(VIOLET, "free    = %lu\n\n" , GetFree    (list));
    COLOR_PRINT(YELLOW, "head    = %lu\n"   , GetHead    (list));
    COLOR_PRINT(YELLOW, "tail    = %lu\n\n" , GetTail    (list));

    COLOR_PRINT(RED,  "    data  next  prev\n" );
    COLOR_PRINT(CYAN, "[ 0] %3d   %3lu  %3lu\n", list->data[0].value, list->data[0].next, list->data[0].prev);

    const size_t capacity = GetCapacity(list);

    for (size_t i = 1; i <= capacity; i++)
    {
        COLOR_PRINT(WHITE, "[%2lu] %3d   %3lu  %3lu\n", i, list->data[i].value, list->data[i].next, list->data[i].prev);
    }

    printf("\n\n");

    COLOR_PRINT(GREEN, "\nDUMP END\n\n");

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
