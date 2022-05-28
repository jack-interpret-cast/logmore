#pragma once

#include <fmt/color.h>

namespace fmt::style {
constexpr auto warn = fmt::emphasis::bold | fg(fmt::color::red);
}