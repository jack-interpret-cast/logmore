#pragma once

#include "types.hpp"

#include <boost/pfr.hpp>
#include <fmt/format.h>

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

    bool parse(std::vector<std::string_view>& args)
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
                    break;
                } else
                {
                    fmt::print(fmt::style::warn, "Missing argument for option'--{}'\n", long_name);
                    return true;
                }
            }
        }
        return false;
    }

    T get() { return _data; }

    const std::string long_name;
    const char short_name{'\0'};
    const std::string help_text;

private:
    template <typename Value> Value parse_detail(std::string_view)
    {
        static_assert(sizeof(Value) == 0 && "There is no parser for type T");
    }
    template <> int parse_detail(std::string_view arg)
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
    template <> std::string parse_detail(std::string_view arg) { return std::string{arg}; }

    T _data;
};

template <> bool ConfigOption<bool>::parse(std::vector<std::string_view>& args)
/* We assume if this option is set, then it must be true */
{
    for (auto iter = args.begin(); iter < args.end(); ++iter)
    {
        if (*iter == "--" + long_name)
        {
            _data = true;
            args.erase(iter, ++iter);
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
    std::vector<std::string_view> args;
    for (ssize_t i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);

    fmt::print("Got cmd args: {}\n", fmt::join(args, ","));

    ConfigT config;

    auto functor = [&args](auto& config) { return config.parse(args); };
    bool parse_error = visit_struct(functor, config);
    if (parse_error)
        fmt::print(fmt::style::warn, "There was an error parsing config options\n");
    if (!args. empty() && !parse_error) // Avoid warning about same error twice
        fmt::print(fmt::style::warn, "There were unparsed arguements: {}\n", fmt::join(args, " "));
    return config;
}