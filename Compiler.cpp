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
    char* Name;
    int   CodePlace;    
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
    const char*   ProgrammFileName;
    FILE*         CodeFilePtr;
    int           FileCmdQuant;
    const char*         Cmd;
    size_t        ProgrammFileLen;
    char**        CmdArr;
    size_t        ProgrammCmdQuant;
    size_t        Cmd_i;
    int*          CmdCodeArr;
    size_t        CmdCodeArr_i;
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

static  CompilerErrorType  ReadCmdFromFile          (CmdDataForAsm* Cmd);
static  CompilerErrorType  WriteCmdCodeArrInFile    (CmdDataForAsm* Cmd);
static  char               GetBufferElem            (char* Buffer, size_t buffer_i);
static  char*              GetBufferElemPtr         (char* Buffer, size_t buffer_i);
static  char*              GetNextCmd               (CmdDataForAsm* Cmd);
static  void               SetCmdArr                (CmdDataForAsm* Cmd, size_t* CmdArr_i, char* SetElem);
static  void               SetProgrammCmdQuant      (CmdDataForAsm* Cmd, size_t SetElem);
static  void               SetCmdArrCodeElem        (CmdDataForAsm* Cmd, int SetElem);
static  size_t             CalcFileLen              (const char* FileName);
static  void               CloseFiles               (FILE* ProgrammFilePtr, FILE* CodeFilePtr);

static  void               LabelCtor          (Label* Lab, const char* Name, int CodePlace);
static  void               LabelDtor          (Label* Lab);
static  int                IsLabelInLabels    (const LabelsTable* Labels, const char* LabelName);
static  CompilerErrorType  LabelsTableCtor    (LabelsTable* Labels);
static  CompilerErrorType  LabelsTableDtor    (LabelsTable* Labels);
static  CompilerErrorType  PushLabel          (LabelsTable* Labels, const Label* Lab);
static  int                GetLabelCodePlace  (CmdDataForAsm* CmdInfo, int Labels_i);
static  CompilerErrorType  FindLabels         (CmdDataForAsm* Cmd);

static  CompilerErrorType  NullArgCmdPattern  (CmdDataForAsm* CmdInfo, Cmd Cmd);
static  CompilerErrorType  JmpCmdPattern      (CmdDataForAsm* CmdInfo, Cmd JumpType, CompilerErrorType* Err);

static  int                GetPushArg    (PushType* Push);
static  int                GetPopArg     (PopType*  Pop );
static  CompilerErrorType  PushTypeCtor  (PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem);
static  CompilerErrorType  PopTypeCtor   (PopType*  Pop , uint8_t Reg, uint8_t Mem);

static  CompilerErrorType  CmdInfoCtor  (CmdDataForAsm* CmdInfo, LabelsTable* Labels, FILE* ProgrammFilePtr, const char* ProgrammFileName, FILE* CodeFilePtr);
static  CompilerErrorType  CmdInfoDtor  (CmdDataForAsm* CmdInfo);

static  bool  IsInt       (const char* Str, const char* StrEnd, const size_t StrSize);
static  bool  IsRegister  (const char* Str, const size_t StrSize);
static  bool  IsMemory    (const char* Str, const size_t StrSize);
static  bool  IsLabel     (const char* Str);

static  const char*        GetCmdName              (size_t Cmd_i);
static  CompilerErrorType  (*GetCmd(size_t Cmd_i)) (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);
static  CompilerErrorType  UpdateBufferForMemory   (char** Buffer, size_t* BufferSize);

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
static  CompilerErrorType  HandleLabel         (CmdDataForAsm* CmdInfo, CompilerErrorType* Err);


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


    LabelsTable Labels = {};
    LabelsTableCtor(&Labels);

    CmdDataForAsm CmdInfo = {};

    COMPILER_RETURN_IF_ERR(CmdInfoCtor(&CmdInfo, &Labels, ProgrammFilePtr, File->ProgrammFile, CodeFilePtr));

    COMPILER_RETURN_IF_ERR(ReadCmdFromFile(&CmdInfo));

    COMPILER_RETURN_IF_ERR(FindLabels(&CmdInfo));

    COMPILER_RETURN_IF_ERR(RunAssembler(&CmdInfo));

    COMPILER_RETURN_IF_ERR(WriteCmdCodeArrInFile(&CmdInfo));

    COMPILER_RETURN_IF_ERR(CmdInfoDtor(&CmdInfo));
    // LabelsTableDtor(&Labels); 
    CloseFiles(ProgrammFilePtr, CodeFilePtr);

    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType RunAssembler(CmdDataForAsm* CmdInfo)
{
    CompilerErrorType Err = {};

    CmdInfo->CmdCodeArr = (int*) calloc(100, sizeof(int));

    while (CmdInfo->Cmd_i < CmdInfo->ProgrammCmdQuant)
    {
        CmdInfo->Cmd = GetNextCmd(CmdInfo);
        bool WasCorrectCmd = false;

        for (size_t CmdFuncArr_i = 0; CmdFuncArr_i < CmdFuncQuant; CmdFuncArr_i++)
        {
            const char* CmdName = GetCmdName(CmdFuncArr_i);

            if (strcmp(CmdName, CmdInfo->Cmd) == 0)
            {
                WasCorrectCmd = true;
                COMPILER_RETURN_IF_ERR(GetCmd(CmdFuncArr_i)(CmdInfo, &Err));
                break;
            }

            if (IsLabel(CmdInfo->Cmd))
            {
                WasCorrectCmd = true;
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

    // for (int i = 0; i < CmdInfo->FileCmdQuant; i++)
    // {
    //     printf("%d ", CmdInfo->CmdCodeArr[i]);
    // }
    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleLabel(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
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

    int LabelIndex = IsLabelInLabels(&CmdInfo->Labels, CmdInfo->Cmd);
    
    if (LabelIndex == -1)
    {
        COMPILER_RETURN_IF_ERR(PushLabel(&CmdInfo->Labels, &Temp));
        LabelDtor(&Temp);
        return COMPILER_VERIF(*Err);
    }
    
    LabelDtor(&Temp);

    Err->MoreOneEqualLables = 1;
    Err->IsFatalError = 1;


    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandlePush(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    char* Buffer = GetNextCmd(CmdInfo);
    size_t BufferLen = strlen(Buffer);
    char* EndBuffer = nullptr;

    StackElem_t PushElem = 0;
    PushElem = (StackElem_t) strtol(Buffer, &EndBuffer, 10);

    PushType Push = {};
    
    if (IsInt(Buffer, EndBuffer, BufferLen))
    {
        PushTypeCtor(&Push, 1, 0, 0);
        SetCmdArrCodeElem(CmdInfo, push);
        SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
        SetCmdArrCodeElem(CmdInfo, PushElem);
    }
    
    else if (IsRegister(Buffer, BufferLen))
    {
        PushTypeCtor(&Push, 0, 1, 0);
        SetCmdArrCodeElem(CmdInfo, push);
        SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
        SetCmdArrCodeElem(CmdInfo, Buffer[0] - 'a');
    }

    else if (IsMemory(Buffer, BufferLen))
    {
        COMPILER_RETURN_IF_ERR(UpdateBufferForMemory(&Buffer, &BufferLen);)

        StackElem_t PushElemMemIndex = 0;
        PushElemMemIndex = (StackElem_t) strtol(Buffer, &EndBuffer, 10);

        if (IsInt(Buffer, EndBuffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 0, 1);
            SetCmdArrCodeElem(CmdInfo, push);
            SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
            SetCmdArrCodeElem(CmdInfo, PushElemMemIndex);
        }

        else if (IsRegister(Buffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 1, 1);
            SetCmdArrCodeElem(CmdInfo, push);
            SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
            SetCmdArrCodeElem(CmdInfo, Buffer[0] - 'a');
        }
    }

    else
    {
        Err->InvalidInputAfterPush = 1;
        Err->IsFatalError = 1;
        FREE(Buffer);
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
    char* Buffer = GetNextCmd(CmdInfo);
    size_t BufferLen = strlen(Buffer);

    PopType Pop = {};

    if (IsRegister(Buffer, BufferLen))
    {
        PopTypeCtor(&Pop, 1, 0);
        SetCmdArrCodeElem(CmdInfo, pop);
        SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
        SetCmdArrCodeElem(CmdInfo, Buffer[0] - 'a');
    }

    else if (IsMemory(Buffer, BufferLen))
    {
        COMPILER_RETURN_IF_ERR(UpdateBufferForMemory(&Buffer, &BufferLen);)
        char* EndBuffer = NULL;
        StackElem_t PopElemMemIndex = (int) strtol(Buffer, &EndBuffer,  10);

        if (IsInt(Buffer, EndBuffer, BufferLen))
        {
            PopTypeCtor(&Pop, 0, 1);
            SetCmdArrCodeElem(CmdInfo, pop);
            SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
            SetCmdArrCodeElem(CmdInfo, PopElemMemIndex);
        }

        else if (IsRegister(Buffer, BufferLen))
        {
            PopTypeCtor(&Pop, 1, 1);
            SetCmdArrCodeElem(CmdInfo, pop);
            SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
            SetCmdArrCodeElem(CmdInfo, Buffer[0] - 'a');
        }
    }

    else
    {
        Err->InvalidInputAfterPop = 1;
        Err->IsFatalError = 1;
        FREE(Buffer);
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
    NullArgCmdPattern(CmdInfo, add);
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleSub(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    NullArgCmdPattern(CmdInfo, sub);
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleMul(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    NullArgCmdPattern(CmdInfo, mul);
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleDiv(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    NullArgCmdPattern(CmdInfo, dive);
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleOut(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    NullArgCmdPattern(CmdInfo, out);
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType HandleHlt(CmdDataForAsm* CmdInfo, CompilerErrorType* Err)
{
    NullArgCmdPattern(CmdInfo, hlt);
    return COMPILER_VERIF(*Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType NullArgCmdPattern(CmdDataForAsm* CmdInfo, Cmd Cmd)
{
    CompilerErrorType Err = {};
    SetCmdArrCodeElem(CmdInfo, Cmd);
    CmdInfo->FileCmdQuant++;
    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType JmpCmdPattern(CmdDataForAsm* CmdInfo, Cmd JumpType, CompilerErrorType* Err)
{
    char* JumpArg = GetNextCmd(CmdInfo);
    CmdInfo->FileCmdQuant += 2;

    char*  JumpArgEndPtr = NULL;
    int    IntJumpArg    = (int) strtol(JumpArg, &JumpArgEndPtr, 10);
    size_t JumpArgLen    = strlen(JumpArg);
    int    JumpArgNumLen = (int) (JumpArgEndPtr - JumpArg);

    if (JumpArgNumLen == (int) JumpArgLen)
    {
        SetCmdArrCodeElem(CmdInfo, JumpType);
        SetCmdArrCodeElem(CmdInfo, IntJumpArg);

        return COMPILER_VERIF(*Err);
    }

    int JumpArgInLabeles = IsLabelInLabels(&CmdInfo->Labels, JumpArg);

    if (JumpArgInLabeles == -1)
    {
        Label Temp = {};
        LabelCtor(&Temp, JumpArg, -1);
        COMPILER_RETURN_IF_ERR(PushLabel(&CmdInfo->Labels, &Temp));
        SetCmdArrCodeElem(CmdInfo, JumpType);
        SetCmdArrCodeElem(CmdInfo, -1);

        LabelDtor(&Temp);   
        return COMPILER_VERIF(*Err);
    }

    int LabelCodePlace = GetLabelCodePlace(CmdInfo, JumpArgInLabeles);
    SetCmdArrCodeElem(CmdInfo, JumpType);
    SetCmdArrCodeElem(CmdInfo, LabelCodePlace);

    return COMPILER_VERIF(*Err);
}

static int GetLabelCodePlace(CmdDataForAsm* CmdInfo, int Labels_i)
{
    return CmdInfo->Labels.Labels[Labels_i].CodePlace;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType PushTypeCtor(PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem)
{
    CompilerErrorType Err = {};
    Push->stk = Stk ? 1 : 0;
    Push->reg = Reg ? 1 : 0;
    Push->mem = Mem ? 1 : 0;
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
    Pop->reg = Reg ? 1 : 0;
    Pop->mem = Mem ? 1 : 0;
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
    return (size_t) Buf.st_size;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void CloseFiles(FILE* ProgrammFilePtr, FILE* CodeFilePtr)
{
    fclose(ProgrammFilePtr);
    fclose(CodeFilePtr);
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void LabelCtor(Label* Lab, const char* Name, int CodePlace)
{
    Lab->CodePlace = CodePlace;
    Lab->Name      = strdup(Name);
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void LabelDtor(Label* Lab)
{
    Lab->CodePlace = -1;
    FREE(Lab->Name);
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

static CompilerErrorType LabelsTableDtor(LabelsTable* Labels)
{
    CompilerErrorType Err = {};
    Labels->FirstFree = 0;
    Labels->Capacity  = 0;

    for (size_t Labels_i = 0; Labels_i < Labels->Capacity; Labels_i++)
    {
        LabelDtor(&Labels->Labels[Labels_i]);
    }

    FREE(Labels->Labels);

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

    LabelCtor(&Labels->Labels[Labels->FirstFree], Lab->Name, Lab->CodePlace);

    Labels->FirstFree++;

    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

//возвращает индекс первой встречи, иначе ммнус -1 
static int IsLabelInLabels(const LabelsTable* Labels, const char* LabelName)
{
    for (int Label_i = 0; Label_i < (int) Labels->FirstFree; Label_i++)
    {
        if (strcmp(Labels->Labels[Label_i].Name, LabelName) == 0)
        {
            return Label_i; 
        }
    }
    return -1;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType FindLabels(CmdDataForAsm* CmdInfo)
{
    CompilerErrorType Err = {};
    for (size_t CmdFuncArr_i = 0; CmdFuncArr_i < CmdFuncQuant; CmdFuncArr_i++)
    {
        CmdInfo->Cmd = GetCmdName(CmdFuncArr_i);
        if (IsLabel(CmdInfo->Cmd))
        {
            COMPILER_RETURN_IF_ERR(HandleLabel(CmdInfo, &Err));
            break;
        }
    }
    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType CmdInfoCtor(CmdDataForAsm* CmdInfo, LabelsTable* Labels, FILE* ProgrammFilePtr, const char* ProgrammFileName, FILE* CodeFilePtr)
{
    CompilerErrorType Err = {};

    CmdInfo->Labels             = *Labels;
    CmdInfo->ProgrammFilePtr    = ProgrammFilePtr;
    CmdInfo->ProgrammFileName   = ProgrammFileName;
    CmdInfo->CodeFilePtr        = CodeFilePtr;
    CmdInfo->Cmd                = NULL;
    CmdInfo->CmdArr             = NULL;
    CmdInfo->CmdCodeArr         = NULL;
    CmdInfo->FileCmdQuant       = 0;
    CmdInfo->ProgrammFileLen    = 0;
    CmdInfo->ProgrammCmdQuant   = 0;
    CmdInfo->Cmd_i              = 0;
    CmdInfo->CmdCodeArr_i       = 0;

    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType CmdInfoDtor(CmdDataForAsm* CmdInfo)
{
    CompilerErrorType Err = {};
    FREE(CmdInfo->CmdArr);
    FREE(CmdInfo->CmdCodeArr);
    LabelsTableDtor(&CmdInfo->Labels);
    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static CompilerErrorType ReadCmdFromFile(CmdDataForAsm* Cmd)
{
    CompilerErrorType Err = {};

    size_t BufferLen = CalcFileLen(Cmd->ProgrammFileName);

    char* Buffer     = (char*)  calloc(BufferLen + 1, sizeof(char));
    Cmd->CmdArr      = (char**) calloc(BufferLen + 1, sizeof(char*));

    if (Buffer == NULL)
    {
        Err.FailedAllocateMemoryBufferTempFile = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    Cmd->ProgrammFileLen = BufferLen;

    fread(Buffer, sizeof(char), BufferLen, Cmd->ProgrammFilePtr);
    Buffer[BufferLen] = '\0';

    char Temp = GetBufferElem(Buffer, 0);
    if (Temp == ' ' || Temp == '\n')
    {
        Err.SyntaxisError = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    Temp = GetBufferElem(Buffer, BufferLen - 1);
    if (Temp != '\n')
    {
        Err.SyntaxisError = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }


    size_t CmdArr_i = 0;
    SetCmdArr(Cmd, &CmdArr_i, GetBufferElemPtr(Buffer, 0));

    for (size_t buffer_i = 0; buffer_i <= BufferLen; buffer_i++)
    {
        Temp = GetBufferElem(Buffer, buffer_i);
        if (Temp == '\n' || Temp == ' ')
        {
            while ((Temp == '\n' || Temp == ' ') && buffer_i <= BufferLen)
            {
                Buffer[buffer_i] = '\0';
                buffer_i++;
                Temp = GetBufferElem(Buffer, buffer_i);
            }
            SetCmdArr(Cmd, &CmdArr_i, GetBufferElemPtr(Buffer, buffer_i));
        }
    }

    SetProgrammCmdQuant(Cmd, CmdArr_i - 1);
    Cmd->CmdArr = (char**) realloc(Cmd->CmdArr, Cmd->ProgrammCmdQuant * sizeof(char*));

    if (Cmd->CmdArr == NULL)
    {
        Err.FailedReallocateMemoryToCmdCodeArr = 1;
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }

    for (size_t i = 0; i < Cmd->ProgrammCmdQuant; i++)
    {
        printf("%s\n", Cmd->CmdArr[i]);
    }

    // FREE(Buffer);

    return COMPILER_VERIF(Err);
}


static CompilerErrorType WriteCmdCodeArrInFile(CmdDataForAsm* Cmd)
{
    CompilerErrorType Err = {};

    fprintf(Cmd->CodeFilePtr, "%d\n", Cmd->FileCmdQuant);

    if (fwrite(Cmd->CmdCodeArr, sizeof(int), (size_t) Cmd->FileCmdQuant, Cmd->CodeFilePtr) != (size_t) Cmd->FileCmdQuant)
    {
        Err.IsFatalError = 1;
        return COMPILER_VERIF(Err);
    }
    return COMPILER_VERIF(Err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static char GetBufferElem(char* Buffer, size_t buffer_i)
{
    return Buffer[buffer_i];
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static char* GetBufferElemPtr(char* Buffer, size_t buffer_i)
{
    return &Buffer[buffer_i];
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void SetCmdArr(CmdDataForAsm* Cmd, size_t* CmdArr_i, char* SetElem)
{
    Cmd->CmdArr[*CmdArr_i] = SetElem;
    (*CmdArr_i)++;
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static char* GetNextCmd(CmdDataForAsm* Cmd)
{
    Cmd->Cmd_i++;
    return Cmd->CmdArr[Cmd->Cmd_i - 1];
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void SetProgrammCmdQuant(CmdDataForAsm* Cmd, size_t SetElem)
{
    Cmd->ProgrammCmdQuant = SetElem;
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

static void SetCmdArrCodeElem(CmdDataForAsm* Cmd, int SetElem)
{
    Cmd->CmdCodeArr[Cmd->CmdCodeArr_i] = SetElem;
    Cmd->CmdCodeArr_i++;
    return;
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

    if (Err->SyntaxisError == 1)
    {
        COLOR_PRINT(RED, "Error: Invalid syntaxis.\n");
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

    if (Err->FailedReallocateMemoryToCmdCodeArr == 1)
    {
        COLOR_PRINT(RED, "Error: Failed to realloc memory for cmd.\n");  
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
