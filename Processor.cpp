#include <stdio.h>
#include "Processor.h"
#include "Compiler.h"
#include "Stack.h"
#include "GlobalInclude.h"

static  ProcessorErrorType  Verif        (SPU* Spu, ProcessorErrorType* Err, const char* File, int Line, const char* Func);
static  void                PrintError   (ProcessorErrorType* Err);
static  void                PrintPlace   (const char* File, int Line, const char* Func);
static  void                ErrPlaceCtor (ProcessorErrorType* Err, const char* File, int Line, const char* Func);

static  void  CodeCtor     (SPU* Spu, ProcessorErrorType* Err, const IOfile* File);
static  void  CodeDtor     (SPU* Spu, ProcessorErrorType* Err);

static  ProcessorErrorType  ArithmeticCmdPattern     (SPU* Spu, ArithmeticOperator Operator, ProcessorErrorType* Err);
static  ProcessorErrorType  JumpsCmdPatter           (SPU* Spu, ComparisonOperator Operator, ProcessorErrorType* Err);


static  ProcessorErrorType  HandlePush   (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandlePushR  (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandlePop    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleAdd    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleSub    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleMul    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleDiv    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleJmp    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleJa     (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleJae    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleJb     (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleJbe    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleJe     (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleJne    (SPU* Spu, ProcessorErrorType* Err);
static  ProcessorErrorType  HandleOut    (SPU* Spu, ProcessorErrorType* Err);

//----------------------------------------------------------------------------------------------------------------------------------------------------

ProcessorErrorType WriteFileInCode(SPU* Spu, const IOfile* File)
{
    ProcessorErrorType Err = {};

    size_t CmdQuant = 0;
    FILE* CodeFilePtr = fopen(File->CodeFile, "rb");

    if (CodeFilePtr == NULL)
    {
        Err.FailedOpenCodeFile = 1;
        Err.IsFatalError = 1;
        return PROCESSOR_VERIF(Spu, Err);
    }

    fscanf(CodeFilePtr, "%u", &CmdQuant);

    for (size_t cmd_i = 0; cmd_i < CmdQuant; cmd_i++)
    {
        int Cmd = 0;
        if (fscanf(CodeFilePtr, "%d", &Cmd) != 1)
        {
            Err.InvalidCmd = 1;
            Err.IsFatalError = 1;
            return PROCESSOR_VERIF(Spu, Err);
        }
        Spu->code.code[cmd_i] = Cmd;
    }

    return PROCESSOR_VERIF(Spu, Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

ProcessorErrorType RunProcessor(SPU* Spu)
{
    ProcessorErrorType Err = {};

    while (Spu->ip < Spu->code.size)
    {
            // printf("cmd = %d\n", Spu->code.code[Spu->ip]);

        switch (Spu->code.code[Spu->ip])
        {
            case push:
            {
                PROCESSOR_RETURN_IF_ERR(HandlePush(Spu, &Err));
                break;
            }

            case pushr:
            {
                PROCESSOR_RETURN_IF_ERR(HandlePushR(Spu, &Err));
                break;
            }

            case pop:
            {
                PROCESSOR_RETURN_IF_ERR(HandlePop(Spu, &Err));
                break;
            }

            case add:
            {
                PROCESSOR_RETURN_IF_ERR(HandleAdd(Spu, &Err));
                break;
            }

            case sub:
            {
                PROCESSOR_RETURN_IF_ERR(HandleSub(Spu, &Err));
                break;
            }

            case mul:
            {
                PROCESSOR_RETURN_IF_ERR(HandleMul(Spu, &Err));
                break;
            }

            case dive:        //name div is already used in stdlib.h
            {
                PROCESSOR_RETURN_IF_ERR(HandleDiv(Spu, &Err));
                break;
            }

            case jmp:
            {
                PROCESSOR_RETURN_IF_ERR(HandleJmp(Spu, &Err));
                break;
            }

            case ja:
            {
                PROCESSOR_RETURN_IF_ERR(HandleJa(Spu, &Err));
                break;
            }

            case jae:
            {
                PROCESSOR_RETURN_IF_ERR(HandleJae(Spu, &Err));
                break;
            }

            case jb:
            {
                PROCESSOR_RETURN_IF_ERR(HandleJb(Spu, &Err));
                break;
            }

            case jbe:
            {
                PROCESSOR_RETURN_IF_ERR(HandleJbe(Spu, &Err));
                break;
            }

            case je:
            {
                PROCESSOR_RETURN_IF_ERR(HandleJe(Spu, &Err));
                break;
            }

            case jne:
            {
                PROCESSOR_RETURN_IF_ERR(HandleJne(Spu, &Err));
                break;
            }

            case out:
            {
                PROCESSOR_RETURN_IF_ERR(HandleOut(Spu, &Err));
                break;
            }

            case hlt:
            {
                return PROCESSOR_VERIF(Spu, Err);                
                break;
            }

            default:
            {
                // Err.InvalidCmd = 1;
                // Err.IsFatalError = 1;
                // return PROCESSOR_VERIF(Spu, Err);
                Spu->ip++;
                break;
            }
        }
    }

    STACK_ASSERT(StackDtor(&Spu->stack));
    return PROCESSOR_VERIF(Spu, Err);
}


//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandlePush(SPU* Spu, ProcessorErrorType* Err)
{
    StackElem_t PushElem = Spu->code.code[Spu->ip + 1];
    STACK_ASSERT(StackPush(&Spu->stack, PushElem));
    Spu->ip += 2;
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandlePushR(SPU* Spu, ProcessorErrorType* Err)
{
    StackElem_t PushElem = Spu->registers[Spu->code.code[Spu->ip + 1]];
    STACK_ASSERT(StackPush(&Spu->stack, PushElem));
    Spu->ip += 2;
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandlePop(SPU* Spu, ProcessorErrorType* Err)
{
    StackElem_t PopElem = 0;
    STACK_ASSERT(StackPop(&Spu->stack, &PopElem));
    Spu->registers[Spu->code.code[Spu->ip + 1]] = PopElem;
    Spu->ip += 2;
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleAdd(SPU* Spu, ProcessorErrorType* Err)
{
    ArithmeticCmdPattern(Spu, plus, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleSub(SPU* Spu, ProcessorErrorType* Err)
{   
    ArithmeticCmdPattern(Spu, minus, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleMul(SPU* Spu, ProcessorErrorType* Err)
{
    ArithmeticCmdPattern(Spu, multiplication, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleDiv(SPU* Spu, ProcessorErrorType* Err)
{
    ArithmeticCmdPattern(Spu, division, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleJmp(SPU* Spu, ProcessorErrorType* Err)
{
    JumpsCmdPatter(Spu, always_true, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleJa(SPU* Spu, ProcessorErrorType* Err)
{
    JumpsCmdPatter(Spu, above, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleJae(SPU* Spu, ProcessorErrorType* Err)
{
    JumpsCmdPatter(Spu, above_or_equal, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleJb(SPU* Spu, ProcessorErrorType* Err)
{
    JumpsCmdPatter(Spu, bellow, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleJbe(SPU* Spu, ProcessorErrorType* Err)
{
    JumpsCmdPatter(Spu, bellow_or_equal, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleJe(SPU* Spu, ProcessorErrorType* Err)
{
    JumpsCmdPatter(Spu, equal, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleJne(SPU* Spu, ProcessorErrorType* Err)
{
    JumpsCmdPatter(Spu, not_equal, Err);
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType HandleOut(SPU* Spu, ProcessorErrorType* Err)
{
    COLOR_PRINT(VIOLET, "Programm out: ");
    STACK_ASSERT(PrintLastStackElem(&Spu->stack));
    Spu->ip++;
    return *Err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

static void CodeCtor(SPU* Spu, ProcessorErrorType* Err, const IOfile* File)
{
    FILE* CodeFilePtr = fopen(File->CodeFile, "r");

    if (CodeFilePtr == NULL)
    {
        Err->FailedOpenCodeFile = 1;
        Err->IsFatalError = 1;
        return;
    }

    size_t Second = 0;
    if (fscanf(CodeFilePtr, "%u", &Second) != 1)
    {
        Err->FailedReadFileLen = 1;
        Err->IsFatalError = 1;
        return;
    }

    Spu->code.size = Second;

    Spu->code.code = (StackElem_t*) calloc(Second, sizeof(StackElem_t));
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

//----------------------------------------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintPlace(const char* File, int Line, const char* Func)
{
    COLOR_PRINT(WHITE, "File [%s]\n",   File);
    COLOR_PRINT(WHITE, "Line [%d]\n",   Line);
    COLOR_PRINT(WHITE, "Func [%s]\n\n", Func);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

static void ErrPlaceCtor(ProcessorErrorType* Err, const char* File, int Line, const char* Func)
{
    Err->Place.File = File;
    Err->Place.Line = Line;
    Err->Place.Func = Func;
    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType ArithmeticCmdPattern(SPU* Spu, ArithmeticOperator Operator, ProcessorErrorType* Err)
{
    StackElem_t SecondOperand  = 0, FirstOperand = 0;
    STACK_ASSERT(StackPop(&Spu->stack, &FirstOperand));
    STACK_ASSERT(StackPop(&Spu->stack, &SecondOperand));

    StackElem_t PushElem = MakeArithmeticOperation(SecondOperand, FirstOperand, Operator);
    STACK_ASSERT(StackPush(&Spu->stack, PushElem));
    Spu->ip++;
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErrorType JumpsCmdPatter(SPU* Spu, ComparisonOperator Operator, ProcessorErrorType* Err)
{
    StackElem_t SecondOperand = 0, FirstOperand = 0;

    if (Operator != always_true)
    {
        STACK_ASSERT(StackPop(&Spu->stack, &FirstOperand));
        STACK_ASSERT(StackPop(&Spu->stack, &SecondOperand));
    }

    if (MakeComparisonOperation(FirstOperand, SecondOperand, Operator))
    {
        Spu->ip = Spu->code.code[Spu->ip + 1];
        printf("after jump = %d", Spu->code.code[Spu->ip + 1]);
        return PROCESSOR_VERIF(Spu, *Err);
    }
    Spu->ip += 2;
    return PROCESSOR_VERIF(Spu, *Err);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------------------------------------------------------------
