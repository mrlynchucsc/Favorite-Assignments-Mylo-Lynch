#include "queue.h"
#include <stdlib.h>
#include <semaphore.h>

typedef struct queue {
    void **queue; // pointer to an array that holds elements of the queue
    int size; // int that indicates the number of elements within the queue
    int capacity; // the total number of elements allowed within the queue
    int front; // an integer indicating the front element of the queue
    int rear; // an integer indicating the rear element
    sem_t mutex; // semaphore for mutual exclusivity or locking
    sem_t filled; // semaphore to track the number of filled slots
    sem_t empty; // semaphore to track the number of empty slots
} queue_t;

queue_t *queue_new(int size) {
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    if (!q)
        return NULL;
    q->queue = (void **) calloc(size, sizeof(void *));
    if (!q->queue) {
        free(q);
        return NULL;
    }
    q->size = 0;
    q->capacity = size;
    q->front = 0;
    q->rear = -1;
    sem_init(&q->mutex, 0, 1);
    sem_init(&q->filled, 0, 0);
    sem_init(&q->empty, 0, size);
    return q;
}

void queue_delete(queue_t **q) {
    if (!q || !*q)
        return;
    sem_destroy(&(*q)->mutex);
    sem_destroy(&(*q)->filled);
    sem_destroy(&(*q)->empty);
    free((*q)->queue);
    free(*q);
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem) {
    if (!q)
        return false;
    sem_wait(&q->empty);
    sem_wait(&q->mutex);
    q->rear = (q->rear + 1) % q->capacity;
    q->queue[q->rear] = elem;
    q->size++;
    sem_post(&q->mutex);
    sem_post(&q->filled);
    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    if (!q || !elem)
        return false;
    sem_wait(&q->filled);
    sem_wait(&q->mutex);
    *elem = q->queue[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    sem_post(&q->mutex);
    sem_post(&q->empty);
    return true;
}
