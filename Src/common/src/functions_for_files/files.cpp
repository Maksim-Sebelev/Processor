#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include "lib/lib.hpp"
#include "functions_for_files/files.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

FILE* SafeFopen(const char* file, const char* modes)
{
    assert(modes);

    if (!file)
        EXIT(EXIT_FAILURE, "trying to open nullptr.");

    FILE* file_ptr = fopen(file, modes);

    if (!file_ptr)
        EXIT(EXIT_FAILURE, "failed open '%s' in mode '%s'\n", file, modes);

    return file_ptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void SafeFclose(FILE* file_ptr)
{
    if (!file_ptr)
        EXIT(EXIT_FAILURE, "trying to close nullptr");

    int fclose_return = fclose(file_ptr);
    
    if (fclose_return != 0)
        EXIT(EXIT_FAILURE, "failed close file (%p)\n", file_ptr);

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

size_t CalcFileLen(const char* file)
{
    assert(file);

    struct stat buffer = {};
    stat(file, &buffer);

    return (size_t) buffer.st_size;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Buffer ReadFileInBuffer(const char* file)
{
    assert(file);

    size_t file_len = CalcFileLen(file      );
    FILE*  file_ptr = SafeFopen  (file, "rb");

    char* buffer = (char*) calloc(file_len + 1, sizeof(*buffer));

    if (!buffer)
        EXIT(EXIT_FAILURE, "failed open '%s' for reading.", file);

    buffer[file_len] = '\0';
        
    size_t fread_return = fread(buffer, sizeof(char), file_len, file_ptr);

    if (fread_return != file_len)
        EXIT(EXIT_FAILURE, "failed reading '%s'.", file);

    Buffer final_buffer =
    {
        .buffer = buffer,
        .size   = file_len,
    };

    return final_buffer;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void BufferDtor(Buffer* buffer)
{
    assert(buffer);
    
    FREE(buffer->buffer);
    buffer->size = 0;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const char* GetFileExtension(const char* file_name)
{
    assert(file_name);

    const char* dot = strrchr(file_name, '.');

    if ((!dot) || (dot == file_name))
        return nullptr;

    return dot + 1;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
