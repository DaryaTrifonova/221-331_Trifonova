#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace minidex {

struct Record {
    std::string kind;
    std::string name;
    std::vector<uint8_t> payload;
    std::vector<std::string> tags;
    int score = 0;
    size_t declaredNameLen = 0;
    size_t declaredDataLen = 0;
    size_t declaredTagCount = 0;
    uint16_t estimatedBudget = 0;
};

struct Document {
    int version = 0;
    uint32_t declaredCount = 0;
    std::vector<Record> records;
};

struct ParseResult {
    bool ok = false;
    Document document;
    std::vector<std::string> errors;
    size_t consumedRecords = 0;
};

ParseResult parseIndexBuffer(const uint8_t *data, size_t size);
ParseResult decodeOneRecord(const uint8_t *data, size_t size);

} // namespace minidex
