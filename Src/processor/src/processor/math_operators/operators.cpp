
#include <assert.h>
#include "processor/math_operators/operators.hpp"
#include "lib/lib.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void CheckDivisionByZero(int devider);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool MakeComparisonOperation(int first_operand, int second_operand, ComparisonOperator comparison_operator)
{
    switch (comparison_operator)
    {
        case ComparisonOperator::always_true:      return true;
        case ComparisonOperator::above:            return first_operand >  second_operand;
        case ComparisonOperator::above_or_equal:   return first_operand >= second_operand;
        case ComparisonOperator::bellow:           return first_operand <  second_operand;
        case ComparisonOperator::bellow_or_equal:  return first_operand <= second_operand;
        case ComparisonOperator::equal:            return first_operand == second_operand;
        case ComparisonOperator::not_equal:        return first_operand != second_operand;
        default: assert (0 && "undefined comparison operator"); break;
    }

    assert(0 && "undefined comparison operator");
    return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int MakeArithmeticOperation(int first_operand, int second_operand, ArithmeticOperator arithmetic_operator)
{
    switch (arithmetic_operator)
    {
        case plus:           return first_operand + second_operand;
        case minus:          return first_operand - second_operand;
        case multiplication: return first_operand * second_operand;
        case division:
        {
            CheckDivisionByZero(second_operand);
            return first_operand / second_operand;
        }
        default: __builtin_unreachable__("udndef math operation"); break;
    }

    __builtin_unreachable__("wtf");
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void CheckDivisionByZero(int devider)
{
    if (devider == 0)
        EXIT(EXIT_FAILURE, "division by zero");

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

