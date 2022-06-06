#pragma once

#include "keys.hpp"

#include <functional>

class Terminal;
class InputBuffer;

class Controller
{
public:
    Controller(Terminal* terminal, InputBuffer* input, std::function<void()> shutdown);

    void key_handler(const Key& key);

private:
    Terminal* _terminal;
    InputBuffer* _input;
    std::function<void()> _shutdown;
};
