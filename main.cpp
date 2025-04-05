#include <stdio.h>
#include "processor/Processor.hpp"
#include "assembler/Assembler.hpp"

int main()
{

    IOfile file = {};
    file.CodeFile = "code.txt";
    file.ProgrammFile = "programm.txt";

    RunAssembler(&file);
    RunProcessor(&file);

    return EXIT_SUCCESS;
}
