#include <stdio.h>
#include <assert.h>
#include "Processor.h"
#include "../Stack/Stack.h"
#include "../Common/GlobalInclude.h"
#include "../Onegin/Onegin.h"

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct Code
{
    int*   code;
    size_t size;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct SPU
{
    Code         code;
    size_t       ip;
    Stack_t      stack;
    StackElem_t  registers[REGISTERS_QUANT];
    int*         RAM;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr   Verif                      (SPU* spu, ProcessorErr* err,  const char* file, int line, const char* func);
static void           PrintError                 (          ProcessorErr* err);

static ProcessorErr   SpuCtor                    (SPU* spu, const IOfile* file);
static ProcessorErr   SpuDtor                    (SPU* spu);
static ProcessorErr   ExecuteCommands            (SPU* spu);

static ProcessorErr   CodeCtor                   (SPU* spu, const IOfile* file);
static ProcessorErr   ReadCodeFromFile           (SPU* spu, FILE* codeFilePtr);


static ProcessorErr   HandleHalt                 (SPU* spu);
static ProcessorErr   HandlePush                 (SPU* spu);
static ProcessorErr   HandlePop                  (SPU* spu);
static ProcessorErr   HandleAdd                  (SPU* spu);
static ProcessorErr   HandleSub                  (SPU* spu);
static ProcessorErr   HandleMul                  (SPU* spu);
static ProcessorErr   HandleDiv                  (SPU* spu);
static ProcessorErr   HandleJmp                  (SPU* spu);
static ProcessorErr   HandleJa                   (SPU* spu);
static ProcessorErr   HandleJae                  (SPU* spu);
static ProcessorErr   HandleJb                   (SPU* spu);
static ProcessorErr   HandleJbe                  (SPU* spu);
static ProcessorErr   HandleJe                   (SPU* spu);
static ProcessorErr   HandleJne                  (SPU* spu);
static ProcessorErr   HandleOut                  (SPU* spu);
static ProcessorErr   HandleOutr                 (SPU* spu);


static ProcessorErr   ArithmeticCmdPattern       (SPU* spu, ArithmeticOperator Operator);
static ProcessorErr   JumpsCmdPatter             (SPU* spu, ComparisonOperator Operator);

static StackElem_t    MakeArithmeticOperation    (StackElem_t FirstOperand, StackElem_t SecondOperand, ArithmeticOperator Operator);
static bool           MakeComparisonOperation    (StackElem_t FirstOperand, StackElem_t SecondOperand, ComparisonOperator Operator);

static PushType       GetPushType                (int PushArg);
static PopType        GetPopType                 (int PopArg);

static int            GetPushPopArg              (SPU* spu);
static int            GetPushPopRegister         (SPU* spu);
static int            GetPushPopMemory           (SPU* spu);
static int            GetPushPopMemoryWithReg    (SPU* spu);

static void           SetPopMemory               (SPU* spu, StackElem_t PopElem);
static void           SetPopMemoryWithRegister   (SPU* spu, StackElem_t PopElem);
static void           SetPopRegister             (SPU* spu, StackElem_t PopElem);

static size_t         GetCodeSize                (SPU* spu);
static size_t         GetIp                      (SPU* spu);
static int            GetCodeElem                (SPU* spu);
static int            GetNextCodeElem            (SPU* spu);
static void           SetCodeElem                (SPU* spu, size_t Code_i, int NewCodeElem);


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void ProcessorDump(const SPU* spu, const char* file, int line, const char* func);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PROCESSSOR_DUMP(SpuPtr) ProcessorDump(SpuPtr, __FILE__, __LINE__, __func__)


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PROCESSOR_VERIF(spu, err) Verif(spu, &err, __FILE__, __LINE__, __func__)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void RunProcessor(const IOfile* file)
{
    assert(file);

    SPU spu = {};
    PROCESSOR_ASSERT(SpuCtor(&spu, file));
    PROCESSOR_ASSERT(ExecuteCommands(&spu));
    PROCESSOR_ASSERT(SpuDtor(&spu));

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr SpuCtor(SPU* spu, const IOfile* file)
{
    assert(spu);
    assert(file);

    ProcessorErr  err = {};

    PROCESSOR_ASSERT(CodeCtor(spu, file));

    spu->ip = 0;

    static const size_t DeafaultStackSize = 128;
    STACK_ASSERT(StackCtor(&spu->stack, DeafaultStackSize));
    
    for (size_t registers_i = 0; registers_i < Registers::REGISTERS_QUANT; registers_i++)
    {
        spu->registers[registers_i] = 0;
    }

    static const size_t DeafaultRamSize = 128;
    spu->RAM = (int*) calloc(DeafaultRamSize, sizeof(int));

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr SpuDtor(SPU* spu)
{
    assert(spu);

    ProcessorErr  err = {};

    FREE(spu->RAM);
    FREE(spu->code.code);

    *spu = {};

    return err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ExecuteCommands(SPU* spu)
{
    assert(spu);

    ProcessorErr  err = {};

    while (GetIp(spu) < GetCodeSize(spu))
    {
        switch (GetCodeElem(spu))
        {
            case Cmd::push: PROCESSOR_ASSERT(HandlePush(spu)); break;
            case Cmd::pop:  PROCESSOR_ASSERT(HandlePop (spu)); break;
            case Cmd::add:  PROCESSOR_ASSERT(HandleAdd (spu)); break;
            case Cmd::sub:  PROCESSOR_ASSERT(HandleSub (spu)); break;
            case Cmd::mul:  PROCESSOR_ASSERT(HandleMul (spu)); break;
            case Cmd::dive: PROCESSOR_ASSERT(HandleDiv (spu)); break;
            case Cmd::jmp:  PROCESSOR_ASSERT(HandleJmp (spu)); break;
            case Cmd::ja:   PROCESSOR_ASSERT(HandleJa  (spu)); break;
            case Cmd::jae:  PROCESSOR_ASSERT(HandleJae (spu)); break;
            case Cmd::jb:   PROCESSOR_ASSERT(HandleJb  (spu)); break;
            case Cmd::jbe:  PROCESSOR_ASSERT(HandleJbe (spu)); break;
            case Cmd::je:   PROCESSOR_ASSERT(HandleJe  (spu)); break;
            case Cmd::jne:  PROCESSOR_ASSERT(HandleJne (spu)); break;
            case Cmd::out:  PROCESSOR_ASSERT(HandleOut (spu)); break;
            case Cmd::outr: PROCESSOR_ASSERT(HandleOutr(spu)); break;
            case Cmd::hlt: PROCESSSOR_DUMP(spu);   return HandleHalt(spu);
            default:
            {
                err.err = ProcessorErrorType::INVALID_CMD;
                return PROCESSOR_VERIF(spu, err);
            }
        }
    }

    err.err = ProcessorErrorType::NO_HALT;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandlePush(SPU* spu)
{
    assert(spu);

    ProcessorErr err = {};

    int      PushTypeInt = GetNextCodeElem(spu);
    PushType Push        = GetPushType(PushTypeInt);

    StackElem_t PushElem = 0;

    if      (Push.stk == 1 && Push.reg == 0 && Push.mem == 0) PushElem = GetPushPopArg           (spu);
    else if (Push.stk == 0 && Push.reg == 1 && Push.mem == 0) PushElem = GetPushPopRegister      (spu);
    else if (Push.stk == 0 && Push.reg == 0 && Push.mem == 1) PushElem = GetPushPopMemory        (spu);
    else if (Push.stk == 0 && Push.reg == 1 && Push.mem == 1) PushElem = GetPushPopMemoryWithReg (spu);
    else
    {
        err.err = ProcessorErrorType::INVALID_CMD;
        return PROCESSOR_VERIF(spu, err);
    }

    STACK_ASSERT(StackPush(&spu->stack, PushElem));

    spu->ip += 3;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandlePop(SPU* spu)
{
    assert(spu);
    ProcessorErr err = {};

    int PopTypeInt = GetNextCodeElem(spu);

    PopType Pop = {};
    Pop = GetPopType(PopTypeInt);

    StackElem_t PopElem = 0;
    STACK_ASSERT(StackPop(&spu->stack, &PopElem));

    if      (Pop.reg == 1 && Pop.mem == 0) SetPopRegister            (spu, PopElem);
    else if (Pop.reg == 0 && Pop.mem == 1) SetPopMemory              (spu, PopElem);
    else if (Pop.reg == 1 && Pop.mem == 1) SetPopMemoryWithRegister  (spu, PopElem);
    else
    {
        err.err = ProcessorErrorType::INVALID_CMD;
        return PROCESSOR_VERIF(spu, err);
    }

    spu->ip += 3;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleAdd(SPU* spu)
{
    assert(spu);

    return ArithmeticCmdPattern(spu, plus);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleSub(SPU* spu)
{   
    assert(spu);
    return ArithmeticCmdPattern(spu, minus);;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleMul(SPU* spu)
{
    assert(spu);
    return ArithmeticCmdPattern(spu, multiplication);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleDiv(SPU* spu)
{
    assert(spu);
    return ArithmeticCmdPattern(spu, division);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJmp(SPU* spu)
{
    assert(spu);
    return JumpsCmdPatter(spu, always_true);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJa(SPU* spu)
{
    assert(spu);
    return JumpsCmdPatter(spu, above);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJae(SPU* spu)
{
    assert(spu);
    return JumpsCmdPatter(spu, above_or_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJb(SPU* spu)
{
    assert(spu);
    return JumpsCmdPatter(spu, bellow);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJbe(SPU* spu)
{
    assert(spu);
    return JumpsCmdPatter(spu, bellow_or_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJe(SPU* spu)
{
    assert(spu);
    return JumpsCmdPatter(spu, equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJne(SPU* spu)
{
    assert(spu);
    return JumpsCmdPatter(spu, not_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOut(SPU* spu)
{
    assert(spu);

    ProcessorErr err = {};

    StackElem_t elem = GetLastStackElem(&spu->stack);

    COLOR_PRINT(VIOLET, "Programm out: %d\n", elem);

    spu->ip++;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOutr(SPU* spu)
{
    assert(spu);

    ProcessorErr err = {};

    StackElem_t elem = 0;

    STACK_ASSERT(StackPop(&spu->stack, &elem));

    COLOR_PRINT(VIOLET, "Programm out: %d\n", elem);

    spu->ip++;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleHalt(SPU* spu)
{
    assert(spu);

    ProcessorErr err = {};

    spu->ip++;
    STACK_ASSERT(StackDtor(&spu->stack));
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ArithmeticCmdPattern(SPU* spu, ArithmeticOperator Operator)
{
    assert(spu);
    ProcessorErr err = {};

    StackElem_t SecondOperand  = 0, FirstOperand = 0;
    STACK_ASSERT(StackPop(&spu->stack, &FirstOperand));
    STACK_ASSERT(StackPop(&spu->stack, &SecondOperand));

    StackElem_t PushElem = MakeArithmeticOperation(SecondOperand, FirstOperand, Operator);
    STACK_ASSERT(StackPush(&spu->stack, PushElem));
    spu->ip++;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr JumpsCmdPatter(SPU* spu, ComparisonOperator Operator)
{
    assert(spu);

    ProcessorErr err = {};

    StackElem_t SecondOperand = 0, FirstOperand = 0;

    if (Operator != always_true)
    {
        STACK_ASSERT(StackPop(&spu->stack, &FirstOperand));
        STACK_ASSERT(StackPop(&spu->stack, &SecondOperand));
    }

    if (MakeComparisonOperation(FirstOperand, SecondOperand, Operator))
    {
        spu->ip = (size_t) GetNextCodeElem(spu);
        return PROCESSOR_VERIF(spu, err);
    }

    spu->ip += 2;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PushType GetPushType(int PushArg)
{
    return *(PushType*) &PushArg;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PopType GetPopType(int PopArg)
{
    return *(PopType*) &PopArg;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopArg(SPU* spu)
{
    assert(spu);

    return spu->code.code[spu->ip + 2];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopRegister(SPU* spu)
{
    assert(spu);

    return spu->registers[GetPushPopArg(spu)];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopMemory(SPU* spu)
{
    assert(spu);

    return spu->RAM[GetPushPopArg(spu)];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopMemoryWithReg(SPU* spu)
{
    assert(spu);

    return spu->RAM[GetPushPopRegister(spu)];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


static void SetPopRegister(SPU* spu, StackElem_t PopElem)
{
    assert(spu);

    spu->registers[GetPushPopArg(spu)] = PopElem;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetPopMemory(SPU* spu, StackElem_t PopElem)
{
    assert(spu);

    spu->RAM[GetPushPopArg(spu)] = PopElem;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetPopMemoryWithRegister(SPU* spu, StackElem_t PopElem)
{
    assert(spu);

    spu->RAM[GetPushPopRegister(spu)] = PopElem;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetCodeElem(SPU* spu)
{
    assert(spu);

    return spu->code.code[spu->ip];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetNextCodeElem(SPU* spu)
{
    assert(spu);

    return spu->code.code[spu->ip + 1];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static size_t GetCodeSize(SPU* spu)
{
    assert(spu);

    return spu->code.size;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static size_t GetIp(SPU* spu)
{
    assert(spu);

    return spu->ip;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetCodeElem(SPU* spu, size_t Code_i, int NewCodeElem)
{
    assert(spu);

    spu->code.code[Code_i] = NewCodeElem;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ReadCodeFromFile(SPU* spu, FILE* codeFilePtr)
{
    assert(spu);
    assert(codeFilePtr);

    ProcessorErr err = {};

    size_t CmdQuant = spu->code.size;

    for (size_t cmd_i = 0; cmd_i < CmdQuant; cmd_i++)
    {
        int Cmd = 0;
        if (fscanf(codeFilePtr, "%d", &Cmd) != 1)
        {
            err.err = ProcessorErrorType::INVALID_CMD;
            return PROCESSOR_VERIF(spu, err);
        }
        SetCodeElem(spu, cmd_i, Cmd);
    }

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr CodeCtor(SPU* spu, const IOfile* file)
{
    assert(spu);
    assert(file);

    ProcessorErr err = {};

    FILE* CodeFilePtr = fopen(file->CodeFile, "rb");

    if (!CodeFilePtr)
    {
        err.err = ProcessorErrorType::FAILED_OPEN_CODE_FILE;
        return PROCESSOR_VERIF(spu, err);
    }

    size_t codeSize = 0;
    if (fscanf(CodeFilePtr, "%lu", &codeSize) != 1)
    {
        err.err = ProcessorErrorType::FAILED_READ_FILE_LEN;
        return PROCESSOR_VERIF(spu, err);
    }

    spu->code.size = codeSize;
    spu->code.code = (int*) calloc(codeSize, sizeof(int));

    if (!spu->code.code)
    {
        err.err = ProcessorErrorType::SPU_CODE_CALLOC_NULL;
        return PROCESSOR_VERIF(spu, err);
    }

    PROCESSOR_ASSERT(ReadCodeFromFile(spu, CodeFilePtr));

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr Verif(SPU* spu, ProcessorErr* err,  const char* file, int line, const char* func)
{
    assert(spu);
    assert(err);
    assert(file);
    assert(func);

    CodePlaceCtor(&err->place, file, line, func);

    // if (spu->code.size > 0 && spu->code.code[spu->code.size - 1] != hlt)
    // {
    //     err->NoHalt = 1;
    //     err->IsFatalError = 1;
    // }

    return *err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool MakeComparisonOperation(StackElem_t FirstOperand, StackElem_t SecondOperand, ComparisonOperator Operator)
{
    switch (Operator)
    {
        case ComparisonOperator::always_true:      return true;
        case ComparisonOperator::above:            return FirstOperand >  SecondOperand;
        case ComparisonOperator::above_or_equal:   return FirstOperand >= SecondOperand;
        case ComparisonOperator::bellow:           return FirstOperand <  SecondOperand;
        case ComparisonOperator::bellow_or_equal:  return FirstOperand <= SecondOperand;
        case ComparisonOperator::equal:            return FirstOperand == SecondOperand;
        case ComparisonOperator::not_equal:        return FirstOperand != SecondOperand;
        default: assert (0 && "undefined comparison operator"); break;
    }

    assert(0 && "undefined comparison operator");
    return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static StackElem_t MakeArithmeticOperation(StackElem_t FirstOperand, StackElem_t SecondOperand, ArithmeticOperator Operator)
{
    switch (Operator)
    {
        case plus:           return FirstOperand + SecondOperand;
        case minus:          return FirstOperand - SecondOperand;
        case multiplication: return FirstOperand * SecondOperand;
        case division: 
        {
            if (SecondOperand == 0)
            {
                ProcessorErr err = {};
                CodePlaceCtor(&err.place, __FILE__, __LINE__, __func__);
                err.err = ProcessorErrorType::DIVISION_BY_ZERO;
                PROCESSOR_ASSERT(err);
            }
            return FirstOperand / SecondOperand;
        }

        default: assert(0 && "undefined ariphmetic type");
    }

    assert(0 && "wtf");
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(ProcessorErr* err)
{
    assert(err);

    ProcessorErrorType type = err->err;

    switch (type)
    {
        case ProcessorErrorType::NO_ERR:
            break;

        case ProcessorErrorType::SPU_CODE_CALLOC_NULL:
            COLOR_PRINT(RED, "Error: code calloc return null.\n");
            break;

        case ProcessorErrorType::SPU_RAM_CALLOC_NULL:
            COLOR_PRINT(RED, "Error: ram calloc return null.\n");
            break;

        case ProcessorErrorType::INVALID_CMD:
            COLOR_PRINT(RED, "Error: invalid cmd.\n");
            break;

        case ProcessorErrorType::NO_HALT:
            COLOR_PRINT(RED, "Error: no halt end.\n");
            break;

        case ProcessorErrorType::FAILED_OPEN_CODE_FILE:
            COLOR_PRINT(RED, "Error: failed open code file.\n");
            break;

        case ProcessorErrorType::FAILED_READ_FILE_LEN:
            COLOR_PRINT(RED, "Error: failed read code file len (file signature).\n");
            break;

        case ProcessorErrorType::DIVISION_BY_ZERO:
            COLOR_PRINT(RED, "Error: division by zero.\n");
            break;

        case ProcessorErrorType::FREAD_BAD_RETURN:
            COLOR_PRINT(RED, "Error: fread has not all file read.\n");
            break;

        default:
            assert(0 && "undefined error type");
            break;
    }

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void ProcessorDump(const SPU* spu, const char* file, int line, const char* func)
{
    assert(spu);
    assert(file);
    assert(func);

    COLOR_PRINT(GREEN, "PROCESSOR DUMP BEGIN\n\n");

    COLOR_PRINT(VIOLET, "Dump made in:\n");
    PrintPlace(file, line, func);


    COLOR_PRINT(VIOLET, "Registers:\n");

    COLOR_PRINT(VIOLET, "ax = %d\n",   spu->registers[ax]);
    COLOR_PRINT(VIOLET, "bx = %d\n",   spu->registers[bx]);
    COLOR_PRINT(VIOLET, "cx = %d\n",   spu->registers[cx]);
    COLOR_PRINT(VIOLET, "dx = %d\n\n", spu->registers[dx]);

    COLOR_PRINT(BLUE, "RAM:\n");

    for (size_t RAM_i = 0; RAM_i < 128; RAM_i++)
    {
        COLOR_PRINT(CYAN, "[%3lu] %d\n", RAM_i, spu->RAM[RAM_i]);
    }

    printf("\n");

    COLOR_PRINT(YELLOW, "Code size = %lu\n\n", spu->code.size);

    COLOR_PRINT(WHITE, "CODE:\n");
    for (size_t code_i = 0; code_i < spu->code.size; code_i++)
    {
        COLOR_PRINT(BLUE, "[%2lu] %d", code_i, spu->code.code[code_i]);
        if (code_i == spu->ip)
        {
            COLOR_PRINT(WHITE, " < ip");
        }
        printf("\n");
    }
    COLOR_PRINT(RED, "CODE END\n");

    COLOR_PRINT(GREEN, "\nPROCESSOR DUMP END\n");

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void ProcessorAssertPrint(ProcessorErr * err, const char* file, int line, const char* func)
{
    assert(err);
    assert(file);
    assert(func);

    COLOR_PRINT(RED, "Assert made in:\n");
    PrintPlace(file, line, func);
    PrintError(err);
    PrintPlace(err->place.file, err->place.line, err->place.func);
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------