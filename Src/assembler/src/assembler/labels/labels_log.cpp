#include <assert.h>
#include "assembler/labels/labels.hpp"
#include "assembler/labels/labels_log.hpp"
#include "logger/log.hpp"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void LogLabelName(const char* name);

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LabelsArrayLog(const LabelsArray* labels_array)
{
    assert(labels_array);

    LOG_TITLE(Red, "LABELS ARRAY!!!");

    const size_t size  = labels_array->size;

    LOG_PRINT(Blue, "labels quant = %lu\n\n", size);

    const Label* array = labels_array->array;

    for (size_t i = 0; i < size; i++)
    {
        const Label label = array[i];

        LOG_PRINT(Yellow, "label[%lu] =\n{\n", i);
        LOG_COLOR(Green);
        LogLabelName(label.name);
        LOG_ADC_PRINT("code pointer = %lu\n", label.code_place);
        LOG_ADC_PRINT("is defined = %s\n", label.is_defined ? "true" : "false");
        LOG_PRINT(Yellow, "}\n\n");
    }

    LOG_TITLE(Red, "LABELS ARRAY END");

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LogLabel(const Label* label)
{
    assert(label);

    LOG_PRINT(Yellow, "label =\n{\n");
    LOG_COLOR(Green);
    LogLabelName(label->name);
    LOG_ADC_PRINT("code pointer = %lu\n", label->code_place);
    LOG_ADC_PRINT("is defined   = %s\n", label->is_defined ? "true" : "false");
    LOG_PRINT(Yellow, "}\n\n");

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void LogLabelName(const char* name)
{
    assert(name);


    LOG_ADC_ONE_P_SECTION_BEGIN();

    LOG_ONE_P_SECTION("name = '");

    for (size_t i = 0; name[i] != ':'; i++)
        LOG_ONE_P_SECTION("%c", name[i]);

    LOG_ONE_P_SECTION("'\n");

    LOG_ONE_P_SECTION_END();

    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
