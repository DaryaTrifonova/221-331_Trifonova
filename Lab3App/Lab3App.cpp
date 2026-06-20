#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <tchar.h>

#include "sgx_urts.h"
#include "sgx_tseal.h"
#include "Lab3Enclave_u.h"

#define ENCLAVE_FILE _T("Lab3Enclave.signed.dll")

namespace {
const size_t RecordBufferSize = 256;

void printSgxError(const char* operation, sgx_status_t status)
{
    std::cout << "Error: " << operation << " failed. SGX status = 0x"
              << std::hex << static_cast<unsigned int>(status) << std::dec << "\n";
}

void printRecordByNumber(sgx_enclave_id_t enclaveId)
{
    std::string input;
    int number = 0;

    std::cout << "Enter record number (1-5): ";
    std::getline(std::cin, input);

    std::istringstream inputStream(input);
    if (!(inputStream >> number)) {
        std::cout << "Warning: input value is not a number.\n";
        return;
    }

    char record[RecordBufferSize] = { '\0' };
    int enclaveResult = -1;

    sgx_status_t status = get_record_by_number(
        enclaveId,
        &enclaveResult,
        number,
        record,
        sizeof(record));

    if (status != SGX_SUCCESS) {
        printSgxError("get_record_by_number", status);
        return;
    }

    switch (enclaveResult) {
    case 0:
        std::cout << "Record #" << number << ": " << record << "\n";
        break;
    case 1:
        std::cout << "Warning: record with number " << number << " does not exist.\n";
        break;
    case 2:
        std::cout << "Warning: record buffer is too small. Truncated record #"
                  << number << ": " << record << "\n";
        break;
    default:
        std::cout << "Warning: enclave returned an unexpected result.\n";
        break;
    }
}
}

int main()
{
    sgx_enclave_id_t eid = 0;
    sgx_status_t ret = SGX_SUCCESS;
    sgx_launch_token_t token = { 0 };
    int updated = 0;

    std::cout << "Intel SGX table storage demo\n";
    std::cout << "Protected records are stored inside Lab3Enclave.\n\n";

    ret = sgx_create_enclave(
        ENCLAVE_FILE,
        SGX_DEBUG_FLAG,
        &token,
        &updated,
        &eid,
        NULL);

    if (ret != SGX_SUCCESS) {
        printf("App: error %#x, failed to create enclave.\n", ret);
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return -1;
    }

    printRecordByNumber(eid);

    ret = sgx_destroy_enclave(eid);
    if (ret != SGX_SUCCESS) {
        printSgxError("sgx_destroy_enclave", ret);
        return -1;
    }

    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}
