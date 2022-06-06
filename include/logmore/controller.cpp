#include "controller.hpp"
#include "file_handler.hpp"
#include "keys.hpp"
#include "terminal.hpp"

#include <utility>

Controller::Controller(Terminal* terminal, InputBuffer* input, std::function<void()> shutdown)
    : _terminal(terminal)
    , _input(input)
    , _shutdown(std::move(shutdown))
{
}

void Controller::key_handler(const Key& key)
{
    _terminal->set_msg_line(fmt::format("{}", key));
    if (key._special == KeySpecials::esc) return _shutdown();
    if (key._letter == 'f')
    {
        auto data_range = _input->get_range();
        // TODO: Make filter for log level
        auto all            = [](const std::string&) { return true; };
        auto string_to_line = [](const std::string& str) { return Line{.chars = str}; };
        auto filtered_range =
            data_range | std::views::filter(all) | std::views::transform(string_to_line);
        _terminal->set_window_lines(filtered_range);
        _terminal->set_msg_line(fmt::format("Setting range with {} lines", _input->num_lines()));
    }
    _terminal->refresh();
}
