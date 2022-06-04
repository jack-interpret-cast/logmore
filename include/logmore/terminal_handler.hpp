#pragma once

#include "keys.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/signal_set.hpp>
#include <ncurses.h>
#include <sys/ioctl.h>

#include <csignal>
#include <memory>

struct coord
{
    int w, h;
};

enum class color {
    message = 1,
    selection,
    mode_line_message,
};

class TerminalHandler
{
public:
    explicit TerminalHandler(boost::asio::io_service& serv,
                             std::function<void(const Key)> input_handler);
    void get_signal();
    void get_input();
    void resize() const;

    coord dims();

private:
    struct cleanup_fn
    {
        void operator()(_win_st* win) const;
    };
    std::unique_ptr<_win_st, cleanup_fn> _win;
    boost::asio::posix::stream_descriptor _input;
    boost::asio::signal_set _signal;
    std::function<void(const Key)> _input_handler;
};
