#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "ConsoleCmd.h"
#include "Compiler.h"
#include "Processor.h"

static  ConsoleCmdErrorType  Verif         (ConsoleCmdErrorType* Err, const char* File, int Line, const char* Func);
static  void                 PrintError    (ConsoleCmdErrorType* Err);
static  void                 ErrPlaceCtor  (ConsoleCmdErrorType* Err, const char* File, int Line, const char* Func);
static  void                 PrintPlace    (const char* File, int Line, const char* Func);

ConsoleCmdErrorType (*ConsoleCmd[]) (const int, const char**, int, IOfile*) = 
{
    CompileCmd,
    RunCodeCmd
};

const size_t CmdQuant = sizeof(ConsoleCmd)/sizeof(ConsoleCmd[0]);

//--------------------------------------------------------------------------------------------------------------------------------------------------

void CallCmd(const int argc, const char** argv, IOfile* File)
{
    for (size_t argv_i = 1; (int) argv_i < argc; argv_i++)
    {
        for (size_t cmd_i = 0; cmd_i < CmdQuant; cmd_i++)
        {
            CONSOLE_ASSERT((*ConsoleCmd[cmd_i]) (argc, argv, argv_i, File));
        }
    }
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

ConsoleCmdErrorType CompileCmd(const int argc, const char** argv, int argv_i, IOfile* File)
{
    ConsoleCmdErrorType Err = {};

    if (strcmp(argv[argv_i], "-compile") == 0)
    {
        if (argc - 1 < argv_i + 2)
        {
            Err.NoInputAfterCompile = 1;
            Err.IsFatalError = 1;
            return VERIF(Err);
        }

        File->ProgrammFile = argv[argv_i + 1];
        File->CodeFile     = argv[argv_i + 2];

        COMPILER_ASSERT(RunCompiler(File));
    }

    return VERIF(Err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

ConsoleCmdErrorType RunCodeCmd(const int argc, const char** argv, int argv_i, IOfile* File)
{
    ConsoleCmdErrorType Err = {};
    if (strcmp(argv[argv_i], "-run") == 0)
    {
        if  (argc - 1 < argv_i + 1)
        {
            Err.NoInputAfterRun = 1;
            Err.IsFatalError = 1;
            return VERIF(Err);
        }

        File->CodeFile = argv[argv_i + 1];

        SPU Spu = {};
        PROCESSOR_ASSERT(SpuCtor(&Spu, File));
        PROCESSOR_ASSERT(WriteFileInCode(&Spu, File));
        PROCESSOR_ASSERT(RunProcessor(&Spu));
        PROCESSOR_ASSERT(SpuDtor(&Spu));
    }
    return VERIF(Err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

void ConsoleCmdAssertPrint(ConsoleCmdErrorType* Err, const char* File, int Line, const char* Func)
{
    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(File, Line, Func);
    PrintError(Err);
    PrintPlace(Err->Place.File, Err->Place.Line, Err->Place.Func);
    printf("\n");
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

static void ErrPlaceCtor(ConsoleCmdErrorType* Err, const char* File, int Line, const char* Func)
{
    Err->Place.File = File;
    Err->Place.Line = Line;
    Err->Place.Func = Func;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintPlace(const char* File, int Line, const char* Func)
{
    COLOR_PRINT(WHITE, "File [%s]\n", File);
    COLOR_PRINT(WHITE, "Line [%d]\n", Line);
    COLOR_PRINT(WHITE, "Func [%s]\n", Func);
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(ConsoleCmdErrorType* Err)
{
    if (Err->IsFatalError == 0)
    {
        return;
    }

    if (Err->InvalidInpurAfterCompile == 1)
    {
        COLOR_PRINT(RED, "Error: Incorrest input format.\n");
    }

    if (Err->NoInputAfterCompile == 1)
    {
        COLOR_PRINT(RED, "Error: No input.\n");
    }

    if (Err->NoInputAfterRun)
    {
        COLOR_PRINT(RED, "Error: No input.\n");
    }

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

static ConsoleCmdErrorType Verif(ConsoleCmdErrorType* Err, const char* File, int Line, const char* Func)
{
    ErrPlaceCtor(Err, File, Line, Func);
    return *Err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
