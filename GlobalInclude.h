#ifndef GLOBAL_H
#define GLOBAL_H

#include "Stack.h"

enum Cmd
{
    hlt = 0,
    push,
    pop,
    add,
    sub,
    mul,
    dive,
    out,
    jmp,
    ja,
    jae,
    jb,
    jbe,
    je,
    jne,
    CMD_QUANT // count
};

struct PushType
{
    unsigned int stk   : 1;
    unsigned int reg   : 1;
    unsigned int mem   : 1;

};


struct PopType
{
    unsigned int reg   : 1;
    unsigned int mem   : 1;
};


enum Registers
{
    ax = 0,
    bx,
    cx,
    dx,
    REGISTERS_QUANT // Count
};


enum ArithmeticOperator
{
    plus, 
    minus,
    multiplication,
    division
};


enum ComparisonOperator
{
    above,
    above_or_equal,
    bellow,
    bellow_or_equal,
    equal,
    not_equal,
    always_true
};


struct IOfile
{
    const char* ProgrammFile;
    const char* CodeFile;
};

bool        MakeComparisonOperation  (StackElem_t FirstOperand, StackElem_t SecondOperand, ComparisonOperator Operator);
StackElem_t MakeArithmeticOperation  (StackElem_t FirstOperand, StackElem_t SecondOperand, ArithmeticOperator Operator);

#define FREE(Arr) free(Arr); Arr = NULL;

#endif
