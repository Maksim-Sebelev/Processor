#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include "Stack.h"



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
    unsigned char TooManyLabels                           : 1;
    unsigned char NoIntAfterJmp                           : 1;
    unsigned char LabelCallocNull                         : 1;
    unsigned char FailedOpenCodeFile                      : 1;
    unsigned char MoreOneEqualLables                      : 1;
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


struct FileSignature
{
    int           Signature;
    unsigned char Version;
    size_t        FileSize;
};


struct Label
{
    const char* Name;
    int         CodePlace;    
};


struct LabelsTable
{
    Label*  Labels;
    size_t  FirstFree;
    size_t  Capacity;
};


struct CmdDataForAsm
{
    FILE*             ProgrammFilePtr;
    FILE*             CodeFilePtr;
    FILE*             TempFilePtr;
    size_t            FileCmdQuant;
    LabelsTable*      Labels;
};


CompilerErrorType RunCompiler          (const IOfile* File);
void              CompilerAssertPrint  (CompilerErrorType* Err, const char* File, int Line, const char* Func);


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
} while (0);                                                              \


#define COMPILER_VERIF(Err) Verif(&Err, __FILE__, __LINE__, __func__)

#endif



