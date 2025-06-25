#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
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
    size_t size    ;
    size_t capacity;
    size_t pointer ;
    Label* labels  ;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct AsmData
{
    TokensArray tokens_array;
    CodeArr    code         ;
    Labels     labels       ;
    IOfile     file         ;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

using PushType = uint8_t;
static_assert(sizeof(PushType) == 1, "for economy of memory PushType must have size 1 byte. In your system uint8_t is not 1 byte");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

using PopType = uint8_t;
static_assert(sizeof(PopType) == 1, "for economy of memory PopType must have size 1 byte. In your system uint8_t is not 1 byte");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

using DrawType = uint8_t;
static_assert(sizeof(PopType) == 1, "for economy of memory DrawType must have size 1 byte. In your system uint8_t is not 1 byte");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

using RGBAtype = uint8_t;
static_assert(sizeof(PopType) == 1, "for economy of memory DrawType must have size 1 byte. In your system uint8_t is not 1 byte");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void         CheckInputFiles          (const IOfile* files);

static AssemblerErr AsmDataCtor              (AsmData* AsmDataInfo, const IOfile* file);
static AssemblerErr AsmDataDtor              (AsmData* AsmDataInfo);
static AssemblerErr WriteCmdInCodeArr        (AsmData* AsmDataInfo);
static AssemblerErr WriteCodeArrInFile       (AsmData* AsmDataInfo);
   
   
static void         SetNextCodeInstruction        (AsmData* AsmDataInfo, int instruction);
static Token GetNextToken(AsmData* AsmDataInfo);

   
static AssemblerErr NullArgCmdPattern        (AsmData* AsmDataInfo, Cmd cmd);
   
static AssemblerErr PpMmPattern              (AsmData* AsmDataInfo, Cmd cmd);
   

static bool IsTokenCommand(const Token* token, size_t* cmd_pointer);


static bool         IsTokenLabel(const Token* token);

static AssemblerErr InitAllLabels           (AsmData* AsmDataInfo);
static AssemblerErr LabelsCtor              (AsmData* AsmDataInfo);
static AssemblerErr LabelsDtor              (AsmData* AsmDataInfo);

static Label        LabelCtor               (const TokenizerLabel* token_label, size_t pointer, bool alreadyDefined);
static AssemblerErr PushLabel               (AsmData* AsmDataInfo, const Label* label);
static bool IsLabelAlready(const AsmData* AsmDataInfo, const TokenizerLabel* label, size_t* labelPlace);


static CmdInfo      GetCmdInfo              (const Token* token);
static size_t       CalcCodeSize            (const TokensArray* tokens_array);

static AssemblerErr JmpCmdPattern           (AsmData* AsmDataInfo, Cmd jump_type);


static bool IsTokenNumber(const Token* token);
static PushType GetPushType(bool stack, bool reg, bool memory, bool sum);
static int GetNumberFromToken(const Token* token);
static bool IsTokenRegister(const Token* token);
static int GetRegisterFromToken(const Token* token);
static bool IsTokenMemory(const Token* token);
static bool IsTokenRightBracket(const Token* token);
static bool IsTokenPlus(const Token* token);
static PopType GetPopType(bool reg, bool memory, bool int_mem_addr, bool aligment);
static DrawType GetDrawType(bool IsReg[3]);


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
        case Cmd::undef_cmd:
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

        if (IsTokenLabel(&token))
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

    assert(0);

    AssemblerErr err = {};

    Token    push_arg_token  = GetNextToken(AsmDataInfo);
    PushType push_type       = 0;
    int      push_arg_int    = 0;
    int      aligment        = 0;

    if (IsTokenNumber(&push_arg_token))
    {
        push_type    = GetPushType(1, 0, 0, 0);
        push_arg_int = GetNumberFromToken(&push_arg_token);
        aligment     = 0;
    }

    else if (IsTokenRegister(&push_arg_token))
    {
        push_type    = GetPushType(0, 1, 0, 0);
        push_arg_int = GetRegisterFromToken(&push_arg_token);
        aligment     = 0;
    }

    else if (IsTokenMemory(&push_arg_token))
    {
              push_arg_token      = GetNextToken(AsmDataInfo);
        Token push_arg_next_token = GetNextToken(AsmDataInfo);

        if (IsTokenRightBracket(&push_arg_next_token))
        {
            if (IsTokenNumber(&push_arg_token))
            {
                push_type    = GetPushType(0, 0, 1, 0);
                push_arg_int = GetNumberFromToken(&push_arg_token);
                aligment     = 0;
            }

            else if (IsTokenRegister(&push_arg_token))
            {
                push_type    = GetPushType(0, 1, 1, 0);
                push_arg_int = GetRegisterFromToken(&push_arg_token);
                aligment     = 0;
            }

            else
            {
                err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
                return ASSEMBLER_VERIF(AsmDataInfo, err, push_arg_token);
            }
        }

        else if (IsTokenRegister(&push_arg_token) &&
                 IsTokenPlus(&push_arg_next_token))
        {
            push_arg_next_token = GetNextToken(AsmDataInfo);

            if (IsTokenNumber(&push_arg_next_token))
            {
                push_type    = GetPushType(0, 1, 0, 1);
                push_arg_int = GetRegisterFromToken(&push_arg_token);
                aligment     = GetNumberFromToken(&push_arg_next_token);
            }
                
            else
            {
                err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
                return ASSEMBLER_VERIF(AsmDataInfo, err, push_arg_token);   
            }

            push_arg_next_token = GetNextToken(AsmDataInfo);

            if (IsTokenRightBracket(&push_arg_next_token))
            {
                err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
                return ASSEMBLER_VERIF(AsmDataInfo, err, push_arg_token);
            }
        }
    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
        return ASSEMBLER_VERIF(AsmDataInfo, err, push_arg_token);
    }

    SetNextCodeInstruction(AsmDataInfo, push);
    SetNextCodeInstruction(AsmDataInfo, push_type);
    SetNextCodeInstruction(AsmDataInfo, push_arg_int);
    SetNextCodeInstruction(AsmDataInfo, aligment);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushType GetPushType(bool stack, bool reg, bool memory, bool sum)
{
    PushType type = 0;
    type = (PushType) ((stack << 3) | (reg << 2) | (memory << 1) | (sum));

    return type;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetNumberFromToken(const Token* token)
{
    assert(token);

    const char* number_str = token->value.number.number    ; assert(number_str);
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

static bool IsTokenPlus(const Token* token)
{
    assert(token);
    return  (token->type            == TokenType   ::token_math_operation) &&
            (token->value.operation == MathOperator::plus);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePop(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    Token   pop_arg_token = GetNextToken(AsmDataInfo);
    PopType pop_type      = 0;
    int     pop_arg_int   = 0;
    int     aligment      = 0;

    if (IsTokenRegister(&pop_arg_token))
    {
        pop_type    = GetPopType(1, 0, 0, 0);
        pop_arg_int = GetRegisterFromToken(&pop_arg_token);
        aligment    = 0;
    }

    else if (IsTokenMemory(&pop_arg_token))
    {
              pop_arg_token      = GetNextToken(AsmDataInfo);
        Token pop_arg_next_token = GetNextToken(AsmDataInfo);

        if (IsTokenRightBracket(&pop_arg_next_token))
        {
            if (IsTokenNumber(&pop_arg_token))
            {
                pop_type    = GetPopType(0, 1, 1, 0);
                pop_arg_int = GetNumberFromToken(&pop_arg_token);
                aligment    = 0;
            }

            else if (IsTokenRegister(&pop_arg_token))
            {
                pop_type    = GetPopType(1, 1, 0, 0);
                pop_arg_int = GetRegisterFromToken(&pop_arg_token);
                aligment    = 0;
            }

            else
            {
                err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
                return ASSEMBLER_VERIF(AsmDataInfo, err, pop_arg_token);
            }
        }

        else if (IsTokenRegister(&pop_arg_token) &&
                 IsTokenPlus    (&pop_arg_next_token))
        {
            pop_arg_next_token = GetNextToken(AsmDataInfo);
            
            if (IsTokenNumber(&pop_arg_next_token))
            {
                pop_type    = GetPopType(1, 1, 0, 1);
                pop_arg_int = GetRegisterFromToken(&pop_arg_token);
                aligment    = GetNumberFromToken(&pop_arg_next_token);
            }

            else
            {
                err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
                return ASSEMBLER_VERIF(AsmDataInfo, err, pop_arg_token);
            }

        }

    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
        return ASSEMBLER_VERIF(AsmDataInfo, err, pop_arg_token);
    }

    SetNextCodeInstruction(AsmDataInfo, Cmd::pop);
    SetNextCodeInstruction(AsmDataInfo, pop_type);
    SetNextCodeInstruction(AsmDataInfo, pop_arg_int);
    SetNextCodeInstruction(AsmDataInfo, aligment);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PopType GetPopType(bool reg, bool memory, bool int_mem_addr, bool aligment)
{
    PopType type = 0;
    type = (PopType) ((reg << 3) | (memory << 2) | (int_mem_addr << 1) | aligment);

    return type;
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

    Token call_arg_token = GetNextToken(AsmDataInfo);
    int   call_arg_int   = 0;

    if (IsTokenLabel(&call_arg_token))
    {
        size_t label_pointer = 0;

        if (IsLabelAlready(AsmDataInfo, &call_arg_token.value.label, &label_pointer))
        {
            Label label  = AsmDataInfo->labels.labels[label_pointer];
            call_arg_int = (int) label.codePlace;
        }

        else
        {
            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err, call_arg_token);
        }
    }

    else
    {
        err.err = AssemblerErrorType::LABEL_REDEFINE;
        return ASSEMBLER_VERIF(AsmDataInfo, err, call_arg_token);
    }

    SetNextCodeInstruction(AsmDataInfo, Cmd::call);
    SetNextCodeInstruction(AsmDataInfo, call_arg_int);

    return ASSEMBLER_VERIF(AsmDataInfo, err, call_arg_token);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleRet(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::ret);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr PpMmPattern(AsmData* AsmDataInfo, Cmd pp)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
        
    Token  pp_arg_token = GetNextToken(AsmDataInfo);
    int    pp_arg_int   = 0;

    if (IsTokenRegister(&pp_arg_token))
    {
        pp_arg_int = GetRegisterFromToken(&pp_arg_token);
    }

    else
    {
        if (pp == Cmd::pp)
        {
            err.err = AssemblerErrorType::INCORRECT_PP_ARG;
            return ASSEMBLER_VERIF(AsmDataInfo, err, pp_arg_token);
        }

        else if (pp== Cmd::mm)
        {
            err.err = AssemblerErrorType::INCORRECT_MM_ARG;
            return ASSEMBLER_VERIF(AsmDataInfo, err, pp_arg_token);
        }

        assert(0 && "undef situation: must be 'pp' or 'mm' cmd");
        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    SetNextCodeInstruction(AsmDataInfo, pp);
    SetNextCodeInstruction(AsmDataInfo, pp_arg_int);

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

    Token draw_arg_token[3] = 
    {
        GetNextToken(AsmDataInfo),
        GetNextToken(AsmDataInfo),
        GetNextToken(AsmDataInfo),
    };

    int  draw_arg_int[3] = {};
    bool IsReg       [3] = {};

    for (size_t i = 0; i < 3; i++)
    {
        Token token = draw_arg_token[i];
        if (IsTokenRegister(&token))
        {
            IsReg[i] = 1;
            draw_arg_int[i] = GetRegisterFromToken(&token);
            continue;
        }

        else if (IsTokenNumber(&token))
        {
            IsReg[i] = 0;
            draw_arg_int[i] = GetNumberFromToken(&token);
            continue;
        }

        err.err = AssemblerErrorType::INCORRECT_PP_ARG;
        return ASSEMBLER_VERIF(AsmDataInfo, err, token);
    }

    DrawType type = GetDrawType(IsReg);

    SetNextCodeInstruction(AsmDataInfo, Cmd::draw);
    SetNextCodeInstruction(AsmDataInfo, type);
    for (size_t i = 0; i < 3; i++)
        SetNextCodeInstruction(AsmDataInfo, draw_arg_int[i]);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static DrawType GetDrawType(bool IsReg[3])
{
    DrawType type = 0;
    type =  (DrawType) ((IsReg[2] << 2) | (IsReg[1] << 1) | (IsReg[0]));
    return type;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef uint8_t RGBATYpe;

static RGBATYpe GetRGBAType(bool isReg[4])
{
    RGBATYpe type = 0;
    type = (RGBATYpe) ((isReg[3] << 3) | (isReg[2] << 2) | (isReg[1] << 1) | (isReg[0]));
    return type;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleRGBA(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    Token rgba_arg_token[4] = 
    {
        GetNextToken(AsmDataInfo),
        GetNextToken(AsmDataInfo),
        GetNextToken(AsmDataInfo),
        GetNextToken(AsmDataInfo),
    };

    int  rgba_arg_int[4] = {};
    bool IsRegister  [4] = {};

    for (size_t i = 0; i < 4; i++)
    {
        Token token = rgba_arg_token[i];

        if (IsTokenRegister(&token))
        {
            rgba_arg_int[i] = GetRegisterFromToken(&token);
            IsRegister  [i] = true;
            continue;
        }
    
        else if (IsTokenNumber(&token))
        {
            rgba_arg_int[i] = GetNumberFromToken(&token);
            IsRegister  [i] = false;
            continue;
        }

        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;
        return ASSEMBLER_VERIF(AsmDataInfo, err, rgba_arg_token[i]);
    }

    RGBAtype type = GetRGBAType(IsRegister);

    SetNextCodeInstruction(AsmDataInfo, Cmd::rgba);
    SetNextCodeInstruction(AsmDataInfo, type);
    for (size_t i = 0; i < 4; i++)
        SetNextCodeInstruction(AsmDataInfo, rgba_arg_int[i]);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr NullArgCmdPattern(AsmData* AsmDataInfo, Cmd cmd)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
    SetNextCodeInstruction(AsmDataInfo, cmd);
    AsmDataInfo->code.size++;
    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr JmpCmdPattern(AsmData* AsmDataInfo, Cmd jump_type)
{ 
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    Token jmp_arg_token = GetNextToken(AsmDataInfo);
    int   jmp_arg_int   = 0;

    if (IsTokenLabel(&jmp_arg_token))
    {
        size_t label_pointer = 0;

        if (IsLabelAlready(AsmDataInfo, &jmp_arg_token.value.label, &label_pointer))
        {
            Label label = AsmDataInfo->labels.labels[label_pointer];
            jmp_arg_int = (int) label.codePlace;
        }

        else
        {
            err.err = AssemblerErrorType::LABEL_REDEFINE;
            return ASSEMBLER_VERIF(AsmDataInfo, err, jmp_arg_token);
        }
    }

    else if (IsTokenNumber(&jmp_arg_token))
    {
        jmp_arg_int = GetNumberFromToken(&jmp_arg_token);
    }

    else
    {
        err.err = AssemblerErrorType::LABEL_REDEFINE;
        return ASSEMBLER_VERIF(AsmDataInfo, err, jmp_arg_token);
    }

    SetNextCodeInstruction(AsmDataInfo, jump_type);
    SetNextCodeInstruction(AsmDataInfo, jmp_arg_int);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr InitAllLabels(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    // CmdArr cmdArr    = AsmDataInfo->cmd;
    // size_t cmdQuant  = AsmDataInfo->cmd.size;

    const Token* tokens_array = AsmDataInfo->tokens_array.array;
    const size_t tokens_quant = AsmDataInfo->tokens_array.size;

    ASSEMBLER_ASSERT(LabelsCtor(AsmDataInfo));


    size_t token_pointer = 0;
    size_t code_pointer = 0; //

    while(token_pointer < tokens_quant)
    {
        bool   defined   = true;
        size_t cmd_index = 0;

        Token token = tokens_array[token_pointer];

        if (IsTokenCommand(&token, &cmd_index))
        {
            token_pointer  += CmdInfoArr[cmd_index].argQuant + 1;
            code_pointer   += CmdInfoArr[cmd_index].codeRecordSize;
            continue;
        }
    
        if (IsTokenLabel(&token))
        {
            const TokenizerLabel token_label = token.value.label;
    
            if (!IsLabelAlready(AsmDataInfo, &token_label, &cmd_index))
            {
                token_pointer++;
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

    static const size_t DefaultLabelsQuant = 10;

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

    for (size_t label_pointer = 0; label_pointer < size; label_pointer++)
    {
        Label label_i = labels.labels[label_pointer];
        

        if (strncmp(label->name, label_i.name, label->name_len) == 0)
        {
            *labelPlace = label_pointer;
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

static bool IsTokenNumber(const Token* token)
{
    assert(token);
    return (token->type == TokenType::token_number);
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

static bool IsTokenRightBracket(const Token* token)
{
    assert(token);
    return  (token->type          == TokenType::token_bracket) &&
            (token->value.bracket == Bracket::right_bracket);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenLabel(const Token* token)
{
    assert(token);
    return (token->type == TokenType::token_label);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetNextCodeInstruction(AsmData* AsmDataInfo, int instruction)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->code.code);

    size_t pointer = AsmDataInfo->code.pointer;
    AsmDataInfo->code.code[pointer] = instruction;
    AsmDataInfo->code.pointer++;
    return;
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

    assert(token.place.line != 1337);

    COLOR_PRINT(RED, 
        "%s '%s'\n",
        msg, "what do you want mothetfucker");

    PrintIncorrectCmdFilePlace(file, token);

    return;
}

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
    const char* cmdName      = "poka jdem";
    

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

        case AssemblerErrorType::BAD_LABELS_REALLOC:
            COLOR_PRINT(RED, "Error: failed to reallocate memory for labels.\n");
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
