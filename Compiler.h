#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include "Stack.h"

enum Cmd
{
    hlt = 0,
    push,
    rpush,
    pop,
    add,
    sub,
    mul,
    dive,
    out,
    jmp,
    ja,
    jae,
    jb,
    jbe,
    je,
    jne,
    CMD_QUANT // count
};


enum Registers
{
    ax = 0,
    bx,
    cx,
    dx,
    REGISTERS_QUANT // Count
};

struct CompilerPlace
{   
    const char* File;
    int         Line;
    const char* Func;
};

struct CompilerErrorType
{
    CompilerPlace Place;

    unsigned char IsFatalError                            : 1;

    unsigned char NoHalt                                  : 1;
    unsigned char InvalidCmd                              : 1;
    unsigned char NoIntAfterJmp                           : 1;
    unsigned char FailedOpenCodeFile                      : 1;
    unsigned char InvalidInputAfterPop                    : 1;
    unsigned char InvalidInputAfterPush                   : 1;
    unsigned char FailedOpenProgrammFile                  : 1;
    unsigned char FailedOpenTempFileRead                  : 1;
    unsigned char FailedOpenTempFileWrite                 : 1;
    unsigned char FailedAllocateMemoryBufferTempFile      : 1;
};

struct IOfile
{
    const char* ProgrammFile;
    const char* CodeFile;
};


CompilerErrorType RunCompiler(const IOfile* File);
void CompilerAssertPrint(CompilerErrorType* Err, const char* File, int Line, const char* Func);


#define COMPILER_RETURN_IF_ERR(Err) do   \
{                                         \
    CompilerErrorType ErrCopy = Err;       \
    if (ErrCopy.IsFatalError == 1)          \
    {                                        \
        return ErrCopy;                       \
    }                                          \
} while (0)                                     \

#define COMPILER_ASSERT(Err) do                                  \
{                                                                 \
    CompilerErrorType ErrCopy = Err;                               \
    if (ErrCopy.IsFatalError == 1)                                  \
    {                                                                \
        CompilerAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);  \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...");                    \
        abort();                                                        \
    }                                                                    \
} while (0);                                                              \


#define COMPILER_VERIF(Err) Verif(&Err, __FILE__, __LINE__, __func__)

#endif



