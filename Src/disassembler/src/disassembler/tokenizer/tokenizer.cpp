#include <assert.h>
#include "disassembler/tokenizer/tokenizer.hpp"
#include "functions_for_files/files.hpp"
#include "lib/lib.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct CodeArray
{
    int*   array;
    size_t size ;
    size_t ip   ; // instruction pointer
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static CodeArray ReadFileInCodeArray(const char* file);
static int GetNextInstruction(CodeArray* code_array);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TokensArray GetTokensArray(const char* bin_file)
{
    assert(bin_file);

    CodeArray code_array   = ReadFileInCodeArray(bin_file);,      assert(code_array.array);
    Token*    tokens_array = ArrayForTokensCtor(code_array.size); assert(tokens_array);

    const size_t code_array_size = code_array.size;

    while (code_array.ip < code_array_size)
    {
        int instruction = GetNextInstruction(&code_array);
        switch (instruction)
        {
            case Cmd::push:
            case Cmd::pop:
            case Cmd::add:
            case Cmd::sub:
            case Cmd::mul:
            case Cmd::dive:
            case Cmd::pp:
            case Cmd::mm:
            case Cmd::jmp:
            case Cmd::ja:
            case Cmd::jae:
            case Cmd::jb:
            case Cmd::jbe:
            case Cmd::je:
            case Cmd::jne:
            case Cmd::draw:
            case Cmd::rgba:
            case Cmd::hlt:
            case Cmd::out:
            case Cmd::outc:
            case Cmd::outr:
            case Cmd::outrc:
            default:
        }

    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void ParseUndefCmd()

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
