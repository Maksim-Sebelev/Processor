#include <assert.h>
#include "disassembler/tokenizer/tokenizer.hpp"
#include "functions_for_files/files.hpp"
#include "lib/lib.hpp"
#include "proc_disasm_common/proc_disasm_common.hpp"
#include "list/list.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

__attribute__((__noreturn__))
static void      ParseUndefCmd           (int command, size_t ip, const char* bin_file);
static CodeArray ReadFileInCodeArray     (const char* file);
static int       GetNextInstruction      (CodeArray* code_array);
static void      CtorAndPushBackToken    (TokensList* tokens_list, TokenType type, TokenValue value, size_t ip);
static Token     CtorToken               (TokenType type, TokenValue value, size_t ip);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PUSH_COMMAND_TOKEN(tokens_list, command_value, ip) do              \
{                                                                           \
    TokenValue value = {.command = command_value};                           \
    CtorAndPushBackToken(tokens_list, TokenType::token_command_t, value, ip); \
} while(0)                                                                     \

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PUSH_REGISTER_TOKEN(tokens_list, reg_value, ip) do                 \
{                                                                           \
    TokenValue value = {.reg = reg_value};                                   \
    CtorAndPushBackToken(tokens_list, TokenType::token_command_t, value, ip); \
} while(0)                                                                     \

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PUSH_NUMBER_TOKEN(tokens_list, number_value, ip) do                \
{                                                                           \
    TokenValue value = {.number = number_value};                             \
    CtorAndPushBackToken(tokens_list, TokenType::token_command_t, value, ip); \
} while(0)                                                                     \

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PUSH_LABEL_TOKEN(tokens_list, label_value, ip) do                  \
{                                                                           \
    TokenValue value = {.label = label_value};                               \
    CtorAndPushBackToken(tokens_list, TokenType::token_command_t, value, ip); \
} while(0)                                                                     \

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PUSH_SEPARATOR_TOKEN(tokens_list, separator_value, ip) do          \
{                                                                           \
    TokenValue value = {.separator = separator_value};                       \
    CtorAndPushBackToken(tokens_list, TokenType::token_command_t, value, ip); \
} while(0)                                                                     \

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PUSH_BRACKET_TOKEN(tokens_list, bracket_value, ip) do              \
{                                                                           \
    TokenValue value = {.bracket = bracket_value};                           \
    CtorAndPushBackToken(tokens_list, TokenType::token_command_t, value, ip); \
} while(0)                                                                     \


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TokensList GetTokensList(const char* bin_file)
{
    assert(bin_file);

    CodeArray code_array   = ReadFileInCodeArray(bin_file);       assert(code_array.array);
    Token*    tokens_array = ArrayForTokensCtor(code_array.size); assert(tokens_array);

    const size_t code_array_size = code_array.size;

    while (code_array.ip < code_array_size)
    {
        int instruction = GetNextInstruction(&code_array);

        switch (instruction)
        {
            case Cmd::push :  break;
            case Cmd::pop  :  break;
            case Cmd::add  :  break;
            case Cmd::sub  :  break;
            case Cmd::mul  :  break;
            case Cmd::dive :  break;
            case Cmd::pp   :  break;
            case Cmd::mm   :  break;
            case Cmd::jmp  :  break;
            case Cmd::ja   :  break;
            case Cmd::jae  :  break;
            case Cmd::jb   :  break;
            case Cmd::jbe  :  break;
            case Cmd::je   :  break;
            case Cmd::jne  :  break;
            case Cmd::draw :  break;
            case Cmd::rgba :  break;
            case Cmd::hlt  :  break;
            case Cmd::out  :  break;
            case Cmd::outc :  break;
            case Cmd::outr :  break;
            case Cmd::outrc:  break;
            default:
                ParseUndefCmd(instruction, code_array.ip, bin_file);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandlePush(TokensList* tokens_list, CodeArray* code_array)
{
    assert(code_array);

    PUSH_COMMAND_TOKEN(tokens_list, Cmd::push, code_array->ip);

    int push_type_int = GetNextInstruction(code_array);
    int push_arg_int  = GetNextInstruction(code_array);
    int aligment      = GetNextInstruction(code_array);

    PushType type = GetPushType(push_type_int);


    if (IsPushTypeNumber(type))
    {
        Number number = GetNextInstruction(code_array);
        PUSH_NUMBER_TOKEN(tokens_list, number, code_array->ip);
    }

    else if (IsPushTypeRegister(type))
    {
        Registers reg = 
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// HandlePush helper functions

static Number GetNumberInPushPop(CodeArray* code_array)
{
    assert(code_array);

    return GetNextInstruction(code_array);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Registers GetRegisterInPushPop(CodeArray* code_array)

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Token CtorToken(TokenType type, TokenValue value, size_t ip)
{
    return (Token)
    {
        .type         = type ,
        .value        = value,
        .code_pointer = ip   ,
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void CtorAndPushBackToken(TokensList* tokens_list, TokenType type, TokenValue value, size_t ip)
{
    assert(tokens_list);

    Token token = CtorToken(type, value, ip);

    LIST_PUSH_BACK(tokens_list, token, nullptr);

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

__attribute__((__noreturn__))
static void ParseUndefCmd(int command, size_t ip, const char* bin_file)
{
    assert(bin_file);

    EXIT(EXIT_FAILURE, "%d - is not assembler instruction.\nip = %lu\n%s\n", command, ip, bin_file);

    __builtin_unreachable__("exit");
}


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetNextInstruction(CodeArray* code_array)
{
    assert(code_array);
    assert(code_array->array);

    const size_t old_ip = code_array->ip;

    code_array->ip++;

    return code_array->array[old_ip];
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Token* ArrayForTokensCtor(size_t code_array_size)
{
    const size_t tokens_max_quant = code_array_size * 6;

    Token* array = (Token*) calloc(tokens_max_quant, sizeof(array[0]));

    if (!array)
        EXIT(EXIT_FAILURE, "failed allocate memory for tokens array");

    return array;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static CodeArray ReadFileInCodeArray(const char* file)
{
    assert(file);

    const size_t file_size = CalcFileLen(file);

    FILE* file_ptr = SafeFopen(file, "wb"); assert(file_ptr);

    size_t code_array_size = 0;

    int fscanf_return = fscanf(file_ptr, "%lu", &code_array_size);

    if (fscanf_return != 1)
        EXIT(EXIT_FAILURE, "failed read binary file len in disasm");

    int* array = (int*) calloc(code_array_size, sizeof(array[0]));

    if (!array)
        EXIT(EXIT_FAILURE, "failed allocare memory for code array");

    int    comand       = 0;
    size_t code_pointer = 0;

    while (fscanf(file_ptr, "%d", comand) != EOF)
    {
        array[code_pointer] = comand;
        code_pointer++;
    }

    CodeArray code_array =
    {
        .array = array          ,
        .size  = code_array_size,
    };

    return code_array;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

