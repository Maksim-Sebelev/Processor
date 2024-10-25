#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <assert.h>
#include "Compiler.h"
#include "GlobalInclude.h"


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
    LabelsTable   Labels;
    FILE*         ProgrammFilePtr;
    FILE*         CodeFilePtr;
    FILE*         TempFilePtr;
    size_t        FileCmdQuant;
    char          Cmd[17];
};



struct CmdFunc
{
    const char* CmdName;
    CompilerErrorType (*CmdFunc) (CmdDataForAsm*, CompilerErrorType*);
};


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

static  int                GetPushArg    (PushType* Push);
static  int                GetPopArg     (PopType*  Pop );
static  CompilerErrorType  PushTypeCtor  (PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem);
static  CompilerErrorType  PopTypeCtor   (PopType*  Pop , uint8_t Reg, uint8_t Mem);

static  CompilerErrorType  CmdInfoCtor  (CmdDataForAsm* CmdInfo, LabelsTable* Labels, FILE* ProgrammFilePtr, FILE* CodeFilePtr, FILE* TempFilePtr);

static  bool IsInt       (const char* Str, const char* StrEnd, const size_t StrSize);
static  bool IsRegister  (const char* Str, const size_t StrSize);
static  bool IsMemory    (const char* Str, const size_t StrSize);
static  bool IsLabel     (const char* Str);

static  const char*  GetCmdName  (size_t Cmd_i);
static  CompilerErrorType (*GetCmd(size_t Cmd_i)) (CmdDataForAsm* CmdInfo, CompilerErrorType* Err); 


static  CompilerErrorType  UpdateBufferForMemory  (char** Buffer, size_t* BufferSize);
 
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
static  CompilerErrorType  HandleLabelOrError  (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);

static  CompilerErrorType  RunAssembler  (CmdDataForAsm* CmdInfo);


static const CmdFunc CmdFuncArr[] = 
{
    {"push",  HandlePush},
    {"pop" ,  HandlePop },
    {"jmp" ,  HandleJmp },
    {"ja"  ,  HandleJa  },
    {"jae" ,  HandleJae },
    {"jb"  ,  HandleJb  },
    {"jbe" ,  HandleJbe },
    {"je"  ,  HandleJe  },
    {"jne" ,  HandleJne },
    {"add" ,  HandleAdd },
    {"sub" ,  HandleSub },
    {"mul" ,  HandleMul },
    {"div" ,  HandleDiv },
    {"out" ,  HandleOut },
    {"hlt" ,  HandleHlt },
};

static const size_t CmdFuncQuant = sizeof(CmdFuncArr) / sizeof(CmdFuncArr[0]);

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

    LabelsTable Labels = {};
    LabelsTableCtor(&Labels);

    CmdDataForAsm CmdInfo = {};
    COMPILER_RETURN_IF_ERR(CmdInfoCtor(&CmdInfo, &Labels, ProgrammFilePtr, CodeFilePtr, TempFilePtr));

    COMPILER_RETURN_IF_ERR(RunAssembler(&CmdInfo));

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

static CompilerErrorType RunAssembler(CmdDataForAsm* CmdInfo)
{
    CompilerErrorType Err = {};

    while (fscanf(CmdInfo->ProgrammFilePtr, "%s", CmdInfo->Cmd) != EOF)
    {
        bool WasCorrectCmd = false;

        for (size_t CmdFuncArr_i = 0; CmdFuncArr_i < CmdFuncQuant; CmdFuncArr_i++)
        {
            if (strcmp(GetCmdName(CmdFuncArr_i), CmdInfo->Cmd) == 0)
            {
                WasCorrectCmd = true;
                COMPILER_RETURN_IF_ERR(GetCmd(CmdFuncArr_i)(CmdInfo, &Err));
                break;
            }

            if (IsLabel(CmdInfo->Cmd))
            {
                WasCorrectCmd = true;
                COMPILER_RETURN_IF_ERR(HandleLabelOrError(CmdInfo, &Err));
                break;
            }
        }

        if (!WasCorrectCmd)
        {
            Err.InvalidCmd = 1;
            Err.IsFatalError = 1;
            return COMPILER_VERIF(Err);
        }
    }

    // for (int i = 0; i < 16; i++)
    // {
    //     printf("label[%2d].name = %s, .codeplace = %d\n", i, CmdInfo->Labels.Labels[i].Name, CmdInfo->Labels.Labels[i].CodePlace);
    // }

    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleLabelOrError(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    size_t LabelSize = strlen(CmdInfo->Cmd);

    if (CmdInfo->Cmd[LabelSize - 1] != ':')
    {
        Err->InvalidCmd = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }

    Label Temp = {};
    LabelCtor(&Temp, CmdInfo->Cmd, CmdInfo->FileCmdQuant);

    CmdInfo->FileCmdQuant++;

    int LabelIndex = IsLabelInLabels(&CmdInfo->Labels, CmdInfo->Cmd);

    if (LabelIndex == -1)
    {
        COMPILER_RETURN_IF_ERR(PushLabel(&CmdInfo->Labels, &Temp));
        return COMPILER_VERIF(*Err);
    }
    
    Err->MoreOneEqualLables = 1;
    Err->IsFatalError = 1;

    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandlePush(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    const size_t MaxBufferSize = 16;
    char* Buffer = (char*) calloc(MaxBufferSize + 1, sizeof(char));
    Buffer[MaxBufferSize] = '\0';
    char* EndBuffer = nullptr;
    fscanf(CmdInfo->ProgrammFilePtr, "%s", Buffer);
    size_t BufferLen = strlen(Buffer);

    StackElem_t PushElem = 0;
    PushElem = (StackElem_t) strtol(Buffer, &EndBuffer, 10);

    PushType Push = {};

    if (IsInt(Buffer, EndBuffer, BufferLen))
    {
        PushTypeCtor(&Push, 1, 0, 0);
        fprintf(CmdInfo->TempFilePtr, "%d %d %d\n", push, GetPushArg(&Push), PushElem);
    }
    
    else if (IsRegister(Buffer, BufferLen))
    {
        PushTypeCtor(&Push, 0, 1, 0);
        fprintf(CmdInfo->TempFilePtr, "%d %d %d\n", push, GetPushArg(&Push), Buffer[0] - 'a');
    }

    else if (IsMemory(Buffer, BufferLen))
    {
        COMPILER_RETURN_IF_ERR(UpdateBufferForMemory(&Buffer, &BufferLen);)

        StackElem_t PushElemMemIndex = 0;
        PushElemMemIndex = (StackElem_t) strtol(Buffer, &EndBuffer, 10);

        if (IsInt(Buffer, EndBuffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 0, 1);
            fprintf(CmdInfo->TempFilePtr, "%d %d %d\n", push, GetPushArg(&Push), PushElemMemIndex);
        }

        else if (IsRegister(Buffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 1, 1);
            fprintf(CmdInfo->TempFilePtr, "%d %d %d\n", push, GetPushArg(&Push), Buffer[0] - 'a');
        }
    }

    else
    {
        Err->InvalidInputAfterPush = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }

    CmdInfo->FileCmdQuant += 3;

    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsInt(const char* Str, const char* StrEnd, const size_t StrSize)
{
    return StrEnd - Str == (int) (StrSize * sizeof(Str[0]));
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsRegister(const char* Str, const size_t StrSize)
{
    return StrSize == 2 && 'a' <= Str[0] && Str[0] <= 'd' &&  Str[1] == 'x';
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsMemory(const char* Str, const size_t StrSize)
{
    return Str[0] == '[' && Str[StrSize - 1] == ']';
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLabel(const char* Str)
{
    return Str[strlen(Str) - 1] == ':';
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetCmdName(size_t Cmd_i)
{
    return CmdFuncArr[Cmd_i].CmdName;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static  CompilerErrorType (*GetCmd(size_t Cmd_i)) (CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    return CmdFuncArr[Cmd_i].CmdFunc;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType UpdateBufferForMemory(char** Buffer, size_t* BufferSize)
{
    CompilerErrorType Err = {};
    (*Buffer)[*BufferSize - 1] = '\0';
    *BufferSize -= 2;
    (*Buffer)++;

    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandlePop(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    static const size_t MaxBufferSize = 16;
    char* Buffer = (char*) calloc(MaxBufferSize + 1, sizeof(char));
    Buffer[MaxBufferSize] = '\0';
    fscanf(CmdInfo->ProgrammFilePtr, "%s", Buffer);

    size_t BufferLen = strlen(Buffer);

    PopType Pop = {};

    if (IsRegister(Buffer, BufferLen))
    {
        PopTypeCtor(&Pop, 1, 0);
        fprintf(CmdInfo->TempFilePtr, "%d %d %d\n", pop, GetPopArg(&Pop), Buffer[0] - 'a');
    }

    else if (IsMemory(Buffer, BufferLen))
    {
        COMPILER_RETURN_IF_ERR(UpdateBufferForMemory(&Buffer, &BufferLen);)
        char* EndBuffer = NULL;
        StackElem_t PopElemMemIndex = (int) strtol(Buffer, &EndBuffer,  10);

        if (IsInt(Buffer, EndBuffer, BufferLen))
        {
            PopTypeCtor(&Pop, 0, 1);
            fprintf(CmdInfo->TempFilePtr, "%d %d %d\n", pop, GetPopArg(&Pop), PopElemMemIndex);
        }

        else if (IsRegister(Buffer, BufferLen))
        {
            PopTypeCtor(&Pop, 1, 1);
            fprintf(CmdInfo->TempFilePtr, "%d %d %d\n", pop, GetPopArg(&Pop), Buffer[0] - 'a');
        }
    }

    else
    {
        Err->InvalidInputAfterPop = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }

    CmdInfo->FileCmdQuant += 3;
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
    JmpCmdPattern(CmdInfo, jae, Err);
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
    fprintf(CmdInfo->TempFilePtr, "%d\n", add);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleSub(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d\n", sub);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleMul(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d\n", mul);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleDiv(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d\n", dive);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleOut(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d\n", out);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleHlt(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    fprintf(CmdInfo->TempFilePtr, "%d\n", hlt);
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
        fprintf(CmdInfo->TempFilePtr, "%d %d\n", JumpType, IntJumpArg);
        return COMPILER_VERIF(*Err);
    }

    int JumpArgInLabeles = IsLabelInLabels(&CmdInfo->Labels, JumpArg);

    if (JumpArgInLabeles == -1)
    {
        Label Temp = {};
        LabelCtor(&Temp, JumpArg, -1);
        COMPILER_RETURN_IF_ERR(PushLabel(&CmdInfo->Labels, &Temp));
        fprintf(CmdInfo->TempFilePtr, "%d %d\n", JumpType, -1);
        return COMPILER_VERIF(*Err);
    }

    fprintf(CmdInfo->TempFilePtr, "%d %d\n", JumpType, CmdInfo->Labels.Labels[JumpArgInLabeles].CodePlace);

    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType PushTypeCtor(PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem)
{
    CompilerErrorType Err = {};
    Push->stk = Stk;
    Push->reg = Reg;
    Push->mem = Mem;
    return Err;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushArg(PushType* Push)
{
    return *(int*) Push;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType PopTypeCtor(PopType* Pop, uint8_t Reg, uint8_t Mem)
{
    CompilerErrorType Err = {};
    Pop->reg = Reg;
    Pop->mem = Mem;
    return Err;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPopArg(PopType* Pop)
{
    return *(int*) Pop;
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

    char* Buffer = (char*) calloc(TempFileLen + 1, sizeof(char));

    if (Buffer == NULL)
    {
        Err->FailedAllocateMemoryBufferTempFile = 1;
        Err->IsFatalError = 1;
        return COMPILER_VERIF(*Err);
    }

    Buffer[TempFileLen] = '\0';
    fread(Buffer, TempFileLen, sizeof(char), TempFilePtr);
    fprintf(CodeFilePtr, "%s", Buffer);

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
    Labels->Capacity  = DefaultLabelsTableSize;
    Labels->Labels    = (Label*) calloc(DefaultLabelsTableSize, sizeof(Label));

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


    if (Lab->Name[strlen(Lab->Name) - 1] != ':')
    {
        Err.PushNotLabel = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    COLOR_PRINT(VIOLET, "in push: name = %s, codeplace = %d\n", Lab->Name, Lab->CodePlace);

    printf("first free  = %u\n", Labels->FirstFree);
    LabelCtor(&Labels->Labels[Labels->FirstFree], Lab->Name, Lab->CodePlace);

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

//---------------------------------------------------------------------------------------------------------------------------------------------------


static CompilerErrorType CmdInfoCtor(CmdDataForAsm* CmdInfo, LabelsTable* Labels, FILE* ProgrammFilePtr, FILE* CodeFilePtr, FILE* TempFilePtr)
{
    CompilerErrorType Err = {};

    *CmdInfo = 
    {
        *Labels, ProgrammFilePtr, CodeFilePtr, TempFilePtr, 0
    };

    return COMPILER_VERIF(Err);
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
        COLOR_PRINT(RED, "Error: No 'hlt'  command.\n");
    }

    if (Err->FailedAllocateMemoryBufferTempFile == 1)
    {
        COLOR_PRINT(RED, "Error: Failed allocalet memory for Buffer\nfor TempFile.\n");
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

    if (Err->PushNotLabel == 1)
    {
        COLOR_PRINT(RED, "Error: In labels array was pushed not label.\n");
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

// void CompilerDump(CmdDataForAsm* Cmd, const char* File, int Line, const char* Func)
// {
//     COLOR_PRINT(GREEN, "DUMP BEGIN\n");

//     COLOR_PRINT(CYAN, "Cmd quant = %d\n\n", Cmd->FileCmdQuant);

//     for (size_t Labels_i = 0; Labels_i < Cmd->Labels->FirstFree; Labels_i++)
//     {
//         COLOR_PRINT(VIOLET, "[%2u]\nName = %s\nCdpl = %d\n", Cmd->Labels->)
//     }

//     COLOR_PRINT(GREEN, "DUMP END\n");
//     return;
// }
