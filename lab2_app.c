#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LAB2_FILE_SIZE 256u
#define LAB2_HEX_PREVIEW_BYTES 64u
#define LAB2_DEFAULT_FILE "test.lab2ext"

static int IsMode(const char* value)
{
    return (_stricmp(value, "read") == 0) ||
           (_stricmp(value, "write") == 0) ||
           (_stricmp(value, "rw") == 0);
}

static void FillPlaintext(unsigned char* buffer, size_t length)
{
    const char* text =
        "Lab2 transparent encryption demo\r\n"
        "This block is written by the test app in one fwrite() call.\r\n"
        "Target extension: .lab2ext\r\n";
    size_t textLength = strlen(text);

    memset(buffer, ' ', length);

    if (textLength > length - 2u) {
        textLength = length - 2u;
    }

    memcpy(buffer, text, textLength);
    buffer[length - 2u] = '\r';
    buffer[length - 1u] = '\n';
}

static void PrintTextView(const unsigned char* buffer, size_t length)
{
    size_t i;

    printf("Text view:\n");

    for (i = 0; i < length; i++) {
        unsigned char c = buffer[i];

        if ((c == '\r') || (c == '\n') || (c == '\t') || isprint(c)) {
            putchar((int)c);
        } else {
            putchar('.');
        }
    }

    printf("\n");
}

static void PrintHexPreview(const unsigned char* buffer, size_t length)
{
    size_t i;
    size_t limit = (length < LAB2_HEX_PREVIEW_BYTES) ? length : LAB2_HEX_PREVIEW_BYTES;

    printf("Hex preview (%zu bytes):", limit);

    for (i = 0; i < limit; i++) {
        if ((i % 16u) == 0u) {
            printf("\n  ");
        }

        printf("%02X ", buffer[i]);
    }

    printf("\n");
}

static int ReadAndPrintFile(const char* path)
{
    FILE* file = NULL;
    errno_t openError;
    long fileSize;
    size_t bytesRead;
    unsigned char buffer[LAB2_FILE_SIZE + 1u];

    openError = fopen_s(&file, path, "rb");

    if (openError != 0) {
        printf("Read: cannot open '%s' (errno=%d).\n", path, openError);
        return 1;
    }

    setvbuf(file, NULL, _IONBF, 0);

    if (fseek(file, 0, SEEK_END) != 0) {
        printf("Read: fseek(SEEK_END) failed.\n");
        fclose(file);
        return 1;
    }

    fileSize = ftell(file);

    if (fileSize < 0) {
        printf("Read: ftell() failed.\n");
        fclose(file);
        return 1;
    }

    if ((size_t)fileSize > LAB2_FILE_SIZE) {
        printf("Read: file is too large for this lab app (%ld > %u bytes).\n",
               fileSize,
               LAB2_FILE_SIZE);
        fclose(file);
        return 1;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        printf("Read: fseek(SEEK_SET) failed.\n");
        fclose(file);
        return 1;
    }

    memset(buffer, 0, sizeof(buffer));
    bytesRead = fread_s(buffer, sizeof(buffer), 1u, (size_t)fileSize, file);

    if (bytesRead != (size_t)fileSize) {
        printf("Read: fread_s read %zu of %ld bytes.\n", bytesRead, fileSize);
        fclose(file);
        return 1;
    }

    fclose(file);

    printf("\nRead '%s': %zu bytes\n", path, bytesRead);
    PrintTextView(buffer, bytesRead);
    PrintHexPreview(buffer, bytesRead);

    return 0;
}

static int WriteFixedFile(const char* path)
{
    FILE* file = NULL;
    errno_t openError;
    size_t bytesWritten;
    unsigned char buffer[LAB2_FILE_SIZE];

    FillPlaintext(buffer, sizeof(buffer));

    openError = fopen_s(&file, path, "wb");

    if (openError != 0) {
        printf("Write: cannot open '%s' (errno=%d).\n", path, openError);
        return 1;
    }

    setvbuf(file, NULL, _IONBF, 0);

    bytesWritten = fwrite(buffer, 1u, sizeof(buffer), file);

    if (bytesWritten != sizeof(buffer)) {
        printf("Write: fwrite wrote %zu of %zu bytes.\n", bytesWritten, sizeof(buffer));
        fclose(file);
        return 1;
    }

    if (fflush(file) != 0) {
        printf("Write: fflush failed.\n");
        fclose(file);
        return 1;
    }

    fclose(file);

    printf("\nWrote '%s': %zu bytes\n", path, bytesWritten);
    return 0;
}

static void PrintUsage(const char* exeName)
{
    printf("Usage: %s [file.lab2ext] [read|write|rw]\n", exeName);
    printf("Default: %s %s rw\n", exeName, LAB2_DEFAULT_FILE);
}

int main(int argc, char** argv)
{
    const char* path = LAB2_DEFAULT_FILE;
    const char* mode = "rw";
    int result = 0;

    if (argc > 3) {
        PrintUsage(argv[0]);
        return 2;
    }

    if (argc >= 2) {
        if (IsMode(argv[1])) {
            mode = argv[1];
        } else {
            path = argv[1];
        }
    }

    if (argc == 3) {
        if (IsMode(argv[2])) {
            mode = argv[2];
        } else {
            path = argv[2];
        }
    }

    if (!IsMode(mode)) {
        PrintUsage(argv[0]);
        return 2;
    }

    if (_stricmp(mode, "read") == 0) {
        return ReadAndPrintFile(path);
    }

    if (_stricmp(mode, "write") == 0) {
        return WriteFixedFile(path);
    }

    (void)ReadAndPrintFile(path);

    result = WriteFixedFile(path);

    if (result == 0) {
        result = ReadAndPrintFile(path);
    }

    return result;
}
