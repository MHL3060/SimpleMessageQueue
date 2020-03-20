//
// Created by Ben Li on 2020-03-19.
//

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <avro.h>

#include "common.h"
#include "message_queue.h"
#include "peer.h"


void transform(char * msg) {

}
int message_create_queue(int queue_size, message_queue_t *queue) {
    queue->data = calloc(queue_size, sizeof(message_t));
    queue->size = queue_size;
    queue->current = 0;
    pthread_mutex_init(&queue->lock, NULL);
    return 0;
}

void delete_message_queue(message_queue_t *queue) {
    free(queue->data);
    queue->data = NULL;
}

int message_enqueue(message_queue_t *queue, message_t *message) {
    int result = 0;
    pthread_mutex_lock(&queue->lock);
    if (queue->current == queue->size) {
        result = -1;
    } else {
        memcpy(&queue->data[queue->current], message, sizeof(message_t));
        queue->current++;
        result = 0;
    }
    pthread_mutex_unlock(&queue->lock);
    return result;
}

int message_dequeue(message_queue_t *queue, message_t *message) {
    int result = 0;
    pthread_mutex_lock(&queue->lock);
    if (queue->current == 0) {
        result = -1;
    } else {
        memcpy(message, &queue->data[queue->current - 1], sizeof(message_t));
        queue->current--;
    }
    pthread_mutex_unlock(&queue->lock);
    return result;
}

int message_dequeue_all(message_queue_t *queue) {
    queue->current = 0;

    return 0;
}