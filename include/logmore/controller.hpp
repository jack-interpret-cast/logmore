#pragma once

#include "keys.hpp"

#include <functional>
#include <algorithm>

class Terminal;
class InputBuffer;

enum class LogLevel {
    DEBUG   = 0,
    INFO    = 1,
    WARN    = 2,
    ERROR   = 3,
    Unknown = 4,
};

LogLevel get_line_log_level(std::string_view line, ssize_t log_start_idx = 25,
                            ssize_t log_len = 10);

size_t find_fuzzy(std::string_view needle, std::string_view haystack, ssize_t start_pos = 0);

class Controller
{
public:
    Controller(Terminal* terminal, InputBuffer* input, std::function<void()> shutdown);

    void key_handler(const Key& key);

private:
    enum class Mode { Normal, Search };
    Mode _mode{Mode::Normal};

    void update_main_window();

    Terminal* _terminal;
    InputBuffer* _input;
    std::function<void()> _shutdown;
    ssize_t _line_num{0};

    LogLevel _min_log_level{LogLevel::DEBUG};
    std::string _search;
};
