#include <sys/socket.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/fcntl.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <regex.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "protocol.h"
#include "queue.h"
#include "rwlock.h"
#include "asgn2_helper_funcs.h"

#define BUFFER_SIZE 2048

#define NUM_REQUESTS 3

#define zero "0"
pthread_mutex_t mutex;

//got permission from Mitchell to use the following code
//for proof, check Slack conversation between
//Mitchell and Mylo Lynch 3/16/24 at 7:45 PM

/**
 * @Files request.h, response.h, connection.h 
 *
 * confirmed I am allowed to use this code with Mitchell. Putting it directly into my httpserver.c
 *
 * @author Andrew Quinn, Mitchell Elliott, and Gurpreet Dhillon.
 */

//response
typedef struct Response Response_t;

extern const Response_t RESPONSE_OK;
extern const Response_t RESPONSE_CREATED;
extern const Response_t RESPONSE_BAD_REQUEST;
extern const Response_t RESPONSE_FORBIDDEN;
extern const Response_t RESPONSE_NOT_FOUND;
extern const Response_t RESPONSE_INTERNAL_SERVER_ERROR;
extern const Response_t RESPONSE_NOT_IMPLEMENTED;
extern const Response_t RESPONSE_VERSION_NOT_SUPPORTED;

uint16_t response_get_code(const Response_t *response);

const char *response_get_message(const Response_t *response);

typedef struct Conn conn_t;

//request
//
typedef struct Request Request_t;

extern const Request_t REQUEST_GET;
extern const Request_t REQUEST_PUT;
extern const Request_t REQUEST_UNSUPPORTED;
extern const Request_t *requests[NUM_REQUESTS];

const char *request_get_str(const Request_t *);

//connection
conn_t *conn_new(int connfd);

// Destructor
void conn_delete(conn_t **conn);

const Response_t *conn_parse(conn_t *conn);

const Request_t *conn_get_request(conn_t *conn);

char *conn_get_uri(conn_t *conn);

char *conn_get_header(conn_t *conn, char *header);

const Response_t *conn_recv_file(conn_t *conn, int fd);

const Response_t *conn_send_file(conn_t *conn, int fd, uint64_t count);

const Response_t *conn_send_response(conn_t *conn, const Response_t *res);

char *conn_str(conn_t *conn);

// Node struct
typedef struct nodeHashTableObj *nodeHashTable;

typedef struct nodeHashTableObj {
    char *URI;
    rwlock_t *hashLock;
    nodeHashTable next;
} nodeHashTableObj;

// Hash table that holds Node
typedef struct hashTableObj *hashTable;

typedef struct hashTableObj {
    nodeHashTable head;
    int length;
} hashTableObj;

// Thread that holds Hash table
typedef struct ThreadObj *Thread;

typedef struct ThreadObj {
    pthread_t thread;
    int id;
    hashTable *hashTable;
    queue_t *queue;
} ThreadObj;

void request_parser(int, hashTable);
void handle_get(conn_t *, hashTable);
void handle_put(conn_t *, hashTable);
void no_coverage(conn_t *);

nodeHashTable HTnode(char *uri, rwlock_t *hashLock) {
    nodeHashTable entry = malloc(sizeof(nodeHashTableObj));

    entry->URI = strdup(uri);
    entry->hashLock = hashLock;
    entry->next = NULL;

    return entry;
}

void attach(hashTable hash, char *URI, rwlock_t *hashLock) {
    if (hash->length == 0) {
        nodeHashTableObj *entry_new = HTnode(URI, hashLock);
        hash->head = entry_new;
        hash->length++;
    } else {
        nodeHashTableObj *this_entry = hash->head;

        while (this_entry->next != NULL) {
            this_entry = this_entry->next;
        }

        nodeHashTableObj *entry_new = HTnode(URI, hashLock);
        this_entry->next = entry_new;
        hash->length++;
    }
}

hashTable newHT(void) {
    hashTable htInstance = malloc(sizeof(hashTableObj));
    if (htInstance == NULL) {
        // Handle memory allocation failure if necessary
        fprintf(stderr, "Error allocating memory for new hash table\n");
        exit(EXIT_FAILURE);
    }
    htInstance->length = 0;
    htInstance->head = NULL;

    return htInstance;
}

void freeHT(hashTable *htInstance) {
    nodeHashTable entry = (*htInstance)->head;
    while (entry != NULL) {
        nodeHashTable entry_n = entry->next;
        free(entry->URI);
        free(entry->hashLock);
        free(entry);
        entry = entry_n;
    }

    free(*htInstance);
    *htInstance = NULL;
}

// Last quarter audited class and went to mitchell's
// section, saved this in my notes since I got warned this
// would be the hardest assignments
rwlock_t *hashLockLookup(nodeHashTable blip, char *URI) {

    if (blip == NULL)
        return NULL;

    nodeHashTable entry = blip;

    while (entry != NULL) {
        if (strcmp(entry->URI, URI) == 0) {
            return entry->hashLock;
        }

        entry = entry->next;
    }

    return NULL;
}

int checkMeth(const char *str) {
    regex_t regex;
    int ret, method = 0;

    // Compile regex for "GET "
    if (regcomp(&regex, "^GET ", REG_EXTENDED) == 0) {
        ret = regexec(&regex, str, 0, NULL, 0);
        if (ret == 0) {
            method = 1; // Match found for "GET "
            regfree(&regex); // Free compiled regex
            return method;
        }
        regfree(&regex); // Free compiled regex if no match
    }

    // Compile regex for "PUT "
    if (regcomp(&regex, "^PUT ", REG_EXTENDED) == 0) {
        ret = regexec(&regex, str, 0, NULL, 0);
        if (ret == 0) {
            method = 2; // Match found for "PUT "
        }
        regfree(&regex); // Free compiled regex
    }

    return method;
}

int checkVers(const char *str) {
    regex_t regex;
    int ret, versionCheck = 0;

    // Compile regex for "HTTP/"
    if (regcomp(&regex, "^HTTP/", REG_EXTENDED) == 0) {
        ret = regexec(&regex, str, 0, NULL, 0);
        if (ret == 0) {
            versionCheck = 1; // Match found for "HTTP/"
        }
        regfree(&regex); // Free compiled regex
    }

    return versionCheck;
}

void no_coverage(conn_t *conn) {
    debug("handling unsupported request");
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
}

bool AlphaNumeric(const char *str) {
    regex_t regex;
    int ret;
    bool match;

    // Compile the regular expression
    ret = regcomp(&regex, "^[a-zA-Z0-9 .:-]+$", REG_EXTENDED);
    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        return false;
    }

    // Execute the regular expression
    ret = regexec(&regex, str, 0, NULL, 0);
    if (!ret) {
        match = true;
    } else if (ret == REG_NOMATCH) {
        match = false;
    } else {
        char errorMsg[256];
        regerror(ret, &regex, errorMsg, sizeof(errorMsg));
        fprintf(stderr, "Regex match failed: %s\n", errorMsg);
        match = false;
    }

    // Free memory allocated to the pattern buffer by regcomp()
    regfree(&regex);

    return match;
}

// Last quarter audited class and went to mitchell's
// section, saved this in my notes since I got warned this
// would be the hardest assignment
void worker_thread(Thread thread) {
    queue_t *queue = thread->queue;

    while (1) {
        uintptr_t connfd = 0;
        queue_pop(queue, (void **) &connfd);
        request_parser((int) connfd, *thread->hashTable);
        close((int) connfd);
    }
}

int main(int argc, char **argv) {
    // Declaration of variables
    char *fin = NULL; // Pointer to store the end character after conversion
    long port; // Variable to store the port number
    int t = 4; // Default number of threads set to 4
    int opt; // Variable to store the option from getopt
    int man = 0; // Flag to check if the thread count was manually specified

    // Loop through command line arguments
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't': // If option is 't', set the thread count and mark it as manually set
            t = atoi(optarg);
            man = 1;
            break;
        default: break; // Ignore unrecognized options
        }
    }

    // Validate the number of arguments
    if (argc < 2 || (man && argc < 4)) {
        fprintf(stderr, "Usage: %s <port> [-t threads]\n",
            argv[0]); // Print usage if arguments are incorrect
        return EXIT_FAILURE; // Exit with a failure status
    }

    char *portStr; // String to store the port number argument

    // Determine which argument contains the port number based on if threads were manually specified
    if (man) {
        portStr = argv[3];
    } else {
        portStr = argv[1];
    }

    // Convert port string to a long integer, storing the end character in 'fin'
    port = strtol(portStr, &fin, 10);

    // Validate the port number
    if (port < 1 || port > 65535 || (fin && *fin != '\0')) {
        fprintf(stderr, "Invalid port number\n"); // Print error if port number is invalid
        return EXIT_FAILURE; // Exit with a failure status
    }

    signal(
        SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent the program from terminating on broken pipes

    Listener_Socket sock; // Declare a variable for the listener socket
    listener_init(&sock, (int) port); // Initialize the listener socket with the specified port

    Thread *threads = malloc(t * sizeof(Thread)); // Allocate memory for the thread pointers
    hashTable ht = newHT(); // Initialize a new hash table
    queue_t *queue = queue_new(t); // Initialize a new queue with the specified number of threads

    // Create worker threads
    for (int i = 0; i < t; i++) {
        threads[i] = malloc(sizeof(ThreadObj)); // Allocate memory for each thread object
        threads[i]->id = i; // Assign an ID to each thread
        threads[i]->hashTable = &ht; // Assign the hash table to each thread
        threads[i]->queue = queue; // Assign the queue to each thread
        pthread_create(&threads[i]->thread, NULL, (void *(*) (void *) ) worker_thread,
            threads[i]); // Create the worker thread
    }

    // Dispatcher loop
    while (1) {
        uintptr_t connfd = listener_accept(&sock); // Accept new connections
        queue_push(
            queue, (void *) connfd); // Push the new connection file descriptor into the queue
    }

    // Code to free allocated resources would go here (not reached in this snippet)
    free(threads);
    freeHT(&ht);

    return EXIT_SUCCESS; // Return success status
}

void request_parser(int connfd, hashTable ht) {

    // Create a new connection object based on the file descriptor from the accepted connection.
    conn_t *conn = conn_new(connfd);

    // Parse the connection to determine the request details and potentially generate a preliminary response (e.g., error response).
    const Response_t *response = conn_parse(conn);

    // Check if the parsing resulted in an immediate response (e.g., due to a parsing error).
    if (response != NULL) {
        // If there's an immediate response, send it back to the client.
        conn_send_response(conn, response);
    } else {
        // Output the connection details for debugging purposes.
        debug("%s", conn_str(conn));
        // Determine the type of HTTP request (GET, PUT, etc.).
        const Request_t *request = conn_get_request(conn);

        // Handle GET requests.
        if (request == &REQUEST_GET) {
            // Extract the URI from the request.
            char *URI = conn_get_uri(conn);

            // Initialize the response pointer to NULL.
            const Response_t *response = NULL;

            // Lock the mutex to ensure thread-safe access to the hash table.
            pthread_mutex_lock(&mutex);

            // Look up the hash lock for the URI. If it doesn't exist, create a new lock and attach it to the hash table.
            rwlock_t *hashLock = hashLockLookup(ht->head, URI);
            if (hashLock == NULL) {
                hashLock = rwlock_new(N_WAY, 1);
                attach(ht, URI, hashLock);
            }
            // Unlock the mutex after modifying the hash table.
            pthread_mutex_unlock(&mutex);

            // Acquire a reader lock for the URI's hash lock.
            reader_lock(hashLock);

            // Check if the URI consists only of alphanumeric characters. If not, prepare a Bad Request response.
            if (!AlphaNumeric(URI)) {
                response = &RESPONSE_BAD_REQUEST;
                char *identification = conn_get_header(conn, "Request-Id");
                // Use "0" as the default request ID if none is provided.
                if (identification == NULL)
                    identification = "0";
                fprintf(stderr, "GET,/%s,400,%s\n", URI, identification);
                goto out; // Jump to the response sending section.
            }

            // Attempt to open the file specified by the URI for reading.
            int fd = open(URI, O_RDONLY);

            // Handle errors encountered while opening the file.
            if (fd < 0) {
                char *reqID = conn_get_header(conn, "Request-Id");
                if (reqID == NULL)
                    reqID = "0";
                // Determine the appropriate error response based on the error code.
                if (errno == ENOENT) {
                    response = &RESPONSE_NOT_FOUND;
                } else if (errno == EACCES || errno == EISDIR) {
                    response = &RESPONSE_FORBIDDEN;
                } else {
                    response = &RESPONSE_INTERNAL_SERVER_ERROR;
                }
                fprintf(stderr, "GET,/%s,%d,%s\n", URI, response_get_code(response), reqID);
                goto out; // Jump to the response sending section.
            }

            // Retrieve file statistics.
            struct stat fileStat;
            fstat(fd, &fileStat);

            // Check if the URI points to a directory. If so, prepare a Forbidden response.
            if (S_ISDIR(fileStat.st_mode)) {
                response = &RESPONSE_FORBIDDEN;
                char *reqID = conn_get_header(conn, "Request-Id");
                if (reqID == NULL)
                    reqID = "0";
                fprintf(stderr, "GET,/%s,403,%s\n", URI, reqID);
                goto out; // Jump to the response sending section.
            }

            // Get the file size from the file statistics.
            int fileSize = (int) fileStat.st_size;

            // Attempt to send the file to the client.
            response = conn_send_file(conn, fd, fileSize);

            // If sending the file succeeds, prepare an OK response.
            if (response == NULL) {
                response = &RESPONSE_OK;
                char *reqID = conn_get_header(conn, "Request-Id");
                if (reqID == NULL)
                    reqID = "0";
                fprintf(stderr, "GET,/%s,200,%s\n", URI, reqID);
            }

            // Release the reader lock and close the file descriptor before returning.
            reader_unlock(hashLock);
            close(fd);
            return;

        out:
            // Send the prepared response to the client and release the reader lock.
            conn_send_response(conn, response);
            reader_unlock(hashLock);

        } else if (request == &REQUEST_PUT) {
            // The process for handling PUT requests is similar to GET requests,
            // with specific differences in file handling and lock types.

            // Extract the URI from the request.
            char *URI = conn_get_uri(conn);
            // Initialize the response pointer to NULL.
            const Response_t *res = NULL;
            // Output debug information indicating a PUT request is being handled.
            debug("handling PUT request for %s", URI);

            // Check if the file already exists.
            bool existed = access(URI, F_OK) == 0;
            debug("%s existed? %d", URI, existed);

            // Lock the mutex to ensure thread-safe access to the hash table.
            pthread_mutex_lock(&mutex);

            // Look up or create a new hash lock for the URI.
            rwlock_t *hashLock = hashLockLookup(ht->head, URI);
            if (hashLock == NULL) {
                hashLock = rwlock_new(N_WAY, 1);
                attach(ht, URI, hashLock);
            }
            // Unlock the mutex after modifying the hash table.
            pthread_mutex_unlock(&mutex);

            // Acquire a writer lock for the URI's hash lock.
            writer_lock(hashLock);

            // Open or create the file specified by the URI for writing.
            int fd = open(URI, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            // Handle file opening errors.
            if (fd < 0) {
                debug("Error opening %s: %d", URI, errno);
                char *reqID = conn_get_header(conn, "Request-Id");
                if (reqID == NULL)
                    reqID = "0";

                // Determine the appropriate error response.
                if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
                    res = &RESPONSE_FORBIDDEN;
                } else {
                    res = &RESPONSE_INTERNAL_SERVER_ERROR;
                }
                fprintf(stderr, "PUT,/%s,403,%s\n", URI, reqID);
                goto finish; // Jump to the response sending section.
            }

            // Attempt to receive and save the file from the client.
            res = conn_recv_file(conn, fd);

            // Determine the appropriate success response based on whether the file existed.
            if (res == NULL) {
                res = existed ? &RESPONSE_OK : &RESPONSE_CREATED;
                char *reqID = conn_get_header(conn, "Request-Id");
                if (reqID == NULL)
                    reqID = "0";
                fprintf(stderr, "PUT,/%s,%d,%s\n", URI, response_get_code(res), reqID);
            }

            // Release the writer lock and close the file descriptor.
            writer_unlock(hashLock);
            close(fd);

        finish:
            // Send the response to the client.
            conn_send_response(conn, res);
        } else {
            // Handle unsupported request types.
            no_coverage(conn);
        }
    }

    // Delete the connection object, freeing its resources.
    conn_delete(&conn);
}
