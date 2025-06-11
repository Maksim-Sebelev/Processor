#include <stdlib.h>
#include "assembler/assembler.hpp"
#include "flags/flags.hpp"
#include "global/global_include.hpp"
#include "lib/lib.hpp"

#ifdef _DEBUG
#include "logger/log.hpp"
#endif


int main(int argc, char* argv[])
{
    ON_DEBUG(
    COLOR_PRINT(GREEN, "\nASSEMBLER START\n");
    LOG_OPEN();
    )

    IOfile files = CallFlags(argc, argv);

    RunAssembler(&files);

    ON_DEBUG(
    LOG_CLOSE();
    COLOR_PRINT(GREEN, "ASSEMBLER END\n");
    )

    return EXIT_SUCCESS;
}
