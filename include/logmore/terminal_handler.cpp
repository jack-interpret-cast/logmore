#include "terminal_handler.hpp"

#include <fmt/format.h>

#include <utility>

TerminalHandler::TerminalHandler(boost::asio::io_service& serv,
                                 std::function<void(const Key)> input_handler)
    : _win(::initscr())
    , _input{serv, ::dup(STDIN_FILENO)}
    , _signal{serv, SIGWINCH}
    , _input_handler(std::move(input_handler))
{
    if (_win.get() != ::stdscr) throw std::runtime_error{"error while initializing ncurses"};

    ::raw();
    ::noecho();
    ::cbreak();
    ::nonl();
    ::keypad(stdscr, true);
    ::nodelay(stdscr, true);

    ::start_color();
    ::use_default_colors();
    ::init_pair((int)color::message, COLOR_YELLOW, -1);
    ::init_pair((int)color::selection, COLOR_BLACK, COLOR_YELLOW);
    ::init_pair((int)color::mode_line_message, COLOR_WHITE, COLOR_RED);

    get_signal();
    get_input();
}

void TerminalHandler::cleanup_fn::operator()(_win_st* win) const
{
    if (win)
    {
        // consume all remaining characters from the terminal so they
        // don't leak in the bash prompt after quitting, then restore
        // the terminal state
        wint_t key;
        while (::get_wch(&key) != ERR)
            ;
        ::endwin();
    }
}

void TerminalHandler::get_signal()
{
    _signal.async_wait([this](const auto error_code, const int sig_num) {
        if (!error_code)
        {
            switch (sig_num)
            {
            case SIGWINCH: resize(); break;
            default: throw std::logic_error(fmt::format("signal {} not handled", sig_num));
            }
        }
        get_signal();
    });
}

void TerminalHandler::get_input()
{
    _input.async_read_some(boost::asio::null_buffers(),
                           [&](const auto error_code, const auto /*bytes read*/) {
                               if (!error_code)
                               {
                                   wint_t key;
                                   int res;
                                   while (ERR != (res = ::wget_wch(_win.get(), &key)))
                                   {
                                       _input_handler(Key{key});
                                       get_input();
                                   }
                               }
                           });
}

void TerminalHandler::resize() const
{
    auto ws = winsize{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
        ::perror("TIOCGWINSZ");
    else
        ::resizeterm(ws.ws_row, ws.ws_col);
}

coord TerminalHandler::dims()
{
    int maxrow, maxcol;
    getmaxyx(stdscr, maxrow, maxcol);
    return {maxrow, maxcol};
}
