#include "console/consoleCmd.hpp"
#include "lib/lib.hpp"
#include "log/log.hpp"

int main(const int argc, const char** argv)
{

    ON_DEBUG(
    OPEN_LOG();
    LOG_PRINT(Green, "Start\n\n");
    )

    CallCmd(argc, argv);

    ON_DEBUG(
    LOG_PRINT(Green, "\nEnd");
    CLOSE_LOG();
    )

    return EXIT_SUCCESS;
}
