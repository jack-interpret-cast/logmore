#pragma once

#include <fire/parser.hpp>

struct Config
{
    ConfigOption<std::string> filename{{.help="file to view logs for"}};
    ConfigOption<bool> help{{.long_name="help", .short_name='h', .default_val=false, .help="Shows the help menu"}};
};