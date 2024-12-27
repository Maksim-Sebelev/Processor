#include <stdio.h>
#include "Processor/Processor.hpp"
#include "Assembler/Assembler.hpp"

int main()
{

    IOfile file = {};
    file.CodeFile = "code.txt";
    file.ProgrammFile = "programm.txt";

    RunAssembler(&file);
    RunProcessor(&file);

    return EXIT_SUCCESS;
}
