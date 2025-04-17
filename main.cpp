#include "console/consoleCmd.hpp"
#include "lib/lib.hpp"
#include "log/log.hpp"

int main(const int argc, const char** argv)
{
    ON_DEBUG(
    OPEN_LOG();
    )    
    CallCmd(argc, argv);
    
    ON_DEBUG(
    CLOSE_LOG();
    )

    return EXIT_SUCCESS;
}
