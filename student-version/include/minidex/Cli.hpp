#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace minidex {

enum class Mode {
    Summary,
    Dump,
    Extract
};

struct Options {
    std::vector<std::string> inputPaths;
    std::string outputPath;
    std::string extractDir = "extract";
    uint16_t maxBytes = 8192;
    Mode mode = Mode::Summary;
    std::string profile = "default";
    bool help = false;
};

Options parseArgs(int argc, char **argv);
uint16_t parseLimit(const std::string &text);
std::string loadProfileName();
std::string modeName(Mode mode);

} // namespace minidex
