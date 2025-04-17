#ifndef GLOBAL_INCLUDE_H
#define GLOBAL_INCLUDE_H

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <stddef.h>
#include "lib/colorPrint.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum Cmd : int
{
    hlt = 0    ,
    pushc      ,
    pushi      ,
    pushd      ,
    popc       ,
    popi       ,
    popd       ,
    add        ,
    sub        ,
    mul        ,
    dive       ,
    pp         , 
    mm         ,
    out        ,  // just out int
    outc       ,  // out char
    outr       ,  // out int and remove it
    outrc      ,  // out char and remove it
    jmp        ,
    ja         ,
    jae        ,
    jb         ,
    jbe        ,
    je         ,
    jne        ,
    call       ,
    ret        ,
    CMD_QUANT  , // count
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct CmdInfo
{
    const Cmd    cmd;
    const size_t argQuant;
    const size_t codeRecordSize;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const CmdInfo CmdInfoArr[] = 
{
    {Cmd::hlt  , .argQuant = 0, .codeRecordSize = 1},
    {Cmd::pushc, .argQuant = 1, .codeRecordSize = 4},
    {Cmd::pushi, .argQuant = 1, .codeRecordSize = 4},
    {Cmd::pushd, .argQuant = 1, .codeRecordSize = 4},
    {Cmd::popc , .argQuant = 1, .codeRecordSize = 4},
    {Cmd::popi , .argQuant = 1, .codeRecordSize = 4},
    {Cmd::popd , .argQuant = 1, .codeRecordSize = 4},
    {Cmd::add  , .argQuant = 0, .codeRecordSize = 1},
    {Cmd::sub  , .argQuant = 0, .codeRecordSize = 1},
    {Cmd::mul  , .argQuant = 0, .codeRecordSize = 1},
    {Cmd::dive , .argQuant = 0, .codeRecordSize = 1},
    {Cmd::pp   , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::mm   , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::out  , .argQuant = 0, .codeRecordSize = 1},
    {Cmd::outc , .argQuant = 0, .codeRecordSize = 1},
    {Cmd::outr , .argQuant = 0, .codeRecordSize = 1},
    {Cmd::outrc, .argQuant = 0, .codeRecordSize = 1},
    {Cmd::jmp  , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::ja   , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::jae  , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::jb   , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::jbe  , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::je   , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::jne  , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::call , .argQuant = 1, .codeRecordSize = 2},
    {Cmd::ret  , .argQuant = 0, .codeRecordSize = 1},
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const size_t CmdInfoArrSize = sizeof(CmdInfoArr) / sizeof(CmdInfoArr[0]);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static_assert((Cmd::CMD_QUANT == (int) CmdInfoArrSize), "You forgot about some Cmd in CmdInfoArr");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum CharRegisters : size_t
{
    cax = 0,
    cbx,
    ccx,
    cdx,
    CHAR_REGISTERS_QUANT, // Count
    CHAR_REGISTERS_NAME_LEN = 3, // in my assebler-standart all registers must have the same name's lenght
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum IntRegisters : size_t
{
    iax = 0,
    ibx,
    icx,
    idx,
    INT_REGISTERS_QUANT, // Count
    INT_REGISTERS_NAME_LEN = 3, // in my assebler-standart all registers must have the same name's lenght
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum DoubleRegisters : size_t
{
    dax = 0,
    dbx,
    dcx,
    ddx,
    DOUBLE_REGISTERS_QUANT, // Count
    DOUBLE_REGISTERS_NAME_LEN = 3, // in my assebler-standart all registers must have the same name's lenght
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct PushType
{
    unsigned int stk : 1;
    unsigned int reg : 1;
    unsigned int mem : 1;
    unsigned int sum : 1;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct PopType
{
    unsigned int reg : 1;
    unsigned int mem : 1;
    unsigned int sum : 1;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum ArithmeticOperator : int
{
    plus           = Cmd::add , 
    minus          = Cmd::sub ,
    multiplication = Cmd::mul ,
    division       = Cmd::dive,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum ComparisonOperator : int
{
    above           = Cmd::ja ,
    above_or_equal  = Cmd::jae,
    bellow          = Cmd::jb ,
    bellow_or_equal = Cmd::jbe,
    equal           = Cmd::je ,
    not_equal       = Cmd::jne,
    always_true     = Cmd::jmp,
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct IOfile
{
    const char* ProgrammFile;
    const char* CodeFile;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif
