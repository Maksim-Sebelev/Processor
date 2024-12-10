#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "Asm.h"
#include "../Onegin/Onegin.h"
#include "../Stack/Stack.h"


struct AsmData
{
    const char** cmdArr;
    size_t       cmdPointer;
    size_t       codeFileSize;
    int*         cmdCodeArr;
    size_t       codePointer;
};



static void         SetCmdArrCodeElem           (AsmData* Cmd, int SetElem);
static const char*  GetNextCmd                  (AsmData* Cmd);
static AssemblerErr UpdateBufferForMemory       (char** buffer, size_t* bufferSize);


static  int           GetPushArg    (PushType* Push);
static  int           GetPopArg     (PopType*  Pop );
static  AssemblerErr  PushTypeCtor  (PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem);
static  AssemblerErr  PopTypeCtor   (PopType*  Pop , uint8_t Reg, uint8_t Mem);


static  bool  IsInt       (const char* str, const char* StrEnd, const size_t strSize);
static  bool  IsRegister  (const char* str, const size_t strSize);
static  bool  IsMemory    (const char* str, const size_t strSize);
static  bool  IsLabel     (const char* str);


static  AssemblerErr  HandlePush          (AsmData* CmdInfo);
static  AssemblerErr  HandlePop           (AsmData* CmdInfo);
static  AssemblerErr  HandleJmp           (AsmData* CmdInfo);
static  AssemblerErr  HandleJa            (AsmData* CmdInfo);
static  AssemblerErr  HandleJae           (AsmData* CmdInfo);
static  AssemblerErr  HandleJb            (AsmData* CmdInfo);
static  AssemblerErr  HandleJbe           (AsmData* CmdInfo);
static  AssemblerErr  HandleJe            (AsmData* CmdInfo);
static  AssemblerErr  HandleJne           (AsmData* CmdInfo);
static  AssemblerErr  HandleAdd           (AsmData* CmdInfo);
static  AssemblerErr  HandleSub           (AsmData* CmdInfo);
static  AssemblerErr  HandleMul           (AsmData* CmdInfo);
static  AssemblerErr  HandleDiv           (AsmData* CmdInfo);
static  AssemblerErr  HandleOut           (AsmData* CmdInfo);
static  AssemblerErr  HandleHlt           (AsmData* CmdInfo);
static  AssemblerErr  HandleLabel         (AsmData* CmdInfo);


//--------------------------------------------------------------------------------------------------------------------------------------------------

struct CmdFunc
{
    const char*    CmdName;
    AssemblerErr (*CmdFunc) (AsmData*);
};

//--------------------------------------------------------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------------------------------------------------------

static const size_t CmdFuncQuant = sizeof(CmdFuncArr) / sizeof(CmdFuncArr[0]);

//--------------------------------------------------------------------------------------------------------------------------------------------------


#define COMPILER_VERIF(AsmData, err) Verif(AsmData, &err, __FILE__, __LINE__, __func__)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr (*GetCmd(size_t cmdPointer)) (AsmData* CmdInfo)
{
    return CmdFuncArr[cmdPointer].CmdFunc;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

AssemblerErr RunAssembler(const char* inputStream, const char* outputStream)
{
    assert(inputStream);
    assert(outputStream);

    AssemblerErr err = {};

    FILE* inputFile  = fopen(inputStream, "rb");
    FILE* outputFile = fopen(inputStream, "wb");

    RETURN_IF_FALSE(inputFile,  err, err.err = AssemblerErrorType::FAILED_OPEN_INPUT_STREAM);
    RETURN_IF_FALSE(outputFile, err, err.err = AssemblerErrorType::FAILED_OPERN_OUTPUT_STREAM);

    

    return err;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePush(AsmData* CmdInfo)
{
    assert(CmdInfo);

    AssemblerErr err = {};

    const char* buffer = GetNextCmd(CmdInfo);
    size_t BufferLen = strlen(buffer);
    char* EndBuffer = nullptr;

    StackElem_t PushElem = (StackElem_t) strtol(buffer, &EndBuffer, 10);
    PushType Push = {};

    if (IsInt(buffer, EndBuffer, BufferLen))
    {
        PushTypeCtor(&Push, 1, 0, 0);
        SetCmdArrCodeElem(CmdInfo, push);
        SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
        SetCmdArrCodeElem(CmdInfo, PushElem);
    }

    else if (IsRegister(buffer, BufferLen))
    {
        PushTypeCtor(&Push, 0, 1, 0);
        SetCmdArrCodeElem(CmdInfo, push);
        SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
        SetCmdArrCodeElem(CmdInfo, buffer[0] - 'a');
    }

    else if (IsMemory(buffer, BufferLen))
    {
        // COMPILER_RETURN_IF_ERR(UpdateBufferForMemory(&buffer, &BufferLen));

        StackElem_t PushElemMemIndex = 0;
        PushElemMemIndex = (StackElem_t) strtol(buffer, &EndBuffer, 10);

        if (IsInt(buffer, EndBuffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 0, 1);
            SetCmdArrCodeElem(CmdInfo, push);
            SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
            SetCmdArrCodeElem(CmdInfo, PushElemMemIndex);
        }

        else if (IsRegister(buffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 1, 1);
            SetCmdArrCodeElem(CmdInfo, push);
            SetCmdArrCodeElem(CmdInfo, GetPushArg(&Push));
            SetCmdArrCodeElem(CmdInfo, buffer[0] - 'a');
        }
    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
        return COMPILER_VERIF(CmdInfo, err);
    }

    CmdInfo->codeFileSize += 3;

    return COMPILER_VERIF(CmdInfo, err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr UpdateBufferForMemory(const char** buffer, size_t* bufferSize)
// {
//     assert(buffer);
//     assert(bufferSize);

//     AssemblerErr err = {};
//     (*buffer)[*bufferSize - 1] = '\0';
//     *bufferSize -= 2;
//     (*buffer)++;

//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr UpdateBufferForMemory(char** buffer, size_t* bufferSize)
// {
//     assert(buffer);
//     assert(bufferSize);

//     AssemblerErr err = {};
//     (*buffer)[*bufferSize - 1] = '\0';
//     *bufferSize -= 2;
//     (*buffer)++;

//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePop(AsmData* CmdInfo, AssemblerErr* err)
{
    assert(CmdInfo);
    assert(err);

    const char* buffer = GetNextCmd(CmdInfo);
    size_t BufferLen = strlen(buffer);

    PopType Pop = {};

    if (IsRegister(buffer, BufferLen))
    {
        PopTypeCtor(&Pop, 1, 0);
        SetCmdArrCodeElem(CmdInfo, pop);
        SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
        SetCmdArrCodeElem(CmdInfo, buffer[0] - 'a');
    }

    else if (IsMemory(buffer, BufferLen))
    {
        // COMPILER_RETURN_IF_ERR(UpdateBufferForMemory(&buffer, &BufferLen);)
        char* EndBuffer = nullptr;
        StackElem_t PopElemMemIndex = (int) strtol(buffer, &EndBuffer,  10);

        if (IsInt(buffer, EndBuffer, BufferLen))
        {
            PopTypeCtor(&Pop, 0, 1);
            SetCmdArrCodeElem(CmdInfo, pop);
            SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
            SetCmdArrCodeElem(CmdInfo, PopElemMemIndex);
        }

        else if (IsRegister(buffer, BufferLen))
        {
            PopTypeCtor(&Pop, 1, 1);
            SetCmdArrCodeElem(CmdInfo, pop);
            SetCmdArrCodeElem(CmdInfo, GetPopArg(&Pop));
            SetCmdArrCodeElem(CmdInfo, buffer[0] - 'a');
        }
    }

    else
    {
        err->err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
        return COMPILER_VERIF(CmdInfo, *err);
    }

    CmdInfo->codeFileSize += 3;

    return COMPILER_VERIF(CmdInfo, *err);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleJmp(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jmp, err);
//     return COMPILER_VERIF(*err);  
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleJa(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, ja, err);
//     return COMPILER_VERIF(*err);  
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleJae(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jae, err);
//     return COMPILER_VERIF(*err);    
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleJb(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jb, err);
//     return COMPILER_VERIF(*err);    
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleJbe(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jbe, err);
//     return COMPILER_VERIF(*err);    
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleJe(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, je, err);
//     return COMPILER_VERIF(*err);  
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleJne(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     JmpCmdPattern(CmdInfo, jne, err);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleAdd(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, add);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleSub(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, sub);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleMul(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, mul);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleDiv(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, dive);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleOut(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, out);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr HandleHlt(AsmData* CmdInfo, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     NullArgCmdPattern(CmdInfo, hlt);
//     return COMPILER_VERIF(*err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr NullArgCmdPattern(AsmData* CmdInfo, Cmd Cmd)
// {
//     assert(CmdInfo);

//     AssemblerErr err = {};
//     SetCmdArrCodeElem(CmdInfo, Cmd);
//     CmdInfo->FileCmdQuant++;
//     return COMPILER_VERIF(err);
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr JmpCmdPattern(AsmData* CmdInfo, Cmd JumpType, AssemblerErr* err)
// {
//     assert(CmdInfo);
//     assert(err);

//     char* JumpArg = GetNextCmd(CmdInfo);
//     CmdInfo->FileCmdQuant += 2;

//     char*  JumpArgEndPtr = nullptr;
//     int    IntJumpArg    = (int) strtol(JumpArg, &JumpArgEndPtr, 10);
//     size_t JumpArgLen    = strlen(JumpArg);
//     int    JumpArgNumLen = (int) (JumpArgEndPtr - JumpArg);

//     if (JumpArgNumLen == (int) JumpArgLen)
//     {
//         SetCmdArrCodeElem(CmdInfo, JumpType);
//         SetCmdArrCodeElem(CmdInfo, IntJumpArg);

//         return COMPILER_VERIF(*err);
//     }

//     int JumpArgInLabeles = IsLabelInLabels(&CmdInfo->Labels, JumpArg);

//     if (JumpArgInLabeles == -1)
//     {
//         Label Temp = {};
//         LabelCtor(&Temp, JumpArg, -1);
//         COMPILER_RETURN_IF_ERR(PushLabel(&CmdInfo->Labels, &Temp));
//         SetCmdArrCodeElem(CmdInfo, JumpType);
//         SetCmdArrCodeElem(CmdInfo, -1);

//         LabelDtor(&Temp);   
//         return COMPILER_VERIF(*err);
//     }

//     int LabelCodePlace = GetLabelCodePlace(CmdInfo, JumpArgInLabeles);
//     SetCmdArrCodeElem(CmdInfo, JumpType);
//     SetCmdArrCodeElem(CmdInfo, LabelCodePlace);

//     return COMPILER_VERIF(*err);
// }

// static int GetLabelCodePlace(AsmData* CmdInfo, int Labels_i)
// {
//     assert(CmdInfo);

//     return CmdInfo->Labels.Labels[Labels_i].CodePlace;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr PushTypeCtor(PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem)
// {
//     assert(Push);

//     AssemblerErr err = {};
//     Push->stk = Stk ? 1 : 0;
//     Push->reg = Reg ? 1 : 0;
//     Push->mem = Mem ? 1 : 0;
//     return err;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static int GetPushArg(PushType* Push)
// {
//     assert(Push);

//     return *(int*) Push;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr PopTypeCtor(PopType* Pop, uint8_t Reg, uint8_t Mem)
// {
//     assert(Pop);
//     AssemblerErr err = {};
//     Pop->reg = Reg ? 1 : 0;
//     Pop->mem = Mem ? 1 : 0;
//     return err;
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static int GetPopArg(PopType* Pop)
// {
//     assert(Pop);
//     return *(int*) Pop;
// }

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsInt(const char* str, const char* StrEnd, size_t strSize)
{
    assert(str);
    assert(StrEnd);

    return StrEnd - str == (int) (strSize * sizeof(str[0]));
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsRegister(const char* str, const size_t strSize)
{
    assert(str);
    return strSize == 2 && 'a' <= str[0] && str[0] <= 'd' &&  str[1] == 'x';
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsMemory(const char* str, const size_t strSize)
{
    assert(str);
    return str[0] == '[' && str[strSize - 1] == ']';
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLabel(const char* str)
{
    assert(str);
    return str[strlen(str) - 1] == ':';
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetCmdName(size_t cmdPointer)
{
    return CmdFuncArr[cmdPointer].CmdName;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetCmdArrCodeElem(AsmData* Cmd, int SetElem)
{
    assert(Cmd);
    size_t pointer = Cmd->codePointer;
    Cmd->cmdCodeArr[pointer] = SetElem;
    Cmd->codePointer++;
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetNextCmd(AsmData* Cmd)
{
    assert(Cmd);

    Cmd->cmdPointer++;
    return Cmd->cmdArr[Cmd->cmdPointer - 1];
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr Verif(AsmData* AsmDataInfo, AssemblerErr* err, const char* file, const int line, const char* func)
{
    assert(AsmDataInfo);
    assert(err);
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);



    return *err;
}