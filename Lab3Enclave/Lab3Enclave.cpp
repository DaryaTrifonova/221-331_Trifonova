#include "Lab3Enclave_t.h"

#include "sgx_trts.h"

#include <stddef.h>
#include <string.h>

namespace {
const char* protectedData[] = {
    "login=admin; password=admin-2026",
    "login=student; password=lab3-sgx",
    "login=operator; password=storage-key",
    "login=auditor; password=readonly",
    "login=test; password=test-password"
};

const size_t protectedDataCount = sizeof(protectedData) / sizeof(protectedData[0]);

int copyRecordToOutput(const char* source, char* destination, size_t destinationSize)
{
    const size_t sourceLength = strlen(source);

    if (sourceLength + 1 > destinationSize) {
        if (destinationSize > 1) {
            memcpy(destination, source, destinationSize - 1);
            destination[destinationSize - 1] = '\0';
        }

        return 2;
    }

    memcpy(destination, source, sourceLength + 1);
    return 0;
}
}

int get_record_by_number(int number, char* record, size_t record_size)
{
    if (record == nullptr || record_size == 0) {
        return -1;
    }

    record[0] = '\0';

    if (number < 1 || number > static_cast<int>(protectedDataCount)) {
        return 1;
    }

    return copyRecordToOutput(protectedData[number - 1], record, record_size);
}
