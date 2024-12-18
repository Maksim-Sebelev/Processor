#ifndef ASSEMBLER_H
#define ASSEMBLER_H


#include <stdio.h>

#include "../Common/GlobalInclude.hpp"


enum class AssemblerErrorType
{
    NO_ERR                      ,
    INVALID_INPUT_AFTER_PUSH    ,
    INVALID_INPUT_AFTER_POP     ,
    FAILED_OPEN_INPUT_STREAM    ,
    FAILED_OPEN_OUTPUT_STREAM   ,
    FWRITE_BAD_RETURN           ,
    UNDEFINED_COMMAND           ,
    BAD_CODE_ARR_REALLOC        ,
    LABEL_REDEFINE              ,
    BAD_LABELS_CALLOC           ,
    BAD_LABELS_REALLOC          ,
};


struct AssemblerErr
{
    CodePlace          place;
    AssemblerErrorType err;
};


void RunAssembler        (const IOfile* file);

void AssemblerAssertPrint(AssemblerErr* err, const char* file, int line, const char* func);

#define ASSEMBLER_ASSERT(Err) do                                  \
{                                                                 \
    AssemblerErr ErrCopy = Err;                                    \
    if (ErrCopy.err != AssemblerErrorType::NO_ERR)                  \
    {                                                                \
        AssemblerAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);  \
        abort();                                                       \
    }                                                                   \
} while (0)                                                              \




#endif
