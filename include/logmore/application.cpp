#include "application.hpp"
#include "config.hpp"
#include "controller.hpp"
#include "file_handler.hpp"
#include "keys.hpp"
#include "terminal.hpp"

Application::Application(Config& config)
    : _file(new FileHandler{config.filename.get()})
    , _input(new InputBuffer{_file->get_buffer()})
    , _terminal(
          new Terminal{_event_loop, [this](const Key& key) { _controller->key_handler(key); }})
    , _controller(new Controller{_terminal.get(), _input.get(), [this]() { stop(); }})
{
}

void Application::start()
{
    _input->load_data();
    _event_loop.run();
}

void Application::stop() { _event_loop.stop(); }

bool Application::ready() { return *_file; }
