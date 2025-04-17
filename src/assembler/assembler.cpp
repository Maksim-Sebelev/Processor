#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "assembler/assembler.hpp"
#include "fileread/fileread.hpp"
#include "common/globalInclude.hpp"
#include "lib/lib.hpp"
#include "lib/colorPrint.hpp"
#include "stack/stack.hpp"
ON_DEBUG(
#include "log/log.hpp"
)

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct CmdArr
{
    size_t       size;
    size_t       pointer;
    const char** cmd;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef Stack_t CodeArr;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct Label
{
    const char* name;
    size_t      codePlace;
    bool       alradyDefined;
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
static AssemblerErr WriteIntCmdInCodeArr     (AsmData* AsmDataInfo);
static AssemblerErr WriteCodeArrInFile    (AsmData* AsmDataInfo, const IOfile* file);


static void         WriteIntCmdInCodeArr     (AsmData* AsmDataInfo, int setElem);
static const char*  GetNextCmd            (AsmData* AsmDataInfo);
static void         UpdateBufferForMemory (const char** buffer, size_t* bufferSize);

static const char * GetCmdName            (size_t cmdPointer);

static AssemblerErr NullArgCmdPattern     (AsmData* AsmDataInfo, Cmd cmd);

static AssemblerErr PpMmPattern           (AsmData* AsmDataInfo, Cmd cmd);


static int           GetPushArg           (PushType* push);
static int           GetPopArg            (PopType*  Pop );
static void          PushTypeCtor         (PushType* push, uint8_t Stk, uint8_t Reg, uint8_t Mem, uint8_t sum);
static void          PopTypeCtor          (PopType*  Pop , uint8_t Reg, uint8_t Mem, uint8_t sum);

static int           GetIntRegisterPointer   (const char* buffer);
static char          GetChar              (const char* buffer, size_t bufferSize);

static bool          IsCharNum            (char c);
static bool          IsStrInt             (const char* str);
static bool          IsChar               (const char* str, const char* strEnd, size_t strSize);
static bool          IsInt                (const char* str, const char* strEnd, size_t strSize);
static bool          IsDouble             (const char* str, const char* strEnd, size_t strSize);
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


static AssemblerErr HandlePushi          (AsmData* AsmDataInfo);
static AssemblerErr HandlePushc          (AsmData* AsmDataInfo);
static AssemblerErr HandlePushd          (AsmData* AsmDataInfo);
static AssemblerErr HandlePopi           (AsmData* AsmDataInfo);
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
    {"pushc",  HandlePushc, CmdInfoArr[ pushc ].argQuant, CmdInfoArr[ pushc ].codeRecordSize},
    {"pushi",  HandlePushi, CmdInfoArr[ pushi ].argQuant, CmdInfoArr[ pushi ].codeRecordSize},
    {"pushd",  HandlePushd, CmdInfoArr[ pushd ].argQuant, CmdInfoArr[ pushd ].codeRecordSize},
    // {"popc" ,  HandlePopc , CmdInfoArr[ popc  ].argQuant, CmdInfoArr[ popc  ].codeRecordSize},
    {"popi" ,  HandlePopi , CmdInfoArr[ popi  ].argQuant, CmdInfoArr[ popi  ].codeRecordSize},
    // {"popd" ,  HandlePopd , CmdInfoArr[ popd  ].argQuant, CmdInfoArr[ popd  ].codeRecordSize},
    {"jmp"  ,  HandleJmp  , CmdInfoArr[ jmp   ].argQuant, CmdInfoArr[ jmp   ].codeRecordSize},
    {"ja"   ,  HandleJa   , CmdInfoArr[ ja    ].argQuant, CmdInfoArr[ ja    ].codeRecordSize},
    {"jae"  ,  HandleJae  , CmdInfoArr[ jae   ].argQuant, CmdInfoArr[ jae   ].codeRecordSize},
    {"jb"   ,  HandleJb   , CmdInfoArr[ jb    ].argQuant, CmdInfoArr[ jb    ].codeRecordSize},
    {"jbe"  ,  HandleJbe  , CmdInfoArr[ jbe   ].argQuant, CmdInfoArr[ jbe   ].codeRecordSize},
    {"je"   ,  HandleJe   , CmdInfoArr[ je    ].argQuant, CmdInfoArr[ je    ].codeRecordSize},
    {"jne"  ,  HandleJne  , CmdInfoArr[ jne   ].argQuant, CmdInfoArr[ jne   ].codeRecordSize},
    {"call" ,  HandleCall , CmdInfoArr[ call  ].argQuant, CmdInfoArr[ call  ].codeRecordSize},
    {"ret"  ,  HandleRet  , CmdInfoArr[ ret   ].argQuant, CmdInfoArr[ ret   ].codeRecordSize},
    {"add"  ,  HandleAdd  , CmdInfoArr[ add   ].argQuant, CmdInfoArr[ add   ].codeRecordSize},
    {"sub"  ,  HandleSub  , CmdInfoArr[ sub   ].argQuant, CmdInfoArr[ sub   ].codeRecordSize},
    {"mul"  ,  HandleMul  , CmdInfoArr[ mul   ].argQuant, CmdInfoArr[ mul   ].codeRecordSize},
    {"div"  ,  HandleDiv  , CmdInfoArr[ dive  ].argQuant, CmdInfoArr[ dive  ].codeRecordSize},
    {"pp"   ,  HandlePp   , CmdInfoArr[ pp    ].argQuant, CmdInfoArr[ pp    ].codeRecordSize},
    {"mm"   ,  HandleMm   , CmdInfoArr[ mm    ].argQuant, CmdInfoArr[ mm    ].codeRecordSize},
    {"out"  ,  HandleOut  , CmdInfoArr[ out   ].argQuant, CmdInfoArr[ out   ].codeRecordSize},
    {"outc" ,  HandleOutc , CmdInfoArr[ outc  ].argQuant, CmdInfoArr[ outc  ].codeRecordSize},
    {"outr" ,  HandleOutr , CmdInfoArr[ outr  ].argQuant, CmdInfoArr[ outr  ].codeRecordSize},
    {"outrc",  HandleOutrc, CmdInfoArr[ outrc ].argQuant, CmdInfoArr[ outrc ].codeRecordSize},
    {"hlt"  ,  HandleHlt  , CmdInfoArr[ hlt   ].argQuant, CmdInfoArr[ hlt   ].codeRecordSize},
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const size_t DefaultCmdQuant = sizeof(DefaultCmd) / sizeof(DefaultCmd[0]);

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

    ASSEMBLER_ASSERT(AsmDataCtor          (&AsmDataInfo, file));
    ASSEMBLER_ASSERT(InitAllLabels        (&AsmDataInfo      ));
    ASSEMBLER_ASSERT(WriteIntCmdInCodeArr (&AsmDataInfo      ));
    ASSEMBLER_ASSERT(WriteCodeArrInFile   (&AsmDataInfo, file));
    ASSEMBLER_ASSERT(AsmDataDtor          (&AsmDataInfo      ));

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

    STACK_ASSERT(StackCtor(&AsmDataInfo->code, codeArrSize));

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr AsmDataDtor(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    STACK_ASSERT(StackDtor(&AsmDataInfo->code));
    BufferDtor(AsmDataInfo->cmd.cmd);
    LabelsDtor(AsmDataInfo);

    *AsmDataInfo = {};

    return err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr WriteIntCmdInCodeArr(AsmData* AsmDataInfo)
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
    assert(AsmDataInfo->code.data);

    AssemblerErr err = {};

    FILE* codeFile = fopen(file->CodeFile, "wb");

    if (!codeFile)
    {
        err.err = AssemblerErrorType::FAILED_OPEN_OUTPUT_STREAM;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    size_t codeArrSize = AsmDataInfo->code.size;

    fprintf(codeFile, "%lu\n", codeArrSize);

    for (size_t i = 0; i < codeArrSize; i++)
    {
        fprintf(codeFile, "%d ", *(int*) ((char*) AsmDataInfo->code.data + i));
        i += sizeof(int);
    }

    fclose(codeFile);

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef bool (*CheckPushArgFunction) (PushArgBuffer);

CheckPushArgFunction GetCheckPushArgFunction(Cmd cmd)
{
    switch (cmd)
    {
        case Cmd::pushc: return IsChar;   break;
        case Cmd::pushi: return IsInt;    break;
        case Cmd::pushd: return IsDouble; break;
        default:
        {
            assert(0 && "undefined situation: this cmd must be not here.");
        }
    }

    assert(0 && "undefined situation: this cmd must be not here.");
    return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct PushArgBuffer
{
    const char* buffer;
    char*       endBuffer;
    size_t      bufferLen; 
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushArgBuffer PushArgBufferCtor(AsmData* AsmDataInfo)
{
    PushArgBuffer buffer = {};
    buffer.buffer    = GetNextCmd(AsmDataInfo);
    buffer.bufferLen = strlen(buffer.buffer);
    buffer.endBuffer = nullptr;

    return buffer;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct PushArg
{

};


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static size_t GetRegisterNameLen(Cmd cmd)
{
    switch (cmd)
    {
        case Cmd::pushc: return (size_t) CharRegisters  ::CHAR_REGISTERS_NAME_LEN    ;
        case Cmd::pushi: return (size_t) IntRegisters   ::INT_REGISTERS_NAME_LEN     ;
        case Cmd::pushd: return (size_t) DoubleRegisters::DOUBLE_REGISTERS_NAME_LEN  ;
        default: assert(0 && "undef situiation: this cmd must not be here"); return 0;
    }

    assert(0 && "undef situiation: we must not be here");
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr PushCmdPattern(AsmData* AsmDataInfo, Cmd cmd)
{
    assert(AsmDataInfo);
    assert((cmd == Cmd::pushi) || (cmd == Cmd::pushc) || (cmd == Cmd::pushd));

    AssemblerErr err = {};

    PushArgBuffer buffer = PushArgBufferCtor(AsmDataInfo);


    int         pushElem  = (int) strtol(buffer.buffer, &buffer.endBuffer, 10);
    PushType    push      = {};
    int         setElem   = 0;
    int         sum       = 0;


    CheckPushArgFunction CheckArgType = GetCheckPushArgFunction(cmd);


    if (CheckArgType(buffer))
    {
        PushTypeCtor(&push, 1, 0, 0, 0);
        setElem = pushElem;
    }

    else if (IsRegister(buffer.buffer, buffer.bufferLen))
    {
        PushTypeCtor(&push, 0, 1, 0, 0);
        setElem = GetIntRegisterPointer(buffer.buffer);
    }

    else if (IsMemory(buffer.buffer, buffer.bufferLen))
    {
        UpdateBufferForMemory(&buffer.buffer, &buffer.bufferLen);
        
        int PushElemMemIndex = (int) strtol(buffer.buffer, &buffer.endBuffer, 10);


        switch (cmd)
        {
            case Cmd::pushc: char   PushElemMemIndex = (char)   strtol(buffer.buffer, &buffer.endBuffer, 10); break;
            case Cmd::pushd: double PushElemMemIndex = (double) strtod(buffer.buffer, &buffer.endBuffer);     break;
        }
        // StackElem_t PushElemMemIndex = (StackElem_t) strtol(buffer, &endBuffer, 10);

        if (IsInt(buffer.buffer, buffer.endBuffer, buffer.bufferLen))
        {
            PushTypeCtor(&push, 0, 0, 1, 0);
            setElem = PushElemMemIndex;
        }

        else if (IsRegister(buffer.buffer, buffer.bufferLen))
        {
            PushTypeCtor(&push, 0, 1, 1, 0);
            setElem = GetIntRegisterPointer(buffer.buffer);
        }

        else if (IsSum(buffer.buffer, buffer.bufferLen))
        {
            PushTypeCtor(&push, 0, 0, 1, 1);
            setElem        = GetIntRegisterPointer(buffer.buffer);
            buffer.buffer += GetRegisterNameLen(cmd) + 1;
            sum            = (int) strtol(buffer.buffer, &buffer.endBuffer, 10);
        }
    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePushi(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    const char* buffer    = GetNextCmd(AsmDataInfo);
    size_t      bufferLen = strlen(buffer);
    char*       endBuffer = nullptr;

    int         pushElem  = (int) strtol(buffer, &endBuffer, 10);
    PushType    push      = {};
    int         setElem   = 0;
    int         sum       = 0;

    if (IsInt(buffer, endBuffer, bufferLen))
    {
        PushTypeCtor(&push, 1, 0, 0, 0);
        setElem = pushElem;
        sum = 0;
    }

    else if (IsChar(buffer, endBuffer, bufferLen))
    {
        PushTypeCtor(&push, 1, 0, 0, 0);
        setElem = GetChar(buffer, bufferLen);
        sum = 0; 
    }

    else if (IsRegister(buffer, bufferLen))
    {
        PushTypeCtor(&push, 0, 1, 0, 0);
        setElem = GetIntRegisterPointer(buffer);
        sum = 0;
    }

    // else if (IsMemory(buffer, bufferLen))
    // {
    //     UpdateBufferForMemory(&buffer, &bufferLen);

    //     StackElem_t PushElemMemIndex = (StackElem_t) strtol(buffer, &endBuffer, 10);

    //     if (IsInt(buffer, endBuffer, bufferLen))
    //     {
    //         PushTypeCtor(&push, 0, 0, 1, 0);
    //         setElem = PushElemMemIndex;
    //         sum = 0;
    //     }

    //     else if (IsRegister(buffer, bufferLen))
    //     {
    //         PushTypeCtor(&push, 0, 1, 1, 0);
    //         setElem = GetIntRegisterPointer(buffer);
    //         sum = 0;
    //     }

    //     else if (IsSum(buffer, bufferLen))
    //     {
    //         PushTypeCtor(&push, 0, 0, 1, 1);
    //         setElem = GetIntRegisterPointer(buffer);
    //         buffer += Registers::REGISTERS_NAME_LEN + 1;
    //         sum     = (int) strtol(buffer, &endBuffer, 10);
    //     }
    // }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    WriteIntCmdInCodeArr(AsmDataInfo, pushi);
    WriteIntCmdInCodeArr(AsmDataInfo, GetPushArg(&push));
    WriteIntCmdInCodeArr(AsmDataInfo, setElem);
    WriteIntCmdInCodeArr(AsmDataInfo, sum);


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

static AssemblerErr HandlePopi(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    const char* buffer = GetNextCmd(AsmDataInfo);
    size_t bufferLen = strlen(buffer);

    PopType Pop = {};
    int setElem = 0;
    int sum     = 0;

    if (IsRegister(buffer, bufferLen))
    {
        PopTypeCtor(&Pop, 1, 0, 0);
        setElem = GetIntRegisterPointer(buffer);
        sum = 0;
    }

    // else if (IsMemory(buffer, bufferLen))
    // {
    //     UpdateBufferForMemory(&buffer, &bufferLen);

    //     char*       endBuffer       = nullptr;
    //     StackElem_t PopElemMemIndex = (StackElem_t) strtol(buffer, &endBuffer,  10);

    //     if (IsInt(buffer, endBuffer, bufferLen))
    //     {
    //         PopTypeCtor(&Pop, 0, 1, 0);
    //         setElem = PopElemMemIndex;
    //         sum = 0;
    //     }

    //     else if (IsRegister(buffer, bufferLen))
    //     {
    //         PopTypeCtor(&Pop, 1, 1, 0);
    //         setElem = GetIntRegisterPointer(buffer);
    //         sum = 0;
    //     }

    //     else if (IsSum(buffer, bufferLen))
    //     {
    //         PopTypeCtor(&Pop, 0, 1, 1);
    //         setElem = GetIntRegisterPointer(buffer);
    //         buffer += Registers::REGISTERS_NAME_LEN + 1;
    //         sum     = (int) strtol(buffer, &endBuffer, 10);
    //     }
    // }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    WriteIntCmdInCodeArr(AsmDataInfo, popi);
    WriteIntCmdInCodeArr(AsmDataInfo, GetPopArg(&Pop));
    WriteIntCmdInCodeArr(AsmDataInfo, setElem);
    WriteIntCmdInCodeArr(AsmDataInfo, sum);


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

    int setElem = 0;

    if (IsLabel(CallArg))
    {
        size_t labelPointer = 0;

        if (IsLabelAlready(AsmDataInfo, CallArg, &labelPointer))
        {
            Label label = AsmDataInfo->labels.labels[labelPointer];
            setElem     = (int) (label.codePlace);
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

    WriteIntCmdInCodeArr(AsmDataInfo, Cmd::call);
    WriteIntCmdInCodeArr(AsmDataInfo, setElem);

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

    int         setElem  = 0;

    if (IsRegister(ppArg, ppArgLen))
    {
        setElem = GetIntRegisterPointer(ppArg);
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

    WriteIntCmdInCodeArr(AsmDataInfo, cmd);
    WriteIntCmdInCodeArr(AsmDataInfo, setElem);

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
    WriteIntCmdInCodeArr(AsmDataInfo, cmd);
    AsmDataInfo->code.size++;
    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr JmpCmdPattern(AsmData* AsmDataInfo, Cmd JumpType)
{ 
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    const char* JumpArg = GetNextCmd(AsmDataInfo);

    int setElem = 0;

    if (IsLabel(JumpArg))
    {
        size_t labelPointer = 0;

        if (IsLabelAlready(AsmDataInfo, JumpArg, &labelPointer))
        {
            Label label = AsmDataInfo->labels.labels[labelPointer];
            setElem     = (int) label.codePlace;
        }

        else
        {
            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err);
        }
    }

    else if (IsStrInt(JumpArg))
    {
        setElem = strintToInt(JumpArg);
    }

    else
    {
        err.err = AssemblerErrorType::LABEL_REDEFINE;
        return ASSEMBLER_VERIF(AsmDataInfo, err);
    }

    WriteIntCmdInCodeArr(AsmDataInfo, JumpType);
    WriteIntCmdInCodeArr(AsmDataInfo, setElem);

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
            if (!IsLabelAlready(AsmDataInfo, cmd, &cmdIndex))
            {
                cmdPointer++;
                Label label = LabelCtor(cmd, codePointer, defined);
                ASSEMBLER_ASSERT(PushLabel(AsmDataInfo, &label));
                continue;
            }

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

    return ASSEMBLER_VERIF(AsmDataInfo, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr LabelsCtor(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    static size_t const DefaultLabelsQuant = 2;

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

static void PushTypeCtor(PushType* push, uint8_t Stk, uint8_t Reg, uint8_t Mem, uint8_t sum)
{
    assert(push);

    push->stk = Stk ? 1 : 0;
    push->reg = Reg ? 1 : 0;
    push->mem = Mem ? 1 : 0;
    push->sum = sum ? 1 : 0;

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushArg(PushType* push)
{
    assert(push);

    return *(int*) push;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PopTypeCtor(PopType* Pop, uint8_t Reg, uint8_t Mem, uint8_t sum)
{
    assert(Pop);

    Pop->reg = Reg ? 1 : 0;
    Pop->mem = Mem ? 1 : 0;
    Pop->sum = sum ? 1 : 0;

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPopArg(PopType* Pop)
{
    assert(Pop);

    return *(int*) Pop;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetIntRegisterPointer(const char* buffer)
{
    assert(buffer);

    const char fisrtBuf  = buffer[0];
    const char secondBuf = buffer[1];

    if ((fisrtBuf < 'a') || (fisrtBuf >= IntRegisters::INT_REGISTERS_QUANT + 'a') || (secondBuf != 'x'))
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

static bool IsDouble(const char* str, const char* strEnd, size_t strSize)
{
    assert(str);
    assert(strEnd);

    int ptrDif = (int) (strEnd - str);
    int strLen = (int) (strSize);

    return ptrDif == strLen;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsInt(const char* str, const char* strEnd, size_t strSize)
{
    assert(str);
    assert(strEnd);

    int ptrDif = (int) (strEnd - str);
    int strLen = (int) (strSize);

    return ptrDif == strLen;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsChar(const char* str, const char* strEnd, size_t strSize)
{
    assert(str);
    assert(strEnd);

    const char str0    = str[0];
    const char strLast = str[strSize - 1];

    int 

    else if ((strSize < 3 || 4 < strSize) ||
         (str0    != '\'')           ||
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

static bool GetIsRegisterFunction(const char* str, size_t strSize, Cmd cmd)
{
    switch (cmd)
    {
        case Cmd::pushc: return IsCharRegister(str, strSize);
        case Cmd::pushi: return IsCharRegister(str, strSize);
        case Cmd::pushd: return IsCharRegister(str, strSize);
        default: assert(0 && "undef situation: this cmd must not be here"); return false;
    }

    assert(0 && "undef situation: we must not be here");

    return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsRegisterPattern(const char* str, size_t strSize, char firstLetter)
{
    assert(str);
    static_assert(CharRegisters   ::CHAR_REGISTERS_NAME_LEN   == 3 &&
                  IntRegisters    ::INT_REGISTERS_NAME_LEN    == 3 &&
                  DoubleRegisters ::DOUBLE_REGISTERS_NAME_LEN == 3, 
                  "in this funtion change 'd' in 3 return's string to char, that ASCII id 'a' + registerts quant");

    return  (strSize == CharRegisters::CHAR_REGISTERS_NAME_LEN) &&
            (str[0] == firstLetter)                             &&
            ('a' <= str[1] && str[1] <= 'd')                    &&
            (str[1] == 'x');
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsCharRegister(const char* str, size_t strSize)
{
    assert(str);
    return IsRegisterPattern(str, strSize, 'c');
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsIntRegister(const char* str, size_t strSize)
{
    assert(str);
    return IsRegisterPattern(str, strSize, 'i');
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsIntRegister(const char* str, size_t strSize)
{
    assert(str);
    return IsRegisterPattern(str, strSize, 'd');
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

static void WriteIntCmdInCodeArr(AsmData* AsmDataInfo, int setElem)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->code.data);

    size_t pointer = AsmDataInfo->code.pointer;
    // AsmDataInfo->code.code[pointer] = setElem;
    AsmDataInfo->code.pointer++;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void WriteDoubleCmdInCodeArr(AsmData* AsmDataInfo, double setElem)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->code.data);

    size_t pointer = AsmDataInfo->code.pointer;
    // AsmDataInfo->code.code[pointer] = setElem;
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

    LOG(RED, "Assert made in:\n");
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
            ON_DEBUG(
            LOG_ERROR("failed open input file.")''
            )
            OFF_DEBUG(
            PRINT_ERROR("failed open input file.");
            )
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