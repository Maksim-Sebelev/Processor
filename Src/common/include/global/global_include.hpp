#ifndef _GLOBAL_HPP_
#define _GLOBAL_HPP_

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum Cmd : int
{
    undef_cmd = 0,
    hlt          ,
    push         ,
    pop          ,
    add          ,
    sub          ,
    mul          ,
    dive         ,
    pp           , 
    mm           ,
    out          ,  // just out int
    outc         ,  // out char
    outr         ,  // out int and remove it
    outrc        ,  // out char and remove it
    jmp          ,
    ja           ,
    jae          ,
    jb           ,
    jbe          ,
    je           ,
    jne          ,
    call         ,
    ret          ,
    draw         ,
    rgba         ,
    CMD_QUANT    , // count
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct CmdInfo
{
    const Cmd    cmd;
    const char*  name; 
    const size_t argQuant;
    const size_t codeRecordSize;
    const size_t nameLen;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define STRLEN(str) (sizeof(str) - 1)

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define HLT   "hlt"
#define PUSH  "push"
#define POP   "pop"
#define ADD   "add"
#define SUB   "sub"
#define MUL   "mul"
#define DIV   "div"
#define PP    "pp"
#define MM    "mm"
#define OUT   "out"
#define OUTC  "outc"
#define OUTR  "outr"
#define OUTRC "outrc"
#define JMP   "jmp"
#define JA    "ja"
#define JAE   "jae"
#define JB    "jb"
#define JBE   "jbe"
#define JE    "je"
#define JNE   "jne"
#define CALL  "call"
#define RET   "ret"
#define DRAW  "draw"
#define RGBA  "rgba"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const CmdInfo CmdInfoArr[] = 
{
    {.cmd = Cmd::hlt      , .name = HLT  , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(HLT  )},
    {.cmd = Cmd::push     , .name = PUSH , .argQuant = 1, .codeRecordSize = 4, .nameLen = STRLEN(PUSH )},
    {.cmd = Cmd::pop      , .name = POP  , .argQuant = 1, .codeRecordSize = 4, .nameLen = STRLEN(POP  )},
    {.cmd = Cmd::add      , .name = ADD  , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(ADD  )},
    {.cmd = Cmd::sub      , .name = SUB  , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(SUB  )},
    {.cmd = Cmd::mul      , .name = MUL  , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(MUL  )},
    {.cmd = Cmd::dive     , .name = DIV  , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(DIV  )},
    {.cmd = Cmd::pp       , .name = PP   , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(PP   )},
    {.cmd = Cmd::mm       , .name = MM   , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(MM   )},
    {.cmd = Cmd::out      , .name = OUT  , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(OUT  )},
    {.cmd = Cmd::outc     , .name = OUTC , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(OUTC )},
    {.cmd = Cmd::outr     , .name = OUTR , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(OUTR )},
    {.cmd = Cmd::outrc    , .name = OUTRC, .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(OUTRC)},
    {.cmd = Cmd::jmp      , .name = JMP  , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(JMP  )},
    {.cmd = Cmd::ja       , .name = JA   , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(JA   )},
    {.cmd = Cmd::jae      , .name = JAE  , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(JAE  )},
    {.cmd = Cmd::jb       , .name = JB   , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(JB   )},
    {.cmd = Cmd::jbe      , .name = JBE  , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(JBE  )},
    {.cmd = Cmd::je       , .name = JE   , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(JE   )},
    {.cmd = Cmd::jne      , .name = JNE  , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(JNE  )},
    {.cmd = Cmd::call     , .name = CALL , .argQuant = 1, .codeRecordSize = 2, .nameLen = STRLEN(CALL )},
    {.cmd = Cmd::ret      , .name = RET  , .argQuant = 0, .codeRecordSize = 1, .nameLen = STRLEN(RET  )},
    {.cmd = Cmd::draw     , .name = DRAW , .argQuant = 3, .codeRecordSize = 5, .nameLen = STRLEN(DRAW )},
    {.cmd = Cmd::rgba     , .name = RGBA , .argQuant = 4, .codeRecordSize = 6, .nameLen = STRLEN(RGBA )},
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef HLT
#undef PUSH
#undef POP
#undef ADD
#undef SUB
#undef MUL
#undef DIV
#undef PP
#undef MM
#undef OUT
#undef OUTC
#undef OUTR
#undef OUTRC
#undef JMP
#undef JA
#undef JAE
#undef JB
#undef JBE
#undef JE
#undef JNE
#undef CALL
#undef RET
#undef DRAW
#undef RGBA

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const size_t CmdInfoArrSize = sizeof(CmdInfoArr) / sizeof(CmdInfoArr[0]);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static_assert((Cmd::CMD_QUANT == ((int) CmdInfoArrSize)), "You forgot about some Cmd in CmdInfoArr");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum Registers : char
{
    ax = 0,
    bx,
    cx,
    dx,
    ex,
    fx,
    REGISTERS_QUANT, // Count
    undef_register,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const size_t REGISTERS_NAME_LEN = 2;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct IOfile
{
    const char* asm_file;
    const char* bin_file;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define asm_extension "asm"
#define bin_extension "bin"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CmdInfo GetCmdInfo(Cmd command);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif //_GLOBAL_HPP_
