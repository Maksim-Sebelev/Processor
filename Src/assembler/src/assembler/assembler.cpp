#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "assembler/assembler.hpp"
#include "global/global_include.hpp"
#include "lib/lib.hpp"
#include "tokenizer/tokenizer.hpp"
#include "functions_for_files/files.hpp"
#include "assembler/labels/labels.hpp"
#include "assembler/code_array/code_array.hpp"

#ifdef _DEBUG
#include "logger/log.hpp"
#include "tokenizer/tokens_log.hpp"
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
    NO_COMMA                     ,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct AssemblerErr
{
    CodePlace          place;
    AssemblerErrorType err  ;
    Token              token;
    IOfile             file ;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct AsmData
{
    TokensArray tokens_array;
    CodeArray   code_array  ;
    LabelsArray labels      ;
    IOfile      file        ;
    Buffer      buffer      ;
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

using RGBATYpe = uint8_t;
static_assert(sizeof(RGBATYpe) == 1, "for economy of memory RGBAType must have size 1 byte. In your system uint8_t is not 1 byte");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct PushCodeArgs
{
    PushType type;
    int      int_arg;
    int      aligmnent;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct PopCodeArgs
{
    PopType type;
    int     int_arg;
    int     aligment;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// main functions

static void         CheckInputFiles                   (const IOfile* files);

static AssemblerErr AsmDataCtor                       (AsmData* AsmDataInfo, const IOfile* file);
static AssemblerErr InitLabels                        (AsmData* AsmDataInfo);
static AssemblerErr WriteCmdInCodeArr                 (AsmData* AsmDataInfo);
static AssemblerErr WriteCodeArrayInFile              (AsmData* AsmDataInfo);
static AssemblerErr AsmDataDtor                       (AsmData* AsmDataInfo);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asm-command processing functions          

static AssemblerErr HandlePush                        (AsmData* AsmDataInfo);
static AssemblerErr HandlePop                         (AsmData* AsmDataInfo);
static AssemblerErr HandleJmp                         (AsmData* AsmDataInfo); // jumps
static AssemblerErr HandleJa                          (AsmData* AsmDataInfo);
static AssemblerErr HandleJae                         (AsmData* AsmDataInfo);
static AssemblerErr HandleJb                          (AsmData* AsmDataInfo);
static AssemblerErr HandleJbe                         (AsmData* AsmDataInfo);
static AssemblerErr HandleJe                          (AsmData* AsmDataInfo);
static AssemblerErr HandleJne                         (AsmData* AsmDataInfo);
static AssemblerErr HandleCall                        (AsmData* AsmDataInfo); // ======
static AssemblerErr HandleRet                         (AsmData* AsmdataInfo); // no args command
static AssemblerErr HandleAdd                         (AsmData* AsmDataInfo);
static AssemblerErr HandleSub                         (AsmData* AsmDataInfo);
static AssemblerErr HandleMul                         (AsmData* AsmDataInfo);
static AssemblerErr HandleDiv                         (AsmData* AsmDataInfo);
static AssemblerErr HandleHlt                         (AsmData* AsmDataInfo);
static AssemblerErr HandleOut                         (AsmData* AsmDataInfo);
static AssemblerErr HandleOutc                        (AsmData* AsmDataInfo);
static AssemblerErr HandleOutr                        (AsmData* AsmDataInfo);
static AssemblerErr HandleOutrc                       (AsmData* AsmDataInfo); // ========
static AssemblerErr HandlePp                          (AsmData* AsmDataInfo);
static AssemblerErr HandleMm                          (AsmData* AsmDataInfo);
static AssemblerErr HandleDraw                        (AsmData* AsmDataInfo);
static AssemblerErr HandleRGBA                        (AsmData* AsmDataInfo);
static AssemblerErr HandleLabel                       (AsmData* AsmDataInfo);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// HandlePush helper funtions

static PushType     GetPushType                       (bool stack, bool reg, bool memory, bool sum);

static PushCodeArgs GetPushCodeArgs                   (PushType type, int int_arg, int aligmnet);

static PushCodeArgs GetPushCodeArgsForPushNumber      (const Token* push_arg_token);
static PushCodeArgs GetPushCodeArgsForPushRegister    (const Token* push_arg_token);
static PushCodeArgs GetPushCodeArgsForPushMemNumber   (const Token* push_arg_token);
static PushCodeArgs GetPushCodeArgsForPushMemRegister (const Token* push_arg_token);
static PushCodeArgs GetPushCodeArgsForPushAligment    (const Token* push_arg_token, const Token* push_arg_next_token);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// HabdlePop helper functions

static PopType     GetPopType                         (bool reg, bool memory, bool aligment);

static PopCodeArgs GetPopCodeArgs                     (PopType type, int int_arg, int aligment);

static PopCodeArgs GetPopCodeArgsForPopRegister       (const Token* pop_arg_token                                 );
static PopCodeArgs GetPopCodeArgsForPopMemInt         (const Token* pop_arg_token                                 );
static PopCodeArgs GetPopCodeArgsForPopMemRegister    (const Token* pop_arg_token                                 );
static PopCodeArgs GetPopCodeArgsForPopMemAligment    (const Token* pop_arg_token, const Token* pop_arg_next_token);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// rgba functions

static DrawType    GetDrawType                        (bool IsReg[3]);
static RGBATYpe    GetRGBAType                        (bool isReg[4]);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// command pattern functions

static AssemblerErr JmpCmdPattern                     (AsmData* AsmDataInfo, Cmd jump_type);
static AssemblerErr NullArgCmdPattern                 (AsmData* AsmDataInfo, Cmd cmd      );
static AssemblerErr PpMmPattern                       (AsmData* AsmDataInfo, Cmd cmd      );

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// functions for tokens

// function for work with tokens array

static Token        GetNextToken                      (AsmData* AsmDataInfo);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// functions for translate tokens to code instructions

static int         GetNumberFromToken                 (const Token* token);
static int         GetRegisterFromToken               (const Token* token);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// bool helper functions for tokens types

static bool         IsTokenCommand                    (const Token* token);
static bool         IsTokenNumber                     (const Token* token);
static bool         IsTokenLabel                      (const Token* token);
static bool         IsTokenLeftBracket                (const Token* token);
static bool         IsTokenRegister                   (const Token* token);
static bool         IsTokenRightBracket               (const Token* token);
static bool         IsTokenComma                      (const Token* token);
static bool         IsTokenPlus                       (const Token* token);
static bool         IsTokenJmpOrCall                  (const Token* token);
static bool         IsAligment                        (const Token* token, const Token* next_token);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// functions for work with code array

static void         SetNextCodeInstruction            (AsmData* AsmDataInfo, int instruction);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// get function to process appropriate command

static AssemblerErr (*GetCmd(Cmd command)) (AsmData* AsmDataInfo);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Error processing functions

static AssemblerErr Verify                     (const AsmData* AsmDataInfo, AssemblerErr* err, Token token, const char* file, int line, const char* func);
static void         PrintError                 (const AssemblerErr* err);
static void         PrintIncorrectCmd          (const char* msg, const char* file, Token token);
static void         PrintIncorrectCmdFilePlace (const char* file , Token token);
static void         AssemblerAssertPrint       (const AssemblerErr* err, const char* file, int line, const char* func);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define ASSEMBLER_VERIF(AsmDataInfo, err, token) Verify(AsmDataInfo, &err, token, __FILE__, __LINE__, __func__)
 
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define ASSEMBLER_ASSERT(Err) do                                   \
{                                                                   \
    AssemblerErr errCopy = Err;                                      \
    if (errCopy.err != AssemblerErrorType::NO_ERR)                    \
    {                                                                  \
        AssemblerAssertPrint(&errCopy, __FILE__, __LINE__, __func__);   \
        exit(EXIT_FAILURE);                                              \
    }                                                                     \
} while (0)                                                                \

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void RunAssembler(const IOfile* file)
{
    assert(file);

    CheckInputFiles(file);

    AsmData AsmDataInfo = {};

    ASSEMBLER_ASSERT(AsmDataCtor         (&AsmDataInfo, file));
    ASSEMBLER_ASSERT(InitLabels          (&AsmDataInfo      ));
    ASSEMBLER_ASSERT(WriteCmdInCodeArr   (&AsmDataInfo      ));
    ASSEMBLER_ASSERT(WriteCodeArrayInFile(&AsmDataInfo      ));
    ASSEMBLER_ASSERT(AsmDataDtor         (&AsmDataInfo      ));

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

    if ((!bin_file_extension) || (strcmp(bin_file_extension, bin_extension) != 0))
        EXIT(EXIT_FAILURE, "bad bin extension: '%s'\nmust be '.bin'", files->bin_file);    

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// main fucntions

static AssemblerErr AsmDataCtor(AsmData* AsmDataInfo, const IOfile* file)
{
    assert(AsmDataInfo);
    assert(file);

    AssemblerErr err = {};

    const char* asm_file      = file->asm_file;

    Buffer      buffer        = ReadFileInBuffer(asm_file              );
    TokensArray tokens_array  = GetTokensArray  (asm_file,      &buffer);
    CodeArray   code_array    = CodeArrayCtor   (&tokens_array         );

    AsmDataInfo->file         = *file;
    AsmDataInfo->buffer       = buffer;
    AsmDataInfo->tokens_array = tokens_array;
    AsmDataInfo->code_array   = code_array;

    ON_DEBUG(
    LOG_PRINT(Blue, "code size = %lu\n\n", code_array.size);
    TokensLog(&tokens_array, asm_file);
    )

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr InitLabels(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    const Token* tokens_array = AsmDataInfo->tokens_array.array;
    const size_t tokens_quant = AsmDataInfo->tokens_array.size;

    static const size_t default_labels_quant = 16;
    LabelsArray         labels_array         = LabelsArrayCtor(default_labels_quant);

    size_t token_pointer = 0;
    size_t code_pointer  = 0;

    while(token_pointer < tokens_quant)
    {
        Token token = tokens_array[token_pointer];
        
        
        if (IsTokenJmpOrCall(&token))
        {
            token_pointer += 2;
            code_pointer  += 2;
            continue;
        }
        
        else if (IsTokenCommand(&token))
        {
            Cmd     command       = token.value.command;
            CmdInfo command_info  = GetCmdInfo(command);
                    code_pointer += command_info.codeRecordSize;
        }

        else if (IsTokenLabel(&token))
        {
            const TokenizerLabel token_label = token.value.label;
            size_t               cmd_index   = 0;

            if (WasLabelAlreadyDefined(&labels_array, token_label.name, &cmd_index))
            {
                err.err = AssemblerErrorType::LABEL_REDEFINE;
                return ASSEMBLER_VERIF(AsmDataInfo, err, token);
            }

            Label label = LabelCtor(token_label.name, code_pointer, true);
            PushLabel(&labels_array, &label);
        }

        token_pointer++;
    }

    AsmDataInfo->labels = labels_array;

    ON_DEBUG(
    LOG_TITLE(Red, "Labels Array!!!");
    size_t size = AsmDataInfo->labels.size;
    for (size_t i = 0; i < size; i++)
    {
        LOG_PRINT(Blue, "label[%2lu] =\n{\nname = '%.*s'\ncode place = '%3lu'\nis_def = '%d'\n}\n\n", i, 10, AsmDataInfo->labels.array[i].name, AsmDataInfo->labels.array[i].code_place, AsmDataInfo->labels.array[i].is_defined);
    }
    LOG_TITLE(Red, "Labels Array End");
    LOG_NS();
    )
    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr WriteCmdInCodeArr(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    size_t tokens_array_size = AsmDataInfo->tokens_array.size;

    while (AsmDataInfo->tokens_array.pointer < tokens_array_size)
    {
        Token token = GetNextToken(AsmDataInfo);

        if (IsTokenCommand(&token))
        {
            ASSEMBLER_ASSERT(GetCmd(token.value.command) (AsmDataInfo));
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

    AsmDataInfo->code_array.size = AsmDataInfo->code_array.pointer;

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr WriteCodeArrayInFile(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    const char* bin_file   = AsmDataInfo->file.bin_file;
    CodeArray   code_array = AsmDataInfo->code_array;

    FILE* file_ptr = SafeFopen(bin_file, "wb"); assert(file_ptr);

    size_t code_array_size = code_array.size;

    /*int fprintf_return = */
    fprintf(file_ptr, "%lu\n", code_array_size);
    /*
    if (fprintf_return != (int) sizeof(code_array_size))
        EXIT(EXIT_FAILURE, "failed fprintf code array size in '%s'.", bin_file);
    */

    int* code = code_array.array;

    for (size_t i = 0; i < code_array_size; i++)
    {
        int instruction = code[i];
    
        /*fprintf_return =*/
        fprintf(file_ptr, "%d ", instruction); 
        /*
        if (fprintf_return != (int) sizeof(instruction))
            EXIT(EXIT_FAILURE, "failed fprintf code[%lu]\n in '%s'.", i, bin_file);
        */
    }
    
    SafeFclose(file_ptr);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr AsmDataDtor(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    BufferDtor     (&AsmDataInfo->buffer      );
    TokensArrayDtor(&AsmDataInfo->tokens_array);
    CodeArrayDtor  (&AsmDataInfo->code_array  );
    LabelsArrayDtor(&AsmDataInfo->labels      );

    *AsmDataInfo = {};

    return err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asm-command processing functions

#define PUSH_BAD_SYNTAX(token) do                          \
{                                                           \
    err.err = AssemblerErrorType::INVALID_INPUT_AFTER_PUSH;  \
    return ASSEMBLER_VERIF(AsmDataInfo, err, token);          \
} while (0)                                                    \

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePush(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    Token        push_arg_token = GetNextToken(AsmDataInfo);
    PushCodeArgs push_code_args = {};

    if (IsTokenNumber(&push_arg_token)) // push 1
        push_code_args = GetPushCodeArgsForPushNumber(&push_arg_token);

    else if (IsTokenRegister(&push_arg_token)) // push ax
        push_code_args = GetPushCodeArgsForPushRegister(&push_arg_token);

    else if (IsTokenLeftBracket(&push_arg_token)) // push [...]
    {
              push_arg_token      = GetNextToken(AsmDataInfo);
        Token push_arg_next_token = GetNextToken(AsmDataInfo);

        if (IsTokenRightBracket(&push_arg_next_token))
        {
            if (IsTokenNumber(&push_arg_token))
                push_code_args = GetPushCodeArgsForPushMemNumber(&push_arg_token);

            else if (IsTokenRegister(&push_arg_token))
                push_code_args = GetPushCodeArgsForPushMemRegister(&push_arg_token);

            else
                PUSH_BAD_SYNTAX(push_arg_token);
        }

        else if (IsAligment(&push_arg_token, &push_arg_next_token))
        {
            push_arg_next_token = GetNextToken(AsmDataInfo);

            if (IsTokenNumber(&push_arg_next_token))
                    push_code_args = GetPushCodeArgsForPushAligment(&push_arg_token, &push_arg_next_token);
                
            else
                PUSH_BAD_SYNTAX(push_arg_token);

            push_arg_next_token = GetNextToken(AsmDataInfo);

            if (!IsTokenRightBracket(&push_arg_next_token))
                PUSH_BAD_SYNTAX(push_arg_next_token);
        }

        else
            PUSH_BAD_SYNTAX(push_arg_token);
    }

    else
        PUSH_BAD_SYNTAX(push_arg_token);


    PushType push_type    = push_code_args.type;
    int      push_arg_int = push_code_args.int_arg;
    int      aligment     = push_code_args.aligmnent; 

    SetNextCodeInstruction(AsmDataInfo, Cmd::push   );
    SetNextCodeInstruction(AsmDataInfo, push_type   );
    SetNextCodeInstruction(AsmDataInfo, push_arg_int);
    SetNextCodeInstruction(AsmDataInfo, aligment    );

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef PUSH_BAD_SYNTAX

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define POP_BAD_SYNTAX(token) do                           \
{                                                           \
    err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;   \
    return ASSEMBLER_VERIF(AsmDataInfo, err, token);          \
} while (0)                                                    \

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePop(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    Token   pop_arg_token = GetNextToken(AsmDataInfo);

    PopCodeArgs pop_code_args = {};

    if (IsTokenRegister(&pop_arg_token)) // pop ax
        pop_code_args = GetPopCodeArgsForPopRegister(&pop_arg_token);

    else if (IsTokenLeftBracket(&pop_arg_token)) // pop [...]
    {
              pop_arg_token      = GetNextToken(AsmDataInfo);
        Token pop_arg_next_token = GetNextToken(AsmDataInfo);

        if (IsTokenRightBracket(&pop_arg_next_token))
        {
            if (IsTokenNumber(&pop_arg_token))  // pop [123]
                pop_code_args = GetPopCodeArgsForPopMemInt(&pop_arg_token);

            else if (IsTokenRegister(&pop_arg_token)) // pop [ax]
                pop_code_args = GetPopCodeArgsForPopMemRegister(&pop_arg_token);

            else
                POP_BAD_SYNTAX(pop_arg_token);
        }

        else if (IsAligment(&pop_arg_token, &pop_arg_next_token)) // pop [ax + ...]
        {
            pop_arg_next_token = GetNextToken(AsmDataInfo);
            
            if (IsTokenNumber(&pop_arg_next_token))   // pop [ax + 1]
                pop_code_args = GetPopCodeArgsForPopMemAligment(&pop_arg_token, &pop_arg_next_token);

            else
                POP_BAD_SYNTAX(pop_arg_token);
        }
    }

    else
    {
        err.err = AssemblerErrorType::INVALID_INPUT_AFTER_POP;
        return ASSEMBLER_VERIF(AsmDataInfo, err, pop_arg_token);
    }


    PopType pop_type    = pop_code_args.type;
    int     pop_arg_int = pop_code_args.int_arg;
    int     aligment    = pop_code_args.aligment;

    SetNextCodeInstruction(AsmDataInfo, Cmd::pop);
    SetNextCodeInstruction(AsmDataInfo, pop_type);
    SetNextCodeInstruction(AsmDataInfo, pop_arg_int);
    SetNextCodeInstruction(AsmDataInfo, aligment);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef POP_BAD_SYNTAX

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJmp(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jmp);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJa(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, ja);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJae(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jae);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJb(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jb);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJbe(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jbe);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJe(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, je);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleJne(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return JmpCmdPattern(AsmDataInfo, jne);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleCall(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    Token call_arg_token = GetNextToken(AsmDataInfo);
    int   call_arg_int   = 0;

    LabelsArray labels_array = AsmDataInfo->labels;

    if (IsTokenLabel(&call_arg_token))
    {
        size_t label_pointer = 0;

        if (WasLabelAlreadyDefined(&labels_array, call_arg_token.value.label.name, &label_pointer))
        {
            Label label  = AsmDataInfo->labels.array[label_pointer];
            call_arg_int = (int) label.code_place; // '2' is call bin code record size
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

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleRet(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::ret);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleAdd(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::add);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleSub(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::sub);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleMul(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::mul);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleDiv(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::dive);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOut(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::out);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOutc(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::outc);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOutr(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::outr);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleOutrc(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, Cmd::outrc);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleHlt(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    return NullArgCmdPattern(AsmDataInfo, hlt);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandlePp(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    return PpMmPattern(AsmDataInfo, Cmd::pp);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


static AssemblerErr HandleMm(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    return PpMmPattern(AsmDataInfo, Cmd::mm);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleDraw(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    Token draw_arg_token[3] = {};

    for (size_t i = 0; i < 2; i++)
    {
        Token arg_token   = GetNextToken(AsmDataInfo);
        Token comma_token = GetNextToken(AsmDataInfo);

        
        if (!IsTokenComma(&comma_token))
        {
            err.err = AssemblerErrorType::NO_COMMA;
            return ASSEMBLER_VERIF(AsmDataInfo, err, comma_token);
        }

        draw_arg_token[i] = arg_token;
    }

    draw_arg_token[2] = GetNextToken(AsmDataInfo);


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

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleRGBA(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    Token rgba_arg_token[4] = {};

    for (size_t i = 0; i < 3; i++)
    {
        Token arg_token   = GetNextToken(AsmDataInfo);
        Token comma_token = GetNextToken(AsmDataInfo);

        
        if (!IsTokenComma(&comma_token))
        {
            err.err = AssemblerErrorType::NO_COMMA;
            return ASSEMBLER_VERIF(AsmDataInfo, err, comma_token);
        }

        rgba_arg_token[i] = arg_token;
    }

    rgba_arg_token[3] = GetNextToken(AsmDataInfo);


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

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr HandleLabel(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};

    AsmDataInfo->labels.pointer++;

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// push helper functions

static PushType GetPushType(bool stack, bool reg, bool memory, bool sum)
{
    PushType type = 0;
    type = (PushType) ((stack << 3) | (reg << 2) | (memory << 1) | (sum));

    return type;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushCodeArgs GetPushCodeArgs(PushType type, int int_arg, int aligmnet)
{
    PushCodeArgs push_code_args = 
    {
        .type      = type    ,
        .int_arg   = int_arg ,
        .aligmnent = aligmnet,
    };

    return push_code_args;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushCodeArgs GetPushCodeArgsForPushNumber(const Token* push_arg_token)
{
    assert(push_arg_token);

    return GetPushCodeArgs(GetPushType       (1, 0, 0, 0    ), 
                           GetNumberFromToken(push_arg_token),
                           0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushCodeArgs GetPushCodeArgsForPushRegister(const Token* push_arg_token)
{
    assert(push_arg_token);

    return GetPushCodeArgs(GetPushType         (0, 1, 0, 0    ), 
                           GetRegisterFromToken(push_arg_token),
                           0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushCodeArgs GetPushCodeArgsForPushMemNumber(const Token* push_arg_token)
{
    assert(push_arg_token);

    return GetPushCodeArgs(GetPushType       (0, 0, 1, 0    ), 
                           GetNumberFromToken(push_arg_token),
                           0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushCodeArgs GetPushCodeArgsForPushMemRegister(const Token* push_arg_token)
{
    assert(push_arg_token);

    return GetPushCodeArgs(GetPushType         (0, 1, 1, 0    ), 
                           GetRegisterFromToken(push_arg_token),
                           0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushCodeArgs GetPushCodeArgsForPushAligment(const Token* push_arg_token, const Token* push_arg_next_token)
{
    assert(push_arg_token);

    return GetPushCodeArgs(GetPushType      (0, 0, 1, 1         ), 
                        GetRegisterFromToken(push_arg_token     ),
                        GetNumberFromToken  (push_arg_next_token));
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// pop helper fnctions

static PopType GetPopType(bool reg, bool memory, bool aligment)
{
    PopType type = 0;
    type = (PopType) ((reg << 2) | (memory << 1) | aligment);

    return type;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PopCodeArgs GetPopCodeArgs(PopType type, int int_arg, int aligment)
{
    PopCodeArgs pop_code_args = 
    {
        .type     = type,
        .int_arg  = int_arg,
        .aligment = aligment,
    };

    return pop_code_args;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PopCodeArgs GetPopCodeArgsForPopRegister(const Token* pop_arg_token)
{
    assert(pop_arg_token);

    return GetPopCodeArgs(GetPopType          (1, 0, 0      ),
                          GetRegisterFromToken(pop_arg_token),
                          0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PopCodeArgs GetPopCodeArgsForPopMemInt(const Token* pop_arg_token)
{
    assert(pop_arg_token);

    return GetPopCodeArgs(GetPopType        (0, 1, 0      ),
                          GetNumberFromToken(pop_arg_token),
                          0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PopCodeArgs GetPopCodeArgsForPopMemRegister(const Token* pop_arg_token)
{
    assert(pop_arg_token);

    return GetPopCodeArgs(GetPopType          (1, 1, 0      ),
                          GetRegisterFromToken(pop_arg_token),
                          0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PopCodeArgs GetPopCodeArgsForPopMemAligment(const Token* pop_arg_token, const Token* pop_arg_next_token)
{
    assert(pop_arg_token);
    assert(pop_arg_next_token);

    return GetPopCodeArgs(GetPopType          (0, 0, 1           ),
                          GetRegisterFromToken(pop_arg_token     ),
                          GetNumberFromToken  (pop_arg_next_token));
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// rgba helper functions

static DrawType GetDrawType(bool IsReg[3])
{
    DrawType type = 0;
    type =  (DrawType) ((IsReg[2] << 2) | (IsReg[1] << 1) | (IsReg[0]));
    return type;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static RGBATYpe GetRGBAType(bool isReg[4])
{
    RGBATYpe type = 0;
    type = (RGBATYpe) ((isReg[3] << 3) | (isReg[2] << 2) | (isReg[1] << 1) | (isReg[0]));
    return type;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// pattern functionsd

//jumps pattern

static AssemblerErr JmpCmdPattern(AsmData* AsmDataInfo, Cmd jump_type)
{ 
    assert(AsmDataInfo);
    
    AssemblerErr err = {};

    Token jmp_arg_token = GetNextToken(AsmDataInfo);
    int   jmp_arg_int   = 0;

    LabelsArray labels_array = AsmDataInfo->labels;

    if (IsTokenLabel(&jmp_arg_token))
    {
        size_t label_pointer = 0;

        if (WasLabelAlreadyDefined(&labels_array, jmp_arg_token.value.label.name, &label_pointer))
        {
            Label label = AsmDataInfo->labels.array[label_pointer];
            jmp_arg_int = (int) label.code_place;
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

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// no args command pattern

static AssemblerErr NullArgCmdPattern(AsmData* AsmDataInfo, Cmd cmd)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
    SetNextCodeInstruction(AsmDataInfo, cmd);
    AsmDataInfo->code_array.size++;
    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr PpMmPattern(AsmData* AsmDataInfo, Cmd pp)
{
    assert(AsmDataInfo);

    AssemblerErr err = {};
        
    Token  pp_arg_token = GetNextToken(AsmDataInfo);
    int    pp_arg_int   = 0;

    if (!IsTokenRegister(&pp_arg_token))
    {
        switch (pp)
        {
            case Cmd::pp:
            {
                err.err = AssemblerErrorType::INCORRECT_PP_ARG;
                return ASSEMBLER_VERIF(AsmDataInfo, err, pp_arg_token);
            }

            case Cmd::mm:
            {
                err.err = AssemblerErrorType::INCORRECT_MM_ARG;
                return ASSEMBLER_VERIF(AsmDataInfo, err, pp_arg_token);
            }

            default: break;
        }

       __builtin_unreachable__("here can be only 'pp' or 'mm' commands");
        return ASSEMBLER_VERIF(AsmDataInfo, err, {});
    }

    pp_arg_int = GetRegisterFromToken(&pp_arg_token);

    SetNextCodeInstruction(AsmDataInfo, pp);
    SetNextCodeInstruction(AsmDataInfo, pp_arg_int);

    return ASSEMBLER_VERIF(AsmDataInfo, err, {});
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Token GetNextToken(AsmData* AsmDataInfo)
{
    assert(AsmDataInfo);
    size_t pointer = AsmDataInfo->tokens_array.pointer;
    AsmDataInfo->tokens_array.pointer++;
    return AsmDataInfo->tokens_array.array[pointer];
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetRegisterFromToken(const Token* token)
{
    assert(token);

    return (int) token->value.reg;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenCommand(const Token* token)
{
    assert(token);

    return (token->type == TokenType::token_command);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenNumber(const Token* token)
{
    assert(token);
    return (token->type == TokenType::token_number);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenRegister(const Token* token)
{
    assert(token);
    return (token->type == TokenType::token_register);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenLeftBracket(const Token* token)
{
    assert(token);
    return  (token->type          == TokenType::token_bracket) &&
            (token->value.bracket == Bracket::left_bracket);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenRightBracket(const Token* token)
{
    assert(token);
    return  (token->type          == TokenType::token_bracket) &&
            (token->value.bracket == Bracket::right_bracket);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenComma(const Token* token)
{
    assert(token);

    return  (token->type  == TokenType::token_separator) &&
            (token->value.seprator == Separator::comma);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenPlus(const Token* token)
{
    assert(token);
    return  (token->type            == TokenType   ::token_math_operation) &&
            (token->value.operation == MathOperator::plus);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenLabel(const Token* token)
{
    assert(token);
    return (token->type == TokenType::token_label);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsTokenJmpOrCall(const Token* token)
{
    assert(token);

    if (token->type != TokenType::token_command)
        return false;

    switch (token->value.command)
    {
        case    Cmd::call:
        case    Cmd::jmp :
        case    Cmd::ja  :
        case    Cmd::jae :
        case    Cmd::jb  :
        case    Cmd::jbe :
        case    Cmd::je  :
        case    Cmd::jne : return true;
        default          : break;
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsAligment(const Token* arg_token, const Token* arg_next_token)
{
    assert(arg_token);
    assert(arg_next_token);

    return IsTokenRegister(arg_token) &&
           IsTokenPlus    (arg_next_token);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetNextCodeInstruction(AsmData* AsmDataInfo, int instruction)
{
    assert(AsmDataInfo);
    assert(AsmDataInfo->code_array.array);

    size_t pointer = AsmDataInfo->code_array.pointer;
    AsmDataInfo->code_array.array[pointer] = instruction;
    AsmDataInfo->code_array.pointer++;
    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr (*GetCmd(Cmd command)) (AsmData* AsmDataInfo)
{
    AssemblerErr err = {};

    switch (command)
    {
        case Cmd::hlt:   return HandleHlt  ;
        case Cmd::push:  return HandlePush ;
        case Cmd::pop:   return HandlePop  ;
        case Cmd::add:   return HandleAdd  ;
        case Cmd::sub:   return HandleSub  ;
        case Cmd::mul:   return HandleMul  ;
        case Cmd::dive:  return HandleDiv  ;
        case Cmd::pp:    return HandlePp   ;
        case Cmd::mm:    return HandleMm   ;
        case Cmd::out:   return HandleOut  ;
        case Cmd::outc:  return HandleOutc ;
        case Cmd::outr:  return HandleOutr ;
        case Cmd::outrc: return HandleOutrc;
        case Cmd::jmp:   return HandleJmp  ;
        case Cmd::ja:    return HandleJa   ;
        case Cmd::jae:   return HandleJae  ;
        case Cmd::jb:    return HandleJb   ;
        case Cmd::jbe:   return HandleJbe  ;
        case Cmd::je:    return HandleJe   ;
        case Cmd::jne:   return HandleJne  ;
        case Cmd::call:  return HandleCall ;
        case Cmd::ret:   return HandleRet  ;
        case Cmd::draw:  return HandleDraw ;
        case Cmd::rgba:  return HandleRGBA ;
        default:         break             ;
    }

    err.err = AssemblerErrorType::UNDEFINED_ENUM_COMMAND;
    ASSEMBLER_ASSERT(err);

    __builtin_unreachable__("we must make return in switchc or fall in stack_assert");

    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static AssemblerErr Verify(const AsmData* AsmDataInfo, AssemblerErr* err, Token token, const char* file, int line, const char* func)
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

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

        case AssemblerErrorType::NO_COMMA:
            PrintIncorrectCmd("Error: expected ',':", inputStream, err->token);
            break;
        
        default: __builtin_unreachable__("maybe I forgot about some error code"); break;
    }

    return;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
