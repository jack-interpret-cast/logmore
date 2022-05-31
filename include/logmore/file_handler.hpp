#pragma once

#include "types.hpp"

#include <fstream>
#include <vector>

class FileHandler;
class InputBuffer;

class FileHandler
{
public:
    explicit FileHandler(std::string_view filename);
    ~FileHandler();
    FileHandler(const FileHandler&)            = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    FileHandler(FileHandler&& fh)              = delete;

    std::ifstream& get_buffer();
    explicit operator bool() { return _buffer.is_open(); }

private:
    std::ifstream _buffer;
};

class InputBuffer
/* Abstraction to make code more testable by allowing us to use a stringstream instead of a
 * ifilestream if we want */
{
public:
    explicit InputBuffer(std::istream& stream) : _stream(stream){};
    void load_data();
    size_t data_size();
    void dump_data();
    size_t num_lines() { return _data.size(); }

private:
    std::istream& _stream;
    std::vector<std::string> _data;
};
