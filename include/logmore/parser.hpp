#pragma once

#include "types.hpp"

#include <boost/pfr.hpp>
#include <fmt/format.h>

#include <concepts>
#include <map>
#include <optional>
#include <string>
#include <vector>

template <typename T>
class ConfigOption
{
public:
    /* Constructor for optional arguement */
    ConfigOption(std::string longName, T defaultArg, char shortName = '\0',
                 std::string helpText = "")
        : long_name(std::move(longName))
        , short_name(shortName)
        , help_text(std::move(helpText))
        , _data(std::move(defaultArg))
    {
    }

    /* Constructor for required arguement */
    explicit ConfigOption(std::string helpText) : help_text(std::move(helpText)) {}

    bool is_default_arg() const { return long_name.empty(); }

    bool parse(std::vector<std::string>& args)
    {
        if (args.empty()) return true;
        if (is_default_arg())
        {
            // this is default arg
            _data = parse_detail<T>(args.back());
            args.pop_back();
            return false;
        }
        for (auto iter = args.begin(); iter < args.end(); ++iter)
        {
            if (*iter == "--" + long_name)
            {
                if (iter + 1 != args.end())
                {
                    _data = parse_detail<T>(*(iter + 1));
                    args.erase(iter, iter + 2);
                } else
                {
                    fmt::print(fmt::style::warn, "Missing argument for option'--{}'\n", long_name);
                }
                break;
            } else if (auto pos = check_short_name_option(*iter); pos)
            {
                if (++*pos == iter->length())
                {
                    _data = parse_detail<T>(*(iter + 1));
                    iter->length() > 1 ? args.erase(iter + 1, iter + 2)
                                       : args.erase(iter, iter + 2);
                } else
                {
                    fmt::print(fmt::style::warn, "Missing argument for option'-{}'\n", short_name);
                }
                break;
            }
        }
        return false;
    }

    T get() { return _data; }

    const std::string long_name;
    const char short_name{'\0'};
    const std::string help_text;

private:
    template <typename Value>
    Value parse_detail(std::string_view) const
    {
        static_assert(sizeof(Value) == 0 && "There is no parser for type T");
    }
    // Compiling with GCC will throw error: "explicit specialization in non-namespace scope ‘struct
    // ConfigOption<T>" template <> [[nodiscard]] int parse_detail(std::string_view arg) const This
    // is actually a bug in GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282 Workaround
    // below:
    template <std::signed_integral Value>
    [[nodiscard]] Value parse_detail(std::string_view arg) const
    {
        try
        {
            return std::stol(arg.data());
        } catch (const std::invalid_argument& e)
        {
            fmt::print(fmt::style::warn, "Could not convert '{}' to integer\n", arg);
            return 0;
        }
    }
    template <std::unsigned_integral Value>
    [[nodiscard]] Value parse_detail(std::string_view arg) const
    {
        try
        {
            return std::stoul(arg.data());
        } catch (const std::invalid_argument& e)
        {
            fmt::print(fmt::style::warn, "Could not convert '{}' to integer\n", arg);
            return 0;
        }
    }
    template <std::floating_point Value>
    [[nodiscard]] Value parse_detail(std::string_view arg) const
    {
        try
        {
            return std::stod(arg.data());
        } catch (const std::invalid_argument& e)
        {
            fmt::print(fmt::style::warn, "Could not convert '{}' to floating point\n", arg);
            return 0;
        }
    }
    template <std::same_as<std::string> Value>
    [[nodiscard]] Value parse_detail(std::string_view arg) const
    {
        return std::string{arg};
    }
    std::optional<size_t /*is end*/> check_short_name_option(std::string& arg)
    /* Looks for valid config short name in arg, if found return index */
    {
        auto pos = arg.find_last_of(short_name);
        if (pos == std::string_view::npos || arg.length() < 2 || arg[0] != '-'
            || !std::isalpha(arg[1]))
            return {};
        arg.erase(pos, 1);
        return --pos;
    }
    T _data;
};

template <>
bool ConfigOption<bool>::parse(std::vector<std::string>& args);

template <typename F, typename ConfigT>
bool visit_struct_with_error(F&& functor, ConfigT& config)
{
    constexpr size_t struct_elems = boost::pfr::tuple_size<ConfigT>::value;
    return []<std::size_t... I>(F && functor, ConfigT & config, std::index_sequence<I...>)
    {
        return std::max({functor(boost::pfr::get<sizeof...(I) - 1U - I>(config))...});
    }
    (functor, config, std::make_index_sequence<struct_elems>());
}

template <typename F, typename ConfigT>
void visit_struct(F&& functor, ConfigT& config)
{
    constexpr size_t struct_elems = boost::pfr::tuple_size<ConfigT>::value;
    return []<std::size_t... I>(F && functor, ConfigT & config, std::index_sequence<I...>)
    {
        (functor(boost::pfr::get<sizeof...(I) - 1U - I>(config)), ...);
    }
    (functor, config, std::make_index_sequence<struct_elems>());
}

template <typename ConfigT>
bool validate_config_struct(const ConfigT& config)
{
    std::map<std::string_view, size_t> long_names;
    auto check_long_names = [&long_names](auto& config_opt) {
        return !config_opt.long_name.empty() && long_names[config_opt.long_name]++;
    };
    std::map<char, size_t> short_names;
    auto check_short_names = [&short_names](auto& config_opt) {
        return config_opt.short_name != '\0' && short_names[config_opt.short_name]++;
    };
    bool short_dups = visit_struct_with_error(check_short_names, config);
    bool long_dups  = visit_struct_with_error(check_long_names, config);
    return short_dups || long_dups;
}

template <typename ConfigT>
std::optional<ConfigT> parse_command_line(int argc, const char* argv[])
{
    std::vector<std::string> args;
    for (ssize_t i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);

    ConfigT config;
    if (validate_config_struct(config))
    {
        fmt::print(fmt::style::warn, "Some configs had a duplicated name\n");
        return {};
    }

    auto parse = [&args](auto& config_opt) { return config_opt.parse(args); };
    if (visit_struct_with_error(parse, config))
    {
        fmt::print(fmt::style::warn, "There was an error parsing config options\n");
        return {};
    }
    if (!args.empty())
    {
        fmt::print(fmt::style::warn, "There were unparsed arguements: {}\n", fmt::join(args, " "));
        return {};
    }
    return config;
}

template <typename ConfigT>
void print_config_help(std::string_view app_name)
{
    // strip off full path to executable
    auto small_app_name = app_name.substr(app_name.find_last_of('/') + 1);

    fmt::print("{0} config help:\n\n    {0} <options> ", small_app_name);

    auto print_defaults = [](const auto& config_opt) {
        if (config_opt.is_default_arg()) fmt::print("[{}] ", config_opt.help_text);
    };
    auto print_entry = [](const auto& config_opt) {
        if (!config_opt.is_default_arg())
            fmt::print("    -{:<2} --{:<15} {:<20}\n", config_opt.short_name, config_opt.long_name,
                       config_opt.help_text);
    };
    ConfigT config{};
    visit_struct(print_defaults, config);
    fmt::print("\n");
    visit_struct(print_entry, config);
}
