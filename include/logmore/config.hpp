#pragma once

#include "parser.hpp"

struct Config
{
    ConfigOption<std::string> filename{"file to view logs for"};
};