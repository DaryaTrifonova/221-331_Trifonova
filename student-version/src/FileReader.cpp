#include "minidex/FileReader.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace minidex {

std::vector<uint8_t> readFile(const std::string &path, size_t maxBytes)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open input file: " + path);
    }

    std::vector<uint8_t> data;
    data.reserve(std::min<size_t>(maxBytes, 4096));

    char buffer[4096];
    size_t remaining = maxBytes;

    while (input && remaining > 0) {
        const size_t chunk = std::min(remaining, sizeof(buffer));
        input.read(buffer, static_cast<std::streamsize>(chunk));
        const std::streamsize got = input.gcount();
        if (got <= 0) {
            break;
        }
        data.insert(data.end(), buffer, buffer + got);
        remaining -= static_cast<size_t>(got);
    }

    return data;
}

} // namespace minidex
