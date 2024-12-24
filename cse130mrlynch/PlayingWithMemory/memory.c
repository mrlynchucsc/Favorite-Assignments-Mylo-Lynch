#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILENAME_LEN 4096
#define MAX_CONTENT_SIZE 10485760 // 10 MB

// Function declarations
void executeGetOperation(const char *filePath);
void executeSetOperation(const char *filePath, const char *dataBuffer,
                         size_t dataSize);
int isValidFilename(const char *filename);
int readLine(int fd, char *buffer, int bufferSize);
int checkForExtraInput();
int checkForMissingNewlineAfterGet();

int main() {
    char operationType[10];
    char filePath[MAX_FILENAME_LEN];
    char dataSizeStr[20];
    char *dataBuffer;

    // Read the operation type
    if (readLine(STDIN_FILENO, operationType, sizeof(operationType)) <= 0) {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }

    // Process based on the operation type
    if (strcmp(operationType, "get") == 0) {
        if (readLine(STDIN_FILENO, filePath, sizeof(filePath)) <= 0 ||
            !isValidFilename(filePath)) {
            fprintf(stderr, "Invalid Command\n");
            exit(1);
        }
        if (checkForMissingNewlineAfterGet()) {
            fprintf(stderr, "Invalid Command\n");
            exit(1);
        }
        executeGetOperation(filePath);
    } else if (strcmp(operationType, "set") == 0) {
        size_t dataSize;
        if (readLine(STDIN_FILENO, filePath, sizeof(filePath)) <= 0 ||
            readLine(STDIN_FILENO, dataSizeStr, sizeof(dataSizeStr)) <= 0 ||
            sscanf(dataSizeStr, "%lu", &dataSize) != 1 ||
            dataSize > MAX_CONTENT_SIZE || !isValidFilename(filePath)) {
            fprintf(stderr, "Invalid Command\n");
            exit(1);
        }
        dataBuffer = (char *)malloc(dataSize);
        if (!dataBuffer) {
            fprintf(stderr, "Operation Failed\n");
            exit(1);
        }
        ssize_t totalBytesRead = 0, bytesRead;
        while ((size_t)totalBytesRead < dataSize) {
            bytesRead = read(STDIN_FILENO, dataBuffer + totalBytesRead,
                             dataSize - totalBytesRead);
            if (bytesRead < 0) {
                if (errno == EINTR)
                    continue; // Interrupted by signal, try again
                fprintf(stderr, "Operation Failed\n");
                free(dataBuffer);
                exit(1);
            } else if (bytesRead == 0)
                break; // End of file or stream closed, stop reading
            totalBytesRead += bytesRead;
        }
        executeSetOperation(filePath, dataBuffer, totalBytesRead);
        free(dataBuffer);
    } else {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }

    return EXIT_SUCCESS;
}

void executeGetOperation(const char *filePath) {
    struct stat path_stat;
    if (stat(filePath, &path_stat) != 0) {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }
    if (S_ISDIR(path_stat.st_mode)) {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }
    int fileDescriptor = open(filePath, O_RDONLY);
    if (fileDescriptor < 0) {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }
    char readBuffer[1024];
    ssize_t numRead;
    while ((numRead = read(fileDescriptor, readBuffer, sizeof(readBuffer))) >
           0) {
        ssize_t totalWritten = 0;
        while (totalWritten < numRead) {
            ssize_t numWritten = write(STDOUT_FILENO, readBuffer + totalWritten,
                                       numRead - totalWritten);
            if (numWritten < 0) {
                if (errno == EINTR)
                    continue; // Interrupted by signal, try again
                fprintf(stderr, "Operation Failed\n");
                close(fileDescriptor);
                exit(1);
            }
            totalWritten += numWritten;
        }
    }
    close(fileDescriptor);
}

void executeSetOperation(const char *filePath, const char *dataBuffer,
                         size_t dataSize) {
    int fileDescriptor = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fileDescriptor < 0) {
        fprintf(stderr, "Operation Failed\n");
        exit(1);
    }
    ssize_t numWritten = write(fileDescriptor, dataBuffer, dataSize);
    if (numWritten != (ssize_t)dataSize) {
        fprintf(stderr, "Operation Failed\n");
        close(fileDescriptor);
        exit(1);
    }
    close(fileDescriptor);
    printf("OK\n");
}

int isValidFilename(const char *filename) {
    if (strlen(filename) >= PATH_MAX)
        return 0;
    for (int i = 0; filename[i] != '\0'; ++i) {
        if (!isascii(filename[i]) || isspace(filename[i]))
            return 0;
    }
    return 1;
}

int readLine(int fd, char *buffer, int bufferSize) {
    int nRead = 0, totalRead = 0;
    char ch;
    while (totalRead < bufferSize - 1) {
        nRead = read(fd, &ch, 1);
        if (nRead == 1) {
            if (ch == '\n')
                break; // End of line
            buffer[totalRead++] = ch;
        } else if (nRead == 0) {
            if (totalRead == 0)
                return 0; // No data read
            else
                break; // End of file
        } else {
            if (errno == EINTR)
                continue; // Interrupted, try again
            return -1;    // Error
        }
    }
    buffer[totalRead] = '\0'; // Null-terminate the string
    return totalRead;
}

int checkForExtraInput() {
    char buffer[1];
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
        return -1; // Error getting flags

    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -1; // Error setting non-blocking
    }

    ssize_t res = read(STDIN_FILENO, buffer, 1);

    if (fcntl(STDIN_FILENO, F_SETFL, flags) == -1) {
        return -1; // Error restoring flags, though you've already read the byte
    }

    if (res > 0) {
        // Extra input detected
        return 1;
    } else if (res == -1 && errno != EAGAIN) {
        // An error other than EAGAIN (no data available) occurred
        return -1;
    }

    // No extra input detected
    return 0;
}

int checkForMissingNewlineAfterGet() {
    char buffer;
    ssize_t bytesRead = read(STDIN_FILENO, &buffer, 1);
    if (bytesRead > 0) {
        // If we read a character and it's not a newline, it's an invalid
        // command
        if (buffer != '\n') {
            return 1; // Missing newline after 'get' command
        }
    }
    // Properly read a newline or EOF without any extra character, indicating a
    // valid command format
    return 0;
}
