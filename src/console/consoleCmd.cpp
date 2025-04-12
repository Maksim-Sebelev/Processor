#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "console/consoleCmd.hpp"
#include "assembler/assembler.hpp"
#include "processor/processor.hpp"
#include "common/globalInclude.hpp"
#include "lib/colorPrint.hpp"
#include "lib/lib.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ConsoleCmdErr Verif      (ConsoleCmdErr* err, const char* file, int Line, const char* func);
static void          PrintError (ConsoleCmdErr* err);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ConsoleCmdErr (*ConsoleCmd[]) (const int, const char**, size_t) = 
{
    CompileCmd,
    RunCodeCmd
};

const size_t CmdQuant = sizeof(ConsoleCmd) / sizeof(ConsoleCmd[0]);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CallCmd(const int argc, const char** argv)
{
    assert(argv);
    assert(*argv);

    for (size_t argv_i = 1; (int) argv_i < argc; argv_i++)
    {
        for (size_t cmd_i = 0; cmd_i < CmdQuant; cmd_i++)
        {
            ConsoleCmdErr  err = (*ConsoleCmd[cmd_i]) (argc, argv, argv_i); 
            CONSOLE_ASSERT(err);
        }
    }
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ConsoleCmdErr CompileCmd(const int argc, const char** argv, size_t argv_i)
{
    assert(argv);
    assert(*argv);

    ConsoleCmdErr err = {};

    if (strcmp(argv[argv_i], "-compile") == 0)
    {
        if (argc - 1 < (int) argv_i + 2)
        {
            err.err = ConsoleCmdErrorType::NO_INPUT_AFTER_COMPILE;
            return VERIF(err);
        }

        IOfile file = {};
    
        file.ProgrammFile = argv[argv_i + 1];
        file.CodeFile     = argv[argv_i + 2];

        RunAssembler(&file);
    }

    return VERIF(err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ConsoleCmdErr RunCodeCmd(const int argc, const char** argv, size_t argv_i)
{
    assert(argv);
    assert(*argv);

    ConsoleCmdErr err = {};
    if (strcmp(argv[argv_i], "-run") == 0)
    {
        if  (argc - 1 < (int) argv_i + 1)
        {
            err.err = ConsoleCmdErrorType::NO_INPUT_AFTER_RUN;
            return VERIF(err);
        }

        IOfile file   = {};
        file.CodeFile = argv[argv_i + 1];

        RunProcessor(&file);
    }
    return VERIF(err);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void ConsoleCmdAssertPrint(ConsoleCmdErr* err, const char* file, int Line, const char* func)
{
    assert(err);
    assert(file);
    assert(func);

    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(file, Line, func);
    PrintError(err);
    PrintPlace(err->place.file, err->place.line, err->place.func);
    printf("\n");
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(ConsoleCmdErr* err)
{
    assert(err);

    ConsoleCmdErrorType errType = err->err;

    switch (errType)
    {
        case ConsoleCmdErrorType::NO_ERR_CONSOLE:                       return;                                                            break;
        case ConsoleCmdErrorType::INVALID_INPUT_AFTER_COMPILE:  COLOR_PRINT(RED,  "Error: Incorrect input after \"-compile\".\n"); break;
        case ConsoleCmdErrorType::NO_INPUT_AFTER_COMPILE:       COLOR_PRINT(RED,  "Error: No input after \"-compile\".\n");        break;
        case ConsoleCmdErrorType::NO_INPUT_AFTER_RUN:           COLOR_PRINT(RED,  "Error: No input after \"-run\".\n");            break;
        default:                                                assert     (0 &&  "undef console cmd error type");                 break;
    }

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ConsoleCmdErr Verif(ConsoleCmdErr* err, const char* file, int Line, const char* func)
{
    CodePlaceCtor(&err->place, file, Line, func);
    return *err;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------