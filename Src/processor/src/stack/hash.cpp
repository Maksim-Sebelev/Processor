#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "stack/hash.hpp"

uint64_t Hash(void* array, size_t arr_size, size_t array_elem_size)
{
    assert(array);

    char* char_array = (char*) array;
    uint64_t array_hash = 5381;

    size_t byte_array_size =  arr_size * array_elem_size;

    for (size_t i = 0; i < byte_array_size; i++)
        array_hash = (array_hash * 33) ^ (uint64_t) char_array[i];

    return array_hash;
}
