#include "Assembler/Assembler.hpp"
#include "Processor/Processor.hpp"
#include "ConsoleCmd/ConsoleCmd.hpp"

// #include "Logger/Log.hpp"

int main()
{
    IOfile File = {};

    File.ProgrammFile = "cmd.txt";
    File.CodeFile     = "code.txt";

    RunAssembler(&File);
    RunProcessor(&File);

    // CallCmd(argc, argv, &File);

    return 0;
}
