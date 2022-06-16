#pragma once

#include "keys.hpp"

#include <functional>

class Terminal;
class InputBuffer;

enum class LogLevel
{
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
    Unknown = 4,
};

LogLevel get_line_log_level(std::string_view line, size_t log_start_idx = 0, size_t log_len = 5);

class Controller
{
public:
    Controller(Terminal* terminal, InputBuffer* input, std::function<void()> shutdown);

    void key_handler(const Key& key);

private:
    void update_main_window();

    Terminal* _terminal;
    InputBuffer* _input;
    std::function<void()> _shutdown;

    LogLevel _min_log_level{LogLevel::Debug};
};
