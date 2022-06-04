#include "logmore/file_handler.hpp"
#include "logmore/keys.hpp"
#include "logmore/parser.hpp"
#include "logmore/terminal_handler.hpp"

#include <fmt/chrono.h>

#include <chrono>
#include <ranges>

struct Config
{
    ConfigOption<bool> boolean{"bool", false};
    ConfigOption<bool> boolshort{"othername", false, 'd'};
    ConfigOption<double> numeric{"numeric", false, 'D'};
    ConfigOption<int> number{"number", 1, 'n', "my number help"};
    ConfigOption<int> number2{"number2", 1, 'R', "my number help"};
    ConfigOption<int64_t> number3{"number3", 1};
    ConfigOption<uint64_t> number4{"number5", 1};
    ConfigOption<std::string> filename{"filename"};
};

int main(int argc, const char* argv[])
{
    using clock = std::chrono::high_resolution_clock;
    auto start  = clock::now();
    auto config = parse_command_line<Config>(argc, argv);

    if (!config) return 1;
    fmt::print("Parse results:");
    auto functor = [](auto& config) {
        fmt::print("{}: {}, ", config.long_name, config.get());
        return true;
    };
    visit_struct(functor, config.value());
    fmt::print("\n");
    auto mid = clock::now();

    FileHandler file("test.log");
    if (!file) return 1;
    InputBuffer buff(file.get_buffer());
    fmt::print("sz:{} chars\n", buff.data_size());
    buff.load_data();
    auto parsing = mid - start;
    auto reading = clock::now() - mid;
    fmt::print("parsing: {}, reading: {}, for: {} lines\n",
               std::chrono::duration_cast<std::chrono::microseconds>(parsing),
               std::chrono::duration_cast<std::chrono::microseconds>(reading), buff.num_lines());
    //    buff.dump_data();
    auto serv        = boost::asio::io_service{};
    auto key_handler = [&](Key key) { throw std::logic_error(fmt::format("{}", key)); };
    TerminalHandler terminal{serv, key_handler};
    serv.run();
    return 0;
}
