#include "logmore/parser.hpp"

#include <fmt/chrono.h>

#include <chrono>

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

int main(int argc, char* argv[])
{
    using clock = std::chrono::high_resolution_clock;
    auto start  = clock::now();
    auto config = parse_command_line<Config>(argc, argv);

    if (!config) return 1;
    fmt::print("Parse results:");
    //               " {}, {}, {}, {}\n", config.number.get(), config.boolean.get(),
    //               config.filename.get(), config.boolshort.get());
    auto functor = [](auto& config) {
        fmt::print("{}: {}, ", config.long_name, config.get());
        return true;
    };
    visit_struct(functor, config.value());
    fmt::print("\n");
    auto delta = clock::now() - start;
    fmt::print("tdelta: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(delta));
    return 0;
}
