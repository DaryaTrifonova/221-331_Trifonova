#pragma once

#include "minidex/Cli.hpp"
#include "minidex/Parser.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace minidex {

struct ValidationResult {
    bool ok = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct Report {
    std::string text;
    int acceptedRecords = 0;
    int averageScore = 0;
};

bool validateRecordScore(int score);
ValidationResult validateDocument(const Document &doc);
uint32_t weakChecksum(const std::vector<uint8_t> &payload);
Report buildReport(const Document &doc, const Options &opts);

bool writeReportToPath(const std::string &path, const Report &report, std::vector<std::string> &errors);
std::vector<std::string> buildExtractionPaths(const Document &doc, const std::string &extractDir);
bool writeExtractedPayloads(const Document &doc, const std::string &extractDir, std::vector<std::string> &errors);

} // namespace minidex
