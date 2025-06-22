#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <string.h>
#include "processor/processor.hpp"
#include "stack/stack.hpp"
#include "global/global_include.hpp"
#include "lib/lib.hpp"
#include "functions_for_files/files.hpp"

#ifdef _DEBUG
#include "logger/log.hpp"
#endif //_DEBUG

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define _PROCESSOR_DEBUG

#ifdef _PROCESSOR_DEBUG
    #define ON_PROCESSOR_DEBUG(...) __VA_ARGS__ 
#else
    #define ON_PROCESSOR_DEBUG(...)
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

enum class ProcessorErrorType
{
    NO_ERR                ,
    INVALID_CMD           ,
    SPU_CODE_CALLOC_NULL  ,
    SPU_RAM_CALLOC_NULL   ,
    FAILED_READ_FILE_LEN  ,
    FAILED_OPEN_CODE_FILE ,
    NO_HALT               ,
    DIVISION_BY_ZERO      ,
    FREAD_BAD_RETURN      ,
    RAM_BAD_CALLOC        ,
    RAM_BAD_REALLOC       ,
    RAM_OVERFLOW          ,
    OUT_CHAR_NOT_CHAR     ,
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct ProcessorErr
{
    CodePlace          place;
    ProcessorErrorType err;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct Code
{
    int*   code;
    size_t size;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct RAM
{
    int*   ram;
    size_t size;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct SPU
{
    Code         code;
    size_t       ip;
    Stack_t      stack;
    StackElem_t  registers[REGISTERS_QUANT];
    RAM          ram;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void           CheckFileExtension         (const char* file_name);

static ProcessorErr   Verif                      (SPU* spu, ProcessorErr* err,  const char* file, int line, const char* func);
static void           PrintError                 (          ProcessorErr* err);
static void           ProcessorAssertPrint       (          ProcessorErr* Err, const char* File, int Line, const char* Func);

static ProcessorErr   SpuCtor                    (SPU* spu, const ProcessorInput* input);
static ProcessorErr   SpuDtor                    (SPU* spu);
static ProcessorErr   ExecuteCommands            (SPU* spu);

static ProcessorErr   CodeCtor                   (SPU* spu, const char* file);
static ProcessorErr   ReadCodeFromFile           (SPU* spu, FILE* codeFilePtr);


static ProcessorErr   HandleHalt                 (SPU* spu);
static ProcessorErr   HandlePush                 (SPU* spu);
static ProcessorErr   HandlePop                  (SPU* spu);
static ProcessorErr   HandleAdd                  (SPU* spu);
static ProcessorErr   HandleSub                  (SPU* spu);
static ProcessorErr   HandleMul                  (SPU* spu);
static ProcessorErr   HandleDiv                  (SPU* spu);
static ProcessorErr   HandlePp                   (SPU* spu);
static ProcessorErr   HandleMm                   (SPU* spu);
static ProcessorErr   HandleJmp                  (SPU* spu);
static ProcessorErr   HandleJa                   (SPU* spu);
static ProcessorErr   HandleJae                  (SPU* spu);
static ProcessorErr   HandleJb                   (SPU* spu);
static ProcessorErr   HandleJbe                  (SPU* spu);
static ProcessorErr   HandleJe                   (SPU* spu);
static ProcessorErr   HandleJne                  (SPU* spu);
static ProcessorErr   HandleCall                 (SPU* spu);
static ProcessorErr   HandleRet                  (SPU* spu);
static ProcessorErr   HandleOut                  (SPU* spu);
static ProcessorErr   HandleOutc                 (SPU* spu);
static ProcessorErr   HandleOutr                 (SPU* spu);
static ProcessorErr   HandleOutrc                (SPU* spu);
static ProcessorErr   HandleDraw                 (SPU* spu);
static ProcessorErr   HandleRGBA                 (SPU* spu);


static ProcessorErr   ArithmeticCmdPattern       (SPU* spu, ArithmeticOperator Operator);
static ProcessorErr   PpMmPattern                (SPU* spu, Cmd cmd);
static ProcessorErr   JumpsCmdPatter             (SPU* spu, ComparisonOperator Operator);

static StackElem_t    MakeArithmeticOperation    (StackElem_t FirstOperand, StackElem_t SecondOperand, ArithmeticOperator Operator);
static bool           MakeComparisonOperation    (StackElem_t FirstOperand, StackElem_t SecondOperand, ComparisonOperator Operator);

static PushType       GetPushType                (int PushArg);
static PopType        GetPopType                 (int PopArg);

static int            GetPushPopArg              (SPU* spu);
static int            GetPushPopSum              (SPU* spu);
static int            GetPushPopRegister         (SPU* spu);
static int            GetPushPopMemory           (SPU* spu);
static int            GetPushPopMemoryWithReg    (SPU* spu);
static int            GetPushPopMemorySum        (SPU* spu);


static ProcessorErr   SetPopMemory               (SPU* spu, StackElem_t PopElem);
static ProcessorErr   SetPopMemoryWithRegister   (SPU* spu, StackElem_t PopElem);
static ProcessorErr   SetPopSum                  (SPU* spu, StackElem_t PopElem);
static ProcessorErr   SetPopRegister             (SPU* spu, StackElem_t PopElem);

static size_t         GetCodeSize                (SPU* spu);
static size_t         GetIp                      (SPU* spu);
static int            GetCodeElem                (SPU* spu);
static int            GetNextCodeElem            (SPU* spu);
static void           SetCodeElem                (SPU* spu, size_t Code_i, int NewCodeElem);
static int            GetMemElem                 (SPU* spu, size_t index);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct RGBA
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void           GetRGBA                    (int pixel, RGBA* rgba);
static ProcessorErr   VertexArrayCtor            (sf::VertexArray& pixels, size_t high, size_t width, SPU* spu);
static int            PackRGBA                   (RGBA rgba);
static void           GetRGBAType                (int rgbaInt, bool* isReg1, bool* isReg2, bool* isReg3, bool* isReg4);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ON_PROCESSOR_DEBUG
(
static void ProcessorDump    (const SPU* spu, const char* file, int line, const char* func);
static void WhereProcessorIs (const char* cmd);
)
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PROCESSOR_ASSERT(Err) do                                   \
{                                                                   \
    ProcessorErr ErrCopy = Err;                                      \
    if (ErrCopy.err != ProcessorErrorType::NO_ERR)                    \
    {                                                                  \
        ProcessorAssertPrint(&ErrCopy, __FILE__, __LINE__, __func__);   \
        COLOR_PRINT(CYAN, "abort() in 3, 2, 1...");                      \
        abort();                                                          \
    }                                                                      \
} while (0)                                                                 \

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PROCESSSOR_DUMP(SpuPtr) ProcessorDump(SpuPtr, __FILE__, __LINE__, __func__)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PROCESSOR_VERIF(spu, err) Verif(spu, &err, __FILE__, __LINE__, __func__)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ProcessorInput GetProcessorInput(int argc, const char* argv[])
{
    assert(argv);
    ProcessorInput input = {};
    if (argc <= 1)
        EXIT(EXIT_FAILURE, "No name of executable file.");

    input.executable_file = argv[1];
    
    if (argc == 1)
        input.console_args = nullptr;

    else
        input.console_args = &argv[1];
    
    return input;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void RunProcessor(const ProcessorInput* input)
{
    CheckFileExtension(input->executable_file);

    SPU spu = {};
    PROCESSOR_ASSERT(SpuCtor(&spu, input));
    PROCESSOR_ASSERT(ExecuteCommands(&spu));

    PROCESSOR_ASSERT(SpuDtor(&spu));

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr SpuCtor(SPU* spu, const ProcessorInput* input)
{
    assert(spu);
    assert(input);

    ProcessorErr  err = {};

    PROCESSOR_ASSERT(CodeCtor(spu, input->executable_file));

    spu->ip = 0;

    static const size_t DefaultStackSize = 128;
    STACK_ASSERT(StackCtor(&spu->stack, DefaultStackSize));
    
    for (size_t registers_i = 0; registers_i < Registers::REGISTERS_QUANT; registers_i++)
    {
        spu->registers[registers_i] = 0;
    }

    static const size_t DefaultRamSize = 1<<25;

    spu->ram.ram = (int*) calloc(DefaultRamSize, sizeof(*spu->ram.ram));

    if (!spu->ram.ram)
    {
        err.err = ProcessorErrorType::RAM_BAD_CALLOC;
        return PROCESSOR_VERIF(spu, err);
    }

    spu->ram.size = DefaultRamSize;

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr SpuDtor(SPU* spu)
{
    assert(spu);

    ProcessorErr  err = {};

    FREE(spu->ram.ram);
    FREE(spu->code.code);

    *spu = {};

    return err;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ExecuteCommands(SPU* spu)
{
    assert(spu);

    ProcessorErr err = {};

    while (GetIp(spu) < GetCodeSize(spu))
    {
        switch (GetCodeElem(spu))
        {
            case Cmd::push:  PROCESSOR_ASSERT(HandlePush (spu)); break;
            case Cmd::pop:   PROCESSOR_ASSERT(HandlePop  (spu)); break;
            case Cmd::add:   PROCESSOR_ASSERT(HandleAdd  (spu)); break;
            case Cmd::sub:   PROCESSOR_ASSERT(HandleSub  (spu)); break;
            case Cmd::mul:   PROCESSOR_ASSERT(HandleMul  (spu)); break;
            case Cmd::dive:  PROCESSOR_ASSERT(HandleDiv  (spu)); break;
            case Cmd::pp:    PROCESSOR_ASSERT(HandlePp   (spu)); break;
            case Cmd::mm:    PROCESSOR_ASSERT(HandleMm   (spu)); break;
            case Cmd::call:  PROCESSOR_ASSERT(HandleCall (spu)); break;
            case Cmd::ret:   PROCESSOR_ASSERT(HandleRet  (spu)); break;
            case Cmd::jmp:   PROCESSOR_ASSERT(HandleJmp  (spu)); break;
            case Cmd::ja:    PROCESSOR_ASSERT(HandleJa   (spu)); break;
            case Cmd::jae:   PROCESSOR_ASSERT(HandleJae  (spu)); break;
            case Cmd::jb:    PROCESSOR_ASSERT(HandleJb   (spu)); break;
            case Cmd::jbe:   PROCESSOR_ASSERT(HandleJbe  (spu)); break;
            case Cmd::je:    PROCESSOR_ASSERT(HandleJe   (spu)); break;
            case Cmd::jne:   PROCESSOR_ASSERT(HandleJne  (spu)); break;
            case Cmd::out:   PROCESSOR_ASSERT(HandleOut  (spu)); break;
            case Cmd::outc:  PROCESSOR_ASSERT(HandleOutc (spu)); break;
            case Cmd::outr:  PROCESSOR_ASSERT(HandleOutr (spu)); break;
            case Cmd::outrc: PROCESSOR_ASSERT(HandleOutrc(spu)); break;
            case Cmd::draw:  PROCESSOR_ASSERT(HandleDraw (spu)); break;
            case Cmd::rgba:  PROCESSOR_ASSERT(HandleRGBA (spu)); break;

            case Cmd::hlt: /* PROCESSSOR_DUMP(spu); */ return HandleHalt(spu);
            default:
            {
                // ON_DEBUG(
                // LOG_PRINT(Red, "udef cmd = '%d'\n", GetCodeElem(spu));
                // )
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
    ON_PROCESSOR_DEBUG(WhereProcessorIs("push"));

    assert(spu);

    ProcessorErr err = {};

    int      PushTypeInt = GetNextCodeElem(spu);
    PushType Push        = GetPushType(PushTypeInt);

    StackElem_t PushElem = 0;

    if      (Push.stk == 1 && Push.reg == 0 && Push.mem == 0 && Push.sum == 0) PushElem = GetPushPopArg           (spu);
    else if (Push.stk == 0 && Push.reg == 1 && Push.mem == 0 && Push.sum == 0) PushElem = GetPushPopRegister      (spu);
    else if (Push.stk == 0 && Push.reg == 0 && Push.mem == 1 && Push.sum == 0) PushElem = GetPushPopMemory        (spu);
    else if (Push.stk == 0 && Push.reg == 1 && Push.mem == 1 && Push.sum == 0) PushElem = GetPushPopMemoryWithReg (spu);
    else if (Push.stk == 0 && Push.reg == 0 && Push.mem == 1 && Push.sum == 1) PushElem = GetPushPopMemorySum     (spu);

    else
    {
        err.err = ProcessorErrorType::INVALID_CMD;
        return PROCESSOR_VERIF(spu, err);
    }

    STACK_ASSERT(StackPush(&spu->stack, PushElem));

    spu->ip += CmdInfoArr[push].codeRecordSize;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandlePop(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("pop"));

    assert(spu);
    ProcessorErr err = {};

    int PopTypeInt = GetNextCodeElem(spu);

    PopType Pop = {};
    Pop = GetPopType(PopTypeInt);

    StackElem_t PopElem = 0;
    STACK_ASSERT(StackPop(&spu->stack, &PopElem));

    if      (Pop.reg == 1 && Pop.mem == 0 && Pop.sum == 0) PROCESSOR_ASSERT(SetPopRegister            (spu, PopElem));
    else if (Pop.reg == 0 && Pop.mem == 1 && Pop.sum == 0) PROCESSOR_ASSERT(SetPopMemory              (spu, PopElem));
    else if (Pop.reg == 1 && Pop.mem == 1 && Pop.sum == 0) PROCESSOR_ASSERT(SetPopMemoryWithRegister  (spu, PopElem));
    else if (Pop.reg == 0 && Pop.mem == 1 && Pop.sum == 1) PROCESSOR_ASSERT(SetPopSum                 (spu, PopElem));
    else
    {
        err.err = ProcessorErrorType::INVALID_CMD;
        return PROCESSOR_VERIF(spu, err);
    }

    spu->ip += CmdInfoArr[pop].codeRecordSize;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleAdd(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("add"));

    assert(spu);

    return ArithmeticCmdPattern(spu, ArithmeticOperator::plus);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleSub(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("sub"));

    assert(spu);
    return ArithmeticCmdPattern(spu, ArithmeticOperator::minus);;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleMul(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("mul"));

    assert(spu);
    return ArithmeticCmdPattern(spu, ArithmeticOperator::multiplication);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleDiv(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("div"));

    assert(spu);
    return ArithmeticCmdPattern(spu, ArithmeticOperator::division);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandlePp(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("pp"));
    
    assert(spu);

    return PpMmPattern(spu, Cmd::pp);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleMm(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("mm"));
    
    assert(spu);

    return PpMmPattern(spu, Cmd::mm);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJmp(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("jmp"));

    assert(spu);
    return JumpsCmdPatter(spu, ComparisonOperator::always_true);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJa(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("ja"));

    assert(spu);
    return JumpsCmdPatter(spu, ComparisonOperator::above);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJae(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("jae"));

    assert(spu);
    return JumpsCmdPatter(spu, ComparisonOperator::above_or_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJb(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("jb"));

    assert(spu);
    return JumpsCmdPatter(spu, ComparisonOperator::bellow);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJbe(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("jbe"));

    assert(spu);
    return JumpsCmdPatter(spu, ComparisonOperator::bellow_or_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJe(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("je"));

    assert(spu);
    return JumpsCmdPatter(spu, ComparisonOperator::equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJne(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("jne"));

    assert(spu);
    return JumpsCmdPatter(spu, ComparisonOperator::not_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleCall(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("call"));

    assert(spu);

    ProcessorErr err = {};

    StackElem_t returnAddr  = (StackElem_t) spu->ip;
                returnAddr += (StackElem_t) CmdInfoArr[call].codeRecordSize; // skip 'call func:' in code array.

    STACK_ASSERT(StackPush(&spu->stack, returnAddr));

    spu->ip = (size_t) GetNextCodeElem(spu);

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleRet(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("ret"));

    assert(spu);

    ProcessorErr err = {};

    StackElem_t returnAddr = 0;

    STACK_ASSERT(StackPop(&spu->stack, &returnAddr));
    spu->ip = (size_t) returnAddr;


    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOut(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("out"));

    assert(spu);

    ProcessorErr err = {};

    StackElem_t elem = GetLastStackElem(&spu->stack);

    COLOR_PRINT(VIOLET, "%d", elem);

    spu->ip += CmdInfoArr[out].codeRecordSize;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOutc(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("outrc"));

    assert(spu);

    ProcessorErr err = {};

    StackElem_t elem = GetLastStackElem(&spu->stack);

    if ((elem < CHAR_MIN) || (CHAR_MAX < elem))
    {
        err.err = ProcessorErrorType::OUT_CHAR_NOT_CHAR;
        return PROCESSOR_VERIF(spu, err);
    }

    COLOR_PRINT(VIOLET, "%c", elem);

    spu->ip += CmdInfoArr[out].codeRecordSize;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOutrc(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("outrc"));

    assert(spu);

    ProcessorErr err = {};

    StackElem_t elem = 0;
    STACK_ASSERT(StackPop(&spu->stack, &elem));

    if ((elem < CHAR_MIN) || (CHAR_MAX < elem))
    {
        err.err = ProcessorErrorType::OUT_CHAR_NOT_CHAR;
        return PROCESSOR_VERIF(spu, err);
    }

    COLOR_PRINT(VIOLET, "%c", elem);

    spu->ip += CmdInfoArr[out].codeRecordSize;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOutr(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("outr"));

    assert(spu);

    ProcessorErr err = {};

    StackElem_t elem = 0;

    STACK_ASSERT(StackPop(&spu->stack, &elem));

    COLOR_PRINT(VIOLET, "%d", elem);

    spu->ip += CmdInfoArr[outr].codeRecordSize;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


static void GetRGBA(int pixel, RGBA* rgba)
{
    assert(rgba);

    rgba->r = (unsigned char) (pixel >> 0)  & 0xFF;
    rgba->g = (unsigned char) (pixel >> 8)  & 0xFF;
    rgba->b = (unsigned char) (pixel >> 16) & 0xFF;
    rgba->a = (unsigned char) (pixel >> 24) & 0xFF;

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void GetRGBAType(int rgbaInt, bool* isReg1, bool* isReg2, bool* isReg3, bool* isReg4)
{
    assert(isReg1);
    assert(isReg2);
    assert(isReg3);
    assert(isReg4);

    *isReg1 = (rgbaInt >> 0)  & 0xFF;
    *isReg2 = (rgbaInt >> 8)  & 0xFF; 
    *isReg3 = (rgbaInt >> 16) & 0xFF; 
    *isReg4 = (rgbaInt >> 24) & 0xFF; 

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr VertexArrayCtor(sf::VertexArray& pixels, size_t high, size_t width, SPU* spu)
{
    assert(spu);

    ProcessorErr err = {};

    for (size_t i = 0; i < width; i++)
    {
        for (size_t j = 0; j < high; j++)
        {
            size_t index = j * width + i;
            int memElem = GetMemElem(spu, index);
            RGBA rgba = {};
            GetRGBA(memElem, &rgba);

            pixels[index].position = sf::Vector2f((float) i, (float) j);
            pixels[index].color    = sf::Color(rgba.r, rgba.g, rgba.b, rgba.a);
        }
    }

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleDraw(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("rgba"));
    assert(spu);

    ProcessorErr err = {};    

    int firstArg  = spu->code.code[GetIp(spu) + 1];
    int secondArg = spu->code.code[GetIp(spu) + 2];


    size_t high  = (size_t) spu->registers[firstArg];
    size_t width = (size_t) spu->registers[secondArg];

    size_t vertexCount = high * width;

    sf::VertexArray pixels(sf::PrimitiveType::Points, vertexCount);
    PROCESSOR_ASSERT(VertexArrayCtor(pixels, high, width, spu));

    // ON_DEBUG(
    // LOG_COLOR(Yellow);

    // for (size_t i = 0; i < vertexCount; i++)
    // {
    //     LOG_ADC_PRINT("pixels[%lu].color = '%d %d %d %d'\n", i, (int) pixels[i].color.r, (int) pixels[i].color.g, (int) pixels[i].color.b, (int) pixels[i].color.a);
    // }
    // )

    sf::RenderWindow window(sf::VideoMode((unsigned int) width, (unsigned int) high), "Press 'esc' or 'space' to close.");

    window.draw(pixels);
    window.display(); 

    while (window.isOpen())
    {
        sf::Event event = {};
    
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                    case sf::Keyboard::Space :
                    case sf::Keyboard::Escape: window.close(); break;
                    default:                                   break; 
                }
            }   

            if (event.type == sf::Event::Closed)
                window.close();
        }
    }
    
    pixels.clear();
    window.clear();

    spu->ip += CmdInfoArr[draw].codeRecordSize;

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleRGBA(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("rgba"));
    assert(spu);

    ProcessorErr err = {};

    int type      = spu->code.code[GetIp(spu) + 1]; 
    int firstArg  = spu->code.code[GetIp(spu) + 2];
    int secondArg = spu->code.code[GetIp(spu) + 3];
    int thirdArg  = spu->code.code[GetIp(spu) + 4];
    int fourthArg = spu->code.code[GetIp(spu) + 5];

    bool isReg1 = false;
    bool isReg2 = false;
    bool isReg3 = false;
    bool isReg4 = false;

    GetRGBAType(type, &isReg1, &isReg3, &isReg3, &isReg4);

    if (isReg1) firstArg  = spu->registers[firstArg];
    if (isReg2) secondArg = spu->registers[secondArg];
    if (isReg3) thirdArg  = spu->registers[thirdArg];
    if (isReg4) fourthArg = spu->registers[fourthArg];

    RGBA rgba = {.r = (unsigned char) firstArg,
                 .g = (unsigned char) secondArg, 
                 .b = (unsigned char) thirdArg,
                 .a = (unsigned char) fourthArg};

    int pixel = PackRGBA(rgba);

    static size_t p = 0;
    p++;

    STACK_ASSERT(StackPush(&spu->stack, pixel));



    spu->ip += CmdInfoArr[Cmd::rgba].codeRecordSize;

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleHalt(SPU* spu)
{
    ON_PROCESSOR_DEBUG(WhereProcessorIs("hlt"));

    assert(spu);

    ProcessorErr err = {};

    spu->ip += CmdInfoArr[hlt].codeRecordSize;
    STACK_ASSERT(StackDtor(&spu->stack));
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ArithmeticCmdPattern(SPU* spu, ArithmeticOperator Operator)
{
    assert(spu);
    ProcessorErr err = {};

    StackElem_t FirstOperand  = 0;
    StackElem_t SecondOperand = 0;

    STACK_ASSERT(StackPop(&spu->stack, &SecondOperand));
    STACK_ASSERT(StackPop(&spu->stack, &FirstOperand));

    StackElem_t PushElem = MakeArithmeticOperation(FirstOperand, SecondOperand, Operator);
    STACK_ASSERT(StackPush(&spu->stack, PushElem));
    spu->ip += CmdInfoArr[Operator].codeRecordSize;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr PpMmPattern(SPU* spu, Cmd cmd)
{
    assert(spu);

    ProcessorErr err = {};

    size_t registerPointer = (size_t) spu->code.code[GetIp(spu) +1];

    if (cmd == Cmd::pp)
    {
        spu->registers[registerPointer]++;
        spu->ip += CmdInfoArr[pp].codeRecordSize; 
    }

    else if (cmd == Cmd::mm)
    {
        spu->registers[registerPointer]--;
        spu->ip += CmdInfoArr[mm].codeRecordSize; 
    }

    else
        assert(0 && "undef situation: msut be 'pp' or 'mm' cmd.");

    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr JumpsCmdPatter(SPU* spu, ComparisonOperator Operator)
{
    assert(spu);

    ProcessorErr err = {};

    StackElem_t FirstOperand  = 0;
    StackElem_t SecondOperand = 0;

    if (Operator != always_true)
    {
        STACK_ASSERT(StackPop(&spu->stack, &SecondOperand));
        STACK_ASSERT(StackPop(&spu->stack, &FirstOperand));
    }

    if (MakeComparisonOperation(FirstOperand, SecondOperand, Operator))
    {
        spu->ip = (size_t) GetNextCodeElem(spu);
        return PROCESSOR_VERIF(spu, err);
    }

    spu->ip += CmdInfoArr[Operator].codeRecordSize;
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
    assert(spu->code.code);

    return spu->code.code[spu->ip + 2];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopSum(SPU* spu)
{
    assert(spu);
    assert(spu->code.code);

    return spu->code.code[spu->ip + 3];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopRegister(SPU* spu)
{
    assert(spu);
    assert(spu->registers);

    return spu->registers[GetPushPopArg(spu)];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopMemory(SPU* spu)
{
    assert(spu);
    assert(spu->ram.ram);

    return spu->ram.ram[GetPushPopArg(spu)];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopMemoryWithReg(SPU* spu)
{
    assert(spu);
    assert(spu->ram.ram);

    return spu->ram.ram[GetPushPopRegister(spu)];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushPopMemorySum(SPU* spu)
{
    assert(spu);
    assert(spu->ram.ram);

    return spu->ram.ram[GetPushPopRegister(spu) + GetPushPopSum(spu)];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


static ProcessorErr SetPopRegister(SPU* spu, StackElem_t PopElem)
{
    assert(spu);
    assert(spu->registers);

    ProcessorErr err = {};

    spu->registers[GetPushPopArg(spu)] = PopElem;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr SetPopMemory(SPU* spu, StackElem_t PopElem)
{
    assert(spu);
    assert(spu->ram.ram);

    ProcessorErr err = {};

    size_t pointer = (size_t) GetPushPopArg(spu);

    if (pointer >= spu->ram.size)
    {
        err.err = ProcessorErrorType::RAM_OVERFLOW;
        return PROCESSOR_VERIF(spu, err);
    }

    spu->ram.ram[pointer] = PopElem;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr SetPopMemoryWithRegister(SPU* spu, StackElem_t PopElem)
{
    assert(spu);
    assert(spu->ram.ram);

    ProcessorErr err = {};

    size_t pointer = (size_t) GetPushPopRegister(spu);

    if (pointer >= spu->ram.size)
    {
        err.err = ProcessorErrorType::RAM_OVERFLOW;
        return PROCESSOR_VERIF(spu, err);
    }

    spu->ram.ram[pointer] = PopElem;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr SetPopSum(SPU* spu, StackElem_t PopElem)
{
    assert(spu);
    assert(spu->ram.ram);

    ProcessorErr err = {};

    size_t pointer = (size_t) (GetPushPopRegister(spu) + GetPushPopSum(spu));

    if (pointer >= spu->ram.size)
    {
        err.err = ProcessorErrorType::RAM_OVERFLOW;
        return PROCESSOR_VERIF(spu, err);
    }

    spu->ram.ram[pointer] = PopElem;
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetCodeElem(SPU* spu)
{
    assert(spu);
    assert(spu->code.code);

    return spu->code.code[spu->ip];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetNextCodeElem(SPU* spu)
{
    assert(spu);
    assert(spu->code.code);

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

static int GetMemElem(SPU* spu, size_t index)
{
    assert(index < spu->ram.size);
    return spu->ram.ram[index];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetCodeElem(SPU* spu, size_t Code_i, int NewCodeElem)
{
    assert(spu);
    assert(spu->code.code);

    spu->code.code[Code_i] = NewCodeElem;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int PackRGBA(RGBA rgba)
{
    return (rgba.a << 24) | (rgba.b << 16) | (rgba.g << 8) | (rgba.r << 0);
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

    ON_DEBUG(
    LOG_ALL_INT_ARRAY(Yellow, spu->code.code, spu->code.size);
    )
    return PROCESSOR_VERIF(spu, err);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr CodeCtor(SPU* spu, const char* file)
{
    assert(spu);
    assert(file);

    ProcessorErr err = {};

    FILE* CodeFilePtr = SafeFopen(file, "rb");

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
    spu->code.code = (int*) calloc(codeSize, sizeof(*spu->code.code));

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

static void CheckFileExtension(const char* file_name)
{
    if (!file_name)
        EXIT(EXIT_FAILURE, "bin file name is nullptr.");

    const char* extension = GetFileExtension(file_name);

    if (strcmp(extension, bin_extension) != 0)
        EXIT(EXIT_FAILURE, "bad bin extension: '%s'.\nmust be '.%s'.", file_name, bin_extension);

    return;
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
        
        case ProcessorErrorType::RAM_BAD_CALLOC:
            COLOR_PRINT(RED, "Error: failed to allocate memory for ram.\n");
            break;
        
        case ProcessorErrorType::RAM_BAD_REALLOC:
            COLOR_PRINT(RED, "Error: failed to reallocate memory for ram.\n");
            break;

        case ProcessorErrorType::RAM_OVERFLOW:
            COLOR_PRINT(RED, "Error: attempt to access a non-existent address in ram.\n");
            break;

        case ProcessorErrorType::OUT_CHAR_NOT_CHAR:
            COLOR_PRINT(RED, "Error: attemp to print not-a-char element like a char.\n");
            break;

        default:
            assert(0 && "undefined error type");
            break;
    }

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ON_PROCESSOR_DEBUG
(
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
    COLOR_PRINT(VIOLET, "ex = %d\n\n", spu->registers[ex]);
    COLOR_PRINT(VIOLET, "fx = %d\n\n", spu->registers[fx]);


    COLOR_PRINT(BLUE, "ram.ram:\n");

    for (size_t RAM_i = 0; RAM_i < 128; RAM_i++)
    {
        COLOR_PRINT(CYAN, "[%3lu] %d\n", RAM_i, spu->ram.ram[RAM_i]);
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

static void WhereProcessorIs(const char* cmd)
{
    assert(cmd);
    ON_DEBUG(
    LOG_PRINT(Green, "processor::%s", cmd);
    )
    return;
}

)
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void ProcessorAssertPrint(ProcessorErr* err, const char* file, int line, const char* func)
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
