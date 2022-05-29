#pragma once

#include "types.hpp"

#include <boost/pfr.hpp>
#include <fmt/format.h>

#include <optional>
#include <string>
#include <vector>

template <typename T> struct ConfigOption
{
    /* Constructor for optional arguement */
    ConfigOption(std::string longName, T defaultArg, char shortName = '\0',
                 std::string helpText = "")
        : long_name(std::move(longName)),
          short_name(shortName),
          help_text(std::move(helpText)),
          _data(std::move(defaultArg))
    {
    }

    /* Constructor for required arguement */
    explicit ConfigOption(std::string helpText) : help_text(std::move(helpText)) {}

    bool parse(std::vector<std::string>& args)
    {
        if (args.empty()) return true;
        if (long_name.empty())
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
    template <typename Value> Value parse_detail(std::string_view) const
    {
        static_assert(sizeof(Value) == 0 && "There is no parser for type T");
    }
    // Compiling with GCC will throw error: "explicit specialization in non-namespace scope ‘struct
    // ConfigOption<T>" template <> [[nodiscard]] int parse_detail(std::string_view arg) const This
    // is actually a bug in GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282 Workaround
    // below:
    template <std::same_as<int> Value> [[nodiscard]] int parse_detail(std::string_view arg) const
    {
        try
        {
            return std::stoi(arg.data());
        } catch (const std::invalid_argument& e)
        {
            fmt::print(fmt::style::warn, "Could not convert '{}' to integer\n", arg);
            return 0;
        }
    }
    template <std::same_as<std::string> Value>
    [[nodiscard]] std::string parse_detail(std::string_view arg) const
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

template <> bool ConfigOption<bool>::parse(std::vector<std::string>& args)
/* We assume if this option is set, then it must be true */
{
    for (auto iter = args.begin(); iter < args.end(); ++iter)
    {
        if (auto pos = check_short_name_option(*iter); *iter == "--" + long_name || pos)
        {
            _data = true;
            if (pos && iter->length() > 1) break;
            args.erase(iter, iter + 1);
            break;
        }
    }
    return false;
}

template <typename F, typename ConfigT> bool visit_struct(F&& functor, ConfigT& config)
{
    constexpr size_t struct_elems = boost::pfr::tuple_size<ConfigT>::value;
    return []<std::size_t... I>(F && functor, ConfigT & config, std::index_sequence<I...>)
    {
        return std::max({functor(boost::pfr::get<sizeof...(I) - 1U - I>(config))...});
    }
    (functor, config, std::make_index_sequence<struct_elems>());
}

template <typename ConfigT> ConfigT parse_command_line(int argc, char* argv[])
{
    // TODO: Ensure config does not contain multiple options with same name
    std::vector<std::string> args;
    for (ssize_t i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);

    fmt::print("Got cmd args: {}\n", fmt::join(args, ","));

    ConfigT config;

    auto functor     = [&args](auto& config) { return config.parse(args); };
    bool parse_error = visit_struct(functor, config);
    if (parse_error) fmt::print(fmt::style::warn, "There was an error parsing config options\n");
    if (!args.empty() && !parse_error) // Avoid warning about same error twice
        fmt::print(fmt::style::warn, "There were unparsed arguements: {}\n", fmt::join(args, " "));
    return config;
}