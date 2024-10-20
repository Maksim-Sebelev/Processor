#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <assert.h>
#include "Compiler.h"
#include "GlobalInclude.h"

static  void               PrintError   (CompilerErrorType* Err);
static  void               PrintPlace   (const char* File, int Line, const char* Func);
static  void               ErrPlaceCtor (CompilerErrorType* Err, const char* File, int Line, const char* Func);
static  CompilerErrorType  Verif        (CompilerErrorType* Err, const char* File, int Line, const char* Func);

static  size_t             CalcFileLen              (const char* FileName);
static  void               CloseFiles               (FILE* ProgrammFilePtr, FILE* CodeFilePtr, FILE* TempFilePtr);
static  CompilerErrorType  WriteTempFileInCodeFile  (const char* TempFile, FILE* TempFilePtr, FILE* CodeFilePtr, CompilerErrorType* Err);

static  void               LabelCtor        (Label* Lab, const char* Name, int CodePlace);
static  int                IsLabelInLabels  (const LabelsTable* Labels, const char* LabelName);
static  CompilerErrorType  LabelsTableCtor  (LabelsTable* Labels);
static  CompilerErrorType  PushLabel        (LabelsTable* Labels, const Label* Lab);

static  CompilerErrorType  JmpCmdPattern  (CmdDataForAsm* CmdInfo, Cmd JumpType, CompilerErrorType* Err);

static  CompilerErrorType  HandlePush          (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandlePop           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleJmp           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleJa            (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleJae           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleJb            (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleJbe           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleJe            (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleJne           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleAdd           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleSub           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleMul           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleDiv           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleOut           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  HandleHlt           (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static CompilerErrorType   HandleLabelOrError  (CmdDataForAsm* CmdInfo, const char* Cmd, CompilerErrorType* Err);

//--------------------------------------------------------------------------------------------------------------------------------------------------

CompilerErrorType RunCompiler(const IOfile* File)
{
    CompilerErrorType Err = {};
    COMPILER_RETURN_IF_ERR(Err);

    FILE* ProgrammFilePtr = fopen(File->ProgrammFile, "rb");
    FILE* CodeFilePtr     = fopen(File->CodeFile, "wb");

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
    FILE* TempFilePtr           = fopen(TempFile, "wb");

    if (TempFilePtr == NULL)
    {
        Err.FailedOpenTempFileWrite = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }


    static const size_t MaxCmdSize = 15;
    char Cmd[MaxCmdSize] = "";

    LabelsTable Labels = {};
    LabelsTableCtor(&Labels);


    CmdDataForAsm CmdInfo = 
    {
        ProgrammFilePtr, CodeFilePtr, TempFilePtr, 0, &Labels
    };

    while (fscanf(CmdInfo.ProgrammFilePtr, "%s", Cmd) != EOF)
    {
        printf("lab[0] = %s\n", CmdInfo.Labels->Labels[0].Name);
        if (strcmp(Cmd, "push") == 0)
        {
            COMPILER_RETURN_IF_ERR(HandlePush(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "pop") == 0)
        {
            COMPILER_RETURN_IF_ERR(HandlePop(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "jmp") == 0)
        {
           COMPILER_RETURN_IF_ERR(HandleJmp(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "ja") == 0)
        {
           COMPILER_RETURN_IF_ERR(HandleJa(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "jae") == 0)
        {
           COMPILER_RETURN_IF_ERR(HandleJae(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "jb") == 0)
        {
           COMPILER_RETURN_IF_ERR(HandleJb(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "jbe") == 0)
        {
           COMPILER_RETURN_IF_ERR(HandleJbe(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "je") == 0)
        {
           COMPILER_RETURN_IF_ERR(HandleJe(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "jne") == 0)
        {
           COMPILER_RETURN_IF_ERR(HandleJne(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "add") == 0)
        {
            COMPILER_RETURN_IF_ERR(HandleAdd(&CmdInfo, &Err));
        }
        
        else if (strcmp(Cmd, "sub") == 0)
        {
            COMPILER_RETURN_IF_ERR(HandleSub(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "mul") == 0)
        {
            COMPILER_RETURN_IF_ERR(HandleMul(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "div") == 0)    
        {
            COMPILER_RETURN_IF_ERR(HandleDiv(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "out") == 0)
        {
            COMPILER_RETURN_IF_ERR(HandleOut(&CmdInfo, &Err));
        }

        else if (strcmp(Cmd, "hlt") == 0)
        {
            COMPILER_RETURN_IF_ERR(HandleHlt(&CmdInfo, &Err));
        }

        else
        {
            COMPILER_RETURN_IF_ERR(HandleLabelOrError(&CmdInfo, Cmd, &Err));
        }
    }

    fclose(CmdInfo.TempFilePtr);
    TempFilePtr = fopen(TempFile, "rb");

    if (TempFilePtr == NULL)
    {
        Err.FailedOpenTempFileRead = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    fprintf(CodeFilePtr, "%d\n", CmdInfo.FileCmdQuant);

    COMPILER_RETURN_IF_ERR
    (
    WriteTempFileInCodeFile(TempFile, TempFilePtr, CodeFilePtr, &Err);
    )

    CloseFiles(ProgrammFilePtr, CodeFilePtr, TempFilePtr);
    remove(TempFile);

    return COMPILER_VERIF(Err);
}


//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleLabelOrError(CmdDataForAsm* CmdInfo, const char* Cmd, CompilerErrorType* Err)
{
    size_t LabelSize = strlen(Cmd);

    if (Cmd[LabelSize - 1] != ':')
    {
        Err->InvalidCmd = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }


    Label Temp = {};
    LabelCtor(&Temp, Cmd, CmdInfo->FileCmdQuant);

    int LabelIndex = IsLabelInLabels(CmdInfo->Labels, Cmd);


    if (LabelIndex == -1)
    {
        COLOR_PRINT(RED, "LabelOrError\n");
        PushLabel(CmdInfo->Labels, &Temp);
        return COMPILER_VERIF(*Err);
    }
    Err->MoreOneEqualLables = 1;
    Err->IsFatalError = 1;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandlePush(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    static const size_t MaxBufferSize = 16;
    char buffer[MaxBufferSize] = "";
    char* EndBuffer = nullptr;
    fscanf(CmdInfo->ProgrammFilePtr, "%s", buffer);

    StackElem_t PushElem = 0;
    PushElem = (int) strtol(buffer, &EndBuffer, 10);

    if (EndBuffer - buffer == (int) (strlen(buffer) * sizeof(buffer[0])))
    {
        fprintf(CmdInfo->TempFilePtr, "%d %d ", push, PushElem);
    }
    
    else if (strlen(buffer) == 2 && 'a' <= buffer[0] && buffer[0] <= 'd' &&  buffer[1] == 'x')
    {
        fprintf(CmdInfo->TempFilePtr, "%d %d ", pushr, buffer[0] - 'a');
    }

    else
    {
        Err->InvalidInputAfterPush = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }

    CmdInfo->FileCmdQuant += 2;

    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandlePop(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    static const size_t MaxBufferSize = 16;
    char buffer[MaxBufferSize] = "";
    fscanf(CmdInfo->ProgrammFilePtr, "%s", buffer);


    if (strlen(buffer) == 2 && 'a' <= buffer[0] && buffer[0] <= 'd' &&  buffer[1] == 'x')
    {
        fprintf(CmdInfo->TempFilePtr, "%d %d ", pop, buffer[0] - 'a');
    }

    else
    {
        Err->InvalidInputAfterPop = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }

    CmdInfo->FileCmdQuant += 2;
    
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleJmp(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    JmpCmdPattern(CmdInfo, jmp, Err);
    return COMPILER_VERIF(*Err);  
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleJa(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    JmpCmdPattern(CmdInfo, ja, Err);
    return COMPILER_VERIF(*Err);  
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleJae(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    COMPILER_RETURN_IF_ERR(JmpCmdPattern(CmdInfo, jae, Err));
    return COMPILER_VERIF(*Err);    
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleJb(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    JmpCmdPattern(CmdInfo, jb, Err);
    return COMPILER_VERIF(*Err);    
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleJbe(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    JmpCmdPattern(CmdInfo, jbe, Err);
    return COMPILER_VERIF(*Err);    
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleJe(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    JmpCmdPattern(CmdInfo, je, Err);
    return COMPILER_VERIF(*Err);  
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleJne(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    JmpCmdPattern(CmdInfo, jne, Err);
    return COMPILER_VERIF(*Err);    
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleAdd(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d ", add);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleSub(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d ", sub);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleMul(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d ", mul);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleDiv(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d ", dive);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleOut(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d ", out);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleHlt(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d ", hlt);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType JmpCmdPattern(CmdDataForAsm* CmdInfo, Cmd JumpType, CompilerErrorType* Err)
{
    static const size_t MaxLabelNameSize = 16;
    char JumpArg[MaxLabelNameSize] = "";
    CmdInfo->FileCmdQuant += 2;

    if (fscanf(CmdInfo->ProgrammFilePtr, "%s", JumpArg) == 0)
    {
        Err->NoIntAfterJmp = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }

    char*  JumpArgEndPtr = NULL;
    int    IntJumpArg    = (int) strtol(JumpArg, &JumpArgEndPtr, 10);
    size_t JumpArgLen    = strlen(JumpArg);
    int    JumpArgNumLen = JumpArgEndPtr - JumpArg;

    if (JumpArgNumLen == (int) JumpArgLen)
    {
        fprintf(CmdInfo->TempFilePtr, "%d %d ", JumpType, IntJumpArg);
        return COMPILER_VERIF(*Err);
    }

    int JumpArgInLabeles = IsLabelInLabels(CmdInfo->Labels, JumpArg);

    if (JumpArgInLabeles == -1)
    {
        COLOR_PRINT(RED, "JmpCmdPattern\n");
        Label Temp = {};
        LabelCtor(&Temp, JumpArg, -1);
        COMPILER_RETURN_IF_ERR(PushLabel(CmdInfo->Labels, &Temp));
        fprintf(CmdInfo->TempFilePtr, "%d %d ", JumpType, -1);
        return COMPILER_VERIF(*Err);
    }

    fprintf(CmdInfo->TempFilePtr, "%d %d ", JumpType, CmdInfo->Labels->Labels[JumpArgInLabeles].CodePlace);

    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

void CompilerAssertPrint(CompilerErrorType* Err, const char* File, int Line, const char* Func)
{
    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(File, Line, Func);
    PrintError(Err);
    PrintPlace(Err->Place.File, Err->Place.Line, Err->Place.Func);
    printf("\n");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType Verif(CompilerErrorType* Err, const char* File, int Line, const char* Func)
{
    ErrPlaceCtor(Err, File, Line, Func);
    return *Err;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

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

    if (Err->InvalidInputAfterPush == 1)
    {
        COLOR_PRINT(RED, "Error: No/invalid input after push.\n");
    }

    if (Err->MoreOneEqualLables == 1)
    {
        COLOR_PRINT(RED, "Error: 2 equal labels is used.\n");
    }
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void ErrPlaceCtor(CompilerErrorType* Err, const char* File, int Line, const char* Func)
{
    Err->Place.File = File;
    Err->Place.Line = Line;
    Err->Place.Func = Func;
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintPlace(const char* File, int Line, const char* Func)
{
    COLOR_PRINT(WHITE, "File [%s]\n", File);
    COLOR_PRINT(WHITE, "Line [%d]\n", Line);
    COLOR_PRINT(WHITE, "Func [%s]\n", Func);
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static size_t CalcFileLen(const char* FileName)
{
    struct stat Buf = {};
    stat(FileName, &Buf);
    return Buf.st_size;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType WriteTempFileInCodeFile(const char* TempFile, FILE* TempFilePtr, FILE* CodeFilePtr, CompilerErrorType* Err)
{
    size_t TempFileLen = CalcFileLen(TempFile);

    char* buffer = (char*) calloc(TempFileLen + 1, sizeof(char));

    if (buffer == NULL)
    {
        Err->FailedAllocateMemoryBufferTempFile = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }

    buffer[TempFileLen] = '\0';
    fread(buffer, TempFileLen, sizeof(char), TempFilePtr);
    fprintf(CodeFilePtr, "%s", buffer);

    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void CloseFiles(FILE* ProgrammFilePtr, FILE* CodeFilePtr, FILE* TempFilePtr)
{
    fclose(ProgrammFilePtr);
    fclose(CodeFilePtr);
    fclose(TempFilePtr);
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void LabelCtor(Label* Lab, const char* Name, int CodePlace)
{
    Lab->CodePlace = CodePlace;
    Lab->Name = Name;
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType LabelsTableCtor(LabelsTable* Labels)
{
    CompilerErrorType Err = {};
    static const size_t DefaultLabelsTableSize = 16;
    Labels->FirstFree = 0;
    Labels->Capacity = DefaultLabelsTableSize;
    Labels->Labels = (Label*) calloc(DefaultLabelsTableSize, sizeof(Label));

    if (Labels->Labels == NULL)
    {
        Err.LabelCallocNull = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    for (size_t Labels_i = 0; Labels_i < DefaultLabelsTableSize; Labels_i++)
    {
        LabelCtor(&Labels->Labels[Labels_i], "", -1);
    }
    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType PushLabel(LabelsTable* Labels, const Label* Lab)
{
    CompilerErrorType Err = {};
    if (Labels->FirstFree == Labels->Capacity)
    {
        Err.TooManyLabels = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    LabelCtor(&Labels->Labels[Labels->FirstFree], Lab->Name, Lab->CodePlace);
    Labels->Labels[Labels->FirstFree].CodePlace = Lab->CodePlace;
    Labels->Labels[Labels->FirstFree].Name = Lab->Name;

    Labels->FirstFree++;

    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

//возвращает индекс первой встречи, иначе ммнус -1 

static int IsLabelInLabels(const LabelsTable* Labels, const char* LabelName)
{
    for (size_t Label_i = 0; Label_i < Labels->FirstFree; Label_i++)
    {
        if (strcmp(Labels->Labels[Label_i].Name, LabelName) == 0)
        {
            return Label_i; 
        }
    }
    return -1;
}

