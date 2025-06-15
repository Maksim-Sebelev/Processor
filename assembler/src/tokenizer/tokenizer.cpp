#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "tokenizer/tokenizer.hpp"
#include "global/global_include.hpp"
#include "functions_for_files/files.hpp"
#include "lib/lib.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct Pointers
{
    size_t ip; // input pointer
    size_t tp; // token pointer
    size_t lp; // line pointer (line in input file)
    size_t sp; // str pointer (pos in line)
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Token*         TokensCalloc         (size_t buffer_len);
static void           TokensRealloc        (Token** tokens_array, size_t tokens_quant);

static bool           GetCmdFlag           (const char* word, const char* command, size_t command_len);
static Cmd            GetCmd               (const char* word, size_t* word_len);
static Registers      GetRegister          (const char* word, size_t* word_len);
static Bracket        GetBracket           (const char* word, size_t* word_len);
static Separator      GetSeparator         (const char* word, size_t* word_len);
static MathOperator   GetMathOperator      (const char* word, size_t* word_len);
static TokenizerLabel GetLabel             (const char* word                  );
static Number         GetNumber            (const char* word                  );

static bool           HandleComment        (const char* word , Pointers* pointer                                               );
static void           HandleGeneralPattern (Token* tokens_arr, Pointers* pointer,                               size_t word_len);
static void           HandleCmd            (Token* tokens_arr, Pointers* pointer, Cmd            cmd          , size_t word_len);
static void           HandleRegister       (Token* tokens_arr, Pointers* pointer, Registers      reg          , size_t word_len);
static void           HandleBracket        (Token* tokens_arr, Pointers* pointer, Bracket        bracket      , size_t word_len);
static void           HandleSeparator      (Token* tokens_arr, Pointers* pointer, Separator      separator    , size_t word_len);
static void           HandleMathOperator   (Token* tokens_arr, Pointers* pointer, MathOperator   math_operator, size_t word_len);
static void           HandleLabel          (Token* tokens_arr, Pointers* pointer, TokenizerLabel label        , size_t word_len);
static void           HandleNumber         (Token* tokens_arr, Pointers* pointer, Number         number       , size_t word_len);

static bool           IsNumSymbol                       (char c);
static bool           IsPointSymbol                     (char c);
static bool           IsColon                           (char c);
static bool           IsLetterSymbol                    (char c);
static bool           IsUnderLineSymbol                 (char c);
static bool           IsLetterOrUnderLineSymbol         (char c);
static bool           IsLetterOrNumberOrUnderLineSymbol (char c);
static bool           IsSpace                           (char c);
static bool           IsSlashN                          (char c);
static bool           IsSlash0                          (char c);
static bool           IsSlashNOrSlashN                  (char c);
static bool           IsPassSymbol                      (char c);
static void           UpdatePointersAfterSpace          (Pointers* pointer);
static void           UpdatePointersAfterSlashN         (Pointers* pointer);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

TokensArray GetTokensArray(const char* asm_file)
{
    assert(asm_file);

    size_t      buffer_size = 0;
    const char* buffer      = ReadFileInBuffer(asm_file, &buffer_size);

    Token* tokens_array = TokensCalloc(buffer_size);

    Pointers pointer = {0, 0, 1, 1};


    while (pointer.ip < buffer_size)
    {
        while (pointer.ip < buffer_size && IsPassSymbol(buffer[pointer.ip]))
        {
            char tmp = buffer[pointer.ip];

            if      (IsSpace  (tmp)) UpdatePointersAfterSpace  (&pointer);
            else if (IsSlashN (tmp)) UpdatePointersAfterSlashN (&pointer);
            else                     assert(0 && "Something went wrong :(");
        }

        const char* word     = buffer + pointer.ip;
        size_t      word_len = 0;

        if (IsSlash0(word[0])) break;

        if (HandleComment(word, &pointer))
            continue;


        Cmd cmd = GetCmd(word, &word_len);
        if (cmd != Cmd::undef_cmd)
        {
            HandleCmd(tokens_array, &pointer, cmd, word_len);
            continue;
        }

        Registers reg = GetRegister(word, &word_len);
        if (reg != Registers::undef_register)
        {
            HandleRegister(tokens_array, &pointer, reg, word_len);
            continue;
        }

        Bracket bracket = GetBracket(word, &word_len);
        if (bracket != Bracket::undef_bracket)
        {
            HandleBracket(tokens_array, &pointer, bracket, word_len);
            continue;
        }
    
        Separator separator = GetSeparator(word, &word_len);
        if (separator != Separator::undef_separator)
        {
            HandleSeparator(tokens_array, &pointer, separator, word_len);
            continue;
        }

        MathOperator math_operator = GetMathOperator(word, &word_len);
        if (math_operator != MathOperator::undef_operator)
        {
            HandleMathOperator(tokens_array, &pointer, math_operator, word_len);
            continue;
        }

        TokenizerLabel label = GetLabel(word);
        if (label.label_len != 0)
        {
            HandleLabel(tokens_array, &pointer, label, word_len);
            continue;
        }

        Number number = GetNumber(word);
        if (number.number_len != 0)
        {
            HandleNumber(tokens_array, &pointer, number, word_len);
            continue;
        }
        
        EXIT(EXIT_FAILURE, 
            "undefined word in:"
            WHITE
            "%s:%lu:%lu",
            asm_file, pointer.lp, pointer.tp
            );
    }

    size_t tokens_array_size = pointer.tp;

    TokensRealloc(&tokens_array, tokens_array_size);

    TokensArray final_tokens_array = 
    {
        .array = tokens_array     ,
        .size  = tokens_array_size,
    };

    return final_tokens_array;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Token* TokensCalloc(size_t buffer_len)
{
    Token* tokens_array = (Token*) calloc(buffer_len + 1, sizeof(*tokens_array));
    
    if (!tokens_array)
        EXIT(EXIT_FAILURE, "failed allocate memory for tokens array.");

    return tokens_array;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void TokensRealloc(Token** tokens_array, size_t tokens_quant)
{
    assert(tokens_array);

    *tokens_array = (Token*) realloc(*tokens_array, tokens_quant * sizeof(**tokens_array));

    if (!(*tokens_array))
        EXIT(EXIT_FAILURE, "failed reallocate memory for tokens array.");

    return;
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
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool GetCmdFlag(const char* word, const char* command, size_t command_len)
{
    assert(word);
    assert(command);

    return  (strncmp(word, command, command_len) == 0) &&
            (word[command_len + 1] == ' ');
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Cmd GetCmd(const char* word, size_t* word_len)
{
    assert(word);

    for (size_t i = 0; i < CmdInfoArrSize; i++)
    {
        CmdInfo cmd = CmdInfoArr[i];
    
        if (GetCmdFlag(word, cmd.name, cmd.nameLen))
        {
            *word_len = cmd.nameLen;
            return cmd.cmd;
        }
    }

    return Cmd::undef_cmd;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Registers GetRegister(const char* word, size_t* word_len)
{
    assert(word);

    const char w0 = word[0];
    const char w1 = word[1];

    bool flag = (word[1] == 'x') &&
                ('a' <= word[0] && word[0] <= 'a' + REGISTERS_QUANT);

    *word_len = Registers::REGISTERS_NAME_LEN;

    if (flag)
        return  (Registers) (w1 - 'a');

    return Registers::undef_register;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Number GetNumber(const char* word)
{
    assert(word);

    size_t number_len = 0;

    for (number_len = 0; isdigit(word[number_len]); number_len++);

    Number number = {.number = word, .number_len = number_len};

    return number;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Bracket GetBracket(const char* word, size_t* word_len)
{
    assert(word);

    *word_len = 1;

    const char w0 = word[0];

    if (w0 == (char) Bracket::left_bracket)
        return Bracket::left_bracket;
    
    if (w0 == (char) Bracket::right_bracket)
        return Bracket::right_bracket;

    return Bracket::undef_bracket;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static Separator GetSeparator(const char* word, size_t* word_len)
{
    assert(word);

    *word_len = 1;

    const char w0 = word[0];

    if (w0 == (char) Separator::comma)
        return Separator::comma;

    if (w0 == (char) Separator::colon)
        return Separator::colon;

    return Separator::undef_separator;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static MathOperator GetMathOperator(const char* word, size_t* word_len)
{
    assert(word);

    *word_len = 1;

    const char w0 = word[0];

    if (w0 == (char) MathOperator::plus)
        return MathOperator::plus;

    return MathOperator::undef_operator;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// returning label len is 0/ if that not label
// else: it's label

static TokenizerLabel GetLabel(const char* word)
{
    assert(word);

    size_t name_len = 0;

    if (IsPointSymbol(word[0]) && !(IsLetterSymbol(word[1]) || IsNumSymbol(word[1])))
        return {};

    if (IsPointSymbol(word[0]))
        name_len++;

    size_t i = name_len;

    for (; IsLetterOrNumberOrUnderLineSymbol(word[i]) && word[i] != (char) Separator::colon; i++);

    if (!IsColon(word[i]))
        return {};

    name_len = i;

    TokenizerLabel label = {.label = word, .label_len = name_len};

    return label;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool HandleComment(const char* word, Pointers* pointer)
{
    assert(word);
    assert(pointer);

    if (word[0] != ';')
        return false;

    size_t comment_len = 1;

    while (!IsSlashNOrSlashN(word[pointer->ip]))
        comment_len++;

    pointer->ip += comment_len;
    pointer->sp += comment_len;
    
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleGeneralPattern(Token* tokens_arr, Pointers* pointer, size_t word_len)
{
    assert(tokens_arr);
    assert(pointer);

    const size_t token_pointer = pointer->tp; 

    tokens_arr[token_pointer].place.line        = pointer->lp;
    tokens_arr[token_pointer].place.pos_in_line = pointer->sp;

    pointer->ip += word_len;
    pointer->sp += word_len;
    pointer->tp++;

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleCmd(Token* tokens_arr, Pointers* pointer, Cmd cmd, size_t word_len)
{
    assert(tokens_arr);
    assert(pointer);

    size_t token_pointer = pointer->tp;

    tokens_arr[token_pointer].type          = TokenType::token_command;
    tokens_arr[token_pointer].value.command = cmd;
    
    HandleGeneralPattern(tokens_arr, pointer, word_len);

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleRegister(Token* tokens_arr, Pointers* pointer, Registers reg, size_t word_len)
{
    assert(tokens_arr);
    assert(pointer);

    size_t token_pointer = pointer->tp;

    tokens_arr[token_pointer].type      = TokenType::token_register;
    tokens_arr[token_pointer].value.reg = reg;
   
    HandleGeneralPattern(tokens_arr, pointer, word_len);
   
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleBracket(Token* tokens_arr, Pointers* pointer, Bracket bracket, size_t word_len)
{
    assert(tokens_arr);
    assert(pointer);

    size_t token_pointer = pointer->tp;

    tokens_arr[token_pointer].type          = TokenType::token_bracket;
    tokens_arr[token_pointer].value.bracket = bracket;
   
    HandleGeneralPattern(tokens_arr, pointer, word_len);
   
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleSeparator(Token* tokens_arr, Pointers* pointer, Separator separator, size_t word_len)
{
    assert(tokens_arr);
    assert(pointer);

    size_t token_pointer = pointer->tp;

    tokens_arr[token_pointer].type           = TokenType::token_separator;
    tokens_arr[token_pointer].value.seprator = separator;
   
    HandleGeneralPattern(tokens_arr, pointer, word_len);
   
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleMathOperator(Token* tokens_arr, Pointers* pointer, MathOperator math_operator, size_t word_len)
{
    assert(tokens_arr);
    assert(pointer);

    size_t token_pointer = pointer->tp;

    tokens_arr[token_pointer].type            = TokenType::token_math_operation;
    tokens_arr[token_pointer].value.operation = math_operator;
   
    HandleGeneralPattern(tokens_arr, pointer, word_len);

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleLabel(Token* tokens_arr, Pointers* pointer, TokenizerLabel label, size_t word_len)
{
    assert(tokens_arr);
    assert(pointer);

    size_t token_pointer = pointer->tp;

    tokens_arr[token_pointer].type        = TokenType::token_label;
    tokens_arr[token_pointer].value.label = label;
   
    HandleGeneralPattern(tokens_arr, pointer, word_len);

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void HandleNumber(Token* tokens_arr, Pointers* pointer, Number number, size_t word_len)
{
    assert(tokens_arr);
    assert(pointer);

    size_t token_pointer = pointer->tp;

    tokens_arr[token_pointer].type         = TokenType::token_label;
    tokens_arr[token_pointer].value.number = number;
   
    HandleGeneralPattern(tokens_arr, pointer, word_len);

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsNumSymbol(char c)
{
    return  ('0' <= c  ) && 
            (c   <= '9');
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPointSymbol(char c)
{
    return (c == '.');
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsColon(char c)
{
    return (c == ':');
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLetterSymbol(char c)
{
    return  ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z');
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsUnderLineSymbol(char c)
{
    return (c == '_');
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLetterOrUnderLineSymbol(char c)
{
    return  IsLetterSymbol   (c) ||
            IsUnderLineSymbol(c);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsLetterOrNumberOrUnderLineSymbol(char c)
{
    return  IsLetterSymbol    (c) ||
            IsUnderLineSymbol (c) ||
            IsNumSymbol       (c);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSpace(char c)
{
    return (c == ' ');
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSlashN(char c)
{
    return (c == '\n');
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSlash0(char c)
{
    return (c == '\0');
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsSlashNOrSlashN(char c)
{
    return  IsSlashN(c) ||
            IsSlash0(c);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPassSymbol(char c)
{
    return (IsSpace  (c) ||
            IsSlashN (c));
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void UpdatePointersAfterSpace(Pointers* pointer)
{
    assert(pointer);

    pointer->ip++;
    pointer->sp++;

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void UpdatePointersAfterSlashN(Pointers* pointer)
{
    assert(pointer);

    pointer->ip++;
    pointer->lp++;
    pointer->sp = 1;

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
