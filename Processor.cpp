#include <stdio.h>
#include "Processor.h"
#include "Compiler.h"
#include "Stack.h"

static ProcessorErrorType Verif (SPU* Spu, ProcessorErrorType* Err, const char* File, int Line, const char* Func);
static void PrintError   (ProcessorErrorType* Err);
static void PrintPlace   (const char* File, int Line, const char* Func);
static void ErrPlaceCtor (ProcessorErrorType* Err, const char* File, int Line, const char* Func);

static void CodeCtor     (SPU* Spu, ProcessorErrorType* Err, const IOfile* File);
static void CodeDtor     (SPU* Spu, ProcessorErrorType* Err);

//---------------------------------------------------------------------------------------------------------------------------

ProcessorErrorType WriteFileInCode(SPU* Spu, const IOfile* File)
{
    ProcessorErrorType Err = {};

    size_t CmdQuant = 0;
    FILE* CodeFilePtr = fopen(File->CodeFile, "r");

    if (CodeFilePtr == NULL)
    {
        Err.FailedOpenCodeFile = 1;
        Err.IsFatalError = 1;
        return PROCESSOR_VERIF(Spu, Err);
    }

    fscanf(CodeFilePtr, "%u", &CmdQuant);

    for (size_t cmd_i = 0; cmd_i < CmdQuant; cmd_i++)
    {
        int temp = 0;
        if (fscanf(CodeFilePtr, "%d", &temp) != 1)
        {
            Err.InvalidCmd = 1;
            Err.IsFatalError = 1;
            return PROCESSOR_VERIF(Spu, Err);
        }
        Spu->code.code[cmd_i] = temp;
    }

    return PROCESSOR_VERIF(Spu, Err);
}

//---------------------------------------------------------------------------------------------------------------------------

ProcessorErrorType RunProcessor(SPU* Spu)
{
    ProcessorErrorType Err = {};

    while (Spu->ip < Spu->code.size)
    {
        switch (Spu->code.code[Spu->ip])
        {
            case push:
            {
                StackElem_t PushElem = Spu->code.code[Spu->ip + 1];
                STACK_ASSERT(StackPush(&Spu->stack, PushElem));
                Spu->ip += 2;
                break;
            }

            case rpush:
            {
                StackElem_t PushElem = Spu->registers[Spu->code.code[Spu->ip + 1]];
                STACK_ASSERT(StackPush(&Spu->stack, PushElem));
                Spu->ip += 2;
                break;
            }

            case pop:
            {
                StackElem_t PopElem = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &PopElem));
                Spu->registers[Spu->code.code[Spu->ip + 1]] = PopElem;
                Spu->ip += 2;
                break;
            }

            case add:
            {
                StackElem_t FirstOperator  = 0, SecondOperator = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperator));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperator));
                StackElem_t PushElem = FirstOperator + SecondOperator;
                STACK_ASSERT(StackPush(&Spu->stack, PushElem));
                Spu->ip++;
                break;
            }

            case sub:
            {
                StackElem_t FirstOperator  = 0, SecondOperator = 0;
                STACK_ASSERT(StackPop (&Spu->stack, &FirstOperator));
                STACK_ASSERT(StackPop (&Spu->stack, &SecondOperator));
                StackElem_t PushElem = SecondOperator - FirstOperator;
                STACK_ASSERT(StackPush(&Spu->stack, PushElem));
                Spu->ip++;
                break;
            }

            case mul:
            {
                StackElem_t FirstOperator  = 0, SecondOperator = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperator));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperator));
                StackElem_t PushElem =  FirstOperator * SecondOperator;
                STACK_ASSERT(StackPush(&Spu->stack, PushElem));
                Spu->ip++;
                break;
            }

            case dive:
            {
                StackElem_t FirstOperator  = 0, SecondOperator = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperator));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperator));
                StackElem_t PushElem = SecondOperator / FirstOperator;
                STACK_ASSERT(StackPush(&Spu->stack, PushElem));
                Spu->ip++;
                break;
            }

            case jmp:
            {
                Spu->ip = Spu->code.code[Spu->ip + 1];
                break;
            }

            case ja:
            {
                StackElem_t FirstOperand = 0, SecondOperand = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperand));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperand));

                if (FirstOperand > SecondOperand)
                {
                    Spu->ip = Spu->code.code[Spu->ip + 1];
                    break;
                }
                Spu->ip += 2;
                break;
            }

            case jae:
            {
                StackElem_t FirstOperand = 0, SecondOperand = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperand));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperand));

                if (FirstOperand >= SecondOperand)
                {
                    Spu->ip = Spu->code.code[Spu->ip + 1];
                    break;
                }
                Spu->ip += 2;
                break;
            }

            case jb:
            {
                StackElem_t FirstOperand = 0, SecondOperand = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperand));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperand));

                if (FirstOperand < SecondOperand)
                {
                    Spu->ip = Spu->code.code[Spu->ip + 1];
                    break;
                }
                Spu->ip += 2;
                break;
            }

            case jbe:
            {
                StackElem_t FirstOperand = 0, SecondOperand = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperand));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperand));

                if (FirstOperand <= SecondOperand)
                {
                    Spu->ip = Spu->code.code[Spu->ip + 1];
                    break;
                }
                Spu->ip += 2;
                break;
            }

            case je:
            {
                StackElem_t FirstOperand = 0, SecondOperand = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperand));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperand));

                if (FirstOperand == SecondOperand)
                {
                    Spu->ip = Spu->code.code[Spu->ip + 1];
                    break;
                }
                Spu->ip += 2;
                break;
            }

            case jne:
            {
                StackElem_t FirstOperand = 0, SecondOperand = 0;
                STACK_ASSERT(StackPop(&Spu->stack, &FirstOperand));
                STACK_ASSERT(StackPop(&Spu->stack, &SecondOperand));

                if (FirstOperand != SecondOperand)
                {
                    Spu->ip = Spu->code.code[Spu->ip + 1];
                    break;
                }
                Spu->ip += 2;
                break;
            }

            case out:
            {
                COLOR_PRINT(VIOLET, "Programm out: ");
                STACK_ASSERT(PrintLastStackElem(&Spu->stack));
                Spu->ip++;
                break;
            }

            case hlt:
            {
                return PROCESSOR_VERIF(Spu, Err);                
                break;
            }

            default:
            {
                Err.InvalidCmd = 1;
                Err.IsFatalError = 1;
                return PROCESSOR_VERIF(Spu, Err);
                break;
            }
        }
    }

    STACK_ASSERT(StackDtor(&Spu->stack));
    return PROCESSOR_VERIF(Spu, Err);
}

//---------------------------------------------------------------------------------------------------------------------------

ProcessorErrorType SpuCtor(SPU* Spu, const IOfile* File)
{
    ProcessorErrorType Err = {};
    CodeCtor(Spu, &Err, File);
    PROCESSOR_RETURN_IF_ERR(Err);
    Spu->ip = 0;
    STACK_ASSERT(StackCtor(&Spu->stack ON_STACK_DEBUG(,__FILE__, __LINE__, __func__, "stack")));
    
    for (size_t registers_i = 0; registers_i < REGISTERS_QUANT; registers_i++)
    {
        Spu->registers[registers_i] = 0;
    }
    return PROCESSOR_VERIF(Spu, Err);
}

//---------------------------------------------------------------------------------------------------------------------------

ProcessorErrorType SpuDtor(SPU* Spu)
{
    ProcessorErrorType Err = {};
    CodeDtor(Spu, &Err);

    PROCESSOR_RETURN_IF_ERR(Err);
    Spu->ip = 0;
    for (size_t register_i = 0; register_i < REGISTERS_QUANT; register_i++)
    {
        Spu->registers[register_i] = 0;
    }
    return Err;
}

//---------------------------------------------------------------------------------------------------------------------------

static void CodeCtor(SPU* Spu, ProcessorErrorType* Err, const IOfile* File)
{
    FILE* CodeFilePtr = fopen(File->CodeFile, "r");

    if (CodeFilePtr == NULL)
    {
        Err->FailedOpenCodeFile = 1;
        Err->IsFatalError = 1;
        return;
    }

    size_t temp = 0;
    if (fscanf(CodeFilePtr, "%u", &temp) != 1)
    {
        Err->FailedReadFileLen = 1;
        Err->IsFatalError = 1;
        return;
    }

    Spu->code.size = temp;

    Spu->code.code = (StackElem_t*) calloc(temp, sizeof(StackElem_t));
    if (Spu->code.code == NULL)
    {
        Err->CtorCallocNull = 1;
        Err->IsFatalError = 1;
        return;
    }

    return;
}

//---------------------------------------------------------------------------------------------------------------------------

static void CodeDtor(SPU* Spu, ProcessorErrorType* Err)
{
    Spu->code.size = 0;
    free(Spu->code.code);
    Spu->code.code = NULL;
    return;
}

//---------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType Verif(SPU* Spu, ProcessorErrorType* Err, const char* File, int Line, const char* Func)
{
    ErrPlaceCtor(Err, File, Line, Func);

    if (Spu->code.size > 0 && Spu->code.code[Spu->code.size - 1] != hlt)
    {
        Err->NoHalt = 1;
        Err->IsFatalError = 1;
    }


    return *Err;
}

//---------------------------------------------------------------------------------------------------------------------------

static void PrintError(ProcessorErrorType* Err)
{
    if (Err->IsFatalError == 0)
    {
        return;
    }

    if (Err->CtorCallocNull == 1)
    {
        COLOR_PRINT(RED, "Error: failed to allocate memory in Ctor.\n");
    }
    
    if (Err->FailedOpenCodeFile == 1)
    {
        COLOR_PRINT(RED, "Error: failed to open file with code in Ctor.\n");
    }

    if (Err->FailedReadFileLen == 1)
    {
        COLOR_PRINT(RED, "Error: failed to read file lenght in file with code.\n");
    }

    if (Err->FailedOpenCodeFile == 1)
    {
        COLOR_PRINT(RED, "Error: failed to open file with code.\n");
    }

    if (Err->InvalidCmd == 1)
    {
        COLOR_PRINT(RED, "Error: invalid cmd.\n");
    }

    if (Err->NoHalt == 1)
    {
        COLOR_PRINT(RED, "Error: no 'hlt' command.\n");
    }

    return;
}

//---------------------------------------------------------------------------------------------------------------------------

void ProcessorDump(const SPU* Spu, const char* File, int Line, const char* Func)
{
    COLOR_PRINT(GREEN, "PROCESSOR DUMP BEGIN\n\n");

    COLOR_PRINT(VIOLET, "Dump made in:\n");
    PrintPlace(File, Line, Func);


    COLOR_PRINT(VIOLET, "Registers:\n");

    COLOR_PRINT(VIOLET, "ax = %d\n",   Spu->registers[ax]);
    COLOR_PRINT(VIOLET, "bx = %d\n",   Spu->registers[bx]);
    COLOR_PRINT(VIOLET, "cx = %d\n",   Spu->registers[cx]);
    COLOR_PRINT(VIOLET, "dx = %d\n\n", Spu->registers[dx]);


    COLOR_PRINT(YELLOW, "Code size = %u\n\n", Spu->code.size);

    COLOR_PRINT(WHITE, "CODE:\n");
    for (size_t code_i = 0; code_i < Spu->code.size; code_i++)
    {
        COLOR_PRINT(BLUE, "[%2u] %d", code_i, Spu->code.code[code_i]);
        if (code_i == Spu->ip)
        {
            COLOR_PRINT(WHITE, " < ip");
        }
        printf("\n");
    }
    COLOR_PRINT(RED, "CODE END\n");

    COLOR_PRINT(GREEN, "\nPROCESSOR DUMP END\n");

    return;
}

//---------------------------------------------------------------------------------------------------------------------------

static void PrintPlace(const char* File, int Line, const char* Func)
{
    COLOR_PRINT(WHITE, "File [%s]\n", File);
    COLOR_PRINT(WHITE, "Line [%d]\n", Line);
    COLOR_PRINT(WHITE, "Func [%s]\n", Func);
    printf("\n");
}

//---------------------------------------------------------------------------------------------------------------------------

static void ErrPlaceCtor(ProcessorErrorType* Err, const char* File, int Line, const char* Func)
{
    Err->Place.File = File;
    Err->Place.Line = Line;
    Err->Place.Func = Func;
    return;
}

//---------------------------------------------------------------------------------------------------------------------------

void ProcessorAssertPrint(ProcessorErrorType* Err, const char* File, int Line, const char* Func)
{
    if (Err->IsFatalError == 0)
    {
        return;
    }

    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(File, Line, Func);
    PrintError(Err);
    PrintPlace(Err->Place.File, Err->Place.Line, Err->Place.Func);
    return;
}