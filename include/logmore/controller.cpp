#include "controller.hpp"
#include "file_handler.hpp"
#include "keys.hpp"
#include "terminal.hpp"

#include <utility>

LogLevel get_line_log_level(std::string_view line, const ssize_t log_start_idx,
                            const ssize_t log_len)
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
    _terminal->set_msg_line(fmt::format("[{}] {}", magic_enum::enum_name(_mode), key));
    switch (_mode)
    {
    case Mode::Normal: {
        // Special key handling must come first to avoid conflicts due to overlap
        if (key._special == KeySpecials::up)
        {
            _line_num--;
        } else if (key._special == KeySpecials::down)
        {
            _line_num++;
        } else if (key._special == KeySpecials::pgup)
        {
            _line_num -= 10;
        } else if (key._special == KeySpecials::pgdn)
        {
            _line_num += 10;
        } else if (key._letter == 'g')
        {
            _line_num = 0;
        } else if (key._letter == 'G')
        {
            _line_num = _input->num_lines();
        } else if (key._letter == 'f')
        {
            _terminal->set_msg_line(
                fmt::format("Setting range with {} lines", _input->num_lines()));
        } else if ('1' <= key._letter && key._letter <= '4')
        {
            _min_log_level = LogLevel{key._letter - '1'};
            _terminal->set_msg_line(
                fmt::format("Updated min log level to {}", magic_enum::enum_name(_min_log_level)));
        } else if (key._letter == '/')
        {
            _mode = Mode::Search;
            _search.clear();
            _terminal->set_command_line("/");
        }
        break;
    }
    case Mode::Search: {
        if (key._special == KeySpecials::enter)
        {
            _mode = Mode::Normal;
        } else if (key._special == KeySpecials::none)
            _search += key._letter;
        _terminal->set_command_line("/" + _search);
    }
    }

    _line_num =
        std::clamp<ssize_t>(_line_num, 0, _input->num_lines() - _terminal->get_visible_lines());
    update_main_window();
    _terminal->refresh();
}

void Controller::update_main_window()
{
    auto data_range = _input->get_range();

    auto log_level_filter = [this](std::string_view line) {
        auto log_level = get_line_log_level(line);
        return (ssize_t)log_level >= (ssize_t)_min_log_level;
    };

    auto string_to_line = [](std::string_view str) {
        using enum TextColour;
        const TextColour log_colours[] = {grey, white, orange, red, white};
        static_assert(magic_enum::enum_count<LogLevel>() == std::size(log_colours));
        auto log_level = get_line_log_level(str);
        return Line{.chars = str, .style = log_colours[(ssize_t)log_level]};
    };

    auto do_search_highlight = [this](Line line) {
        if (!_search.empty())
        {

            for (auto it = line.chars.find(_search); it != std::string_view::npos;
                 it      = line.chars.find(_search, it + 1))
            {
                line.highlights.push_back({._begin = ssize_t(it), ._length = std::ssize(_search)});
            }
        }
        return line;
    };
    /* clang-format off */
    auto filtered_range = data_range
                          | std::views::filter(log_level_filter)
                          | std::views::drop(_line_num)
                          | std::views::transform(string_to_line)
                          | std::views::transform(do_search_highlight);
    /* clang-format on */
    _terminal->set_window_lines(filtered_range);
}
