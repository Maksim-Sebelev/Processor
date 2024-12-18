#include "Assembler2/Assembler.h"
#include "Processor/Processor.h"
#include "ConsoleCmd/ConsoleCmd.h"

// #include "Logger/Log.h"

int main()
{
    COLOR_PRINT(RED, "here we go\n");

    IOfile File = {};

    File.ProgrammFile = "cmd.txt";
    File.CodeFile     = "code.txt";

    RunAssembler(&File);
    RunProcessor(&File);

    // CallCmd(argc, argv, &File);

    return 0;
}
