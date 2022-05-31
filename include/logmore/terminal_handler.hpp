#pragma once
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
    explicit TerminalHandler(boost::asio::io_service& serv)
        : _win(::initscr())
        , input_{serv, ::dup(STDIN_FILENO)}
        , signal_{serv, SIGTERM}
    {
        ::raw();
        ::noecho();
        ::keypad(stdscr, true);
        ::nodelay(stdscr, true);

        ::start_color();
        ::use_default_colors();
        ::init_pair((int)color::message, COLOR_YELLOW, -1);
        ::init_pair((int)color::selection, COLOR_BLACK, COLOR_YELLOW);
        ::init_pair((int)color::mode_line_message, COLOR_WHITE, COLOR_RED);

        //        resize();
        signal_.async_wait([](auto /*error_code*/, int /*sig_num*/) {
            throw std::logic_error("got shutdown request");
        });
    }

    void resize() const
    {
        auto ws = winsize{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
            ::perror("TIOCGWINSZ");
        else
            ::resizeterm(ws.ws_row, ws.ws_col);
    }

    coord dims()
    {
        int maxrow, maxcol;
        getmaxyx(stdscr, maxrow, maxcol);
        return {maxrow, maxcol};
    }

private:
    struct cleanup_fn
    {
        void operator()(_win_st* win) const;
    };
    std::unique_ptr<_win_st, cleanup_fn> _win;
    boost::asio::posix::stream_descriptor input_;
    boost::asio::signal_set signal_;
};

void TerminalHandler::cleanup_fn::operator()(_win_st* win) const
{
    if (win)
    {
        // consume all remaining characters from the terminal so they
        // don't leak in the bash prompt after quitting, then restore
        // the terminal state
        auto key = wint_t{};
        while (::get_wch(&key) != ERR)
            ;
        ::endwin();
    }
}
