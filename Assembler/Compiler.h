#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include "../Stack/Stack.h"
#include "../Common/GlobalInclude.h"



struct CompilerPlace
{   
    const char* file;
    int         line;
    const char* func;
};

struct CompilerErrorType
{
    CompilerPlace Place;

    unsigned char IsFatalError                            : 1;

    unsigned char NoHalt                                  : 1;
    unsigned char InvalidCmd                              : 1;
    unsigned char PushNotLabel                            : 1;
    unsigned char SyntaxisError                           : 1;
    unsigned char TooManyLabels                           : 1;
    unsigned char NoIntAfterJmp                           : 1;
    unsigned char LabelCallocNull                         : 1;
    unsigned char CmdCodeArrNull                          : 1;     
    unsigned char FailedOpenCodeFile                      : 1;
    unsigned char MoreOneEqualLables                      : 1;
    unsigned char InvalidInputAfterPop                    : 1;
    unsigned char InvalidInputAfterPush                   : 1;
    unsigned char FailedOpenProgrammFile                  : 1;
    unsigned char FailedOpenTempFileRead                  : 1;
    unsigned char FailedOpenTempFileWrite                 : 1;
    unsigned char FailedAllocateMemoryBufferTempFile      : 1;
    unsigned char FailedReallocateMemoryToCmdCodeArr      : 1;
    unsigned char FailedAllocateMemoryForProgrammBuffer   : 1;

};



struct FileSignature
{
    int           Signature;
    unsigned char Version;
    size_t        FileSize;
};


CompilerErrorType RunCompiler          (const IOfile* file);
void              CompilerAssertPrint  (CompilerErrorType* Err, const char* file, int line, const char* func);


#define COMPILER_RETURN_IF_ERR(Err) do   \
{                                         \
    CompilerErrorType ErrCopy = Err;       \
    if (ErrCopy.IsFatalError == 1)          \
    {                                        \
        return ErrCopy;                       \
    }                                          \
} while (0);                                    \


#define COMPILER_ASSERT(Err) do                                  \
{                                                                 \
    CompilerErrorType ErrCopy = Err;                               \
    if (ErrCopy.IsFatalError == 1)                                  \
    {                                                                \
        CompilerAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);  \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...");                    \
        abort();                                                        \
    }                                                                    \
} while (0)                                                               \

    
#define COMPILER_VERIF(Err) Verif(&Err, __FILE__, __LINE__, __func__)


#endif
