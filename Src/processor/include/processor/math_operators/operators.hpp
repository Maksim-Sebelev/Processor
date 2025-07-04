#ifndef __OPERATORS_HPP__
#define __OPERATORS_HPP__

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "global/global_include.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool MakeComparisonOperation(int first_operand, int second_operand, ComparisonOperator comparison_operator);
int  MakeArithmeticOperation(int first_operand, int second_operand, ArithmeticOperator arithmetic_operator);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // __OPERATORS_HPP__