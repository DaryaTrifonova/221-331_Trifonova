#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace minidex {

std::vector<uint8_t> readFile(const std::string &path, size_t maxBytes);

} // namespace minidex
