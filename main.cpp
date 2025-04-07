#include <stdlib.h>
#include "processor/processor.hpp"
#include "assembler/assembler.hpp"
#include "common/globalInclude.hpp"

int main()
{

    IOfile file = {};
    file.CodeFile = "code.txt";
    file.ProgrammFile = "programm.txt";

    RunAssembler(&file);
    RunProcessor(&file);

    return EXIT_SUCCESS;
}
