#include "minidex/Cli.hpp"
#include "minidex/Parser.hpp"
#include "minidex/Pipeline.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

int g_failures = 0;

void expect(bool condition, const std::string &message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << "\n";
        ++g_failures;
    }
}

minidex::ParseResult parseText(const std::string &text)
{
    return minidex::parseIndexBuffer(reinterpret_cast<const uint8_t *>(text.data()), text.size());
}

bool hasMessageContaining(const std::vector<std::string> &messages, const std::string &needle)
{
    return std::any_of(messages.begin(), messages.end(), [&](const std::string &message) {
        return message.find(needle) != std::string::npos;
    });
}

void validFileParses()
{
    const std::string input =
        "MDX1\n"
        "VERSION 1\n"
        "COUNT 1\n"
        "REC DOC 5 12 10 1\n"
        "alpha\n"
        "hello world!\n"
        "TAG 3 lab\n";

    const minidex::ParseResult parsed = parseText(input);
    expect(parsed.ok, "valid MDX file should parse");
    expect(parsed.document.records.size() == 1, "valid MDX file should contain one record");
    expect(parsed.document.records[0].name == "alpha", "record name should be parsed");
    expect(parsed.document.records[0].payload.size() == 12, "payload length should match DATA_LEN");
    expect(parsed.document.records[0].tags.size() == 1, "tag count should match TAG_COUNT");
}

void validMultiRecordFileParses()
{
    const std::string input =
        "MDX1\n"
        "VERSION 1\n"
        "COUNT 2\n"
        "REC DOC 4 11 20 1\n"
        "beta\n"
        "first entry\n"
        "TAG 4 test\n"
        "REC NOTE 5 11 15 2\n"
        "gamma\n"
        "second item\n"
        "TAG 3 ops\n"
        "TAG 4 2026\n";

    const minidex::ParseResult parsed = parseText(input);
    expect(parsed.ok, "valid multi-record MDX file should parse");
    expect(parsed.document.records.size() == 2, "declared records should be parsed");
    expect(parsed.document.records[1].tags.size() == 2, "second record should keep both tags");
}

void badMagicIsRejected()
{
    const minidex::ParseResult parsed = parseText(
        "NOTMDX\n"
        "VERSION 1\n"
        "COUNT 0\n");

    expect(!parsed.ok, "bad magic should be rejected");
    expect(hasMessageContaining(parsed.errors, "bad magic"), "bad magic should be reported");
}

void badCountIsRejected()
{
    const minidex::ParseResult parsed = parseText(
        "MDX1\n"
        "VERSION 1\n"
        "COUNT -1\n");

    expect(!parsed.ok, "negative COUNT should be rejected");
    expect(hasMessageContaining(parsed.errors, "bad COUNT"), "bad COUNT should be reported");
}

void countMismatchIsRejected()
{
    const minidex::ParseResult parsed = parseText(
        "MDX1\n"
        "VERSION 1\n"
        "COUNT 2\n"
        "REC DOC 5 1 10 0\n"
        "alpha\n"
        "x\n");

    expect(!parsed.ok, "COUNT mismatch should reject the file");
    expect(parsed.consumedRecords == 1, "parser should report the consumed record count");
    expect(hasMessageContaining(parsed.errors, "COUNT mismatch"), "COUNT mismatch should be reported");
}

void maximumSafeNameLengthParses()
{
    const std::string name(31, 'n');
    const minidex::ParseResult parsed = parseText(
        "MDX1\n"
        "VERSION 1\n"
        "COUNT 1\n"
        "REC DOC 31 1 10 0\n"
        + name + "\n"
        "x\n");

    expect(parsed.ok, "31-byte record name should parse");
    expect(parsed.document.records[0].name == name, "31-byte record name should be preserved");
}

void tooLargeTagLengthIsRejected()
{
    const std::string tag(65, 't');
    const minidex::ParseResult parsed = parseText(
        "MDX1\n"
        "VERSION 1\n"
        "COUNT 1\n"
        "REC DOC 5 1 10 1\n"
        "alpha\n"
        "x\n"
        "TAG 65 " + tag + "\n");

    expect(!parsed.ok, "oversized tag length should reject the file");
    expect(hasMessageContaining(parsed.errors, "tag length too large"), "oversized tag should be reported");
}

void validationRejectsEmptyName()
{
    minidex::Document doc;
    doc.version = 1;
    doc.declaredCount = 1;

    minidex::Record record;
    record.score = 10;
    record.payload = { 'x' };
    doc.records.push_back(record);

    const minidex::ValidationResult validation = minidex::validateDocument(doc);
    expect(!validation.ok, "validation should reject records with empty names");
    expect(hasMessageContaining(validation.errors, "empty name"), "empty name should be reported");
}

void validationRejectsOutOfRangeScores()
{
    expect(minidex::validateRecordScore(0), "score 0 should be accepted");
    expect(minidex::validateRecordScore(100), "score 100 should be accepted");
    expect(!minidex::validateRecordScore(-1), "negative score should be rejected");
    expect(!minidex::validateRecordScore(101), "score greater than 100 should be rejected");
}

void extractionPathTraversalIsRejected()
{
    minidex::Document doc;
    minidex::Record record;
    record.name = "../outside";
    doc.records.push_back(record);

    const std::vector<std::string> paths = minidex::buildExtractionPaths(doc, "extract");
    expect(paths.size() == 1, "one extraction path should be built");
    expect(paths[0].find("..") == std::string::npos, "record names should not allow path traversal");
}

void emptyPayloadProducesValidationWarning()
{
    minidex::Document doc;
    doc.version = 1;
    doc.declaredCount = 1;

    minidex::Record record;
    record.name = "empty";
    record.score = 10;
    doc.records.push_back(record);

    const minidex::ValidationResult validation = minidex::validateDocument(doc);
    expect(validation.ok, "empty payload should be a warning, not a validation error");
    expect(hasMessageContaining(validation.warnings, "empty payload"), "empty payload warning should be reported");
}

void binaryPayloadIsPreserved()
{
    std::string input =
        "MDX1\n"
        "VERSION 1\n"
        "COUNT 1\n"
        "REC BIN 3 3 10 0\n"
        "bin\n";
    input.append("a\0b", 3);
    input.push_back('\n');

    const minidex::ParseResult parsed = parseText(input);
    expect(parsed.ok, "payload containing NUL byte should parse");
    expect(parsed.document.records[0].payload.size() == 3, "binary payload size should be preserved");
    expect(parsed.document.records[0].payload[1] == 0, "NUL byte should be preserved in payload");
}

void singleRecordDecodeParses()
{
    const std::string input =
        "REC NOTE 5 4 7 1\n"
        "delta\n"
        "data\n"
        "TAG 3 lab\n";

    const minidex::ParseResult parsed = minidex::decodeOneRecord(
        reinterpret_cast<const uint8_t *>(input.data()),
        input.size());

    expect(parsed.ok, "single record decoder should parse a standalone REC block");
    expect(parsed.document.records.size() == 1, "single record decoder should return one record");
}

void crashNameOverflow()
{
    const std::string name(50, 'A');
    const minidex::ParseResult parsed = parseText(
        "MDX1\n"
        "VERSION 1\n"
        "COUNT 1\n"
        "REC DOC 50 1 10 0\n"
        + name + "\n"
        "x\n");
    std::cerr << "unexpectedly returned from name_overflow, ok=" << parsed.ok << "\n";
}

void crashTagOverflow()
{
    const std::string tag(64, 'T');
    const minidex::ParseResult parsed = parseText(
        "MDX1\n"
        "VERSION 1\n"
        "COUNT 1\n"
        "REC DOC 5 1 10 1\n"
        "alpha\n"
        "x\n"
        "TAG 64 " + tag + "\n");
    std::cerr << "unexpectedly returned from tag_overflow, ok=" << parsed.ok << "\n";
}

void crashEnvOverflow()
{
    const std::string profile(64, 'P');
#if defined(_WIN32)
    _putenv_s("MINIDEX_PROFILE", profile.c_str());
#else
    setenv("MINIDEX_PROFILE", profile.c_str(), 1);
#endif

    const std::string loaded = minidex::loadProfileName();
    std::cerr << "unexpectedly returned from env_overflow, profile=" << loaded << "\n";
}

void crashDumpNullOutput()
{
    char arg0[] = "minidex";
    char arg1[] = "--mode";
    char arg2[] = "dump";
    char *args[] = { arg0, arg1, arg2 };

    const minidex::Options opts = minidex::parseArgs(3, args);
    std::cerr << "unexpectedly returned from dump_null_output, output=" << opts.outputPath << "\n";
}

void crashEmptyReportDivision()
{
    minidex::Document doc;
    doc.version = 1;
    doc.declaredCount = 0;

    minidex::Options opts;
    opts.profile = "crash";
    opts.mode = minidex::Mode::Summary;

    const minidex::Report report = minidex::buildReport(doc, opts);
    std::cerr << "unexpectedly returned from empty_report_divzero, report bytes="
              << report.text.size() << "\n";
}

void ubEmptyChecksum()
{
    const std::vector<uint8_t> payload;
    const uint32_t checksum = minidex::weakChecksum(payload);
    std::cerr << "empty payload checksum=" << checksum << "\n";
}

bool runCrashRepro(const std::string &name)
{
    if (name == "name_overflow") {
        crashNameOverflow();
        return true;
    }
    if (name == "tag_overflow") {
        crashTagOverflow();
        return true;
    }
    if (name == "env_overflow") {
        crashEnvOverflow();
        return true;
    }
    if (name == "dump_null_output") {
        crashDumpNullOutput();
        return true;
    }
    if (name == "empty_report_divzero") {
        crashEmptyReportDivision();
        return true;
    }
    if (name == "empty_checksum_ub") {
        ubEmptyChecksum();
        return true;
    }
    return false;
}

void printCrashReproUsage()
{
    std::cerr
        << "Crash/UB repro modes:\n"
        << "  name_overflow\n"
        << "  tag_overflow\n"
        << "  env_overflow\n"
        << "  dump_null_output\n"
        << "  empty_report_divzero\n"
        << "  empty_checksum_ub\n";
}

} // namespace

int main(int argc, char **argv)
{
    if (argc > 1) {
        const std::string reproName = argv[1];
        if (!runCrashRepro(reproName)) {
            std::cerr << "unknown crash repro: " << reproName << "\n";
            printCrashReproUsage();
            return 2;
        }
        return 0;
    }

    validFileParses();
    validMultiRecordFileParses();
    badMagicIsRejected();
    badCountIsRejected();
    countMismatchIsRejected();
    maximumSafeNameLengthParses();
    tooLargeTagLengthIsRejected();
    validationRejectsEmptyName();
    validationRejectsOutOfRangeScores();
    extractionPathTraversalIsRejected();
    emptyPayloadProducesValidationWarning();
    binaryPayloadIsPreserved();
    singleRecordDecodeParses();

    if (g_failures != 0) {
        std::cerr << g_failures << " test expectation(s) failed\n";
    }
    return g_failures == 0 ? 0 : 1;
}
