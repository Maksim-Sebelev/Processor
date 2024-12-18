#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include "../Stack/Stack.hpp"
#include "../Common/GlobalInclude.hpp"


enum class ProcessorErrorType
{
    NO_ERR,
    INVALID_CMD,
    SPU_CODE_CALLOC_NULL,
    SPU_RAM_CALLOC_NULL,
    FAILED_READ_FILE_LEN,
    FAILED_OPEN_CODE_FILE,
    NO_HALT,
    DIVISION_BY_ZERO,
    FREAD_BAD_RETURN,

};



struct ProcessorErr
{
    CodePlace          place;
    ProcessorErrorType err;
};


void RunProcessor(const IOfile* file);

void  ProcessorAssertPrint    (ProcessorErr* Err, const char* File, int Line, const char* Func);



#define PROCESSOR_ASSERT(Err) do                                   \
{                                                                   \
    ProcessorErr ErrCopy = Err;                                      \
    if (ErrCopy.err != ProcessorErrorType::NO_ERR)                    \
    {                                                                  \
        ProcessorAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);   \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...");                      \
        abort();                                                          \
    }                                                                      \
} while (0)                                                                 \


#endif
