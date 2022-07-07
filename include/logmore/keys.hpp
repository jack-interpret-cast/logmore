#pragma once

#include <fmt/format.h>

#define MAGIC_ENUM_RANGE_MAX 360
#include <magic_enum.hpp>

#include <ostream>
#include <tuple>

enum class KeySpecials : int32_t {
    none = 0,
    ctrl = -1,
    // Below keys map directly to values from ncurses
    // Above must be negative to prevent clashes
    enter     = 13,
    esc       = 27,
    up        = 259,
    down      = 258,
    left      = 260,
    right     = 261,
    home      = 262,
    end       = 360,
    backspace = 263,
    del       = 330,
    pgdn      = 338,
    pgup      = 339,
};

struct Key
{
    using key_t = unsigned int;
    explicit Key(key_t curses_key) : _value(curses_key)
    {
        const key_t alpha_len           = 26;
        const key_t control_chars_start = 1;
        //        const key_t alt_chars_start   = 26;
        //        const key_t lower_chars_start = 97;
        //        const key_t upper_chars_start = 65;
        if (auto key = magic_enum::enum_cast<KeySpecials>(curses_key); key.has_value())
        {
            _special = key.value();
        } else if (curses_key < control_chars_start + alpha_len)
        {
            _letter  = static_cast<char>(curses_key + 'a' - 1);
            _special = KeySpecials::ctrl;
        } else
        {
            _letter = static_cast<char>(curses_key);
        }
    };

    // marking this consteval means this constructor can only be used at compile time
    // And also causes clang frontend to crash :(
    explicit consteval Key(char letter, KeySpecials special = KeySpecials::none)
        : _letter{letter}
        , _special{special}
    {
    }

    constexpr bool operator==(const Key& rhs) const
    {
        return std::tie(_letter, _special) == std::tie(rhs._letter, rhs._special);
    }

    char _letter{'\0'};
    KeySpecials _special{KeySpecials::none};
    key_t _value{0}; // value returned by ncurses
};

template <>
struct fmt::formatter<Key>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(Key const& key, FormatContext& ctx)
    {
        if (std::isprint(key._letter))
            return fmt::format_to(ctx.out(), "charint: {}, modifier: {}, value: {}, char: {}",
                                  (int)key._letter, magic_enum::enum_name(key._special), key._value,
                                  key._letter);
        else
            return fmt::format_to(ctx.out(), "charint: {}, modifier: {}, value: {}",
                                  (int)key._letter, magic_enum::enum_name(key._special),
                                  key._value);
    }
};