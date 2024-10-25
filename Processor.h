#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include "Stack.h"
#include "GlobalInclude.h"


struct ProcessorErrPlace
{
    const char* File;
    int         Line;
    const char* Func;
};


struct ProcessorErrorType
{
    ProcessorErrPlace Place;

    unsigned char IsFatalError         : 1;
 
    unsigned char InvalidCmd           : 1;
    unsigned char CtorCallocNull       : 1;
    unsigned char FailedReadFileLen    : 1;
    unsigned char FailedOpenCodeFile   : 1;
    unsigned char NoHalt               : 1;
};


struct Code
{
    StackElem_t*   code;
    size_t size;
};


struct SPU
{
    Code         code;
    size_t       ip;
    Stack_t      stack;
    StackElem_t  registers[REGISTERS_QUANT];
    int*         RAM;
};


ProcessorErrorType ReadCodeFromFile(SPU* Spu, const IOfile* File);
ProcessorErrorType RunProcessor(SPU* Spu);

void ProcessorDump(const SPU* Spu, const char* File, int Line, const char* Func);
ProcessorErrorType SpuCtor(SPU* Spu, const IOfile* File);
ProcessorErrorType SpuDtor(SPU* Spu);
void ProcessorAssertPrint(ProcessorErrorType* Err, const char* File, int Line, const char* Func);



#define PROCESSOR_VERIF(Spu, Err) Verif(Spu, &Err, __FILE__, __LINE__, __func__)

#define PROCESSOR_RETURN_IF_ERR(Err) do                 \
{                                                        \
    ProcessorErrorType ErrCopy = Err;                     \
    Verif(Spu, &ErrCopy, __FILE__, __LINE__, __func__);    \
    if (ErrCopy.IsFatalError == 1)                          \
    {                                                        \
       return ErrCopy;                                        \
    }                                                          \
} while (0)                                                     \


#define PROCESSSOR_DUMP(SpuPtr) ProcessorDump(SpuPtr, __FILE__, __LINE__, __func__)


#define PROCESSOR_ASSERT(Err) do                                   \
{                                                                   \
    ProcessorErrorType ErrCopy = Err;                                \
    if (ErrCopy.IsFatalError == 1)                                    \
    {                                                                  \
        ProcessorAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);   \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...");                      \
        abort();                                                          \
    }                                                                      \
} while (0)                                                                 \


#endif
