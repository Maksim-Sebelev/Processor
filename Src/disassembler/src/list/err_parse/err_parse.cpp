#include <assert.h>
#include <stdlib.h>
#include "list/list.hpp"
#include "list/err_parse/err_parse.hpp"

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static WayToErr* WayToErrCtor               (WayToErr* old_way, CodePlace* place);
static void      WayToErrDtor               (WayToErr* way);

static void      ListAssertPrint            (const ListError_t* err, const CodePlace* assert_place);
static void      PrintErrorOrWarn           (const ListError_t* err);    // return true if status OK, else return false
static void      PrintError                 (const ListError_t* err);
static void      PrintWarning               (const ListError_t* err);
static void      PrintWayToErr              (const WayToErr   * way_to_err, const ListStatus* status);
static void      PrintFullWayToErrWithAssert(const CodePlace  * assert_place, const WayToErr* way_to_err, const ListStatus* status);


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void ListAssert(List_t* list, ListError_t err, const char* file, unsigned int line, const char* func)
{
    assert(file);
    assert(func);

    ListStatus status = err.status;

    if (status == ListStatus::OK) return;

    CodePlace assert_place = CodePlaceCtor(file, line, func);

    ListAssertPrint(&err, &assert_place);

    WayToErrDtor(err.err_way);

    
    if (status == ListStatus::WARN) return;
    
    if (list->data) // if need to free allocated memory
        ListDtor(list);

    exit(EXIT_FAILURE);

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ListError_t ErrTransfer(ListError_t* err, const char* file, unsigned int line, const char* func)
{
    assert(err);
    assert(file);
    assert(func);

    if (err->status == ListStatus::OK)
        return {};

    CodePlace now_place = CodePlaceCtor(file, line, func);

    err->err_way = WayToErrCtor(err->err_way, &now_place); // update way to err;

    return *err;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ListError_t ListErrorStatusCtor(ListErrorType err_type, const char* file, unsigned int line, const char* func)
{
    assert(file);
    assert(func);

    ListError_t err = {};

    CodePlace place = CodePlaceCtor(file, line, func);
    err.err_way     = WayToErrCtor(nullptr, &place);

    err.status      = ListStatus::ERR;
    err.value.err   = err_type;

    return err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ListError_t ListWarningStatusCtor(ListWarningType warn_type, const char* file, unsigned int line, const char* func)
{
    assert(file);
    assert(func);

    ListError_t err = {};

    CodePlace place = CodePlaceCtor(file, line, func);
    err.err_way     =  WayToErrCtor(nullptr, &place);

    err.status     = ListStatus::WARN;
    err.value.warn = warn_type;

    return err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ListError_t ListOkStatusCtor()
{
    return (ListError_t)
    {
        .status = ListStatus::OK
    };
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static WayToErr* WayToErrCtor(WayToErr* old_way, CodePlace* place)
{
    assert(place);

    WayToErr* way_to_err = (WayToErr*) calloc(1, sizeof(WayToErr));

    if (!way_to_err)
        EXIT(EXIT_FAILURE, "failed allocate memory for error code place.\nsys error, i'm so sorry :(\n");
    
    way_to_err->previous_place = old_way;
    way_to_err->now_place      = *place ;

    return way_to_err;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void WayToErrDtor(WayToErr* way)
{
    if (!way) return;

    WayToErr* next_way = way->previous_place;

    free(way);

    return WayToErrDtor(next_way);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void ListAssertPrint(const ListError_t* err, const CodePlace* assert_place)
{
    assert(err );
    assert(assert_place);
    assert(err->status != ListStatus::OK);

    PrintErrorOrWarn(err);

    WayToErr* way_to_err = err->err_way;
    ListStatus status = err->status; 

    PrintFullWayToErrWithAssert(assert_place, way_to_err, &status);

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintError(const ListError_t* err)
{
    assert(err);
    assert(err->status == ListStatus::ERR);

    ListErrorType err_type = err->value.err;

    printf(RED); // made console color red

    printf("Error: ");

    switch (err_type)
    {
        case ListErrorType::FAILED_ALLOCATE_MEMORY_IN_CTOR:
            printf("failed allocate memory for List_t::data.");
            break;

        case ListErrorType::TRY_TO_DTOR_NULLPTR_DATA:
            printf("try to free nullptr data in dtor.");
            break;

        case ListErrorType::NO_ERR:  __builtin_unreachable__("try to print error, when no err");
        default:                     __builtin_unreachable__("undef error type");
    }

    printf(RESET); // ger default console text settings
    
    printf("\n\n");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintWarning(const ListError_t* err)
{
    assert(err);
    assert(err->status == ListStatus::WARN);

    ListWarningType warn_type = err->value.warn;

    printf(YELLOW); // made console color yellow;

    printf("Warning: ");

    switch (warn_type)
    {
        case ListWarningType::FAILED_REALLOCATE_DATA_AFTER_INSTERT:
            printf("Failed reallocate memory after insert.");
            break;

        case ListWarningType::FAILED_REALLOCATE_DATA_AFTER_ERASE:
            printf("failed reallocate memory after erase.");
            break;

        case ListWarningType::ERASE_IN_EMPTY_LIST:
            printf("try to erase in empty list.");
            break;

        case ListWarningType::TO_BIG_CAPACITY:
            printf("you insert too many elements in your list. we can't insert one more element.");
            break;

        case ListWarningType::NO_WARN: __builtin_unreachable__("try to print warning, when no warning"); break;
        default:                       __builtin_unreachable__("undef warning type");                    break;
    }

    printf("\nList will not change after this operation, for secure.\n\n");

    printf(RESET); // made console default settings

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintWayToErr(const WayToErr* way_to_err, const ListStatus* status)
{
    assert(status);

    if (!way_to_err)
    {
        switch (*status)
        {
            case ListStatus::ERR:  COLOR_PRINT(RED   , " < error detected here.\n"  ); return;
            case ListStatus::WARN: COLOR_PRINT(YELLOW, " < warning detected here.\n"); return;
            default: __builtin_unreachable__("here parse only err situation");
        }
        __builtin_unreachable__("wtf");
    }

    printf("\n");
    CodePlace now_place = way_to_err->now_place;

    PrintStructPlace(&now_place);

    return PrintWayToErr(way_to_err->previous_place, status);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintFullWayToErrWithAssert(const CodePlace* assert_place, const WayToErr* way_to_err, const ListStatus* status)
{
    assert(assert_place);
    assert(status);

    PrintStructPlace(assert_place);

    switch (*status)
    {
        case ListStatus::ERR:  printf(RED   ); break;
        case ListStatus::WARN: printf(YELLOW); break;
        default: __builtin_unreachable__("we parse only err here");
    }

    printf(" < assert made here.");
    printf(RESET);

    PrintWayToErr   (way_to_err, status); printf("\n");

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void PrintErrorOrWarn(const ListError_t* err) // return true if status OK, else return false
{
    assert(err);

    ListStatus status = err->status;

    switch (status)
    {
        case ListStatus::ERR : return PrintError  (err);                                 break;
        case ListStatus::WARN: return PrintWarning(err);                                 break;
        case ListStatus::OK  : __builtin_unreachable__("ok status is not parsing here"); break;
        default:               __builtin_unreachable__("undef list status");             break;
    }

    __builtin_unreachable__("we must return or fall in switch");

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
