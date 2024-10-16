#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include "Compiler.h"

static void PrintError(CompilerErrorType* Err);
static void ErrPlaceCtor(CompilerErrorType* Err, const char* File, int Line, const char* Func);
static void PrintPlace(const char* File, int Line, const char* Func);
static CompilerErrorType Verif(CompilerErrorType* Err, const char* File, int Line, const char* Func);


CompilerErrorType RunCompiler(const IOfile* File)
{
    CompilerErrorType Err = {};
    COMPILER_RETURN_IF_ERR(Err);

    FILE* ProgrammFilePtr = fopen(File->ProgrammFile, "r");
    FILE* CodeFilePtr     = fopen(File->CodeFile, "w");

    if (ProgrammFilePtr == NULL)
    {
        Err.FailedOpenProgrammFile = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    if (CodeFilePtr == NULL)
    {
        Err.FailedOpenCodeFile = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    static const char* TempFile = "TempFile.txt";
    FILE* TempFilePtr           = fopen(TempFile, "w");

    if (TempFilePtr == NULL)
    {
        Err.FailedOpenTempFileWrite = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    size_t FileCmdQuant = 0;

    while (1)
    {
        char cmd[15] = "";

        if (fscanf(ProgrammFilePtr, "%s", cmd) == EOF)
        {
            Err.NoHalt = 1;
            Err.IsFatalError = 1;
            return COMPILER_VERIF(Err);
        }

        if (strcmp(cmd, "push") == 0)
        {
            char buffer[15] = "";
            char* EndBuffer = nullptr;
            fscanf(ProgrammFilePtr, "%s", buffer);

            StackElem_t PushElem = 0;
            PushElem = (int) strtol(buffer, &EndBuffer, 10);

            if (EndBuffer - buffer == (int) (strlen(buffer) * sizeof(buffer[0])))
            {
                fprintf(TempFilePtr, "%d %d ", push, PushElem);
                FileCmdQuant += 2;
                continue;
            }

            if (strcmp(buffer, "ax") == 0)
            {
                fprintf(TempFilePtr, "%d %d ", rpush, ax);
                FileCmdQuant += 2;
                continue;
            }

            if (strcmp(buffer, "bx") == 0)
            {
                fprintf(TempFilePtr, "%d %d ", rpush, bx);
                FileCmdQuant += 2;
                continue;
            }

            if (strcmp(buffer, "cx") == 0)
            {
                fprintf(TempFilePtr, "%d %d ", rpush, cx);
                FileCmdQuant += 2;
                continue;
            }

            if (strcmp(buffer, "dx") == 0)
            {
                fprintf(TempFilePtr, "%d %d ", rpush, dx);
                FileCmdQuant += 2;
                continue;
            }
            
            Err.InvalidInputAfterPush = 1;
            Err.IsFatalError = 1;
            return COMPILER_VERIF(Err);
        }

        if (strcmp(cmd, "pop") == 0)
        {
            char buffer[15] = "";
            fscanf(ProgrammFilePtr, "%s", buffer);

            if (strcmp(buffer, "ax") == 0)
            {
                fprintf(TempFilePtr, "%d %d ", pop, ax);
                FileCmdQuant += 2;
                continue;
            }

            if (strcmp(buffer, "bx") == 0)
            {
                fprintf(TempFilePtr, "%d %d ", pop, bx);
                FileCmdQuant += 2;
                continue;
            }

            if (strcmp(buffer, "cx") == 0)
            {
                fprintf(TempFilePtr, "%d %d ", pop, cx);
                FileCmdQuant += 2;
                continue;
            }

            if (strcmp(buffer, "dx") == 0)
            {
                fprintf(TempFilePtr, "%d %d ", pop, dx);
                FileCmdQuant += 2;
                continue;
            }

            Err.InvalidInputAfterPop = 1;
            Err.IsFatalError = 1;
            return COMPILER_VERIF(Err);
        }

        if (strcmp(cmd, "jmp") == 0)
        {
            StackElem_t JumpPlace = 0;

            if (fscanf(ProgrammFilePtr, "%d", &JumpPlace) == 0)
            {
                Err.NoIntAfterJmp = 1;
                Err.IsFatalError = 1;
                return COMPILER_VERIF(Err);
            }

            fprintf(TempFilePtr, "%d %d ", jmp, JumpPlace);
            FileCmdQuant += 2;
            continue;
        }

        if (strcmp(cmd, "ja") == 0)
        {
            StackElem_t JumpPlace = 0;

            if (fscanf(ProgrammFilePtr, "%d", &JumpPlace) == 0)
            {
                Err.NoIntAfterJmp = 1;
                Err.IsFatalError = 1;
                return COMPILER_VERIF(Err);
            }

            fprintf(TempFilePtr, "%d %d ", ja, JumpPlace);
            FileCmdQuant += 2;
            continue;
        }

        if (strcmp(cmd, "jae") == 0)
        {
            StackElem_t JumpPlace = 0;

            if (fscanf(ProgrammFilePtr, "%d", &JumpPlace) == 0)
            {
                Err.NoIntAfterJmp = 1;
                Err.IsFatalError = 1;
                return COMPILER_VERIF(Err);
            }

            fprintf(TempFilePtr, "%d %d ", jae, JumpPlace);
            FileCmdQuant += 2;
            continue;
        }

        if (strcmp(cmd, "jb") == 0)
        {
            StackElem_t JumpPlace = 0;

            if (fscanf(ProgrammFilePtr, "%d", &JumpPlace) == 0)
            {
                Err.NoIntAfterJmp = 1;
                Err.IsFatalError = 1;
                return COMPILER_VERIF(Err);
            }

            fprintf(TempFilePtr, "%d %d ", jb, JumpPlace);
            FileCmdQuant += 2;
            continue;
        }

        if (strcmp(cmd, "jbe") == 0)
        {
            StackElem_t JumpPlace = 0;

            if (fscanf(ProgrammFilePtr, "%d", &JumpPlace) == 0)
            {
                Err.NoIntAfterJmp = 1;
                Err.IsFatalError = 1;
                return COMPILER_VERIF(Err);
            }

            fprintf(TempFilePtr, "%d %d ", jbe, JumpPlace);
            FileCmdQuant += 2;
            continue;
        }

        if (strcmp(cmd, "je") == 0)
        {
            StackElem_t JumpPlace = 0;

            if (fscanf(ProgrammFilePtr, "%d", &JumpPlace) == 0)
            {
                Err.NoIntAfterJmp = 1;
                Err.IsFatalError = 1;
                return COMPILER_VERIF(Err);
            }

            fprintf(TempFilePtr, "%d %d ", je, JumpPlace);
            FileCmdQuant += 2;
            continue;
        }

        if (strcmp(cmd, "jne") == 0)
        {
            StackElem_t JumpPlace = 0;

            if (fscanf(ProgrammFilePtr, "%d", &JumpPlace) == 0)
            {
                Err.NoIntAfterJmp = 1;
                Err.IsFatalError = 1;
                return COMPILER_VERIF(Err);
            }

            fprintf(TempFilePtr, "%d %d ", jne, JumpPlace);
            FileCmdQuant += 2;
            continue;
        }

        if (strcmp(cmd, "add") == 0)
        {
            fprintf(TempFilePtr, "%d ", add);
            FileCmdQuant++;
            continue;
        }
        

        if (strcmp(cmd, "sub") == 0)
        {
            fprintf(TempFilePtr, "%d ", sub);
            FileCmdQuant++;
            continue;
        }

        if (strcmp(cmd, "mul") == 0)
        {
            fprintf(TempFilePtr, "%d ", mul);
            FileCmdQuant++;
            continue;
        }

        if (strcmp(cmd, "div") == 0)    
        {
            fprintf(TempFilePtr, "%d ", dive);
            FileCmdQuant++;
            continue;
        }

        if (strcmp(cmd, "out") == 0)
        {
            fprintf(TempFilePtr, "%d ", out);
            FileCmdQuant++;
            continue;
        }

        if (strcmp(cmd, "hlt") == 0)
        {
            fprintf(TempFilePtr, "%d\n", hlt);
            FileCmdQuant++;
            break;
        }

        Err.InvalidCmd = 1;
        Err.IsFatalError = 1;

        return COMPILER_VERIF(Err);
    }

    fprintf(CodeFilePtr, "%d\n", FileCmdQuant);

    fclose(TempFilePtr);
    TempFilePtr = fopen(TempFile, "r");

    if (TempFilePtr == NULL)
    {
        Err.FailedOpenTempFileRead = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    struct stat buf = {};
    stat(TempFile, &buf);
    size_t TempFileLen = buf.st_size;

    char* buffer = (char*) calloc(TempFileLen + 1, sizeof(char));

    if (buffer == NULL)
    {
        Err.FailedAllocateMemoryBufferTempFile = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    buffer[TempFileLen] = '\0';
    fread(buffer, TempFileLen, sizeof(char), TempFilePtr);
    fprintf(CodeFilePtr, "%s", buffer);

    fclose(CodeFilePtr);
    fclose(ProgrammFilePtr);

    fclose(TempFilePtr);
    remove(TempFile);

    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------

void CompilerAssertPrint(CompilerErrorType* Err, const char* File, int Line, const char* Func)
{
    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(File, Line, Func);
    PrintError(Err);
    PrintPlace(Err->Place.File, Err->Place.Line, Err->Place.Func);
    printf("\n");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType Verif(CompilerErrorType* Err, const char* File, int Line, const char* Func)
{
    ErrPlaceCtor(Err, File, Line, Func);

    return *Err;
}

//---------------------------------------------------------------------------------------------------------------------------

static void PrintError(CompilerErrorType* Err)
{
    if (Err->IsFatalError == 0)
    {
        return;
    }

    if (Err->InvalidCmd == 1)
    {
        COLOR_PRINT(RED, "Error: Invalid commamd.\n");
    }

    if (Err->InvalidInputAfterPop == 1)
    {
        COLOR_PRINT(RED, "Error: No/Invalid input after pop.\n");
    }

    if (Err->NoHalt == 1)
    {
        COLOR_PRINT(RED, "Error: No 'hlt' command.\n");
    }

    if (Err->FailedAllocateMemoryBufferTempFile == 1)
    {
        COLOR_PRINT(RED, "Error: Failed allocalet memory for buffer\nfor TempFile.\n");
    }

    if (Err->FailedOpenCodeFile == 1)
    {
        COLOR_PRINT(RED, "Error: Failed open file with code.\n");
    }

    if (Err->FailedOpenProgrammFile == 1)
    {
        COLOR_PRINT(RED, "Error: Failed open programm file.\n");
    }

    if (Err->FailedOpenTempFileRead == 1)
    {
        COLOR_PRINT(RED, "Error: Failed open TempFile for reading.\n");
    }

    if (Err->FailedOpenTempFileWrite == 1)
    {
        COLOR_PRINT(RED, "Error: Failed open TempFile for wirting.\n");
    }

    if (Err->NoIntAfterJmp == 1)
    {
        COLOR_PRINT(RED, "Error: No number after jmp.\n");
    }

    if (Err->InvalidInputAfterPush)
    {
        COLOR_PRINT(RED, "Error: No/invalid input after push.\n");
    }
    return;
}

//---------------------------------------------------------------------------------------------------------------------------

static void ErrPlaceCtor(CompilerErrorType* Err, const char* File, int Line, const char* Func)
{
    Err->Place.File = File;
    Err->Place.Line = Line;
    Err->Place.Func = Func;
    return;
}

//---------------------------------------------------------------------------------------------------------------------------

static void PrintPlace(const char* File, int Line, const char* Func)
{
    COLOR_PRINT(WHITE, "File [%s]\n", File);
    COLOR_PRINT(WHITE, "Line [%d]\n", Line);
    COLOR_PRINT(WHITE, "Func [%s]\n", Func);
    return;
}

//---------------------------------------------------------------------------------------------------------------------------
