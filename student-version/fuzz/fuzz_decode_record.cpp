#include "minidex/Parser.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace {

volatile size_t g_fuzzSink = 0;

void consumeParseResult(const minidex::ParseResult &result)
{
    g_fuzzSink ^= result.consumedRecords;
    g_fuzzSink ^= result.errors.size();
    g_fuzzSink ^= result.document.records.size();

    for (const minidex::Record &record : result.document.records) {
        g_fuzzSink ^= record.kind.size();
        g_fuzzSink ^= record.name.size();
        g_fuzzSink ^= record.payload.size();
        g_fuzzSink ^= record.tags.size();
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    const minidex::ParseResult result = minidex::decodeOneRecord(data, size);
    consumeParseResult(result);
    return 0;
}

int main(int argc, char **argv)
{
    std::vector<uint8_t> input;

    if (argc > 1) {
        std::ifstream file(argv[1], std::ios::binary);
        input.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    } else {
        input.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
    }

    return LLVMFuzzerTestOneInput(input.data(), input.size());
}
