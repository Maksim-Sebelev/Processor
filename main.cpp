#include "Assembler/Compiler.h"
#include "Processor/Processor.h"
#include "ConsoleCmd/ConsoleCmd.h"

// #include "Logger/Log.h"

int main(const int argc, const char** argv)
{
    IOfile File = {};
    
    CallCmd(argc, argv, &File);

    return 0;
}
