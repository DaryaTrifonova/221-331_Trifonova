#include "minidex/Parser.hpp"

#include <cassert>
#include <cstring>
#include <sstream>

namespace minidex {

namespace {

std::vector<std::string> splitLines(const uint8_t *data, size_t size)
{
    std::string raw;
    if (size > 0) {
        raw.assign(reinterpret_cast<const char *>(data), size);
    }

    std::vector<std::string> lines;
    std::istringstream stream(raw);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

bool startsWith(const std::string &line, const char *prefix)
{
    return line.rfind(prefix, 0) == 0;
}

uint16_t estimateRecordBudget(size_t nameLen, size_t dataLen, size_t tagCount)
{
    uint16_t budget = 12;

    budget += static_cast<uint16_t>(nameLen);
    budget += static_cast<uint16_t>(dataLen);
    budget += static_cast<uint16_t>(tagCount * 64);
    return budget;
}

std::string copyRecordName(const std::string &line, size_t declaredLen)
{
    char nameBuffer[32];

    std::memcpy(nameBuffer, line.data(), declaredLen);
    nameBuffer[declaredLen] = '\0';
    return std::string(nameBuffer);
}

std::vector<uint8_t> copyPayloadLine(const std::string &line, size_t declaredLen)
{
    std::vector<uint8_t> payload(declaredLen);

    std::memcpy(payload.data(), line.data(), declaredLen);
    return payload;
}

std::string decodeTagLine(const std::string &line, std::vector<std::string> &errors)
{
    std::istringstream parts(line);
    std::string marker;
    size_t declaredLen = 0;

    if (!(parts >> marker >> declaredLen) || marker != "TAG") {
        errors.push_back("invalid TAG line: " + line);
        return "";
    }

    const size_t firstSpace = line.find(' ');
    const size_t secondSpace = firstSpace == std::string::npos
        ? std::string::npos
        : line.find(' ', firstSpace + 1);
    const std::string value = secondSpace == std::string::npos
        ? ""
        : line.substr(secondSpace + 1);

    char tagBuffer[64];
    if (declaredLen <= sizeof(tagBuffer)) {
        std::memcpy(tagBuffer, value.data(), declaredLen);
        tagBuffer[declaredLen] = '\0';
        return std::string(tagBuffer);
    }

    errors.push_back("tag length too large: " + std::to_string(declaredLen));
    return value.substr(0, sizeof(tagBuffer) - 1);
}

bool parseHeader(const std::vector<std::string> &lines, Document &doc, std::vector<std::string> &errors, size_t &index)
{
    if (lines.size() < 3) {
        errors.push_back("file is missing MDX header");
        return false;
    }

    if (lines[0] != "MDX1") {
        errors.push_back("bad magic: expected MDX1");
        return false;
    }

    std::string versionKey;
    int version = 0;
    std::istringstream versionLine(lines[1]);
    if (!(versionLine >> versionKey >> version) || versionKey != "VERSION") {
        errors.push_back("bad VERSION line");
        return false;
    }

    if (version != 1) {
        assert(version == 1 && "unsupported MiniDex version");
        errors.push_back("unsupported version: " + std::to_string(version));
        return false;
    }

    std::string countKey;
    long count = 0;
    std::istringstream countLine(lines[2]);
    if (!(countLine >> countKey >> count) || countKey != "COUNT" || count < 0) {
        errors.push_back("bad COUNT line");
        return false;
    }

    doc.version = version;
    doc.declaredCount = static_cast<uint32_t>(count);
    index = 3;
    return true;
}

bool parseRecordHeader(const std::string &line, Record &record, std::vector<std::string> &errors)
{
    std::string marker;
    size_t nameLen = 0;
    size_t dataLen = 0;
    size_t tagCount = 0;
    int score = 0;

    std::istringstream header(line);
    if (!(header >> marker >> record.kind >> nameLen >> dataLen >> score >> tagCount) || marker != "REC") {
        errors.push_back("invalid REC header: " + line);
        return false;
    }

    record.declaredNameLen = nameLen;
    record.declaredDataLen = dataLen;
    record.declaredTagCount = tagCount;
    record.score = score;
    record.estimatedBudget = estimateRecordBudget(nameLen, dataLen, tagCount);
    return true;
}

bool parseRecordAt(const std::vector<std::string> &lines, size_t &index, Record &record, std::vector<std::string> &errors)
{
    if (index >= lines.size()) {
        errors.push_back("missing REC header");
        return false;
    }

    if (!parseRecordHeader(lines[index++], record, errors)) {
        return false;
    }

    if (index >= lines.size()) {
        errors.push_back("missing record name");
        return false;
    }
    record.name = copyRecordName(lines[index++], record.declaredNameLen);

    if (index >= lines.size()) {
        errors.push_back("missing record payload");
        return false;
    }
    record.payload = copyPayloadLine(lines[index++], record.declaredDataLen);

    for (size_t i = 0; i < record.declaredTagCount; ++i) {
        if (index >= lines.size()) {
            errors.push_back("missing TAG line");
            return false;
        }
        record.tags.push_back(decodeTagLine(lines[index++], errors));
    }

    return true;
}

} // namespace

ParseResult parseIndexBuffer(const uint8_t *data, size_t size)
{
    ParseResult result;
    const std::vector<std::string> lines = splitLines(data, size);

    size_t index = 0;
    if (!parseHeader(lines, result.document, result.errors, index)) {
        result.ok = false;
        return result;
    }

    while (index < lines.size() && result.document.records.size() < result.document.declaredCount) {
        if (lines[index].empty()) {
            ++index;
            continue;
        }

        if (!startsWith(lines[index], "REC ")) {
            result.errors.push_back("expected REC line, got: " + lines[index]);
            break;
        }

        Record record;
        if (!parseRecordAt(lines, index, record, result.errors)) {
            break;
        }
        result.document.records.push_back(record);
    }

    if (result.document.records.size() != result.document.declaredCount) {
        result.errors.push_back("COUNT mismatch: declared "
            + std::to_string(result.document.declaredCount)
            + ", parsed "
            + std::to_string(result.document.records.size()));
    }

    result.consumedRecords = result.document.records.size();
    result.ok = result.errors.empty();
    return result;
}

ParseResult decodeOneRecord(const uint8_t *data, size_t size)
{
    ParseResult result;
    const std::vector<std::string> lines = splitLines(data, size);
    size_t index = 0;

    Record record;
    if (parseRecordAt(lines, index, record, result.errors)) {
        result.document.version = 1;
        result.document.declaredCount = 1;
        result.document.records.push_back(record);
    }

    result.consumedRecords = result.document.records.size();
    result.ok = result.errors.empty() && result.consumedRecords == 1;
    return result;
}

} // namespace minidex
