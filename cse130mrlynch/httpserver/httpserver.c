#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include "asgn2_helper_funcs.h"

#define BUFFER_SIZE 2048

#define OK_MSG          "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n"
#define CREATED_MSG     "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n"
#define BAD_REQUEST_MSG "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n"
#define FORBIDDEN_MSG   "HTTP/1.1 403 Forbidden\r\nContent-Length: 11\r\n\r\nForbidden\n"
#define NOT_FOUND_MSG   "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n"
#define NOT_IMPLEMENTED_MSG                                                                        \
    "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n"
#define VERSION_NOT_SUPPORTED_MSG                                                                  \
    "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion Not Supported\n"
#define DEFAULT_MSG                                                                                \
    "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 26\r\n\r\nInternal Server Error\n"

int get_content_length(int client_socket);

bool is_valid_method(const char *method) {
    return (strncmp(method, "GET ", 4) == 0) || (strncmp(method, "PUT ", 4) == 0);
}

bool is_valid_version(const char *version) {
    return strncmp(version, "HTTP/", 5) == 0
           && (version[5] == '1' && version[6] == '.' && (version[7] == '0' || version[7] == '1'));
}

bool is_valid_filename(const char *filename) {
    for (const char *p = filename; *p; ++p) {
        if (!isalnum(*p) && *p != ' ' && *p != '.' && *p != '-') {
            return false;
        }
    }
    return true;
}

void send_response(int client_socket, int status_code) {
    char *message = NULL;

    switch (status_code) {
    case 200: message = OK_MSG; break;
    case 201: message = CREATED_MSG; break;
    case 400: message = BAD_REQUEST_MSG; break;
    case 403: message = FORBIDDEN_MSG; break;
    case 404: message = NOT_FOUND_MSG; break;
    case 501: message = NOT_IMPLEMENTED_MSG; break;
    case 505: message = VERSION_NOT_SUPPORTED_MSG; break;
    default: message = DEFAULT_MSG; break;
    }

    write_n_bytes(client_socket, message, strlen(message));
    close(client_socket);
}

bool read_request_line(int client_socket, char *method, char *filename, char *version) {
    char buffer[BUFFER_SIZE];
    int i = 0;

    while (i < BUFFER_SIZE - 1) {
        ssize_t bytes_read = read_n_bytes(client_socket, &buffer[i], 1);

        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue; // Interrupted system call, try again
            } else {
                return false; // Error reading from socket
            }
        }

        if (buffer[i] == '\n') {
            buffer[i] = '\0'; // Null-terminate the line
            break;
        }

        if (bytes_read == 0 || i == BUFFER_SIZE - 1) {
            return false;
        }

        i++;
    }

    return sscanf(buffer, "%s %s %s", method, filename, version) == 3;
}

void process_get_request(int client_socket, const char *filename) {
    int file_descriptor = open(filename, O_RDONLY);
    if (file_descriptor == -1) {
        send_response(client_socket, 404); // File not found
        return;
    }

    struct stat file_stat;
    if (fstat(file_descriptor, &file_stat) == -1) {
        send_response(client_socket, 500); // Server error accessing file
        close(file_descriptor);
        return;
    }

    if (S_ISDIR(file_stat.st_mode)) {
        send_response(client_socket, 403); // Forbidden (directory)
        close(file_descriptor);
        return;
    }

    send_response(client_socket, 200); // OK
    pass_n_bytes(file_descriptor, client_socket, file_stat.st_size);
    close(file_descriptor);
}

int get_content_length(int client_socket) {
    char headerField[BUFFER_SIZE];
    int headerIter = 0;
    bool headerEndFound = false;

    while (!headerEndFound) {
        if (read_n_bytes(client_socket, &headerField[headerIter], 1) < 1) {
            send_response(client_socket, 400); // Bad Request
            return -1;
        }
        if (headerIter >= 3 && headerField[headerIter - 3] == '\r'
            && headerField[headerIter - 2] == '\n' && headerField[headerIter - 1] == '\r'
            && headerField[headerIter] == '\n') {
            headerEndFound = true;
        }
        headerIter++;
    }

    headerField[headerIter] = '\0'; // Null-terminate the header field

    // Find Content-Length header
    const char *searchHeader = "Content-Length:";
    char *headerStart = strstr(headerField, searchHeader);
    if (!headerStart) {
        send_response(client_socket, 400); // Bad Request
        return -1;
    }
    headerStart += strlen(searchHeader);

    char *headerEnd = strstr(headerStart, "\r\n");
    if (!headerEnd) {
        send_response(client_socket, 400); // Bad Request
        return -1;
    }

    *headerEnd = '\0'; // Null-terminate the Content-Length value
    int contentLength = atoi(headerStart);
    return contentLength;
}

void process_put_request(int client_socket, const char *filename) {
    int contentLength = get_content_length(client_socket);
    if (contentLength <= 0) {
        // Appropriate response already sent by get_content_length
        return;
    }

    bool fileWasCreated = (access(filename, F_OK) == -1);
    int file_descriptor = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (file_descriptor < 0) {
        send_response(client_socket, 500); // Internal Server Error
        return;
    }

    if (pass_n_bytes(client_socket, file_descriptor, contentLength) < contentLength) {
        send_response(client_socket, 500); // Internal Server Error
        close(file_descriptor);
        return;
    }

    close(file_descriptor);
    send_response(client_socket, fileWasCreated ? 201 : 200); // Created or OK
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        return 1;
    }

    int port = strtol(argv[1], NULL, 10);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid Port: %s\n", argv[1]);
        return 1;
    }

    Listener_Socket server_socket;
    if (listener_init(&server_socket, port) < 0) {
        fprintf(stderr, "Failed to initialize server socket on port %d\n", port);
        return 1;
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        int client_socket = listener_accept(&server_socket);
        if (client_socket < 0) {
            fprintf(stderr, "Failed to accept client connection\n");
            continue;
        }

        char method[10], filename[PATH_MAX], version[10];
        if (!read_request_line(client_socket, method, filename, version) || !is_valid_method(method)
            || !is_valid_version(version) || !is_valid_filename(filename)) {
            send_response(client_socket, 400); // Bad Request
            continue;
        }

        if (strcmp(method, "GET") == 0) {
            process_get_request(client_socket, filename);
        } else if (strcmp(method, "PUT") == 0) {
            process_put_request(client_socket, filename);
        } else {
            send_response(client_socket, 501); // Not Implemented
        }
    }

    return 0;
}
