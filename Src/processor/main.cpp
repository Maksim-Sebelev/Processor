#include <stdlib.h>
#include "processor/processor.hpp"
#include "lib/lib.hpp"

#ifdef _DEBUG
#include "logger/log.hpp"
#endif // _DEBUG

int main(int argc, const char* argv[])
{
    ON_DEBUG(
    COLOR_PRINT(GREEN, "\nPROCESSOR START\n");
    LOG_OPEN();
    )

    const ProcessorInput input = GetProcessorInput(argc, argv);
    RunProcessor(&input);

    ON_DEBUG(
    LOG_CLOSE();
    COLOR_PRINT(GREEN, "PROCESSOR END\n");
    )

    return EXIT_SUCCESS;
}
