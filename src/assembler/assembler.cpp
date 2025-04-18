#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "assembler/assembler.hpp"
#include "fileread/fileread.hpp"
#include "common/globalInclude.hpp"
#include "lib/lib.hpp"
#include "stack/stack.hpp"
#include "lib/colorPrint.hpp"
#include "log/log.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct CmdArr
{
    size_t       size;
    size_t       pointer;
    const char** cmd;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct CodeArr
{
    size_t size;
    size_t pointer;
    int*   code;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct Label
{
    const char* name;
    size_t      codePlace;
    bool        alradyDefined;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct Labels
{
    size_t size;
    size_t capacity;
    size_t pointer;
    Label* labels;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct AsmData
{
    CmdArr  cmd;
    CodeArr code;
    Labels  labels;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr AsmDataCtor           (AsmData* AsmDataInfo, const IOfile* file);
static AssemblerErr AsmDataDtor           (AsmData* AsmDataInfo);
static AssemblerErr WriteCmdInCodeArr     (AsmData* AsmDataInfo);
static AssemblerErr WriteCodeArrInFile    (AsmData* AsmDataInfo, const IOfile* file);


static void         SetCmdArrCodeElem     (AsmData* AsmDataInfo, int SetElem);
static const char*  GetNextCmd            (AsmData* AsmDataInfo);
static void         UpdateBufferForMemory (const char** buffer, size_t* bufferSize);

static const char * GetCmdName            (size_t cmdPointer);

static AssemblerErr NullArgCmdPattern     (AsmData* AsmDataInfo, Cmd cmd);

static AssemblerErr PpMmPattern           (AsmData* AsmDataInfo, Cmd cmd);


static int           GetPushArg           (PushType* Push);
static int           GetPopArg            (PopType*  Pop );
static void          PushTypeCtor         (PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem, uint8_t Sum);
static void          PopTypeCtor          (PopType*  Pop , uint8_t Reg, uint8_t Mem, uint8_t Sum);

static int           GetRegisterPointer   (const char* buffer);
static char          GetChar              (const char* buffer, size_t bufferSize);

static bool          IsCharNum            (char c);
static bool          IsStrInt             (const char* str);
static bool          IsInt                (const char* str, const char* StrEnd, size_t strSize);
static bool          IsChar               (const char* str, const char* StrEnd, size_t strSize);
static bool          IsRegister           (const char* str,                     size_t strSize);
static bool          IsMemory             (const char* str,                     size_t strSize);
static bool          IsSum                (const char* str,                     size_t strSize);
static bool          IsLabel              (const char* str);
static bool          IsCommentBegin       (const char* str);
static bool          IsCommentEnd         (const char* str);


static AssemblerErr InitAllLabels         (AsmData* AsmDataInfo);
static AssemblerErr LabelsCtor            (AsmData* AsmDataInfo);
static AssemblerErr LabelsDtor            (AsmData* AsmDataInfo);

static Label        LabelCtor            (const char* name, size_t pointer, bool alreadyDefined);
static AssemblerErr PushLabel            (  AsmData* AsmDataInfo, const Label* label);
static bool         IsLabelAlready       (const AsmData* AsmDataInfo, const char* label, size_t* labelPlace);


static bool         FindDefaultCmd       (const char* cmd, size_t* defaultCmdPointer);
static size_t       CalcCodeSize         (const CmdArr* cmd);

static AssemblerErr JmpCmdPattern        (AsmData* AsmDataInfo, Cmd JumpType);


static AssemblerErr HandlePush           (AsmData* AsmDataInfo);
static AssemblerErr HandlePop            (AsmData* AsmDataInfo);
static AssemblerErr HandleJmp            (AsmData* AsmDataInfo);
static AssemblerErr HandleJa             (AsmData* AsmDataInfo);
static AssemblerErr HandleJae            (AsmData* AsmDataInfo);
static AssemblerErr HandleJb             (AsmData* AsmDataInfo);
static AssemblerErr HandleJbe            (AsmData* AsmDataInfo);
static AssemblerErr HandleJe             (AsmData* AsmDataInfo);
static AssemblerErr HandleJne            (AsmData* AsmDataInfo);
static AssemblerErr HandleCall           (AsmData* AsmDataInfo);
static AssemblerErr HandleRet            (AsmData* AsmdataInfo);
static AssemblerErr HandleAdd            (AsmData* AsmDataInfo);
static AssemblerErr HandleSub            (AsmData* AsmDataInfo);
static AssemblerErr HandleMul            (AsmData* AsmDataInfo);
static AssemblerErr HandleDiv            (AsmData* AsmDataInfo);
static AssemblerErr HandlePp             (AsmData* AsmDataInfo);
static AssemblerErr HandleMm             (AsmData* AsmDataInfo);
static AssemblerErr HandleOut            (AsmData* AsmDataInfo);
static AssemblerErr HandleOutc           (AsmData* AsmDataInfo);
static AssemblerErr HandleOutr           (AsmData* AsmDataInfo);
static AssemblerErr HandleOutrc          (AsmData* AsmDataInfo);
static AssemblerErr HandleDraw           (AsmData* AsmDataInfo);
static AssemblerErr HandleHlt            (AsmData* AsmDataInfo);
static AssemblerErr HandleLabel          (AsmData* AsmDataInfo);
static AssemblerErr HandleComment        (AsmData* AsmDataInfo);


static AssemblerErr Verif                (AsmData* AsmDataInfo, AssemblerErr* err, const char* file, int line, const char* func);
static void         PrintError           (AssemblerErr* err);


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct CmdFunc
{
    const char*    CmdName;
    AssemblerErr (*CmdFunc) (AsmData*);
    size_t         argQuant;
    size_t         codeRecordSize;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const CmdFunc DefaultCmd[] =
{
    {"push" ,  HandlePush , CmdInfoArr[ Cmd::push  ].argQuant, CmdInfoArr[ Cmd::push  ].codeRecordSize},
    {"pop"  ,  HandlePop  , CmdInfoArr[ Cmd::pop   ].argQuant, CmdInfoArr[ Cmd::pop   ].codeRecordSize},
    {"jmp"  ,  HandleJmp  , CmdInfoArr[ Cmd::jmp   ].argQuant, CmdInfoArr[ Cmd::jmp   ].codeRecordSize},
    {"ja"   ,  HandleJa   , CmdInfoArr[ Cmd::ja    ].argQuant, CmdInfoArr[ Cmd::ja    ].codeRecordSize},
    {"jae"  ,  HandleJae  , CmdInfoArr[ Cmd::jae   ].argQuant, CmdInfoArr[ Cmd::jae   ].codeRecordSize},
    {"jb"   ,  HandleJb   , CmdInfoArr[ Cmd::jb    ].argQuant, CmdInfoArr[ Cmd::jb    ].codeRecordSize},
    {"jbe"  ,  HandleJbe  , CmdInfoArr[ Cmd::jbe   ].argQuant, CmdInfoArr[ Cmd::jbe   ].codeRecordSize},
    {"je"   ,  HandleJe   , CmdInfoArr[ Cmd::je    ].argQuant, CmdInfoArr[ Cmd::je    ].codeRecordSize},
    {"jne"  ,  HandleJne  , CmdInfoArr[ Cmd::jne   ].argQuant, CmdInfoArr[ Cmd::jne   ].codeRecordSize},
    {"draw" ,  HandleDraw , CmdInfoArr[ Cmd::draw  ].argQuant, CmdInfoArr[ Cmd::draw  ].codeRecordSize},
    {"call" ,  HandleCall , CmdInfoArr[ Cmd::call  ].argQuant, CmdInfoArr[ Cmd::call  ].codeRecordSize},
    {"ret"  ,  HandleRet  , CmdInfoArr[ Cmd::ret   ].argQuant, CmdInfoArr[ Cmd::ret   ].codeRecordSize},
    {"add"  ,  HandleAdd  , CmdInfoArr[ Cmd::add   ].argQuant, CmdInfoArr[ Cmd::add   ].codeRecordSize},
    {"sub"  ,  HandleSub  , CmdInfoArr[ Cmd::sub   ].argQuant, CmdInfoArr[ Cmd::sub   ].codeRecordSize},
    {"mul"  ,  HandleMul  , CmdInfoArr[ Cmd::mul   ].argQuant, CmdInfoArr[ Cmd::mul   ].codeRecordSize},
    {"div"  ,  HandleDiv  , CmdInfoArr[ Cmd::dive  ].argQuant, CmdInfoArr[ Cmd::dive  ].codeRecordSize},
    {"pp"   ,  HandlePp   , CmdInfoArr[ Cmd::pp    ].argQuant, CmdInfoArr[ Cmd::pp    ].codeRecordSize},
    {"mm"   ,  HandleMm   , CmdInfoArr[ Cmd::mm    ].argQuant, CmdInfoArr[ Cmd::mm    ].codeRecordSize},
    {"out"  ,  HandleOut  , CmdInfoArr[ Cmd::out   ].argQuant, CmdInfoArr[ Cmd::out   ].codeRecordSize},
    {"outc" ,  HandleOutc , CmdInfoArr[ Cmd::outc  ].argQuant, CmdInfoArr[ Cmd::outc  ].codeRecordSize},
    {"outr" ,  HandleOutr , CmdInfoArr[ Cmd::outr  ].argQuant, CmdInfoArr[ Cmd::outr  ].codeRecordSize},
    {"outrc",  HandleOutrc, CmdInfoArr[ Cmd::outrc ].argQuant, CmdInfoArr[ Cmd::outrc ].codeRecordSize},
    {"hlt"  ,  HandleHlt  , CmdInfoArr[ Cmd::hlt   ].argQuant, CmdInfoArr[ Cmd::hlt   ].codeRecordSize},
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const size_t DefaultCmdQuant = sizeof(DefaultCmd) / sizeof(DefaultCmd[0]);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static_assert(DefaultCmdQuant == CmdInfoArrSize, "in include/common/globalIncude.hpp is init array with all cmd. this numbers must be equal, or you forgot about some cmd");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define ASSEMBLER_VERIF(AsmDataInfo, err) Verif(AsmDataInfo, &err, __FILE__, __LINE__, __func__)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr (*GetCmd(size_t cmdPointer)) (AsmData* AsmDataInfo)
{
    return DefaultCmd[cmdPointer].CmdFunc;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void RunAssembler(const IOfile* file)
{
    assert(file);
    assert(file->ProgrammFile);
    assert(file->CodeFile);

    AsmData AsmDataInfo = {};


    ASSEMBLER_ASSERT(AsmDataCtor         (&AsmDataInfo, file));
    ASSEMBLER_ASSERT(InitAllLabels       (&AsmDataInfo));
    ASSEMBLER_ASSERT(WriteCmdInCodeArr   (&AsmDataInfo));
    ASSEMBLER_ASSERT(WriteCodeArrInFile  (&AsmDataInfo, file));
    ASSEMBLER_ASSERT(AsmDataDtor         (&AsmDataInfo));


    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr AsmDataCtor(AsmData* AsmDataInfo, const IOfile* file)
{
    assert(AsmDataInfo);
    assert(file);
    assert(file->ProgrammFile);
    assert(file->CodeFile);

    AssemblerErr err = {};

    AsmDataInfo->cmd.cmd = ReadFile(file->ProgrammFile, &AsmDataInfo->cmd.size);

    size_t codeArrSize     = CalcCodeSize(&AsmDataInfo->cmd);
    AsmDataInfo->code.size = codeArrSize;
    AsmDataInfo->code.code = (int*) calloc(codeArrSize, sizeof(int));

    assert(AsmDataInfo->code.code);

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr AsmDataDtor(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    FREE(AsmDataInfo->code.code);
    BufferDtor(AsmDataInfo->cmd.cmd);
    LabelsDtor(AsmDataInfo);

    *AsmDataInfo = {};

    return err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr WriteCmdInCodeArr(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    size_t cmdQuant = AsmDataInfo->cmd.size;

    while (AsmDataInfo->cmd.pointer < cmdQuant)
    {
        const char* cmd = GetNextCmd(AsmDataInfo);

        size_t defaultCmdPointer = 0;

        if (FindDefaultCmd(cmd, &defaultCmdPointer))
        {
            ASSEMBLER_ASSERT(GetCmd(defaultCmdPointer)(AsmDataInfo));
            continue;
        }

        if (IsLabel(cmd))
        {
            ASSEMBLER_ASSERT(HandleLabel(AsmDataInfo));
            continue;
        }

        if (IsCommentBegin(cmd))
        {
            ASSEMBLER_ASSERT(HandleComment(AsmDataInfo));
            continue;
        }

        err.err = AssemblerErrorType::UNDEFINED_COMMAND;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    AsmDataInfo->code.size = AsmDataInfo->code.pointer;

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr WriteCodeArrInFile(AsmData* AsmDataInfo, const IOfile* file)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->code.code);

    AssemblerErr err = {};

    FILE* codeFile = fopen(file->CodeFile, "wb");

    if (!codeFile)
    {
        err.err = AssemblerErrorType::FAILED_OPEN_OUTPUT_STREAM;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    size_t codeArrSize = AsmDataInfo->code.size;

    ON_DEBUG(
    LOG_PRINT(Red, "code arr size = '%lu'\n", codeArrSize);
    LOG_ALL_INT_ARRAY(Yellow, AsmDataInfo->code.code, codeArrSize, 3);
    )

    fprintf(codeFile, "%lu\n", codeArrSize);

    for (size_t i = 0; i < codeArrSize; i++)
    {
        ON_DEBUG(
        LOG_PRINT(Red, "code[%2lu] = '%d'\n", i, AsmDataInfo->code.code[i]);
        )
        fprintf(codeFile, "%d ", AsmDataInfo->code.code[i]);
    }

    fclose(codeFile);

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePush(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    const char* buffer    = GetNextCmd(AsmDataInfo);
    size_t      BufferLen = strlen(buffer);
    char*       EndBuffer = nullptr;

    StackElem_t PushElem = (StackElem_t) strtol(buffer, &EndBuffer, 10);
    PushType    Push     = {};
    int         SetElem  = 0;
    int         Sum      = 0;

    if (IsInt(buffer, EndBuffer, BufferLen))
    {
        PushTypeCtor(&Push, 1, 0, 0, 0);
        SetElem = PushElem;
        Sum = 0;
    }

    else if (IsChar(buffer, EndBuffer, BufferLen))
    {
        PushTypeCtor(&Push, 1, 0, 0, 0);
        SetElem = GetChar(buffer, BufferLen);
        Sum = 0; 
    }

    else if (IsRegister(buffer, BufferLen))
    {
        PushTypeCtor(&Push, 0, 1, 0, 0);
        SetElem = GetRegisterPointer(buffer);
        Sum = 0;
    }

    else if (IsMemory(buffer, BufferLen))
    {
        UpdateBufferForMemory(&buffer, &BufferLen);

        StackElem_t PushElemMemIndex = (StackElem_t) strtol(buffer, &EndBuffer, 10);

        if (IsInt(buffer, EndBuffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 0, 1, 0);
            SetElem = PushElemMemIndex;
            Sum = 0;
        }

        else if (IsRegister(buffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 1, 1, 0);
            SetElem = GetRegisterPointer(buffer);
            Sum = 0;
        }

        else if (IsSum(buffer, BufferLen))
        {
            PushTypeCtor(&Push, 0, 0, 1, 1);
            SetElem = GetRegisterPointer(buffer);
            buffer += Registers::REGISTERS_NAME_LEN + 1;
            Sum     = (int) strtol(buffer, &EndBuffer, 10);
        }
    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    SetCmdArrCodeElem(AsmDataInfo, push);
    SetCmdArrCodeElem(AsmDataInfo, GetPushArg(&Push));
    SetCmdArrCodeElem(AsmDataInfo, SetElem);
    SetCmdArrCodeElem(AsmDataInfo, Sum);


    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void UpdateBufferForMemory(const char** buffer, size_t* bufferSize)
{
    assert(buffer);
    assert(*buffer);
    assert(bufferSize);

    *bufferSize -= 2;
    (*buffer)++;

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static AssemblerErr UpdateBufferForMemory(char** buffer, size_t* bufferSize)
// {
//     assert(buffer);
//     assert(bufferSize);

//     AssemblerErr err = {};
//     (*buffer)[*bufferSize - 1] = '\0';
//     *bufferSize -= 2;
//     (*buffer)++;

//     return ASSEMBLER_VERIF(err);
// }

// //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePop(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    const char* buffer = GetNextCmd(AsmDataInfo);
    size_t BufferLen = strlen(buffer);

    PopType Pop = {};
    int SetElem = 0;
    int Sum     = 0;

    if (IsRegister(buffer, BufferLen))
    {
        PopTypeCtor(&Pop, 1, 0, 0);
        SetElem = GetRegisterPointer(buffer);
        Sum = 0;
    }

    else if (IsMemory(buffer, BufferLen))
    {
        UpdateBufferForMemory(&buffer, &BufferLen);

        char*       EndBuffer       = nullptr;
        StackElem_t PopElemMemIndex = (StackElem_t) strtol(buffer, &EndBuffer,  10);

        if (IsInt(buffer, EndBuffer, BufferLen))
        {
            PopTypeCtor(&Pop, 0, 1, 0);
            SetElem = PopElemMemIndex;
            Sum = 0;
        }

        else if (IsRegister(buffer, BufferLen))
        {
            PopTypeCtor(&Pop, 1, 1, 0);
            SetElem = GetRegisterPointer(buffer);
            Sum = 0;
        }

        else if (IsSum(buffer, BufferLen))
        {
            PopTypeCtor(&Pop, 0, 1, 1);
            SetElem = GetRegisterPointer(buffer);
            buffer += Registers::REGISTERS_NAME_LEN + 1;
            Sum     = (int) strtol(buffer, &EndBuffer, 10);
        }
    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    SetCmdArrCodeElem(AsmDataInfo, pop);
    SetCmdArrCodeElem(AsmDataInfo, GetPopArg(&Pop));
    SetCmdArrCodeElem(AsmDataInfo, SetElem);
    SetCmdArrCodeElem(AsmDataInfo, Sum);


    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJmp(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jmp);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJa(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, ja);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJae(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jae);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJb(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jb);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJbe(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jbe);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJe(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, je);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJne(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jne);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleCall(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    const char* CallArg = GetNextCmd(AsmDataInfo);

    int SetElem = 0;

    if (IsLabel(CallArg))
    {
        size_t labelPointer = 0;

        if (IsLabelAlready(AsmDataInfo, CallArg, &labelPointer))
        {
            Label label = AsmDataInfo->labels.labels[labelPointer];
            SetElem     = (int) (label.codePlace);
        }

        else
        {
            COLOR_PRINT(RED, "Call arg = '%s'\n", CallArg);
            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err);
        }
    }

    else
    {
        err.err = AssemblerErrorType::LABEL_REDEFINE;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    SetCmdArrCodeElem(AsmDataInfo, Cmd::call);
    SetCmdArrCodeElem(AsmDataInfo, SetElem);

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleRet(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::ret);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr PpMmPattern(AsmData* AsmDataInfo, Cmd cmd)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
        
    const char* ppArg    = GetNextCmd(AsmDataInfo);
    size_t      ppArgLen = strlen(ppArg);

    int         SetElem  = 0;

    if (IsRegister(ppArg, ppArgLen))
    {
        SetElem = GetRegisterPointer(ppArg);
    }

    else
    {
        if (cmd == Cmd::pp)
        {
            err.err = AssemblerErrorType::INCORRECT_PP_ARG;
        }

        else if (cmd == Cmd::mm)
        {
            err.err = AssemblerErrorType::INCORRECT_MM_ARG;
        }

        else
        {
            assert(0 && "undef situation: must be 'pp' or 'mm' cmd");
        }

        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    SetCmdArrCodeElem(AsmDataInfo, cmd);
    SetCmdArrCodeElem(AsmDataInfo, SetElem);

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePp(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    return PpMmPattern(AsmDataInfo, Cmd::pp);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


static AssemblerErr HandleMm(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    return PpMmPattern(AsmDataInfo, Cmd::mm);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleAdd(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::add);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleSub(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::sub);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleMul(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::mul);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleDiv(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::dive);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOut(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::out);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOutc(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::outc);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOutr(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::outr);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOutrc(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::outrc);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleHlt(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, hlt);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleLabel(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->labels.labels);

    AssemblerErr err = {};

    AsmDataInfo->labels.pointer++;

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleDraw(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    const char* firstArg  = GetNextCmd(AsmDataInfo);
    const char* secondArg = GetNextCmd(AsmDataInfo);

    size_t firstArgLen  = strlen(firstArg);
    size_t secondArgLen = strlen(secondArg);

    char* firstArgEndPtr  = nullptr;
    char* secondArgEndPtr = nullptr;

    strtol(firstArg,  &firstArgEndPtr,  10);
    strtol(secondArg, &secondArgEndPtr, 10);
    
    if (IsRegister(firstArg, firstArgLen) && IsRegister(secondArg, secondArgLen))
    {
        SetCmdArrCodeElem(AsmDataInfo, Cmd::draw                    );
        SetCmdArrCodeElem(AsmDataInfo, GetRegisterPointer(firstArg ));
        SetCmdArrCodeElem(AsmDataInfo, GetRegisterPointer(secondArg));

        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleComment(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->cmd.cmd);
    assert(AsmDataInfo->cmd.pointer >= 1);

    AssemblerErr err = {};

    size_t       cmdQuant = AsmDataInfo->cmd.size;

    const char** cmdArr   = AsmDataInfo->cmd.cmd;
    size_t       pointer  = AsmDataInfo->cmd.pointer - 1;
    const char*  cmd      = cmdArr[pointer];

    while (!IsCommentEnd(cmd) && pointer < cmdQuant)
    {
        pointer++;
        cmd = cmdArr[pointer];
    }

    pointer++;

    AsmDataInfo->cmd.pointer = pointer;

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr NullArgCmdPattern(AsmData* AsmDataInfo, Cmd cmd)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
    SetCmdArrCodeElem(AsmDataInfo, cmd);
    AsmDataInfo->code.size++;
    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr JmpCmdPattern(AsmData* AsmDataInfo, Cmd JumpType)
{ 
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    const char* JumpArg = GetNextCmd(AsmDataInfo);

    int SetElem = 0;

    if (IsLabel(JumpArg))
    {
        size_t labelPointer = 0;

        if (IsLabelAlready(AsmDataInfo, JumpArg, &labelPointer))
        {
            Label label = AsmDataInfo->labels.labels[labelPointer];
            SetElem     = (int) label.codePlace;
        }

        else
        {
            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err);
        }
    }

    else if (IsStrInt(JumpArg))
    {
        SetElem = strintToInt(JumpArg);
    }

    else
    {
        err.err = AssemblerErrorType::LABEL_REDEFINE;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    SetCmdArrCodeElem(AsmDataInfo, JumpType);
    SetCmdArrCodeElem(AsmDataInfo, SetElem);

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr InitAllLabels(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->cmd.cmd);

    AssemblerErr err = {};

    const char** cmdArr    = AsmDataInfo->cmd.cmd;
    size_t       cmdQuant  = AsmDataInfo->cmd.size;

    ASSEMBLER_ASSERT(LabelsCtor(AsmDataInfo));

    size_t cmdPointer  = 0;
    size_t codePointer = 0;

    while(cmdPointer < AsmDataInfo->cmd.size)
    {
        const char* cmd      = AsmDataInfo->cmd.cmd[cmdPointer];
        bool        defined  = true;
        size_t      cmdIndex = cmdPointer;

        if (FindDefaultCmd(cmd, &cmdIndex))
        {
            cmdPointer  += DefaultCmd[cmdIndex].argQuant + 1;
            codePointer += DefaultCmd[cmdIndex].codeRecordSize;
            continue;
        }
    
        if (IsLabel(cmd))
        {
            ON_DEBUG(
            LOG_PRINT(Green, "label: '%s'\n", cmd);
            )
            if (!IsLabelAlready(AsmDataInfo, cmd, &cmdIndex))
            {
                cmdPointer++;
                Label label = LabelCtor(cmd, codePointer, defined);
                ASSEMBLER_ASSERT(PushLabel(AsmDataInfo, &label));
                continue;
            }
            ON_DEBUG(
            LOG_PRINT(Red, "redefine: '%s'\n", cmd);
            )
            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err);
        }

        if (IsCommentBegin(cmd))
        {
            while (!IsCommentEnd(cmd) && cmdPointer < cmdQuant)
            {
                cmdPointer++;
                cmd = cmdArr[cmdPointer];
            }

            cmdPointer++;

            continue;
        }

        err.err = AssemblerErrorType::UNDEFINED_COMMAND;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }



    ON_DEBUG(
    size_t size = AsmDataInfo->labels.size;
    for (size_t i = 0; i < size; i++)
    {
        LOG_PRINT(Blue, "label[%2lu] = .name = '%10s', .codePlace = '%3lu', .alreadyDefined = '%d'\n", i, AsmDataInfo->labels.labels[i].name, AsmDataInfo->labels.labels[i].codePlace, AsmDataInfo->labels.labels[i].alradyDefined);
    }
    )
    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr LabelsCtor(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    static size_t const DefaultLabelsQuant = 10;

    AsmDataInfo->labels.labels = (Label*) calloc(DefaultLabelsQuant, sizeof(Label));

    if (!AsmDataInfo->labels.labels)
    {
        err.err = AssemblerErrorType::BAD_LABELS_CALLOC;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    AsmDataInfo->labels.capacity = DefaultLabelsQuant;

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr LabelsDtor(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
    
    FREE(AsmDataInfo->labels.labels);
    AsmDataInfo->labels = {};

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Label LabelCtor(const char* name, size_t pointer, bool alreadyDefined)
{
    assert(name);

    Label label = {};

    label.name          = name;
    label.codePlace     = pointer;
    label.alradyDefined = alreadyDefined;

    return label;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr PushLabel(AsmData* AsmDataInfo, const Label* label)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->labels.labels);
    assert(label);

    AssemblerErr err = {};

    Labels* Labels = &AsmDataInfo->labels;

    Labels->size++;


    size_t size = Labels->size;
    size_t capacity = Labels->capacity;

    if (size <= capacity)
    {
        Labels->labels[Labels->size - 1] = *label;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    size_t new_capacity = 2 * capacity;
    Labels->labels = (Label*) realloc(Labels->labels, new_capacity * sizeof(Label));

    if (!Labels->labels)
    {
        err.err = AssemblerErrorType::BAD_CODE_ARR_REALLOC;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    Labels->labels[Labels->size - 1] = *label;
    

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLabelAlready(const AsmData* AsmDataInfo, const char* label, size_t* labelPlace)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->labels.labels);
    
    Labels labels = AsmDataInfo->labels;
    size_t size   = labels.size;

    for (size_t labelPointer = 0; labelPointer < size; labelPointer++)
    {
        const char* temp = labels.labels[labelPointer].name;

        if (strcmp(label, temp) == 0)
        {
            *labelPlace = labelPointer;
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static size_t CalcCodeSize(const CmdArr* cmd)
{
    assert(cmd);

    size_t codeSize = 0;

    for (size_t cmdPointer = 0; cmdPointer < cmd->size; cmdPointer++)
    {
        const char* temp    = cmd->cmd[cmdPointer];
        size_t defCmdPoiner = 0;

        if (FindDefaultCmd(temp, &defCmdPoiner))
        {
            CmdFunc cmdf           = DefaultCmd[defCmdPoiner];
            size_t  argQuant       = cmdf.argQuant;
            size_t  codeRecordSize = cmdf.codeRecordSize;

            codeSize              += codeRecordSize;
            cmdPointer            += argQuant;
        }
    }
    return codeSize;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool FindDefaultCmd(const char* cmd, size_t* defaultCmdPointer)
{
    assert(cmd);
    assert(defaultCmdPointer);

    for (size_t i = 0; i < DefaultCmdQuant; i++)
    {
        const char* defCmd = GetCmdName(i);
        if (strcmp(cmd, defCmd) == 0)
        {
            *defaultCmdPointer = i;
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PushTypeCtor(PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem, uint8_t Sum)
{
    assert(Push);

    Push->stk = Stk ? 1 : 0;
    Push->reg = Reg ? 1 : 0;
    Push->mem = Mem ? 1 : 0;
    Push->sum = Sum ? 1 : 0;

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushArg(PushType* Push)
{
    assert(Push);

    return *(int*) Push;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PopTypeCtor(PopType* Pop, uint8_t Reg, uint8_t Mem, uint8_t Sum)
{
    assert(Pop);

    Pop->reg = Reg ? 1 : 0;
    Pop->mem = Mem ? 1 : 0;
    Pop->sum = Sum ? 1 : 0;

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPopArg(PopType* Pop)
{
    assert(Pop);

    return *(int*) Pop;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetRegisterPointer(const char* buffer)
{
    assert(buffer);

    const char fisrtBuf  = buffer[0];
    const char secondBuf = buffer[1];

    if ((fisrtBuf < 'a') || (fisrtBuf >= Registers::REGISTERS_QUANT + 'a') || (secondBuf != 'x'))
    {
        AssemblerErr err = {};
        err.err = AssemblerErrorType::INCORRECT_SUM_FIRST_OPERAND;
        ASSEMBLER_ASSERT(ASSEMBLER_VERIF(nullptr, err));
    }

    return fisrtBuf - 'a';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static char GetChar(const char* buffer, size_t bufferSize)
{
    assert(buffer);

    if (bufferSize == 4)
    {
        const char buffer2 = buffer[2]; 

        if      (buffer2 == 'n') return '\n';
        else if (buffer2 == '_') return ' ';

        assert(0 && "undef situation: must be '\n' only");
    }

    else if (bufferSize != 3)
    {
        assert(0 && "undef situation: must be size = 3 or 4");
    }

    return buffer[1];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsStrInt(const char* str)
{
    assert(str);

    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (!IsCharNum(str[i]))
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsCharNum(char c)
{
    return  ('0' <=  c) &&
            ( c  <= '9');
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsInt(const char* str, const char* StrEnd, size_t strSize)
{
    assert(str);
    assert(StrEnd);

    int ptrDif = (int) (StrEnd - str);
    int strLen = (int)  strSize;

    return ptrDif == strLen;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsChar(const char* str, const char* StrEnd, size_t strSize)
{
    assert(str);
    assert(StrEnd);

    const char str0    = str[0];
    const char strLast = str[strSize - 1];

    if (!(3 <= strSize && strSize <= 4) ||
         (str0    != '\'')              ||
         (strLast != '\''))
    {
        return false;
    }

    else if (strSize == 4)
    {
        const char str1 = str[1];
        const char str2 = str[2];

        return  (str1 == '\\') &&
               ((str2 == 'n') ||
                (str2 == '_'));
    }

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsRegister(const char* str, size_t strSize)
{
    assert(str);
    return strSize == 2 && 'a' <= str[0] && str[0] <= 'a' + Registers::REGISTERS_QUANT &&  str[1] == 'x';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsMemory(const char* str, size_t strSize)
{
    assert(str);
    return str[0] == '[' && str[strSize - 1] == ']';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSum(const char* str, size_t strSize)
{
    assert(str);

    bool WasPlus = false;

    for (size_t i = 0; i < strSize; i++)
    {
        if      (str[i] == '+' && !WasPlus) WasPlus = true;
        else if (str[i] == '+' && WasPlus) return false;
    }

    return WasPlus;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLabel(const char* str)
{
    assert(str);
    return str[strlen(str) - 1] == ':';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsCommentBegin(const char* str)
{
    assert(str);

    return str[0] == '#';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsCommentEnd(const char* str)
{
    assert(str);

    size_t strLen = strlen(str);

    return str[strLen - 1] == '/';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetCmdName(size_t cmdPointer)
{
    return DefaultCmd[cmdPointer].CmdName;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetCmdArrCodeElem(AsmData* AsmDataInfo, int SetElem)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->code.code);

    size_t pointer = AsmDataInfo->code.pointer;
    AsmDataInfo->code.code[pointer] = SetElem;
    AsmDataInfo->code.pointer++;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetNextCmd(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AsmDataInfo->cmd.pointer++;
    return AsmDataInfo->cmd.cmd[AsmDataInfo->cmd.pointer - 1];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void AssemblerAssertPrint(AssemblerErr* err, const char* file, int line, const char* func)
{
    
    assert(file);
    assert(func);

    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(file, line, func);
    PrintError(err);
    PrintPlace(err->place.file, err->place.line, err->place.func);
    COLOR_PRINT(CYAN, "\nabort() in 3, 2, 1...");

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr Verif(AsmData* AsmDataInfo, AssemblerErr* err, const char* file, int line, const char* func)
{    
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);

    if (!AsmDataInfo)
    {
        return *err;
    }

    return *err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(AssemblerErr* err)
{
    assert(err);

    switch (err->err)
    {
        case AssemblerErrorType::NO_ERR:
            return;

        case AssemblerErrorType::FAILED_OPEN_INPUT_STREAM:
            COLOR_PRINT(RED, "Error: failed open input file.\n");
            break;

        case AssemblerErrorType::FAILED_OPEN_OUTPUT_STREAM:
            COLOR_PRINT(RED, "Error: failed open output file.\n");
            break;
        
        case AssemblerErrorType::INVALID_INPUT_AFTER_PUSH:
            COLOR_PRINT(RED, "Error: invalid input after push.\n");
            break;
        
        case AssemblerErrorType::INVALID_INPUT_AFTER_POP:
            COLOR_PRINT(RED, "Error: invalid input after pop.\n");
            break;

        case AssemblerErrorType::UNDEFINED_COMMAND:
            COLOR_PRINT(RED, "Error: undefined command.\n");
            break;
    
        case AssemblerErrorType::BAD_CODE_ARR_REALLOC:
            COLOR_PRINT(RED, "Error: realloc in write cmd in code return nullptr.\n");
            break;

        case AssemblerErrorType::FWRITE_BAD_RETURN:
            COLOR_PRINT(RED, "Error: failed to write all code arr in output stream.\n");
            break;

        case AssemblerErrorType::LABEL_REDEFINE:
            COLOR_PRINT(RED, "Error: redefined label.\n");
            break;

        case AssemblerErrorType::BAD_LABELS_CALLOC:
            COLOR_PRINT(RED, "Error: failed to allocate memory for labels.\n");
            break;
        
        case AssemblerErrorType::BAD_LABELS_REALLOC:
            COLOR_PRINT(RED, "Error: failer to reallocate memory for labels.\n");
            break;

        case AssemblerErrorType::INCORRECT_SUM_FIRST_OPERAND:
            COLOR_PRINT(RED, "Error: inccorrect first opearand in sum in push/pop.\n");
            break;

        case AssemblerErrorType::INCORRECT_SUM_SECOND_OPERAND:
            COLOR_PRINT(RED, "Error: incorrect second operand in sum in push/pop\n");
            break;

        case AssemblerErrorType::INCORRECT_PP_ARG:
            COLOR_PRINT(RED, "Error: incorrect pp arg\n");
            break;

        case AssemblerErrorType::INCORRECT_MM_ARG:
            COLOR_PRINT(RED, "Error: incorrect mm arg\n");
            break;

        default: 
            assert(0 && "yoy forgot about some error in err print");
            break;
    }

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static void LabelDump(const Label *label)
// {
//     assert(label);

//     COLOR_PRINT(GREEN, "\nlabel dump begin:\n");

//     COLOR_PRINT(RED,    "name:   '%s'\n", label->name);
//     COLOR_PRINT(YELLOW, "place:  '%lu'\n", label->codePlace);
//     COLOR_PRINT(CYAN,   "is def: '%d'\n", label->alradyDefined);

//     COLOR_PRINT(GREEN, "label dump end.\n\n");

//     return;
// }

// //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------