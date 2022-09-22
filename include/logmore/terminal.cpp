#include "terminal.hpp"

#include <fmt/format.h>

extern "C" {
#include <ncurses.h>
}

#include <cctype>
#include <utility>

Terminal::Terminal(boost::asio::io_service& serv, std::function<void(const Key)> input_handler)
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

    // Colours for main window
    for (auto colour : magic_enum::enum_values<TextColour>())
    {
        ::init_pair((short)colour, (short)colour, -1);
    }
    // Special colours for message & command lines
    ::init_pair((short)TextColour::message, (short)TextColour::black, (short)TextColour::orange);
    ::init_pair((short)TextColour::command, (short)TextColour::white, (short)TextColour::grey);
    ::init_pair((short)TextColour::highlight, (short)TextColour::white, (short)TextColour::grey);

    get_signal();
    get_input();
    ESCDELAY = 0; // ESC key input is not buffered
    TABSIZE  = 1; // Make tabs only 1 space wide (simplifies logic elsewhere)

    // (TODO: rm): DEBUGGING
    attrset(A_NORMAL);
    for (auto colour : magic_enum::enum_values<TextColour>())
    {
        ::attron(COLOR_PAIR((short)colour));
        addstr("this is a test of the different colours that you can havethis is a test of the "
               "different colours that you can havethis is a test of the different colours that "
               "you can have\n");
    }
    set_command_line("This is a test of the command line\n");
    set_msg_line(fmt::format("coords row: {}, col: {}, colours: {}", dims().h, dims().w, COLORS));
    refresh();
}

void Terminal::cleanup_fn::operator()(_win_st* win) const
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

void Terminal::get_signal()
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

void Terminal::get_input()
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

void Terminal::resize()
{
    auto ws = winsize{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
        ::perror("TIOCGWINSZ");
    else
    {
        ::resizeterm(ws.ws_row, ws.ws_col);
        set_msg_line(fmt::format("resize with row: {}, col: {}", ws.ws_row, ws.ws_col));
    }
    refresh();
}

coord Terminal::dims() const
{
    int maxrow, maxcol;
    getmaxyx(stdscr, maxrow, maxcol);
    return {maxcol, maxrow};
}

void Terminal::set_msg_line(std::string_view str) { _msg_line = str; }

void Terminal::set_command_line(std::string_view str)
{
    if (std::iscntrl(str.back())) str.remove_suffix(1);
    _cmd_line = str;
}

void Terminal::write_msg_line()
{
    attrset(A_ITALIC);
    ::attron(COLOR_PAIR(TextColour::message));
    auto [w, h] = dims();
    mvaddstr(h - 2, 0, fmt::format("{:<{}}", _msg_line, w).c_str());
}

void Terminal::write_command_line()
{
    attrset(A_BOLD);
    ::attron(COLOR_PAIR(TextColour::command));
    auto [w, h] = dims();
    mvaddstr(h - 1, 0, fmt::format("{:<{}}", _cmd_line, w).c_str());
}

void Terminal::refresh()
{
    write_msg_line();
    write_command_line();
    ::refresh();
}

bool Terminal::write_next_window_line(const Line& line, int line_num)
{
    attrset(A_NORMAL);
    ::attron(COLOR_PAIR(line.style));
    auto [w, h] = dims();
    mvaddstr(line_num, 0, fmt::format("{:<{}}", line.chars, w).c_str());

    // Overwrite previous characters with the highlight here
    for (const auto [begin, len] : line.highlights)
    {
        attrset(A_BOLD);
        ::attron(COLOR_PAIR(TextColour::highlight));
        mvaddstr(line_num, begin, fmt::format("{}", line.chars.substr(begin, len), w).c_str());
    }
    return false;
}
