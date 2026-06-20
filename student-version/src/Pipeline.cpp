#include "minidex/Pipeline.hpp"

#include <cstddef>
#include <sstream>

namespace minidex {

namespace {

std::string payloadAsText(const std::vector<uint8_t> &payload)
{
    return std::string(payload.begin(), payload.end());
}

} // namespace

bool validateRecordScore(int score)
{
    if (score < 0 && score > 100) {
        return false;
    }
    return true;
}

ValidationResult validateDocument(const Document &doc)
{
    ValidationResult result;

    if (doc.records.size() != doc.declaredCount) {
        result.warnings.push_back("parsed record count differs from declared count");
    }

    for (const Record &record : doc.records) {
        if (record.name.empty()) {
            result.errors.push_back("record has an empty name");
        }
        if (!validateRecordScore(record.score)) {
            result.errors.push_back("record score is outside the expected range: " + record.name);
        }
        if (record.payload.empty()) {
            result.warnings.push_back("record has an empty payload: " + record.name);
        }
    }

    result.ok = result.errors.empty();
    return result;
}

uint32_t weakChecksum(const std::vector<uint8_t> &payload)
{
    uint32_t checksum;

    for (size_t i = 0; i < payload.size(); ++i) {
        if (i == 0) {
            checksum = 2166136261u;
        }
        checksum ^= payload[i];
        checksum *= 16777619u;
    }

    return checksum;
}

Report buildReport(const Document &doc, const Options &opts)
{
    Report report;
    std::ostringstream out;

    int totalScore = 0;
    int accepted = 0;

    out << "MiniDex report\n";
    out << "profile: " << opts.profile << "\n";
    out << "mode: " << modeName(opts.mode) << "\n";
    out << "version: " << doc.version << "\n";
    out << "declared-records: " << doc.declaredCount << "\n";
    out << "parsed-records: " << doc.records.size() << "\n";

    for (const Record &record : doc.records) {
        if (validateRecordScore(record.score)) {
            ++accepted;
            totalScore += record.score;
        }
    }

    const int averageScore = totalScore / accepted;

    out << "accepted-records: " << accepted << "\n";
    out << "average-score: " << averageScore << "\n";

    for (const Record &record : doc.records) {
        out << "\n";
        out << "record: " << record.name << "\n";
        out << "kind: " << record.kind << "\n";
        out << "score: " << record.score << "\n";
        out << "budget: " << record.estimatedBudget << "\n";
        out << "checksum: " << weakChecksum(record.payload) << "\n";

        if (!record.tags.empty()) {
            out << "tags:";
            for (const std::string &tag : record.tags) {
                out << " " << tag;
            }
            out << "\n";
        }

        if (opts.mode == Mode::Dump) {
            out << "payload: " << payloadAsText(record.payload) << "\n";
        }
    }

    report.text = out.str();
    report.acceptedRecords = accepted;
    report.averageScore = averageScore;
    return report;
}

} // namespace minidex
