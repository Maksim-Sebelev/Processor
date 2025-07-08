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
#include "processor/math_operators/operators.hpp"

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

#define WHERE_PROCESSOR_IS() ON_PROCESSOR_DEBUG(WhereProcessorIs(__func__))

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

struct CodeArray
{
    int*   array;
    size_t size ;
    size_t ip   ; // instruction pointer
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct RAM
{
    int*   array;
    size_t size ;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct SPU
{
    CodeArray code_array                ;
    Stack_t   stack                     ;
    int       registers[REGISTERS_QUANT];
    RAM       ram                       ;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

using one_bit_t = unsigned char;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct PushType
{
    one_bit_t number   : 1;
    one_bit_t reg      : 1;
    one_bit_t memory   : 1;
    one_bit_t aligment : 1;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct PopType
{
    one_bit_t reg      : 1;
    one_bit_t memory   : 1;
    one_bit_t aligment : 1;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct RGBA
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// check input files
static void         CheckFileExtension                 (const char* file_name);

// main functions in RunProcessor        
static ProcessorErr SpuCtor                            (SPU* spu, const ProcessorInput* input);
static ProcessorErr ExecuteCommands                    (SPU* spu);
static ProcessorErr SpuDtor                            (SPU* spu);

// ctor spu helper funtions        
static ProcessorErr CodeCtor                           (SPU* spu, const char* file);
static ProcessorErr RamCtor                            (SPU* spu);
static ProcessorErr ProcessorStackCtor                 (SPU* spu);
static ProcessorErr RegistersCtor                      (SPU* spu);
static void         OutputColorCtor                    ();

// dtor spu helper functions        
static ProcessorErr CodeDtor                           (SPU* spu);
static ProcessorErr RamDtor                            (SPU* spu);
static ProcessorErr ProcessorStackDtor                 (SPU* spu);
static ProcessorErr RegistersDtor                      (SPU* spu);
static void         OutputColorDtor                    ();

// processing functions commands        
static ProcessorErr HandlePush                         (SPU* spu);
static ProcessorErr HandlePop                          (SPU* spu);
static ProcessorErr HandleAdd                          (SPU* spu); // arimthetic commands
static ProcessorErr HandleSub                          (SPU* spu);
static ProcessorErr HandleMul                          (SPU* spu);
static ProcessorErr HandleDiv                          (SPU* spu); // ===============
static ProcessorErr HandlePp                           (SPU* spu);
static ProcessorErr HandleMm                           (SPU* spu);
static ProcessorErr HandleJmp                          (SPU* spu); // jumps
static ProcessorErr HandleJa                           (SPU* spu);
static ProcessorErr HandleJae                          (SPU* spu);
static ProcessorErr HandleJb                           (SPU* spu);
static ProcessorErr HandleJbe                          (SPU* spu);
static ProcessorErr HandleJe                           (SPU* spu);
static ProcessorErr HandleJne                          (SPU* spu); // =================
static ProcessorErr HandleCall                         (SPU* spu);
static ProcessorErr HandleRet                          (SPU* spu);
static ProcessorErr HandleOut                          (SPU* spu); // console out commands
static ProcessorErr HandleOutc                         (SPU* spu);
static ProcessorErr HandleOutr                         (SPU* spu);
static ProcessorErr HandleOutrc                        (SPU* spu); // =================
static ProcessorErr HandleDraw                         (SPU* spu); // functions for work with video memory
static ProcessorErr HandleRGBA                         (SPU* spu); // =================

// pattern for similar functions        
static ProcessorErr PpMmPattern                        (SPU* spu, Cmd command);
static ProcessorErr ArithmeticCmdPattern               (SPU* spu, ArithmeticOperator arithmetic_operator);
static ProcessorErr JumpsCmdPatter                     (SPU* spu, ComparisonOperator comparison_operator);

// HandlePush helper functioins        
static PushType     GetPushType                        (int push_type_int);

static bool         IsPushTypeNumber                   (PushType push_type);
static bool         IsPushTypeRegister                 (PushType push_type);
static bool         IsPushTypeMemory                   (PushType push_type);
static bool         IsPushTypeMemoryRegister           (PushType push_type);
static bool         IsPushTypeAligment                 (PushType push_type);

static int          GetPushElementTypeRegister         (const SPU* spu, int push_arg);
static int          GetPushElementTypeMemory           (const SPU* spu, int push_arg);
static int          GetPushElementTypeMemoryRegister   (const SPU* spu, int push_arg);
static int          GetPushElementTypeAligment         (const SPU* spu, int push_arg, int aligment);

// HandlePop  helper functions
static PopType      GetPopType                         (int PopArg);

static bool         IsPopTypeRegister                  (PopType pop_type);
static bool         IsPopTypeMemory                    (PopType pop_type);
static bool         IsPopTypeMemoryRegister            (PopType pop_type);
static bool         IsPopTypeAligment                  (PopType pop_type);

static void         SetRegisterInPopTypeRegister       (SPU* spu, int pop_element, int pop_argument);
static void         SetRamInPopTypeMemory              (SPU* spu, int pop_element, int pop_argument);
static void         SetRamInPopTypeMemoryRegister      (SPU* spu, int pop_element, int pop_argument);
static void         SetRamInPopTypeAligment            (SPU* spu, int pop_element, int pop_argument, int aligment);

// rgba functions
static int          PackRGBA                           (RGBA rgba);
static RGBA         UnpackRGBA                         (int pixel);
static ProcessorErr VertexArrayCtor                    (sf::VertexArray& pixels, size_t ram_addr_begin, size_t high, size_t width, SPU* spu);

// CodeCtor helpeer function
static ProcessorErr ReadCodeFromFile                   (SPU* spu, FILE* code_file_ptr);
static void         SetCodeElem                        (SPU* spu, size_t code_pointer, int setting_element);


// functions for work with struct SPU
static size_t       GetIp                              (SPU* spu);
static size_t       GetCodeSize                        (SPU* spu);
static int          GetNextCodeInstruction             (SPU* spu);
static int          GetMemoryElement                   (SPU* spu, size_t index);
static int          GetRegisterOrInt                   (const SPU* spu, bool is_arg_register, int argument);
static one_bit_t    GetIBitOfInt                       (int n, size_t i);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// error processing functions
static ProcessorErr ErrCtor                            (ProcessorErrorType err_type, const char* file, int line, const char* func);
static void         PrintError                         (          ProcessorErr* err                                              );
static void         ProcessorAssertPrint               (          ProcessorErr* err, const char* file, int line, const char* func);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ON_PROCESSOR_DEBUG
(
static void ProcessorDump    (const SPU* spu, const char* file, int line, const char* func);
static void WhereProcessorIs (const char* command);
)
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PROCESSOR_ASSERT(error) do                                 \
{                                                                   \
    ProcessorErr err_copy = error;                                   \
    if (err_copy.err != ProcessorErrorType::NO_ERR)                   \
    {                                                                  \
        ProcessorAssertPrint(&err_copy, __FILE__, __LINE__, __func__);  \
        exit(EXIT_FAILURE);                                              \
    }                                                                     \
} while (0)                                                                \

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define ERR_CTOR(err_type) ErrCtor(err_type, __FILE__, __LINE__, __func__)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PROCESSOR_RETURN(err_type) ERR_CTOR(err_type)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define PROCESSSOR_DUMP(SpuPtr) ProcessorDump(SpuPtr, __FILE__, __LINE__, __func__)

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void RunProcessor(const ProcessorInput* input)
{
    assert(input);

    CheckFileExtension(input->executable_file);

    SPU spu = {};

    PROCESSOR_ASSERT(SpuCtor        (&spu, input));
    PROCESSOR_ASSERT(ExecuteCommands(&spu       ));
    PROCESSOR_ASSERT(SpuDtor        (&spu       ));


    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// main functions in RunProcessor

static ProcessorErr SpuCtor(SPU* spu, const ProcessorInput* input)
{
    assert(spu);
    assert(input);

    PROCESSOR_ASSERT(CodeCtor               (spu, input->executable_file));
    PROCESSOR_ASSERT(ProcessorStackCtor     (spu                        ));
    PROCESSOR_ASSERT(RegistersCtor          (spu                        ));
    PROCESSOR_ASSERT(RamCtor                (spu                        ));
    OutputColorCtor();

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ExecuteCommands(SPU* spu)
{
    assert(spu);

    size_t code_size = GetCodeSize(spu);

    while (GetIp(spu) < code_size)
    {
        int command = GetNextCodeInstruction(spu);
        switch (command)
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
            case Cmd::hlt:   return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR     );
            default:         return PROCESSOR_RETURN(ProcessorErrorType::INVALID_CMD);
        }
    }

    return PROCESSOR_RETURN(ProcessorErrorType::NO_HALT);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr SpuDtor(SPU* spu)
{
    assert(spu);

    OutputColorDtor();
    PROCESSOR_ASSERT(CodeDtor           (spu));
    PROCESSOR_ASSERT(RamDtor            (spu));
    PROCESSOR_ASSERT(RegistersDtor      (spu));
    PROCESSOR_ASSERT(ProcessorStackDtor (spu));

    *spu = {};

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// spu ctor helper functions

static ProcessorErr CodeCtor(SPU* spu, const char* file)
{
    assert(spu);
    assert(file);

    FILE* code_file_ptr = SafeFopen(file, "rb"); assert(code_file_ptr);

    size_t code_array_size = 0;

    if (fscanf(code_file_ptr, "%lu", &code_array_size) != 1)
        return PROCESSOR_RETURN(ProcessorErrorType::FAILED_READ_FILE_LEN);

    spu->code_array.size = code_array_size;
    spu->code_array.array = (int*) calloc(code_array_size, sizeof(*spu->code_array.array));

    if (!spu->code_array.array)
        return PROCESSOR_RETURN(ProcessorErrorType::SPU_CODE_CALLOC_NULL);

    PROCESSOR_ASSERT(ReadCodeFromFile(spu, code_file_ptr));

    SafeFclose(code_file_ptr);

    spu->code_array.ip = 0;

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ProcessorStackCtor(SPU* spu)
{
    assert(spu);

    static const size_t DefaultStackSize = 128;
    STACK_ASSERT(StackCtor(&spu->stack, DefaultStackSize));

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr RamCtor(SPU* spu)
{
    assert(spu);

    static const size_t default_ram_size = 1 << 30;
    
    int* ram = (int*) calloc(default_ram_size, sizeof(ram[0]));

    if (!ram)
        return PROCESSOR_RETURN(ProcessorErrorType::RAM_BAD_CALLOC);

   spu->ram.array = ram             ;
   spu->ram.size  = default_ram_size;
   
   return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr RegistersCtor(SPU* spu)
{
    assert(spu);

    for (size_t i = 0; i < (size_t) Registers::REGISTERS_QUANT; i++)
        spu->registers[i] = 0;

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void OutputColorCtor()
{
    printf(VIOLET);
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// spu dtor helper functions

static ProcessorErr CodeDtor(SPU* spu)
{
    assert(spu);

    assert(spu->code_array.array);
    FREE(spu->code_array.array);

    spu->code_array = {};

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ProcessorStackDtor(SPU* spu)
{
    assert(spu);

    STACK_ASSERT(StackDtor(&spu->stack));

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr RamDtor(SPU* spu)
{
    assert(spu);

    assert(spu->ram.array);
    FREE(spu->ram.array);

    spu->ram.size  = 0;
   
   return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr RegistersDtor(SPU* spu)
{
    assert(spu);

    for (size_t i = 0; i < (size_t) Registers::REGISTERS_QUANT; i++)
        spu->registers[i] = 0;

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void OutputColorDtor()
{
    printf(RESET);

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// processing commands functions

static ProcessorErr HandlePush(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    int      push_type_int = GetNextCodeInstruction(spu);
    PushType push_type     = GetPushType           (push_type_int);

    stack_element_t pushing_element = 0;

    int push_arg = GetNextCodeInstruction(spu);
    int aligment = GetNextCodeInstruction(spu);

    if (IsPushTypeNumber(push_type))
        pushing_element = push_arg;

    else if (IsPushTypeRegister(push_type))
        pushing_element = GetPushElementTypeRegister(spu, push_arg);

    else if (IsPushTypeMemory(push_type))
        pushing_element = GetPushElementTypeMemory(spu, push_arg);

    else if (IsPushTypeMemoryRegister(push_type))
        pushing_element = GetPushElementTypeMemoryRegister(spu, push_arg);

    else if (IsPushTypeAligment(push_type))
        pushing_element = GetPushElementTypeAligment(spu, push_arg, aligment);

    else
        return PROCESSOR_RETURN(ProcessorErrorType::INVALID_CMD);

    STACK_ASSERT(StackPush(&spu->stack, pushing_element));

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandlePop(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    int     pop_type_int = GetNextCodeInstruction(spu);
    PopType pop_type     = GetPopType        (pop_type_int);

    int     pop_arg      = GetNextCodeInstruction(spu);
    int     aligment     = GetNextCodeInstruction(spu);

    stack_element_t pop_element = 0;
    STACK_ASSERT(StackPop(&spu->stack, &pop_element));

    if (IsPopTypeRegister(pop_type))
        SetRegisterInPopTypeRegister(spu, pop_element, pop_arg);
    
    else if (IsPopTypeMemory(pop_type))
        SetRamInPopTypeMemory(spu, pop_element, pop_arg);
    
    else if (IsPopTypeMemoryRegister(pop_type))
        SetRamInPopTypeMemoryRegister(spu, pop_element, pop_arg);    

    else if (IsPopTypeAligment(pop_type))
        SetRamInPopTypeAligment(spu, pop_element, pop_arg, aligment);

    else
        return PROCESSOR_RETURN(ProcessorErrorType::INVALID_CMD);

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// arithmetic commands

static ProcessorErr HandleAdd(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return ArithmeticCmdPattern(spu, ArithmeticOperator::plus);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleSub(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return ArithmeticCmdPattern(spu, ArithmeticOperator::minus);;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleMul(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return ArithmeticCmdPattern(spu, ArithmeticOperator::multiplication);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleDiv(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return ArithmeticCmdPattern(spu, ArithmeticOperator::division);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandlePp(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return PpMmPattern(spu, Cmd::pp);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleMm(SPU* spu)
{
    WHERE_PROCESSOR_IS();
    
    assert(spu);

    return PpMmPattern(spu, Cmd::mm);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// jump commands

static ProcessorErr HandleJmp(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return JumpsCmdPatter(spu, ComparisonOperator::always_true);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJa(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return JumpsCmdPatter(spu, ComparisonOperator::above);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJae(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return JumpsCmdPatter(spu, ComparisonOperator::above_or_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJb(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return JumpsCmdPatter(spu, ComparisonOperator::bellow);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJbe(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return JumpsCmdPatter(spu, ComparisonOperator::bellow_or_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJe(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return JumpsCmdPatter(spu, ComparisonOperator::equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleJne(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    return JumpsCmdPatter(spu, ComparisonOperator::not_equal);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// commands for work with asm-functions

static ProcessorErr HandleCall(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    stack_element_t return_pointer  = (stack_element_t) GetIp(spu) + 1; // skip 'call func:' is made in assembler

    STACK_ASSERT(StackPush(&spu->stack, return_pointer));

    spu->code_array.ip = (size_t) GetNextCodeInstruction(spu);

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleRet(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    stack_element_t return_pointer = 0;
    STACK_ASSERT(StackPop(&spu->stack, &return_pointer));

    spu->code_array.ip = (size_t) return_pointer;


    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// functions for console out

static ProcessorErr HandleOut(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    stack_element_t elem = GetLastStackElem(&spu->stack);

    printf("%d", elem);

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOutc(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    stack_element_t elem = GetLastStackElem(&spu->stack);

    if ((elem < CHAR_MIN) || (CHAR_MAX < elem))
        return PROCESSOR_RETURN(ProcessorErrorType::OUT_CHAR_NOT_CHAR);

    printf("%c", elem);

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOutrc(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    stack_element_t elem = 0;
    STACK_ASSERT(StackPop(&spu->stack, &elem));

    if ((elem < CHAR_MIN) || (CHAR_MAX < elem))
        return PROCESSOR_RETURN(ProcessorErrorType::OUT_CHAR_NOT_CHAR);

    printf("%c", elem);

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleOutr(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    stack_element_t elem = 0;

    STACK_ASSERT(StackPop(&spu->stack, &elem));

    printf("%d", elem);

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// commands for work with video memory

static ProcessorErr HandleDraw(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    const int draw_type_int = GetNextCodeInstruction(spu);

    const int argument[3] =
    {
        GetNextCodeInstruction(spu),
        GetNextCodeInstruction(spu),
        GetNextCodeInstruction(spu),
    };

    const bool is_arg_register[3] =
    {
        (bool) GetIBitOfInt(draw_type_int, 0),
        (bool) GetIBitOfInt(draw_type_int, 1),
        (bool) GetIBitOfInt(draw_type_int, 2),
    };
    

    const size_t vertex_array_ram_begin_addr = (size_t) GetRegisterOrInt(spu, is_arg_register[0], argument[0]);
    const size_t high                        = (size_t) GetRegisterOrInt(spu, is_arg_register[1], argument[1]);
    const size_t width                       = (size_t) GetRegisterOrInt(spu, is_arg_register[2], argument[2]);


    const size_t vertext_quant = high * width;

    sf::VertexArray pixels(sf::PrimitiveType::Points, vertext_quant);
    PROCESSOR_ASSERT(VertexArrayCtor(pixels, vertex_array_ram_begin_addr, high, width, spu));

    sf::RenderWindow window(sf::VideoMode((unsigned int) width, (unsigned int) high), "Press 'esc' or 'space' to close.");

    window.draw(pixels);
    window.display(); 

    while (window.isOpen())
    {
        sf::Event event = {};
    
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::KeyPressed:
                {
                    switch (event.key.code)
                    {
                        case sf::Keyboard::Space :
                        case sf::Keyboard::Escape: window.close(); break;
                        default:                                   break; 
                    }
                    break;
                }

                case sf::Event::Closed: window.close(); break;
    
                default: break;
            }
        }
    }
    
    pixels.clear();
    window.clear();

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr HandleRGBA(SPU* spu)
{
    WHERE_PROCESSOR_IS();

    assert(spu);

    const int rgba_type_int = GetNextCodeInstruction(spu);    

    const int argument[4] =
    {
        GetNextCodeInstruction(spu),
        GetNextCodeInstruction(spu),
        GetNextCodeInstruction(spu),
        GetNextCodeInstruction(spu),
    };

    const bool is_argmunet_register[4] =
    {
        (bool) GetIBitOfInt(rgba_type_int, 0),
        (bool) GetIBitOfInt(rgba_type_int, 1),
        (bool) GetIBitOfInt(rgba_type_int, 2),
        (bool) GetIBitOfInt(rgba_type_int, 3),
    };


    const unsigned char format_argument[4] =
    {
        (unsigned char) GetRegisterOrInt(spu, is_argmunet_register[0], argument[0]),
        (unsigned char) GetRegisterOrInt(spu, is_argmunet_register[1], argument[1]),
        (unsigned char) GetRegisterOrInt(spu, is_argmunet_register[2], argument[2]),
        (unsigned char) GetRegisterOrInt(spu, is_argmunet_register[3], argument[3]),
    };

    RGBA rgba = 
    {
        .r = format_argument[0],
        .g = format_argument[1],
        .b = format_argument[2],
        .a = format_argument[3],
    };

    int rgba_int_result = PackRGBA(rgba);

    STACK_ASSERT(StackPush(&spu->stack, rgba_int_result));

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// pattern for similar functions

static ProcessorErr PpMmPattern(SPU* spu, Cmd command)
{
    assert(spu);

    int argument = GetNextCodeInstruction(spu);

    switch (command)
    {
        case Cmd::pp: spu->registers[argument]++; break;
        case Cmd::mm: spu->registers[argument]--; break;
        default: __builtin_unreachable__("here must be 'pp' or 'mm' commands"); break;
    }

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr ArithmeticCmdPattern(SPU* spu, ArithmeticOperator arithmetic_operator)
{
    assert(spu);

    stack_element_t first_operand  = 0;
    stack_element_t second_operand = 0;

    STACK_ASSERT(StackPop(&spu->stack, &second_operand));
    STACK_ASSERT(StackPop(&spu->stack, &first_operand));

    stack_element_t push_elem = MakeArithmeticOperation(first_operand, second_operand, arithmetic_operator);
    STACK_ASSERT(StackPush(&spu->stack, push_elem));

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr JumpsCmdPatter(SPU* spu, ComparisonOperator comparison_operator)
{
    assert(spu);

    stack_element_t first_operand  = 0;
    stack_element_t second_operand = 0;

    size_t jmp_place = (size_t) GetNextCodeInstruction(spu);

    if (comparison_operator != always_true)
    {
        STACK_ASSERT(StackPop(&spu->stack, &second_operand));
        STACK_ASSERT(StackPop(&spu->stack, &first_operand));
    }

    if (MakeComparisonOperation(first_operand, second_operand, comparison_operator))
        spu->code_array.ip = jmp_place;

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// push and pop helper macroses
#define register_assert(spu, arg)       \
assert(arg >= 0);                        \
assert(arg < Registers::REGISTERS_QUANT); \
assert(spu->registers)                     \

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define ram_assert(spu, arg)                                    \
assert(spu->ram.array);                                          \
assert(arg >= 0);                                                 \
if ((size_t) arg >= spu->ram.size)                                 \
{                                                                   \
    ProcessorErr error = ERR_CTOR(ProcessorErrorType::RAM_OVERFLOW); \
    PROCESSOR_ASSERT(error);                                          \
}                                                                      \

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// HandlePush helper functions

static PushType GetPushType(int push_type_int)
{
    one_bit_t   number_flag = GetIBitOfInt(push_type_int, 3);
    one_bit_t register_flag = GetIBitOfInt(push_type_int, 2);
    one_bit_t   memory_flag = GetIBitOfInt(push_type_int, 1);
    one_bit_t aligment_flag = GetIBitOfInt(push_type_int, 0);

    PushType type = 
    {
        .number   = (one_bit_t) (number_flag   ? 1 : 0),
        .reg      = (one_bit_t) (register_flag ? 1 : 0),
        .memory   = (one_bit_t) (memory_flag   ? 1 : 0),
        .aligment = (one_bit_t) (aligment_flag ? 1 : 0),
    };

    return type;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPushTypeNumber(PushType push_type)
{
    return (push_type.number   == 1) &&
           (push_type.reg      == 0) &&
           (push_type.memory   == 0) &&
           (push_type.aligment == 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPushTypeRegister(PushType push_type)
{
    return (push_type.number   == 0) &&
           (push_type.reg      == 1) &&
           (push_type.memory   == 0) &&
           (push_type.aligment == 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPushTypeMemory(PushType push_type)
{
    return  (push_type.number   == 0) &&
            (push_type.reg      == 0) &&
            (push_type.memory   == 1) &&
            (push_type.aligment == 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPushTypeMemoryRegister(PushType push_type)
{
    return (push_type.number   == 0) &&
           (push_type.reg      == 1) &&
           (push_type.memory   == 1) &&
           (push_type.aligment == 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPushTypeAligment(PushType push_type)
{
    return (push_type.number   == 0) &&
           (push_type.reg      == 0) &&
           (push_type.memory   == 1) &&
           (push_type.aligment == 1);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushElementTypeRegister(const SPU* spu, int push_arg)
{
    assert(spu);
    register_assert(spu, push_arg);

    return spu->registers[push_arg];
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushElementTypeMemory(const SPU* spu, int push_arg)
{
    assert(spu);
    ram_assert(spu, push_arg);

    return spu->ram.array[push_arg];
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushElementTypeMemoryRegister(const SPU* spu, int push_arg)
{
    assert(spu);
    register_assert(spu, push_arg);
    
    int register_value = spu->registers[push_arg];

    ram_assert(spu, register_value);

    return spu->ram.array[register_value];
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetPushElementTypeAligment(const SPU* spu, int push_arg, int aligment)
{
    assert(spu);
    register_assert(spu, push_arg);

    int register_value = spu->registers[push_arg];

    int ram_addr = register_value + aligment;

    ram_assert(spu, ram_addr);

    return spu->ram.array[ram_addr];
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// HandlePop helper functions

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static PopType GetPopType(int pop_type_int)
{
    one_bit_t register_flag = GetIBitOfInt(pop_type_int, 2);
    one_bit_t memory_flag   = GetIBitOfInt(pop_type_int, 1);
    one_bit_t aligment_flag = GetIBitOfInt(pop_type_int, 0);

    PopType type =
    {
        .reg      = (one_bit_t) (register_flag ? 1 : 0),
        .memory   = (one_bit_t) (  memory_flag ? 1 : 0),
        .aligment = (one_bit_t) (aligment_flag ? 1 : 0),
    };

    return type;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPopTypeRegister(PopType pop_type)
{
    return (pop_type.reg      == 1) &&
           (pop_type.memory   == 0) &&
           (pop_type.aligment == 0);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------static bool IsPopTypeRegisterMemo

static bool IsPopTypeMemory(PopType pop_type)
{
    return (pop_type.reg      == 0) &&
           (pop_type.memory   == 1) &&
           (pop_type.aligment == 0);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------static bool IsPopTypeRegisterMemo

static bool IsPopTypeMemoryRegister(PopType pop_type)
{
    return (pop_type.reg      == 1) &&
           (pop_type.memory   == 1) &&
           (pop_type.aligment == 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static bool IsPopTypeAligment(PopType pop_type)
{
    return (pop_type.reg      == 0) &&
           (pop_type.memory   == 0) &&
           (pop_type.aligment == 1);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetRegisterInPopTypeRegister(SPU* spu, int pop_element, int pop_argument)
{
    assert(spu);
    register_assert(spu, pop_argument);

    spu->registers[pop_argument] = pop_element;

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetRamInPopTypeMemory(SPU* spu, int pop_element, int pop_argument)
{
    assert(spu);
    ram_assert(spu, pop_argument);

    spu->ram.array[pop_argument] = pop_element;

    return;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetRamInPopTypeMemoryRegister(SPU* spu, int pop_element, int pop_argument)
{
    assert(spu);
    register_assert(spu, pop_argument);

    int register_value = spu->registers[pop_argument];

    ram_assert(spu, register_value);

    spu->ram.array[register_value] = pop_element;

    return;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetRamInPopTypeAligment(SPU* spu, int pop_element, int pop_argument, int aligment)
{
    assert(spu);
    register_assert(spu, pop_argument);

    int register_value = spu->registers[pop_argument];

    int ram_addr = register_value + aligment;
    
    ram_assert(spu, ram_addr);

    spu->ram.array[ram_addr] = pop_element;

    return;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef register_assert
#undef ram_assert

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int PackRGBA(RGBA rgba)
{
    return (rgba.a << 24) | (rgba.b << 16) | (rgba.g << 8) | (rgba.r << 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static RGBA UnpackRGBA(int pixel)
{
    RGBA rgba =
    {
        .r = (unsigned char) ((pixel >> 0 ) & 0xFF),
        .g = (unsigned char) ((pixel >> 8 ) & 0xFF),
        .b = (unsigned char) ((pixel >> 16) & 0xFF),
        .a = (unsigned char) ((pixel >> 24) & 0xFF),
    };

    return rgba;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static ProcessorErr VertexArrayCtor(sf::VertexArray& pixels, size_t ram_addr_begin, size_t high, size_t width, SPU* spu)
{
    assert(spu);

    size_t   ram_pointer = ram_addr_begin;
    size_t pixel_poiter  = 0;

    for (size_t j = 0; j < high; j++)
    {
        for (size_t i = 0; i < width; i++)
        {
            int    memory_element = GetMemoryElement(spu, ram_pointer);
            RGBA   rgba           = UnpackRGBA      (memory_element  );

            pixels[pixel_poiter].position = sf::Vector2f((float) i, (float) j);
            pixels[pixel_poiter].color    = sf::Color(rgba.r, rgba.g, rgba.b, rgba.a);
    
            ram_pointer++;
            pixel_poiter++;
        }
    }

    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CodeCtor helper function

static ProcessorErr ReadCodeFromFile(SPU* spu, FILE* code_file_ptr)
{
    assert(spu);
    assert(code_file_ptr);

    size_t CmdQuant = spu->code_array.size;

    for (size_t cmd_i = 0; cmd_i < CmdQuant; cmd_i++)
    {
        int command = 0;
        int fscanf_return = fscanf(code_file_ptr, "%d", &command);
    
        if (fscanf_return != 1)
            return PROCESSOR_RETURN(ProcessorErrorType::INVALID_CMD);

        SetCodeElem(spu, cmd_i, command);
    }

    ON_DEBUG(
    LOG_ALL_INT_ARRAY(Yellow, spu->code_array.array, spu->code_array.size);
    )
    return PROCESSOR_RETURN(ProcessorErrorType::NO_ERR);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void SetCodeElem(SPU* spu, size_t code_pointer, int setting_element)
{
    assert(spu);
    assert(spu->code_array.array);

    spu->code_array.array[code_pointer] = setting_element;
    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// functions for work with struct SPU

static int GetNextCodeInstruction(SPU* spu)
{
    assert(spu);
    assert(spu->code_array.array);

    size_t old_ip = spu->code_array.ip;
    spu->code_array.ip++;

    return spu->code_array.array[old_ip];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static size_t GetCodeSize(SPU* spu)
{
    assert(spu);

    return spu->code_array.size;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static size_t GetIp(SPU* spu)
{
    assert(spu);

    return spu->code_array.ip;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetMemoryElement(SPU* spu, size_t index)
{
    assert(spu);

    if (index >= spu->ram.size)
    {
        ProcessorErr err = ERR_CTOR(ProcessorErrorType::RAM_OVERFLOW);
        PROCESSOR_ASSERT(err);
    }

    return spu->ram.array[index];
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static one_bit_t GetIBitOfInt(int n, size_t i)
{
    return (n >> i) & 0x1;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int GetRegisterOrInt(const SPU* spu, bool is_arg_register, int argument)
{
    assert(spu);
    assert(spu->registers);

    return (is_arg_register) ? spu->registers[argument] : argument;
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

static ProcessorErr ErrCtor(ProcessorErrorType err_type, const char* file, int line, const char* func)
{
    assert(file);
    assert(func);

    ProcessorErr err = {};

    err.err = err_type;

    if (err_type != ProcessorErrorType::NO_ERR)
        CodePlaceCtor(&err.place, file, line, func);

    return err;
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
            COLOR_PRINT(RED, "Error: code_array calloc return null.\n");
            break;

        case ProcessorErrorType::SPU_RAM_CALLOC_NULL:
            COLOR_PRINT(RED, "Error: ram calloc return null.\n");
            break;

        case ProcessorErrorType::INVALID_CMD:
            COLOR_PRINT(RED, "Error: invalid command.\n");
            break;

        case ProcessorErrorType::NO_HALT:
            COLOR_PRINT(RED, "Error: no halt end.\n");
            break;

        case ProcessorErrorType::FAILED_OPEN_CODE_FILE:
            COLOR_PRINT(RED, "Error: failed open code_array file.\n");
            break;

        case ProcessorErrorType::FAILED_READ_FILE_LEN:
            COLOR_PRINT(RED, "Error: failed read code_array file len (file signature).\n");
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


    COLOR_PRINT(BLUE, "ram.array:\n");

    for (size_t RAM_i = 0; RAM_i < 128; RAM_i++)
    {
        COLOR_PRINT(CYAN, "[%3lu] %d\n", RAM_i, spu->ram.array[RAM_i]);
    }

    printf("\n");

    COLOR_PRINT(YELLOW, "CodeArray size = %lu\n\n", spu->code_array.size);

    COLOR_PRINT(WHITE, "CODE:\n");
    for (size_t code_i = 0; code_i < spu->code_array.size; code_i++)
    {
        COLOR_PRINT(BLUE, "[%2lu] %d", code_i, spu->code_array.array[code_i]);
        if (code_i == spu->code_array.ip)
        {
            COLOR_PRINT(WHITE, " < code_array.ip");
        }
        printf("\n");
    }

    COLOR_PRINT(RED, "CODE END\n");

    COLOR_PRINT(GREEN, "\nPROCESSOR DUMP END\n");

    return;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void WhereProcessorIs(const char* command)
{
    assert(command);
    ON_DEBUG(
    LOG_PRINT(Green, "processor::%s", command);
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
