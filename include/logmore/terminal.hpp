#pragma once

#include "keys.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/signal_set.hpp>
#include <fire/types.hpp>
#include <sys/ioctl.h>

#include <csignal>
#include <memory>
#include <ranges>

struct _win_st;
struct coord
{
    int w, h;
};

enum class TextColour {
    // May vary slightly depending on a terminal's specific settings
    // Similar colours have been omitted
    black       = 0,
    red         = 1,
    green       = 2,
    orange      = 3,
    blue        = 4,
    purple      = 5,
    white       = 7,
    grey        = 8,
    light_red   = 9,
    light_green = 10,
    yellow      = 11,
    light_blue  = 12,
    // These are not direct ncurses colours
    message,
    command,
};

struct Line
/*
 * Holds view to output line and contains desired formatting for the line
 */
{
    std::string_view chars;
    TextColour style = TextColour::white;
    std::vector<size_t> highlights{};
};

class Terminal
{
public:
    explicit Terminal(boost::asio::io_service& serv, std::function<void(const Key)> input_handler);
    template <range_of<Line> R>
    size_t set_window_lines(R&&, bool reverse = false);

    void set_msg_line(std::string_view str);
    void set_command_line(std::string_view str);

    /*
     * Making this public is leaking an implementation detail, but only the caller knows if a
     * refresh is required, and refreshing on every update will result in poor performance
     */
    void refresh() const;

private:
    void get_signal();
    void get_input();
    void resize();
    coord dims() const;

    // Helper func so we can template 'set_window_lines'
    // but keep the ncurses include inside the .cpp only
    bool set_next_window_line(const Line&, int line_num);

    struct cleanup_fn
    {
        void operator()(_win_st* win) const;
    };
    std::unique_ptr<_win_st, cleanup_fn> _win;
    boost::asio::posix::stream_descriptor _input;
    boost::asio::signal_set _signal;
    std::function<void(const Key)> _input_handler;
    size_t _cursor_offset{0};
};

template <range_of<Line> R>
size_t Terminal::set_window_lines(R&& range, bool)
{
    int count   = 0;
    auto [_, h] = dims();
    for (const auto& line :
         range | std::views::take_while([&](const Line&) { return count < h - 2; }))
    {
        set_next_window_line(line, count++);
    }
    return 0;
}
