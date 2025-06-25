#include <assert.h>
#include "logger/log.hpp"
#include "tokenizer/tokenizer.hpp"

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void        LogValue            (Token          token        );

static const char* GetTypeInStr        (TokenType      type         );
static const char* GetValueInStr       (Token          token        );
static const char* GetBracketInStr     (Bracket        bracket      );
static const char* GetSeparatorInStr   (Separator      separator    );
static const char* GetMathOperatorInStr(MathOperator   math_operator);
static const char* GetCmdInStr         (Cmd            cmd          );
static const char* GetRegisterInStr    (Registers      reg          );

static void        LogLabel            (TokenizerLabel label        );
static void        LogNumber           (Number         number       );

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void TokensLog(const TokensArray* tokens_array, const char* asm_file)
{
    assert(tokens_array);

    LOG_TITLE(Red, "Tokens Array!!!");
    
    size_t size = tokens_array->size;

    LOG_PRINT(Green, "size = %lu\n", size);

    for (size_t i = 0; i < size; i++)
    {
        Token token = tokens_array->array[i];

        LOG_PRINT(Yellow, "token[%lu] =\n{\n", i);
        LOG_PRINT(Green , "\ttype = %s\n", GetTypeInStr(token.type));
        LogValue(token);
        LOG_PRINT(Green , "\t%s:%lu:%lu\n", asm_file, token.place.line, token.place.pos_in_line);
        LOG_PRINT(Yellow, "}\n\n");
    }

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void TokenLog(const Token* token, size_t token_pointer)
{
    assert(token);

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetTypeInStr(TokenType type)
{
    switch (type)
    {
        case TokenType::token_command:        return "command";
        case TokenType::token_bracket:        return "bracket";
        case TokenType::token_label:          return "label";
        case TokenType::token_math_operation: return "math opeation";
        case TokenType::token_number:         return "number";
        case TokenType::token_register:       return "register";
        case TokenType::token_separator:      return "separator";
        case TokenType::undef_token_type:
        default:                              return "undef";
    }

    assert(0);
    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void LogValue(Token token)
{
    const char* str_value = GetValueInStr(token);

    if (str_value)
        LOG_PRINT(Green, "\tvalue = '%s'\n", str_value);


    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetValueInStr(Token token)
{
    switch (token.type)
    {
        case TokenType::token_label:                 LogLabel            (token.value.label    ); return nullptr;
        case TokenType::token_number:                LogNumber           (token.value.number   ); return nullptr;
        case TokenType::token_bracket:        return GetBracketInStr     (token.value.bracket  );
        case TokenType::token_command:        return GetCmdInStr         (token.value.command  );
        case TokenType::token_math_operation: return GetMathOperatorInStr(token.value.operation);
        case TokenType::token_register:       return GetRegisterInStr    (token.value.reg      );
        case TokenType::token_separator:      return GetSeparatorInStr   (token.value.seprator );
        case TokenType::undef_token_type:
        default:                                                                           return "undef register value";
    }

    assert(0);
    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetBracketInStr(Bracket bracket)
{
    switch (bracket)
    {
        case Bracket::left_bracket:  return "[";
        case Bracket::right_bracket: return "]";
        case Bracket::undef_bracket:
        default:                     return "undef bracket";
    }

    assert(0);
    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetSeparatorInStr(Separator separator)
{
    switch (separator)
    {
        case Separator::colon: return ":";
        case Separator::comma: return ",";
        case Separator::undef_separator:
        default:               return "undef separator";
    }

    assert(0);
    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetMathOperatorInStr(MathOperator math_operator)
{
    switch (math_operator)
    {
        case MathOperator::plus:           return "+";
        case MathOperator::undef_operator:
        default:                           return "undef";
    }

    assert(0);
    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetCmdInStr(Cmd cmd)
{
    for (size_t i = 0; i < CmdInfoArrSize; i++)
    {
        CmdInfo cmd_info = CmdInfoArr[i];

        if (cmd == cmd_info.cmd)
            return cmd_info.name;
    }

    assert(0);
    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetRegisterInStr(Registers reg)
{
    switch (reg)
    {
        case Registers::ax: return "ax";
        case Registers::bx: return "bx";
        case Registers::cx: return "cx";
        case Registers::dx: return "dx";
        case Registers::ex: return "ex";
        case Registers::fx: return "fx";
        case Registers::REGISTERS_QUANT:
        case Registers::undef_register:
        default:            return "undef register";
    }

    assert(0);
    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void LogLabel(TokenizerLabel label)
{
    const size_t len   = label.name_len;
    const char*  value = label.name;

    LOG_PRINT(Green, "\tvalue = '%.*s'\n", len, value);

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void LogNumber(Number number)
{
    const size_t len   = number.number_len;
    const char*  value = number.number;

    LOG_PRINT(Green, "\tvalue = '%.*s'\n", len, value);

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
