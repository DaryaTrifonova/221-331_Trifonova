#include "minidex/Cli.hpp"
#include "minidex/FileReader.hpp"
#include "minidex/Parser.hpp"
#include "minidex/Pipeline.hpp"

#include <iostream>
#include <stdexcept>

namespace {

void printUsage()
{
    std::cout
        << "MiniDex educational vulnerable parser\n"
        << "\n"
        << "Usage:\n"
        << "  minidex --input FILE [--input FILE...] [--mode summary|dump|extract]\n"
        << "          [--output FILE] [--extract-dir DIR] [--limit KIB]\n"
        << "\n"
        << "Examples:\n"
        << "  minidex --input samples/valid_basic.mdx --mode summary\n"
        << "  minidex --input samples/valid_tags.mdx --mode dump --output report.txt\n";
}

void printMessages(const char *label, const std::vector<std::string> &messages)
{
    for (const std::string &message : messages) {
        std::cerr << label << ": " << message << "\n";
    }
}

} // namespace

int main(int argc, char **argv)
{
    try {
        const minidex::Options opts = minidex::parseArgs(argc, argv);

        if (opts.help || opts.inputPaths.empty()) {
            printUsage();
            return opts.help ? 0 : 1;
        }

        int exitCode = 0;

        for (const std::string &inputPath : opts.inputPaths) {
            const std::vector<uint8_t> data = minidex::readFile(inputPath, opts.maxBytes);
            minidex::ParseResult parsed = minidex::parseIndexBuffer(data.data(), data.size());

            if (!parsed.ok) {
                printMessages("parse error", parsed.errors);
                exitCode = 2;
                continue;
            }

            const minidex::ValidationResult validation = minidex::validateDocument(parsed.document);
            printMessages("warning", validation.warnings);
            if (!validation.ok) {
                printMessages("validation error", validation.errors);
                exitCode = 3;
            }

            const minidex::Report report = minidex::buildReport(parsed.document, opts);

            if (opts.mode == minidex::Mode::Dump) {
                std::vector<std::string> errors;
                if (!minidex::writeReportToPath(opts.outputPath, report, errors)) {
                    printMessages("output error", errors);
                    exitCode = 4;
                }
            } else {
                std::cout << report.text;
            }

            if (opts.mode == minidex::Mode::Extract) {
                std::vector<std::string> errors;
                if (!minidex::writeExtractedPayloads(parsed.document, opts.extractDir, errors)) {
                    printMessages("extract error", errors);
                    exitCode = 5;
                }
            }
        }

        return exitCode;
    } catch (const std::exception &ex) {
        std::cerr << "fatal: " << ex.what() << "\n";
        return 1;
    }
}
