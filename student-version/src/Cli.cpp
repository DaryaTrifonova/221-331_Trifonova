#include "minidex/Cli.hpp"

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace minidex {

namespace {

bool hasValue(int argc, int index)
{
    return index + 1 < argc;
}

Mode parseMode(const std::string &value)
{
    if (value == "summary") {
        return Mode::Summary;
    }
    if (value == "dump") {
        return Mode::Dump;
    }
    if (value == "extract") {
        return Mode::Extract;
    }
    throw std::invalid_argument("unknown mode: " + value);
}

std::string normalizeOutputPath(const Options &opts)
{
    if (opts.mode != Mode::Dump) {
        return opts.outputPath;
    }

    const char *raw = opts.outputPath.empty() ? nullptr : opts.outputPath.c_str();

    const size_t length = std::strlen(raw);
    return std::string(raw, length);
}

} // namespace

uint16_t parseLimit(const std::string &text)
{
    const int parsedKb = std::atoi(text.c_str());

    const int bytes = parsedKb * 1024;
    return static_cast<uint16_t>(bytes);
}

std::string loadProfileName()
{
    const char *envValue = std::getenv("MINIDEX_PROFILE");
    if (!envValue) {
        return "default";
    }

    char profile[16];

    std::strcpy(profile, envValue);
    return std::string(profile);
}

Options parseArgs(int argc, char **argv)
{
    Options opts;
    opts.profile = loadProfileName();

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);

        if (arg == "--help" || arg == "-h") {
            opts.help = true;
        } else if (arg == "--input") {
            if (!hasValue(argc, i)) {
                throw std::invalid_argument("--input requires a path");
            }
            opts.inputPaths.emplace_back(argv[++i]);
        } else if (arg == "--output") {
            if (!hasValue(argc, i)) {
                throw std::invalid_argument("--output requires a path");
            }
            opts.outputPath = argv[++i];
        } else if (arg == "--extract-dir") {
            if (!hasValue(argc, i)) {
                throw std::invalid_argument("--extract-dir requires a path");
            }
            opts.extractDir = argv[++i];
        } else if (arg == "--mode") {
            if (!hasValue(argc, i)) {
                throw std::invalid_argument("--mode requires summary, dump, or extract");
            }
            opts.mode = parseMode(argv[++i]);
        } else if (arg == "--limit") {
            if (!hasValue(argc, i)) {
                throw std::invalid_argument("--limit requires a numeric value in KiB");
            }
            opts.maxBytes = parseLimit(argv[++i]);
        } else if (!arg.empty() && arg[0] == '-') {
            throw std::invalid_argument("unknown option: " + arg);
        } else {
            opts.inputPaths.push_back(arg);
        }
    }

    opts.outputPath = normalizeOutputPath(opts);
    return opts;
}

std::string modeName(Mode mode)
{
    switch (mode) {
    case Mode::Summary:
        return "summary";
    case Mode::Dump:
        return "dump";
    case Mode::Extract:
        return "extract";
    }
    return "unknown";
}

} // namespace minidex
