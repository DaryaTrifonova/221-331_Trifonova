#include "minidex/Pipeline.hpp"

#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace minidex {

bool writeReportToPath(const std::string &path, const Report &report, std::vector<std::string> &errors)
{
    std::ofstream output(path);
    if (!output) {
        errors.push_back("cannot open report output: " + path);
        return false;
    }

    output << report.text;
    return static_cast<bool>(output);
}

std::vector<std::string> buildExtractionPaths(const Document &doc, const std::string &extractDir)
{
    std::vector<std::string> paths;
    const std::string base = extractDir.empty() ? "." : extractDir;

    for (const Record &record : doc.records) {
        paths.push_back(base + "/" + record.name + ".bin");
    }

    return paths;
}

bool writeExtractedPayloads(const Document &doc, const std::string &extractDir, std::vector<std::string> &errors)
{
    const std::string base = extractDir.empty() ? "." : extractDir;
    mkdir(base.c_str(), 0755);

    const std::vector<std::string> paths = buildExtractionPaths(doc, base);
    bool ok = true;

    for (size_t i = 0; i < doc.records.size(); ++i) {
        std::ofstream output(paths[i], std::ios::binary);
        if (!output) {
            errors.push_back("cannot open extract output: " + paths[i]);
            ok = false;
            continue;
        }

        const std::vector<uint8_t> &payload = doc.records[i].payload;
        output.write(reinterpret_cast<const char *>(payload.data()), static_cast<std::streamsize>(payload.size()));
        if (!output) {
            errors.push_back("failed writing extract output: " + paths[i]);
            ok = false;
        }
    }

    return ok;
}

} // namespace minidex
