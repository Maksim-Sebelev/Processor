#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "assembler/assembler.hpp"
#include "fileread/fileread.hpp"
#include "global/global_include.hpp"
#include "lib/lib.hpp"
#include "stack/stack.hpp"
#include "functions_for_files/files.hpp"
#include "assembler/tokenizer/tokenizer.hpp"

#ifdef _DEBUG
#include "logger/log.hpp"
#endif // _DEBUG

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum class AssemblerErrorType
{
    NO_ERR                       ,
    INVALID_INPUT_AFTER_PUSH     ,
    INVALID_INPUT_AFTER_POP      ,
    FAILED_OPEN_INPUT_STREAM     ,
    FAILED_OPEN_OUTPUT_STREAM    ,
    FWRITE_BAD_RETURN            ,
    UNDEFINED_COMMAND            ,
    UNDEFINED_ENUM_COMMAND       ,
    BAD_CODE_ARRAY_CALLOC        ,
    LABEL_REDEFINE               ,
    BAD_LABELS_CALLOC            ,
    BAD_LABELS_REALLOC           ,
    INCORRECT_SUM_FIRST_OPERAND  ,
    INCORRECT_SUM_SECOND_OPERAND ,
    INCORRECT_PP_ARG             ,
    INCORRECT_MM_ARG             ,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct AssemblerErr
{
    CodePlace          place;
    AssemblerErrorType err;
    Token              token;
    IOfile             file;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

typedef WordArray CmdArr;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct AsmData
{
    TokensArray tokens_array;
    CmdArr cmd;
    CodeArr    code         ;
    Labels     labels       ;
    IOfile     file         ;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void         CheckInputFiles          (const IOfile* files);

static AssemblerErr AsmDataCtor              (AsmData* AsmDataInfo, const IOfile* file);
static AssemblerErr AsmDataDtor              (AsmData* AsmDataInfo);
static AssemblerErr WriteCmdInCodeArr        (AsmData* AsmDataInfo);
static AssemblerErr WriteCodeArrInFile       (AsmData* AsmDataInfo);
   
   
static void         SetCmdArrCodeElem        (AsmData* AsmDataInfo, int SetElem);
static Word         GetNextCmd               (AsmData* AsmDataInfo);
static Token GetNextToken(AsmData* AsmDataInfo);

static void         UpdateBufferForMemory    (Word* buffer);
   
static const char*  GetCmdName               (size_t cmd_pointer);
   
static AssemblerErr NullArgCmdPattern        (AsmData* AsmDataInfo, Cmd cmd);
   
static AssemblerErr PpMmPattern              (AsmData* AsmDataInfo, Cmd cmd);
   

static bool IsTokenCommand(const Token* token, size_t* cmd_pointer);


static int          GetPushArg              (PushType* Push);
static int          GetPopArg               (PopType*  Pop );
static void         PushTypeCtor            (PushType* Push, uint8_t Stk, uint8_t Reg, uint8_t Mem, uint8_t Sum);
static void         PopTypeCtor             (PopType*  Pop , uint8_t Reg, uint8_t Mem, uint8_t Sum);
   
static int          GetRegisterPointer      (const Word* word);
static char         GetChar                 (const Word* word);
   
static bool         IsCharNum               (char c);
static bool         IsStrInt                (const Word* word);
static bool         IsInt                   (const Word* word, char* wordEndPtr);
static bool         IsChar                  (const Word* word, char* wordEndPtr);
static bool         IsRegister              (const Word* word);
static bool         IsMemory                (const Word* word);
static bool         IsSum                   (const Word* word);
static bool         IsLabel                 (const Word* word);

static bool IsLabel(const Token* token);

static bool         IsOneLineComment        (const Word* cmd);

static bool         IsTwoSymbolCommentBegin (const Word* word);
static bool         IsTwoSymbolCommentEnd   (const Word* word);


static AssemblerErr InitAllLabels           (AsmData* AsmDataInfo);
static AssemblerErr LabelsCtor              (AsmData* AsmDataInfo);
static AssemblerErr LabelsDtor              (AsmData* AsmDataInfo);

static Label        LabelCtor               (const TokenizerLabel* token_label, size_t pointer, bool alreadyDefined);
static AssemblerErr PushLabel               (AsmData* AsmDataInfo, const Label* label);
static bool         IsLabelAlready          (const AsmData* AsmDataInfo, const Word* label, size_t* labelPlace);
static bool IsLabelAlready(const AsmData* AsmDataInfo, const TokenizerLabel* label, size_t* labelPlace);


static CmdInfo      GetCmdInfo              (const Token* token);
static bool         FindDefaultCmd          (const Word* cmd, size_t* defaultCmdPointer);
static size_t       CalcCodeSize            (const TokensArray* tokens_array);

static AssemblerErr JmpCmdPattern           (AsmData* AsmDataInfo, Cmd JumpType);


static AssemblerErr HandlePush              (AsmData* AsmDataInfo);
static AssemblerErr HandlePop               (AsmData* AsmDataInfo);
static AssemblerErr HandleJmp               (AsmData* AsmDataInfo);
static AssemblerErr HandleJa                (AsmData* AsmDataInfo);
static AssemblerErr HandleJae               (AsmData* AsmDataInfo);
static AssemblerErr HandleJb                (AsmData* AsmDataInfo);
static AssemblerErr HandleJbe               (AsmData* AsmDataInfo);
static AssemblerErr HandleJe                (AsmData* AsmDataInfo);
static AssemblerErr HandleJne               (AsmData* AsmDataInfo);
static AssemblerErr HandleCall              (AsmData* AsmDataInfo);
static AssemblerErr HandleRet               (AsmData* AsmdataInfo);
static AssemblerErr HandleAdd               (AsmData* AsmDataInfo);
static AssemblerErr HandleSub               (AsmData* AsmDataInfo);
static AssemblerErr HandleMul               (AsmData* AsmDataInfo);
static AssemblerErr HandleDiv               (AsmData* AsmDataInfo);
static AssemblerErr HandlePp                (AsmData* AsmDataInfo);
static AssemblerErr HandleMm                (AsmData* AsmDataInfo);
static AssemblerErr HandleOut               (AsmData* AsmDataInfo);
static AssemblerErr HandleOutc              (AsmData* AsmDataInfo);
static AssemblerErr HandleOutr              (AsmData* AsmDataInfo);
static AssemblerErr HandleOutrc             (AsmData* AsmDataInfo);
static AssemblerErr HandleDraw              (AsmData* AsmDataInfo);
static AssemblerErr HandleHlt               (AsmData* AsmDataInfo);
static AssemblerErr HandleLabel             (AsmData* AsmDataInfo);
static AssemblerErr HandleOneLineComment    (AsmData* AsmDataInfo);
static AssemblerErr HandleTwoSymbolComment  (AsmData* AsmDataInfo);
static AssemblerErr HandleRGBA              (AsmData* AsmDataInfo);


static AssemblerErr Verif                      (const AsmData* AsmDataInfo, AssemblerErr* err, Token token, const char* file, int line, const char* func);
static void         PrintError                 (const AssemblerErr* err);
static void         PrintIncorrectCmd          (const char* msg, const char* file, Token token);
static void         PrintIncorrectCmdFilePlace (const char* file , Token token);
static void         AssemblerAssertPrint       (const AssemblerErr* err, const char* file, int line, const char* func);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define ASSEMBLER_VERIF(AsmDataInfo, err, token) Verif(AsmDataInfo, &err, token, __FILE__, __LINE__, __func__)

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define ASSEMBLER_ASSERT(Err) do                                                   \
{                                                                                   \
    AssemblerErr errCopy = Err;                                                      \
    if (errCopy.err != AssemblerErrorType::NO_ERR)                                    \
    {                                                                                  \
        AssemblerAssertPrint(&errCopy, __FILE__, __LINE__, __func__);                   \
        exit(EXIT_FAILURE);                                                              \
    }                                                                                     \
} while (0)                                                                                \

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void RunAssembler(const IOfile* file)
{
    assert(file);

    CheckInputFiles(file);

    AsmData AsmDataInfo = {};

    ASSEMBLER_ASSERT(AsmDataCtor         (&AsmDataInfo, file));
    ASSEMBLER_ASSERT(InitAllLabels       (&AsmDataInfo)      );
    ASSEMBLER_ASSERT(WriteCmdInCodeArr   (&AsmDataInfo)      );
    ASSEMBLER_ASSERT(WriteCodeArrInFile  (&AsmDataInfo)      );
    ASSEMBLER_ASSERT(AsmDataDtor         (&AsmDataInfo)      );

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr AsmDataCtor(AsmData* AsmDataInfo, const IOfile* file)
{
    assert(AsmDataInfo);
    assert(file);
    assert(file->asm_file);
    assert(file->bin_file);

    AssemblerErr err = {};

    // IOfile

    AsmDataInfo->file.asm_file = file->asm_file;
    AsmDataInfo->file.bin_file = file->bin_file;
    
    // code array

    AsmDataInfo->tokens_array = GetTokensArray(file->asm_file);
    const size_t code_size    = CalcCodeSize(&AsmDataInfo->tokens_array);
    int*         code_array   = (int*) calloc(code_size, sizeof(*code_array));

    if (!code_array)
    {
        err.err = AssemblerErrorType::BAD_CODE_ARRAY_CALLOC;
        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    AsmDataInfo->code.code = code_array;

    // log of tokens array

    ON_DEBUG(
    LOG_PRINT(Blue, "code size = %lu\n\n", AsmDataInfo->code.size);
    TokensLog(&AsmDataInfo->tokens_array, file->asm_file);
    )

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr AsmDataDtor(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    FREE(AsmDataInfo->code.code);
    // BufferDtor(&AsmDataInfo->cmd);
    TokensArrayDtor(&AsmDataInfo->tokens_array);
    LabelsDtor(AsmDataInfo);

    *AsmDataInfo = {};

    return err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr (*GetCmd(size_t cmd_pointer)) (AsmData* AsmDataInfo)
{
    Cmd cmd = (Cmd) cmd_pointer;

    AssemblerErr err = {};

    switch (cmd)
    {
        case Cmd::hlt:       return HandleHlt;
        case Cmd::push:      return HandlePush;
        case Cmd::pop:       return HandlePop;
        case Cmd::add:       return HandleAdd;
        case Cmd::sub:       return HandleSub;
        case Cmd::mul:       return HandleMul;
        case Cmd::dive:      return HandleDiv;
        case Cmd::pp:        return HandlePp;
        case Cmd::mm:        return HandleMm;
        case Cmd::out:       return HandleOut;
        case Cmd::outc:      return HandleOutc;
        case Cmd::outr:      return HandleOutr;
        case Cmd::outrc:     return HandleOutrc;
        case Cmd::jmp:       return HandleJmp;
        case Cmd::ja:        return HandleJa;
        case Cmd::jae:       return HandleJae;
        case Cmd::jb:        return HandleJb;
        case Cmd::jbe:       return HandleJbe;
        case Cmd::je:        return HandleJe;
        case Cmd::jne:       return HandleJne;
        case Cmd::call:      return HandleCall;
        case Cmd::ret:       return HandleRet;
        case Cmd::draw:      return HandleDraw;
        case Cmd::rgba:      return HandleRGBA;
        case Cmd::CMD_QUANT:
        default:
        {
            err.err = AssemblerErrorType::UNDEFINED_ENUM_COMMAND;
            ASSEMBLER_ASSERT(err);
            return nullptr;
        }
    }

    assert(0 && "we must not be here");
    return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr WriteCmdInCodeArr(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    size_t tokens_array_size = AsmDataInfo->tokens_array.size;

    while (AsmDataInfo->tokens_array.pointer < tokens_array_size)
    {
        Token token = GetNextToken(AsmDataInfo);
        size_t defaultCmdPointer = 0;

        if (IsTokenCommand(&token, &defaultCmdPointer))
        {
            ASSEMBLER_ASSERT(GetCmd(defaultCmdPointer)(AsmDataInfo));
            continue;
        }

        if (IsLabel(&token))
        {
            ASSEMBLER_ASSERT(HandleLabel(AsmDataInfo));
            continue;
        }

        err.err = AssemblerErrorType::UNDEFINED_COMMAND;
        return ASSEMBLER_VERIF(AsmDataInfo, err, token);
    }

    AsmDataInfo->code.size = AsmDataInfo->code.pointer;

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr WriteCodeArrInFile(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->code.code);

    AssemblerErr err = {};

    const char* codeFileName = AsmDataInfo->file.bin_file;

    FILE* codeFile = fopen(codeFileName, "wb");

    if (!codeFile)
    {
        err.err = AssemblerErrorType::FAILED_OPEN_OUTPUT_STREAM;
        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    size_t codeArrSize = AsmDataInfo->code.size;

    // ON_DEBUG(
    // LOG_PRINT(Red, "code arr size = '%lu'\n", codeArrSize);
    // LOG_ALL_INT_ARRAY(Yellow, AsmDataInfo->code.code, codeArrSize, 3);
    // )

    fprintf(codeFile, "%lu\n", codeArrSize);

    for (size_t i = 0; i < codeArrSize; i++)
    {
        // ON_DEBUG(
        // LOG_PRINT(Red, "code[%2lu] = '%d'\n", i, AsmDataInfo->code.code[i]);
        // )
        fprintf(codeFile, "%d ", AsmDataInfo->code.code[i]);
    }

    fclose(codeFile);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePush(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    char*       endBuffer = nullptr;

    StackElem_t PushElem = (StackElem_t) strtol(buffer.word, &endBuffer, 10);
    PushType    Push     = {};
    int         SetElem  = 0;
    int         Sum      = 0;


    Token    push_arg_token  = GetNextToken(AsmDataInfo);
    PushType push_type       = 0;
    int      push_arg_int    = 0;
    int      sum             = 0;

    if (IsTokenNumber(&push_arg_token))
    {
        push_type    = GetPushType(1, 0, 0, 0);
        push_arg_int = GetNumberFromToken(&push_arg_token);  
        sum          = 0;
    }

    else if (IsTokenRegister(&push_arg_token))
    {
        push_type    = GetPushType(0, 1, 0, 0);
        push_arg_int = GetRegisterFromToken(&push_arg_token);
        sum          = 0;
    }

    else if (IsTokenMemory(&push_arg_token))
    {
        UpdateBufferForMemory(&buffer);

        StackElem_t PushElemMemIndex = (StackElem_t) strtol(buffer.word, &endBuffer, 10);

        if (IsInt(&buffer, endBuffer))
        {
            PushTypeCtor(&Push, 0, 0, 1, 0);
            SetElem = PushElemMemIndex;
            Sum = 0;
        }

        else if (IsRegister(&buffer))
        {
            PushTypeCtor(&Push, 0, 1, 1, 0);
            SetElem = GetRegisterPointer(&buffer);
            Sum = 0;
        }

        else if (IsSum(&buffer))
        {
            PushTypeCtor(&Push, 0, 0, 1, 1);
            SetElem      = GetRegisterPointer(&buffer);
            buffer.word += REGISTERS_NAME_LEN + 1;
            Sum          = (int) strtol(buffer.word, &endBuffer, 10);
        }
    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
        return ASSEMBLER_VERIF(AsmDataInfo, err, buffer);
    }

    SetCmdArrCodeElem(AsmDataInfo, push);
    SetCmdArrCodeElem(AsmDataInfo, GetPushArg(&Push));
    SetCmdArrCodeElem(AsmDataInfo, SetElem);
    SetCmdArrCodeElem(AsmDataInfo, Sum);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushType GetPushType(bool stack, bool reg, bool memory, bool sum)
{
    PushType type = 0;
    type = (stack << 3) | (reg << 2) | (memory << 1) | (sum);

    return type;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetNumberFromToken(const Token* token)
{
    assert(token);

    const char* number_str = token->value.number.number;
    size_t      str_len    = token->value.number.number_len;

    char* number_str_end = nullptr;
    int number = (int) strtol(number_str, &number_str_end, 10);

    if ((size_t) (number_str_end - number_str) == str_len)
        return number;

    // it's char

    if (str_len != 4) // '\n'
        return number_str[1];

    return '\n';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetRegisterFromToken(const Token* token)
{
    assert(token);

    return (int) token->value.reg;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void UpdateBufferForMemory(Word* buffer)
{
    assert(buffer);
    assert(buffer->word);

    buffer->len -= 2;
    buffer->word++;

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

    Word buffer = GetNextCmd(AsmDataInfo);

    PopType Pop = {};
    int SetElem = 0;
    int Sum     = 0;

    if (IsRegister(&buffer))
    {
        PopTypeCtor(&Pop, 1, 0, 0);
        SetElem = GetRegisterPointer(&buffer);
        Sum = 0;
    }

    else if (IsMemory(&buffer))
    {
        UpdateBufferForMemory(&buffer);

        char*       endBuffer       = nullptr;
        StackElem_t PopElemMemIndex = (StackElem_t) strtol(buffer.word, &endBuffer,  10);

        if (IsInt(&buffer, endBuffer))
        {
            PopTypeCtor(&Pop, 0, 1, 0);
            SetElem = PopElemMemIndex;
            Sum = 0;
        }

        else if (IsRegister(&buffer))
        {
            PopTypeCtor(&Pop, 1, 1, 0);
            SetElem = GetRegisterPointer(&buffer);
            Sum = 0;
        }

        else if (IsSum(&buffer))
        {
            PopTypeCtor(&Pop, 0, 1, 1);
            SetElem      = GetRegisterPointer(&buffer);
            buffer.word += REGISTERS_NAME_LEN + 1;
            Sum          = (int) strtol(buffer.word, &endBuffer, 10);
        }
    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
        return ASSEMBLER_VERIF(AsmDataInfo, err, buffer);
    }

    SetCmdArrCodeElem(AsmDataInfo, pop);
    SetCmdArrCodeElem(AsmDataInfo, GetPopArg(&Pop));
    SetCmdArrCodeElem(AsmDataInfo, SetElem);
    SetCmdArrCodeElem(AsmDataInfo, Sum);


    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
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

    Word callArg = GetNextCmd(AsmDataInfo);

    int SetElem = 0;

    if (IsLabel(&callArg))
    {
        size_t labelPointer = 0;

        if (IsLabelAlready(AsmDataInfo, &callArg, &labelPointer))
        {
            Label label = AsmDataInfo->labels.labels[labelPointer];
            SetElem     = (int) (label.codePlace);
        }

        else
        {
            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err, callArg);
        }
    }

    else
    {
        err.err = AssemblerErrorType::LABEL_REDEFINE;
        return ASSEMBLER_VERIF(AsmDataInfo, err, callArg);
    }

    SetCmdArrCodeElem(AsmDataInfo, Cmd::call);
    SetCmdArrCodeElem(AsmDataInfo, SetElem);

    return ASSEMBLER_VERIF(AsmDataInfo, err, callArg);
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
        
    Word        ppArg    = GetNextCmd(AsmDataInfo);
    int         SetElem  = 0;

    if (IsRegister(&ppArg))
    {
        SetElem = GetRegisterPointer(&ppArg);
    }

    else
    {
        if (cmd == Cmd::pp)
        {
            err.err = AssemblerErrorType::INCORRECT_PP_ARG;
            return ASSEMBLER_VERIF(AsmDataInfo, err, ppArg);
        }

        else if (cmd == Cmd::mm)
        {
            err.err = AssemblerErrorType::INCORRECT_MM_ARG;
            return ASSEMBLER_VERIF(AsmDataInfo, err, ppArg);
        }

        assert(0 && "undef situation: must be 'pp' or 'mm' cmd");
        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    SetCmdArrCodeElem(AsmDataInfo, cmd);
    SetCmdArrCodeElem(AsmDataInfo, SetElem);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
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

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleDraw(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    Word firstArg  = GetNextCmd(AsmDataInfo);
    Word secondArg = GetNextCmd(AsmDataInfo);

    // char* firstArgEndPtr  = nullptr;
    // char* secondArgEndPtr = nullptr;

    // strtol(firstArg.word,  &firstArgEndPtr,  10);
    // strtol(secondArg.word, &secondArgEndPtr, 10);
    
    bool IsReg1 = IsRegister(&firstArg);
    bool IsReg2 = IsRegister(&secondArg);

    
    if (IsReg1 && IsReg2)
    {
        SetCmdArrCodeElem(AsmDataInfo, Cmd::draw                     );
        SetCmdArrCodeElem(AsmDataInfo, GetRegisterPointer(&firstArg ));
        SetCmdArrCodeElem(AsmDataInfo, GetRegisterPointer(&secondArg));

        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    else if (!IsReg1)
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
        return ASSEMBLER_VERIF(AsmDataInfo, err, firstArg);
    }

    else if (!IsReg2)
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
        return ASSEMBLER_VERIF(AsmDataInfo, err, secondArg);
    }

    err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
    return ASSEMBLER_VERIF(AsmDataInfo, err, firstArg);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef int RGBATYpe;

static RGBATYpe GetRGBAType(bool isReg[4])
{
    return (isReg[3] << 24) | (isReg[2] << 16) | (isReg[1] << 8) | isReg[0];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleRGBA(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    Word   arg   [4] = {};
    char*  argEnd[4] = {};
    int    argInt[4] = {};
    bool   isReg [4] = {};
    bool   isInt [4] = {};

    for (size_t i = 0; i < 4; i++) arg   [i] =       GetNextCmd (AsmDataInfo                 );
    for (size_t i = 0; i < 4; i++) argInt[i] = (int) strtol     ( arg[i].word, &argEnd[i], 10);
    for (size_t i = 0; i < 4; i++) isReg [i] =       IsRegister (&arg[i]                     );
    for (size_t i = 0; i < 4; i++) isInt [i] =       IsInt      (&arg[i],       argEnd[i]    );


    SetCmdArrCodeElem(AsmDataInfo, Cmd::rgba);
    SetCmdArrCodeElem(AsmDataInfo, GetRGBAType(isReg));

    for (size_t i = 0; i < 4; i++)
    {
        if (isReg[i])
        {
            SetCmdArrCodeElem(AsmDataInfo, GetRegisterPointer(&arg[i]));
            continue;
        }
    
        else if (isInt[i])
        {
            SetCmdArrCodeElem(AsmDataInfo, argInt[i]);
            continue;
        }
        
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
        return ASSEMBLER_VERIF(AsmDataInfo, err, arg[i]);
    }

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOneLineComment(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->cmd.words);
    assert(AsmDataInfo->cmd.pointer >= 1);

    AssemblerErr err = {};

    size_t       cmdQuant = AsmDataInfo->cmd.size;

    CmdArr       cmdArr   = AsmDataInfo->cmd;
    size_t       pointer  = AsmDataInfo->cmd.pointer - 1;
    Word         cmd      = cmdArr.words[pointer];
    const size_t line     = cmd.line;

    
    while (cmd.line == line && pointer < cmdQuant)
    {
        pointer++;
        cmd = cmdArr.words[pointer];
    }

    // pointer++;

    AsmDataInfo->cmd.pointer = pointer;

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleTwoSymbolComment(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->cmd.words);
    assert(AsmDataInfo->cmd.pointer >= 1);

    AssemblerErr err = {};

    size_t       cmdQuant = AsmDataInfo->cmd.size;

    CmdArr       cmdArr   = AsmDataInfo->cmd;
    size_t       pointer  = AsmDataInfo->cmd.pointer - 1;
    Word         cmd      = cmdArr.words[pointer];

    while (!IsTwoSymbolCommentEnd(&cmd) && pointer < cmdQuant)
    {
        pointer++;
        cmd = cmdArr.words[pointer];
    }

    pointer++;

    AsmDataInfo->cmd.pointer = pointer;

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr NullArgCmdPattern(AsmData* AsmDataInfo, Cmd cmd)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
    SetCmdArrCodeElem(AsmDataInfo, cmd);
    AsmDataInfo->code.size++;
    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr JmpCmdPattern(AsmData* AsmDataInfo, Cmd JumpType)
{ 
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    Word JumpArg = GetNextCmd(AsmDataInfo);

    int SetElem = 0;

    if (IsLabel(&JumpArg))
    {
        size_t labelPointer = 0;

        if (IsLabelAlready(AsmDataInfo, &JumpArg, &labelPointer))
        {
            Label label = AsmDataInfo->labels.labels[labelPointer];
            SetElem     = (int) label.codePlace;
        }

        else
        {
            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err, JumpArg);
        }
    }

    else if (IsStrInt(&JumpArg))
    {
        SetElem = WordToInt(&JumpArg);
    }

    else
    {
        err.err = AssemblerErrorType::LABEL_REDEFINE;
        return ASSEMBLER_VERIF(AsmDataInfo, err, JumpArg);
    }

    SetCmdArrCodeElem(AsmDataInfo, JumpType);
    SetCmdArrCodeElem(AsmDataInfo, SetElem);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr InitAllLabels(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->cmd.words);

    AssemblerErr err = {};

    // CmdArr cmdArr    = AsmDataInfo->cmd;
    // size_t cmdQuant  = AsmDataInfo->cmd.size;

    const Token* tokens_array = AsmDataInfo->tokens_array.array;
    const size_t tokens_quant = AsmDataInfo->tokens_array.size;

    ASSEMBLER_ASSERT(LabelsCtor(AsmDataInfo));


    size_t token_pointer = 0;

    size_t cmd_pointer  = 0; //
    size_t code_pointer = 0; //

    while(cmd_pointer < AsmDataInfo->cmd.size)
    {
        // Word   cmd      = AsmDataInfo->cmd.words[cmd_pointer];
        bool   defined   = true;
        size_t cmd_index = cmd_pointer;

        Token token = tokens_array[token_pointer];

        if (IsTokenCommand(&token, &cmd_index))
        {
            cmd_pointer  += CmdInfoArr[cmd_index].argQuant + 1;
            code_pointer += CmdInfoArr[cmd_index].codeRecordSize;
            continue;
        }
    
        if (IsLabel(&token))
        {
            const TokenizerLabel token_label = token.value.label;
    
            if (!IsLabelAlready(AsmDataInfo, &token_label, &cmd_index))
            {
                cmd_pointer++;
                Label label = LabelCtor(&token_label, code_pointer, defined);
                ASSEMBLER_ASSERT(PushLabel(AsmDataInfo, &label));
                continue;
            }

            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err, token);
        }

        err.err = AssemblerErrorType::UNDEFINED_COMMAND;
        return ASSEMBLER_VERIF(AsmDataInfo, err, token);
    }


    // ON_DEBUG(
    // size_t size = AsmDataInfo->labels.size;
    // for (size_t i = 0; i < size; i++)
    // {
    //     LOG_PRINT(Blue, "label[%2lu] = .name = '%10s', .codePlace = '%3lu', .alreadyDefined = '%d'\n", i, AsmDataInfo->labels.labels[i].name, AsmDataInfo->labels.labels[i].codePlace, AsmDataInfo->labels.labels[i].alradyDefined);
    // }
    // )
    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenCommand(const Token* token, size_t* cmd_pointer)
{
    assert(token);
    assert(cmd_pointer);

    if (token->type != TokenType::token_command)
        return false;

    Cmd command = token->value.command;

    for (size_t i = 0; i < CmdInfoArrSize; i++)
    {
        if (command == CmdInfoArr[i].cmd)
        {
            *cmd_pointer = i;
            return true;
        }
    }

    assert(0 && "undef cmd");

    return false;
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
        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    AsmDataInfo->labels.capacity = DefaultLabelsQuant;

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr LabelsDtor(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
    
    FREE(AsmDataInfo->labels.labels);
    AsmDataInfo->labels = {};

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Label LabelCtor(const TokenizerLabel* token_label, size_t pointer, bool alreadyDefined)
{
    assert(token_label);

    Label label = {};

    label.name          = token_label->name;
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
        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    size_t new_capacity = 2 * capacity;
    Labels->labels = (Label*) realloc(Labels->labels, new_capacity * sizeof(Label));

    if (!Labels->labels)
    {
        err.err = AssemblerErrorType::BAD_LABELS_REALLOC;
        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    Labels->labels[Labels->size - 1] = *label;

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLabelAlready(const AsmData* AsmDataInfo, const TokenizerLabel* label, size_t* labelPlace)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->labels.labels);
    
    Labels labels = AsmDataInfo->labels;
    size_t size   = labels.size;

    for (size_t labelPointer = 0; labelPointer < size; labelPointer++)
    {
        const char* temp = labels.labels[labelPointer].name;

        if (strcmp(label->name, temp) == 0)
        {
            *labelPlace = labelPointer;
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static size_t CalcCodeSize(const TokensArray* tokens_array)
{
    assert(tokens_array);

    const size_t tokens_array_size = tokens_array->size;
    const Token* tokens            = tokens_array->array;
    
    size_t code_array_size = 0;

    for (size_t cmd_pointer = 0; cmd_pointer < tokens_array_size; cmd_pointer++)
    {
        const Token token = tokens[cmd_pointer];

        const CmdInfo cmd_info = GetCmdInfo(&token);

        if (cmd_info.cmd != Cmd::undef_cmd)
        {
            cmd_pointer            += cmd_info.argQuant;
            code_array_size        += cmd_info.codeRecordSize;
        }
    }

    return code_array_size;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static CmdInfo GetCmdInfo(const Token* token)
{
    assert(token);

    if (token->type != TokenType::token_command)
        return {};

    Cmd token_cmd = token->value.command;

    for (size_t i = 0; i < CmdInfoArrSize; i++)
    {
        CmdInfo cmd = CmdInfoArr[i];
    
        if (token_cmd == cmd.cmd)
            return cmd;
    }

    assert(0 && "we must find cmd and return in cycle");
    return {};
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool FindDefaultCmd(const Token* token, size_t* defaultCmdPointer)
{
    assert(token);
    assert(defaultCmdPointer);


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

static int GetRegisterPointer(const Word* buffer)
{
    assert(buffer);

    const char fisrtBuf  = buffer->word[0];

    return fisrtBuf - 'a';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static char GetChar(const Word* buffer)
{
    assert(buffer);
    assert(buffer->word);

    size_t      wordLen = buffer->len;
    const char* word    = buffer->word;

    if (wordLen == 4)
    {
        const char word2 = word[2]; 

        if      (word2 == 'n') return '\n';
        else if (word2 == '_') return ' ';

        assert(0 && "undef situation: must be '\n' only");
    }

    else if (wordLen != 3)
    {
        assert(0 && "undef situation: must be size = 3 or 4");
    }

    return word[1];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsStrInt(const Word* word)
{
    assert(word);

    const char* str = word->word;    

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

static bool IsTokenNumber(const Token* token)
{
    assert(token);
    return (token->type == TokenType::token_number);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsChar(const Word* word, char* wordEndPtr)
{
    assert(word);
    assert(wordEndPtr);

    size_t     strLen  = word->len;
    const char str0    = word->word[0];
    const char strLast = word->word[strLen - 1];

    if (!(3 <= strLen && strLen <= 4) ||
         (str0    != '\'')              ||
         (strLast != '\''))
    {
        return false;
    }

    else if (strLen == 4)
    {
        const char str1 = word->word[1];
        const char str2 = word->word[2];

        return  (str1 == '\\') &&
               ((str2 == 'n' ) || (str2 == '_'));
    }

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenRegister(const Token* token)
{
    assert(token);
    return (token->type == TokenType::token_register);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenMemory(const Token* token)
{
    assert(token);
    return  (token->type          == TokenType::token_bracket) &&
            (token->value.bracket == Bracket::left_bracket);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSum(const Word* word)
{
    assert(word);

    const char* str = word->word;
    size_t      len = word->len;

    bool WasPlus = false;

    for (size_t i = 0; i < len; i++)
    {
        if      (str[i] == '+' && !WasPlus) WasPlus = true;
        else if (str[i] == '+' &&  WasPlus) return false;
    }

    return WasPlus;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLabel(const Token* token)
{
    assert(token);
    return (token->type == TokenType::token_label);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsOneLineComment(const Word* cmd)
{
    assert(cmd);

    return cmd->word[0] == ';';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTwoSymbolCommentBegin(const Word* cmd)
{
    assert(cmd);

    return cmd->word[0] == '#';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTwoSymbolCommentEnd(const Word* cmd)
{
    assert(cmd);

    size_t len = cmd->len;
    char   lastCmdNameChar = cmd->word[len - 1];

    return lastCmdNameChar == '/';
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetCmdName(size_t cmd_pointer)
{
    return CmdInfoArr[cmd_pointer].name;
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

static Word GetNextCmd(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AsmDataInfo->cmd.pointer++;
    size_t pointer = AsmDataInfo->cmd.pointer - 1;
    return AsmDataInfo->cmd.words[pointer];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Token GetNextToken(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    size_t pointer = AsmDataInfo->tokens_array.pointer;
    AsmDataInfo->tokens_array.pointer++;
    return AsmDataInfo->tokens_array.array[pointer];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void AssemblerAssertPrint(const AssemblerErr* err, const char* file, int line, const char* func)
{
    assert(file);
    assert(func);
    
    PrintError(err);

    ON_DEBUG(
    printf("\n");
    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(file, line, func);
    printf("\n");

    COLOR_PRINT(RED, "Error detected in:\n");
    PrintPlace(err->place.file, err->place.line, err->place.func);
    )

    COLOR_PRINT(CYAN, "\nexit() in 3, 2, 1...\n");
    
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void CheckInputFiles(const IOfile* files)
{
    assert(files);

    if (!files->asm_file)
        EXIT(EXIT_FAILURE, "asm file name is nullptr.");

    if (!files->bin_file)
        EXIT(EXIT_FAILURE, "bin file name is nullptr.");


    const char* asm_file_extension = GetFileExtension(files->asm_file);

    if ((!asm_file_extension) || (strcmp(asm_file_extension, "asm") != 0))
        EXIT(EXIT_FAILURE, "bad asm extension: '%s'\nmust be '.asm'", files->asm_file);
    

    const char* bin_file_extension = GetFileExtension(files->bin_file);

    if ((!bin_file_extension) ||strcmp(bin_file_extension, bin_extension) != 0)
        EXIT(EXIT_FAILURE, "bad bin extension: '%s'\nmust be '.bin'", files->bin_file);    

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr Verif(const AsmData* AsmDataInfo, AssemblerErr* err, Token token, const char* file, int line, const char* func)
{    
    assert(file);
    assert(func);
    assert(err);

    CodePlaceCtor(&err->place, file, line, func);

    err->token = token;

    if (AsmDataInfo)
        err->file = AsmDataInfo->file;


    if (!AsmDataInfo)
    {
        return *err;
    }

    return *err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintIncorrectCmd(const char* msg, const char* file, Token token)
{
    assert(msg);
    assert(file);

    COLOR_PRINT(RED, 
        "%s '%s'\n",
        msg, "what do you want mothetfucker");

    PrintIncorrectCmdFilePlace(file, token);

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetTokenInStr(Token token)
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintIncorrectCmdFilePlace(const char* file, Token token)
{
    assert(file);

    COLOR_PRINT(
        WHITE,
        "%s:%lu:%lu\n",
        file, token.place.line, token.place.pos_in_line
    );

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(const AssemblerErr* err)
{
    assert(err);

    const char* inputStream  = err->file.asm_file;
    const char* outputStream = err->file.bin_file;
    const char* cmdName      = nullptr;
    

    switch (err->err)
    {
        case AssemblerErrorType::NO_ERR:
            return;

        case AssemblerErrorType::FAILED_OPEN_INPUT_STREAM:
            COLOR_PRINT(RED, "Error: failed open input file: '%s'.\n", inputStream);
            break;

        case AssemblerErrorType::FAILED_OPEN_OUTPUT_STREAM:
            COLOR_PRINT(RED, "Error: failed open output file: '%s'.\n", outputStream);
            break;

        case AssemblerErrorType::INVALID_INPUT_AFTER_PUSH:
            COLOR_PRINT(RED, "Error: invalid input after push: '%s'.\n", cmdName);
            PrintIncorrectCmdFilePlace(inputStream, err->token);
            break;

        case AssemblerErrorType::INVALID_INPUT_AFTER_POP:
            COLOR_PRINT(RED, "Error: invalid input after pop: '%s'.\n", cmdName);
            PrintIncorrectCmdFilePlace(inputStream, err->token);
            break;

        case AssemblerErrorType::UNDEFINED_COMMAND:
            PrintIncorrectCmd("undefined reference to:", inputStream, err->token);
            break;

        case AssemblerErrorType::UNDEFINED_ENUM_COMMAND:
            COLOR_PRINT(RED, "Error: undefined enum command\n");
            break;

        case AssemblerErrorType::BAD_CODE_ARRAY_CALLOC:
            COLOR_PRINT(RED, "Errod: failed calloc for code array.\n");
            break;

        case AssemblerErrorType::FWRITE_BAD_RETURN:
            COLOR_PRINT(RED, "Error: failed to write all code arr in output stream.\n");
            break;

        case AssemblerErrorType::LABEL_REDEFINE:
            PrintIncorrectCmd("label redefine:", inputStream, err->token);
            break;

        case AssemblerErrorType::BAD_LABELS_CALLOC:
            COLOR_PRINT(RED, "Error: failed to allocate memory for labels.\n");
            break;

        case AssemblerErrorType::INCORRECT_SUM_FIRST_OPERAND:
            PrintIncorrectCmd("incorrect first sum argument in push/pop:", inputStream, err->token);
            break;

        case AssemblerErrorType::INCORRECT_SUM_SECOND_OPERAND:
            PrintIncorrectCmd("incorrect second sum argument in push/pop:", inputStream, err->token);
            break;

        case AssemblerErrorType::INCORRECT_PP_ARG:
            COLOR_PRINT(RED, "Error: incorrect pp arg\n");
            PrintIncorrectCmd("incorrect 'pp' arg:", inputStream, err->token);
            break;

        case AssemblerErrorType::INCORRECT_MM_ARG:            
            PrintIncorrectCmd("incorrect 'mm' arg:", inputStream, err->token);
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
