#include "GlobalInclude.h"
#include "Stack.h"

bool MakeComparisonOperation(StackElem_t FirstOperand, StackElem_t SecondOperand, ComparisonOperator Operator)
{
    switch (Operator)
    {
        case always_true:
        {
            return true;
            break;
        }

        case above:
        {
            return FirstOperand > SecondOperand;
            break;
        }

        case above_or_equal:
        {
            return FirstOperand >= SecondOperand;
            break;
        }

        case bellow:
        {
            return FirstOperand < SecondOperand;
            break;
        }

        case bellow_or_equal:
        {
            return FirstOperand <= SecondOperand;
            break;
        }

        case equal:
        {
            return FirstOperand == SecondOperand;
            break;
        }

        case not_equal:
        {
            return FirstOperand != SecondOperand;
            break;
        }

        default:
        {
            break;
        }
    }

    return false;
}


StackElem_t MakeArithmeticOperation(StackElem_t FirstOperand, StackElem_t SecondOperand, ArithmeticOperator Operator)
{
    switch (Operator)
    {
        case plus:
        {
            return FirstOperand + SecondOperand;
            break;
        }

        case minus:
        {
            return FirstOperand - SecondOperand;
            break;
        }

        case multiplication:
        {
            return FirstOperand * SecondOperand;
            break;
        }

        case division:
        {
            return FirstOperand / SecondOperand;
            break;
        }
        
        default:
        {
            break;
        }
    }

    return 0;
}
