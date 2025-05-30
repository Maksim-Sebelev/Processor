#ifndef CONSOLE_CMD_H
#define CONSOLE_CMD_H

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include "lib/lib.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum ConsoleCmdErrorType : int
{
    NO_ERR_CONSOLE = 0          ,
    NO_INPUT_AFTER_COMPILE      ,
    INVALID_INPUT_AFTER_COMPILE ,
    NO_INPUT_AFTER_RUN          ,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct ConsoleCmdErr
{
    CodePlace           place;
    ConsoleCmdErrorType err;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void          CallCmd      (const int argc, const char** argv);
ConsoleCmdErr CompileCmd   (const int argc, const char** argv, size_t argv_i);
ConsoleCmdErr RunCodeCmd   (const int argc, const char** argv, size_t argv_i);

void ConsoleCmdAssertPrint (ConsoleCmdErr* Err, const char* File, int Line, const char* Func);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define VERIF(err) Verif(&err, __FILE__, __LINE__, __func__)

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define CONSOLE_ASSERT(err) do                                     \
{                                                                   \
    ConsoleCmdErr       errCopy = err;                               \
    ConsoleCmdErrorType errType = errCopy.err;                        \
    if (errType != ConsoleCmdErrorType::NO_ERR_CONSOLE)                \
    {                                                                   \
        ConsoleCmdAssertPrint(&errCopy, __FILE__, __LINE__, __func__);   \
        COLOR_PRINT(CYAN, "exit in 3, 2, 1...\n");                        \
        exit(errType);                                                     \
    }                                                                       \
} while (0)                                                                  \

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif //CONSOLE_CMD_H
