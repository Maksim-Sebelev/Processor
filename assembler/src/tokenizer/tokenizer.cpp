#include <assert.h>
#include <string.h>
#include "tokenizer/tokenizer.hpp"
#include "global/global_include.hpp"
#include "functions_for_files/files.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool GetFlag(const char* word, const char* command, size_t command_len);
static Cmd HandleCmd(const char* word);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TokensArray GetTokensArray(const char* asm_file)
{
    const char* buffer = ReadFileInBuffer(asm_file);



}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// enum class TokenType
// {
//     token_command       ,
//     token_register      ,
//     token_number        ,
//     token_bracket       ,
//     token_separator     ,
//     token_math_operation,
// };


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool GetFlag(const char* word, const char* command, size_t command_len)
{
    assert(word);
    assert(command);

    return  (strncmp(word, command, command_len) == 0) &&
            (word[command_len + 1] == ' ');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Cmd HandleCmd(const char* word)
{
    assert(word);

    for (size_t i = 0; i < CmdInfoArrSize; i++)
    {
        CmdInfo cmd = CmdInfoArr[i];        
        if (GetFlag(word, cmd.name, cmd.nameLen))
            return cmd.cmd;
    }

    return Cmd::undef_cmd;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Registers HandleRegister(const char* word)
{
    assert(word);
    
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
