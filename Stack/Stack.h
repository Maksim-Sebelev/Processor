
#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../Common/ColorPrint.h"

//--------------------------------------------------------------------------------------------------------------------------------
#define STACK_DEBUG    
#define STACK_CANARY     
#define STACK_DATA_CANARY
#define STACK_HASH
#define STACK_DATA_HASH
#define STACK_DATA_POISON
//--------------------------------------------------------------------------------------------------------------------------------

#ifdef STACK_DEBUG
    #define ON_STACK_DEBUG(...) __VA_ARGS__
    #define OFF_STACK_DEBUG(...)
#else
    #define ON_STACK_DEBUG(...)
    #define OFF_STACK_DEBUG(...) __VA_ARGS__
#endif


#ifdef STACK_CANARY
    #define ON_STACK_CANARY(...) __VA_ARGS__
#else
    #define ON_STACK_CANARY(...)
#endif


#ifdef STACK_DATA_CANARY
    #define ON_STACK_DATA_CANARY(...)  __VA_ARGS__
#else
    #define ON_STACK_DATA_CANARY(...)
#endif


#ifdef STACK_HASH
    #define ON_STACK_HASH(...) __VA_ARGS__
#else   
    #define ON_STACK_HASH(...)
#endif


#ifdef STACK_DATA_HASH
    #define ON_STACK_DATA_HASH(...) __VA_ARGS__
#else
    #define ON_STACK_DATA_HASH(...)
#endif


#ifdef STACK_DATA_POISON
    #define ON_STACK_DATA_POISON(...) __VA_ARGS__
#else
    #define ON_STACK_DATA_POISON(...)
#endif


typedef int StackElem_t;

ON_STACK_CANARY(typedef uint64_t StackCanary_t;)

ON_STACK_DEBUG
(
struct NamePlaceVar
{
    const char* File;
    int         Line;
    const char* Func;
    const char* Name;
};
)

struct Stack_t
{
    ON_STACK_CANARY(StackCanary_t LeftStackCanary;)
    size_t Size;
    size_t Capacity;
    StackElem_t* Data;
    ON_STACK_DEBUG(NamePlaceVar Var;)
    ON_STACK_DATA_HASH(uint64_t DataHash;)
    ON_STACK_HASH(uint64_t StackHash;)
    ON_STACK_CANARY(StackCanary_t RightStackCanary;)
};


struct Warnings
{
    unsigned char PopInEmptyStack             : 1;
    unsigned char TooBigCapacity               : 1;
    unsigned char PushInFullStack             : 1;
};

 
struct FatalErrors
{
    unsigned char StackNull                   : 1;
    unsigned char DataNull                    : 1;
    unsigned char CallocCtorNull              : 1;
    unsigned char ReallocPushNull             : 1;
    unsigned char ReallocPopNull              : 1;
    ON_STACK_CANARY
    (
    unsigned char LeftStackCanaryChanged      : 1;
    unsigned char RightStackCanaryChanged     : 1;
    )
    ON_STACK_DATA_CANARY
    (
    unsigned char LeftDataCanaryChanged       : 1;
    unsigned char RightDataCanaryChanged      : 1;
    )
    ON_STACK_DATA_POISON
    (
    unsigned char DataElemBiggerSizeNotPoison : 1;
    )
    ON_STACK_HASH
    (    
    unsigned char StackHashChanged            : 1;
    )
    ON_STACK_DATA_HASH
    (
    unsigned char DataHashChanged             : 1;
    )
    ON_STACK_DEBUG
    (
    unsigned char SizeBiggerCapacity          : 1;
    unsigned char CapacitySmallerMin          : 1;
    unsigned char CapacityBiggerMax           : 1;
    unsigned char CtorStackNameNull           : 1;
    unsigned char CtorStackFileNull           : 1;
    unsigned char CtorStackFuncNull           : 1;
    unsigned char CtorStackLineNegative       : 1;
    )
};


struct StackErrorType
{
    unsigned int IsFatalError : 1;
    unsigned int IsWarning    : 1;
    Warnings Warning;
    FatalErrors FatalError;
    const char* File;
    int Line;
    const char* Func;
};

//operation with stack
StackErrorType StackCtor          (Stack_t* Stack ON_STACK_DEBUG(, const char* File, int Line, const char* Func, const char* Name));
StackErrorType StackDtor          (Stack_t* Stack);
StackErrorType PrintStack         (Stack_t* Stack);
StackErrorType PrintLastStackElem (Stack_t* Stack);
StackErrorType StackPush          (Stack_t* Stack, StackElem_t PushElem);
StackErrorType StackPop           (Stack_t* Stack, StackElem_t* PopElem);

//--------------------------------------------------------------------------------------------------------------------------

//stack error func

#define STACK_VERIF(StackPtr, Err) Verif(StackPtr, &Err ON_STACK_DEBUG(, __FILE__, __LINE__, __func__))

ON_STACK_DEBUG
(
// void StackDump(const Stack_t* Stack, const char* File, int Line, const char* Func);
// #define STACK_DUMP(Stack) StackDump(Stack, __FILE__, __LINE__, __func__)
)

#define STACK_RETURN_IF_ERR_OR_WARN(StackPtr, Err) do                     \
{                                                                          \
    StackErrorType ErrCopy = Err;                                           \
    Verif(Stack, &ErrCopy ON_STACK_DEBUG(, __FILE__, __LINE__, __func__));   \
    if (ErrCopy.IsFatalError == 1 || ErrCopy.IsWarning == 1)                  \
    {                                                                          \
        return ErrCopy;                                                         \
    }                                                                            \
} while (0)                                                                       \


#ifdef STACK_DEBUG
    #define STACK_ASSERT(Err) do                                \
    {                                                            \
        StackErrorType ErrCopy = Err;                             \
        StackAssertPrint(ErrCopy, __FILE__, __LINE__, __func__);   \
        if (ErrCopy.IsFatalError == 1 || ErrCopy.IsWarning == 1)    \
        {                                                            \
            COLOR_PRINT(CYAN, "abort() in 3, 2, 1...\n");             \
            abort();                                                   \
        }                                                               \
    } while (0)                                                          \

#else
    #define STACK_ASSERT(Err) StackAssertPrint(Err, __FILE__, __LINE__, __func__)
#endif

void StackAssertPrint(StackErrorType Err, const char* File, int Line, const char* Func);


#endif
