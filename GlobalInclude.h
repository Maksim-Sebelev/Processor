#ifndef GLOBAL_H
#define GLOBAL_H

#include "Stack.h"

enum Cmd
{
    hlt = 0,
    push,
    pushr,
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
    lab,
    CMD_QUANT // count
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


bool        MakeComparisonOperation  (StackElem_t FirstOperand, StackElem_t SecondOperand, ComparisonOperator Operator);
StackElem_t MakeArithmeticOperation  (StackElem_t FirstOperand, StackElem_t SecondOperand, ArithmeticOperator Operator);

#endif
