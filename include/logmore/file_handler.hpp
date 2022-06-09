#pragma once

#include <fire/types.hpp>

#include <fstream>
#include <ranges>
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
    operator bool() { return _buffer.is_open(); }

private:
    std::ifstream _buffer;
};

class InputBuffer
/* Abstraction to make code more testable by allowing us to use a stringstream instead of a
 * ifilestream if we want
 */
{
public:
    using data_t = std::vector<std::string>;
    explicit InputBuffer(std::istream& stream) : _stream(stream){};
    void load_data();
    size_t data_size(); // bytes
    void dump_data();   // print data to console
    size_t num_lines() { return _data.size(); };
    std::ranges::ref_view<data_t> get_range() { return std::views::all(_data); };

private:
    std::istream& _stream;
    data_t _data;
};
