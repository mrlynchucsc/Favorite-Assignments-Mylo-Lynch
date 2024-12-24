#include "rwlock.h" // Include the header file for the reader-writer lock definition.
#include <pthread.h> // Include pthread library for threading support.
#include <stdlib.h> // Include standard library for dynamic memory allocation.

// Define the structure for the reader-writer lock.
typedef struct rwlock {
    pthread_mutex_t mutex; // Mutex for protecting critical sections.
    pthread_cond_t read, write; // Condition variables for readers and writers.
    int readers, writers, waiting_writers,
        waiting_readers; // Count of active and waiting readers and writers.
    PRIORITY priority; // Priority type (READERS, WRITERS, N_WAY).
    int n_way, served_readers; // n_way for the N_WAY priority and count of served readers.
} rwlock_t;

// Function to create a new reader-writer lock object.
rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t *rw = (rwlock_t *) malloc(sizeof(rwlock_t)); // Allocate memory for the lock.
    if (rw == NULL) {
        return NULL;
    }
    // Initialize lock properties.
    rw->priority = p; // Set the priority.
    rw->n_way = n; // Set the N_WAY value.

    // Initialize counters to 0.
    rw->readers = rw->writers = rw->waiting_writers = rw->waiting_readers = 0;

    // Initialize the mutex and condition variables.
    pthread_mutex_init(&rw->mutex, NULL);
    pthread_cond_init(&rw->read, NULL);
    pthread_cond_init(&rw->write, NULL);

    return rw; // Return the new lock object.
}

// Function to delete a reader-writer lock object and free its resources.
void rwlock_delete(rwlock_t **rw) {
    if (rw && *rw) { // Check if the lock pointer is valid.
        // Destroy the mutex and condition variables.
        pthread_mutex_destroy(&(*rw)->mutex);
        pthread_cond_destroy(&(*rw)->write);
        pthread_cond_destroy(&(*rw)->read);

        free(*rw); // Free the allocated memory for the lock.
        *rw = NULL; // Set the lock pointer to NULL.
    }
}

// Function for a reader to acquire the lock.
void reader_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex); // Lock the mutex for thread safety.
    rw->waiting_readers++; // Increment the count of waiting readers.
    // Wait condition for readers based on the lock's priority and state.
    for (; (rw->writers > 0) || (rw->priority == WRITERS && rw->waiting_writers > 0)
           || (rw->waiting_writers > 0 && rw->served_readers >= (int) rw->n_way);) {
        pthread_cond_wait(&rw->read, &rw->mutex); // Wait on the reader condition variable.
    }

    rw->served_readers++; // Increment the count of served readers.
    rw->waiting_readers--; // Decrement the count of waiting readers.
    rw->readers++; // Increment the count of active readers.
    pthread_mutex_unlock(&rw->mutex); // Unlock the mutex.
}

// Function for a reader to release the lock.
void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex); // Lock the mutex for thread safety.

    rw->readers--; // Decrement the count of active readers.

    // Signal a waiting writer if conditions are met based on priority.
    if (rw->priority == WRITERS && rw->waiting_writers > 0 && rw->readers == 0) {
        pthread_cond_signal(&rw->write); // Signals one waiting writer.
    } else if (rw->priority == READERS && rw->readers == 0) {
        pthread_cond_signal(&rw->write); // Signals one waiting writer for READERS priority.
    } else {
        pthread_cond_signal(&rw->write); // Signals one waiting writer for N_WAY priority.
    }
    pthread_mutex_unlock(&rw->mutex); // Unlock the mutex.
}

// Function for a writer to acquire the lock.
void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex); // Lock the mutex for thread safety.
    rw->waiting_writers++; // Increment the count of waiting writers.
    // Wait condition for writers based on the lock's priority and state.
    for (; rw->writers > 0 || rw->readers > 0
           || (rw->priority == N_WAY && rw->waiting_readers > 0 && rw->served_readers == 0);) {
        pthread_cond_wait(&rw->write, &rw->mutex); // Wait on the writer condition variable.
    }

    rw->waiting_writers--; // Decrement the count of waiting writers.
    rw->writers++; // Increment the count of active writers.
    pthread_mutex_unlock(&rw->mutex); // Unlock the mutex.
}

// Function for a writer to release the lock.
void writer_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex); // Lock the mutex for thread safety.
    rw->writers--; // Decrement the count of active writers.
    rw->served_readers = 0; // Reset the count of served readers.
    // Signal waiting readers or writers based on the priority and current state.
    if (rw->priority == N_WAY) {
        int i = 0;
        while (i <= rw->n_way) {
            pthread_cond_signal(&rw->read); // Signals n readers for N_WAY priority.
            i++; // Increment counter
        }
        if (rw->waiting_readers == 0) {
            pthread_cond_signal(
                &rw->write); // Signals one waiting writer if no readers are waiting.
        }
    } else if (rw->priority == READERS) {
        if (rw->waiting_readers > 0) {
            pthread_cond_broadcast(
                &rw->read); // Broadcasts to all waiting readers for READERS priority.
        } else {
            pthread_cond_signal(
                &rw->write); // Signals one waiting writer if no readers are waiting.
        }
    } else {
        if (rw->waiting_writers > 0) {
            pthread_cond_signal(&rw->write); // Signals one waiting writer if writers have priority.
        } else {
            pthread_cond_broadcast(&rw->read); // Broadcasts to all waiting readers otherwise.
        }
    }
    pthread_mutex_unlock(&rw->mutex); // Unlock the mutex.
}
