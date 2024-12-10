#ifndef ASSEMBLER_H
#define ASSEMBLER_H


#include <stdio.h>

#include "../Common/GlobalInclude.h"


enum AssemblerErrorType
{
    NO_ERR,
    INVALID_INPUT_AFTER_PUSH,
    INVALID_INPUT_AFTER_POP,
    FAILED_OPEN_INPUT_STREAM,
    FAILED_OPERN_OUTPUT_STREAM,
};

struct AssemblerErr
{
    CodePlace          place;
    AssemblerErrorType err;
};



#define COMPILER_RETURN_IF_ERR(Err) do   \
{                                         \
    AssemblerErr ErrCopy = Err;            \
    if (ErrCopy.err != NO_ERR )             \
    {                                        \
        return ErrCopy;                       \
    }                                          \
} while (0);                                    \


#define COMPILER_ASSERT(Err) do                                  \
{                                                                 \
    AssemblerErr ErrCopy = Err;                                    \
    if (ErrCopy.err != NO_ERR)                                      \
    {                                                                \
        CompilerAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);  \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...");                    \
        abort();                                                        \
    }                                                                    \
} while (0)                                                               \


#define COMPILER_VERIF(Err) Verif(&Err, __FILE__, __LINE__, __func__)


#endif
