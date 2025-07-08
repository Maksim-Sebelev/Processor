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
static Token     TokenCtor               (TokenType type, const void* value, size_t ip);
static void      CtorAndPushToken        (TokensList* tokens_list, TokenType type, const void* value, size_t ip);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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

    CtorAndPushToken(tokens_list, TokenType::token_command_t, )

    int push_type_int = GetNextInstruction(code_array);
    int push_arg_int  = GetNextInstruction(code_array);
    int aligment      = GetNextInstruction(code_array);

    PushType type = GetPushType(push_type_int);


    if (IsPushTypeNumber(type))
    {

    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void CtorAndPushToken(TokensList* tokens_list, TokenType type, const void* value, size_t ip)
{
    assert(tokens_list);
    assert(value);

    Token token = TokenCtor(type, value, ip);

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

#define get_token_value(type, value) *(type*) value

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Token TokenCtor(TokenType type, const void* value, size_t ip)
{
    assert(value);

    Token token = {};

    token.type         = type;
    token.code_pointer = ip;

    switch (type)
    {
        case TokenType::token_command_t  : token.value.command   = get_token_value(Cmd       , value);
        case TokenType::token_register_t : token.value.reg       = get_token_value(Registers , value);
        case TokenType::token_number_t   : token.value.number    = get_token_value(Number    , value);
        case TokenType::token_label_t    : token.value.label     = get_token_value(TokenLabel, value);
        case TokenType::token_separator_t: token.value.separator = get_token_value(Separator , value);
        default: __builtin_unreachable__("undef token type");
    }

    return token;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef get_token_value

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static void HandlePush()
