#include "file_handler.hpp"

#include <filesystem>

FileHandler::FileHandler(std::string_view filename)
{
    namespace fs  = std::filesystem;
    fs::path path = filename;
    if (!fs::exists(path))
    {
        fmt::print(fmt::style::warn, "The path: {} was not found\n", path.string());
        return;
    }
    if (!fs::is_regular_file(path))
    {
        fmt::print(fmt::style::warn, "The path: {} is not a regular file\n", path.string());
        return;
    }
    _buffer.open(path);
    if (!_buffer.is_open())
    {
        fmt::print(fmt::style::warn, "Unable to open: {}\n", path.string());
        return;
    }
}

FileHandler::~FileHandler() { _buffer.close(); }

std::ifstream& FileHandler::get_buffer() { return _buffer; }

void InputBuffer::load_data()
{
    while (!_stream.eof())
    {
        std::getline(_stream, _data.emplace_back());
    }
    // eof flag not set until trying to read last line + 1
    _data.pop_back();
}

ssize_t InputBuffer::data_size()
{
    auto start = _stream.tellg();
    _stream.seekg(0, std::ios::end);
    auto end = _stream.tellg();
    _stream.seekg(start, std::ios::beg);
    return end - start;
}
void InputBuffer::dump_data()
{
    fmt::print("Got {}  lines\n", _data.size());
    for (const auto& line : _data)
        fmt::print("{}\n", line);
}
