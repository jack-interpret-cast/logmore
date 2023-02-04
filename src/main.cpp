#include "logmore/application.hpp"
#include "logmore/config.hpp"
#include "logmore/controller.hpp"
#include "logmore/file_handler.hpp"
#include "logmore/terminal.hpp"

#include <fire/parser.hpp>

int main(int argc, const char* argv[])
{
    auto config = parse_command_line<Config>(argc, argv);

    if (!config)
    {
        print_config_help<Config>(argv[0]);
        return 1;
    }

    Application application{*config};
    return application.start();
}
