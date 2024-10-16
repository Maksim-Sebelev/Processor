#include "Compiler.h"
#include "Processor.h"
#include "ConsoleCmd.h"

int main(const int argc, const char** argv)
{
    IOfile File = {};
    
    // File.CodeFile = "test2_res.txt";
    // File.ProgrammFile = "test2.txt";

    CallCmd(argc, argv, &File);

    // for (size_t argv_i = 1; (int) argv_i < argc; argv_i++)
    // {
    //     for (size_t cmd_i = 0; cmd_i < CmdQuant; cmd_i++)
    //     {
    //         CONSOLE_ASSERT(ConsoleCmd[cmd_i] (argc, argv, argv_i, &File));
    //     }
    // }

    return 0;
}
