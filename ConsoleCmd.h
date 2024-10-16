#ifndef CONSOLE_CMD_H
#define CONSOLE_CMD_H

#include "Compiler.h"

struct ConsolePlace
{
    const char* File;
    int         Line;
    const char* Func;;
};


struct ConsoleCmdErrorType
{
    ConsolePlace Place;
    unsigned char IsFatalError             : 1;

    unsigned char NoInputAfterCompile      : 1;
    unsigned char InvalidInpurAfterCompile : 1;    
    unsigned char NoInputAfterRun          : 1;

};


void CallCmd(const int argc, const char** argv, IOfile* File);
ConsoleCmdErrorType CompileCmd(const int argc, const char** argv, int argv_i, IOfile* File);
ConsoleCmdErrorType RunCodeCmd(const int argc, const char** argv, int argv_i, IOfile* File);

void ConsoleCmdAssertPrint(ConsoleCmdErrorType* Err, const char* File, int Line, const char* Func);


#define VERIF(Err) Verif(&Err, __FILE__, __LINE__, __func__)

#define CONSOLE_ASSERT(Err) do                                      \
{                                                                    \
    ConsoleCmdErrorType ErrCopy = Err;                                \
    if (ErrCopy.IsFatalError == 1)                                     \
    {                                                                   \
        ConsoleCmdAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);   \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...\n");                     \
        abort();                                                           \
    }                                                                       \
} while (0)                                                                  \


#endif
