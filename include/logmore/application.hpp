#pragma once

#include <boost/asio/io_service.hpp>

#include <memory>

struct Config;
class FileHandler;
class InputBuffer;
class Terminal;
class Controller;

class Application
{
public:
    explicit Application(Config& config);

    // This method does not return and blocks until the application shuts down
    int start();
    void stop();

private:
    boost::asio::io_service _event_loop{};

    std::unique_ptr<FileHandler> _file;
    std::unique_ptr<InputBuffer> _input;
    std::unique_ptr<Terminal> _terminal;
    std::unique_ptr<Controller> _controller;
};