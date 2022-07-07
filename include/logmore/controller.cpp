#include "controller.hpp"
#include "file_handler.hpp"
#include "keys.hpp"
#include "terminal.hpp"

#include <utility>

LogLevel get_line_log_level(std::string_view line, const size_t log_start_idx, const size_t log_len)
{
    const auto level_str = line.substr(log_start_idx, log_len);
    for (const auto& [index, name] : magic_enum::enum_entries<LogLevel>())
        if (level_str.starts_with(name)) return LogLevel{index};
    return LogLevel::Unknown;
}

Controller::Controller(Terminal* terminal, InputBuffer* input, std::function<void()> shutdown)
    : _terminal(terminal)
    , _input(input)
    , _shutdown(std::move(shutdown))
{
}

void Controller::key_handler(const Key& key)
{
    _terminal->set_msg_line(fmt::format("{}", key));
    // Special key handling must come first to avoid conflicts due to overlap
    if (key._special == KeySpecials::up && _line_num > 0)
    {
        _line_num--;
    } else if (key._special == KeySpecials::down)
    {
        _line_num++;
    } else if (key._special == KeySpecials::pgup && _line_num > 10)
    {
        _line_num -= 10;
    } else if (key._special == KeySpecials::pgdn)
    {
        _line_num += 10;
    } else if (key._letter == 'f')
    {
        _terminal->set_msg_line(fmt::format("Setting range with {} lines", _input->num_lines()));
    } else if ('1' <= key._letter && key._letter <= '4')
    {
        _min_log_level = LogLevel{key._letter - '1'};
        _terminal->set_msg_line(
            fmt::format("Updated min log level to {}", magic_enum::enum_name(_min_log_level)));
    }
    update_main_window();
    _terminal->refresh();
}

void Controller::update_main_window()
{
    auto data_range       = _input->get_range();
    auto log_level_filter = [this](std::string_view line) {
        auto log_level = get_line_log_level(line);
        return (size_t)log_level >= (size_t)_min_log_level;
    };
    auto string_to_line = [](std::string_view str) { return Line{.chars = str}; };
    auto filtered_range = data_range | std::views::filter(log_level_filter)
                          | std::views::drop(_line_num) | std::views::transform(string_to_line);
    _terminal->set_window_lines(filtered_range);
}
