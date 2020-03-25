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
int message_create_queue(int queue_size, MessageQueue *queue) {
    queue->data = calloc(queue_size, sizeof(Message));
    queue->size = queue_size;
    queue->current = 0;
    pthread_mutex_init(&queue->lock, NULL);
    return 0;
}

void delete_message_queue(MessageQueue *queue) {
    free(queue->data);
    queue->data = NULL;
}

int message_enqueue(MessageQueue *queue, Message *message) {
    int result = 0;
    pthread_mutex_lock(&queue->lock);
    if (queue->current == queue->size) {
        result = -1;
    } else {
        memcpy(&queue->data[queue->current], message, sizeof(Message));
        queue->current++;
        result = 0;
    }
    pthread_mutex_unlock(&queue->lock);
    return result;
}
/**
 * this method must run inside the pthread_mutex_lock or it cannot guarantee the position.
 * @param queue
 * @param message
 * @return
 */
int message_peak(MessageQueue * queue, Message *message) {
    if (queue->current == 0) {
        return -1;
    } else {
        memcpy(message, &queue->data[queue->current - 1], sizeof(Message));
        return 0;
    }
}
int message_dequeue(MessageQueue *queue, Message *message) {

    pthread_mutex_lock(&queue->lock);
    int result = message_dequeue_no_lock(queue, message);
    pthread_mutex_unlock(&queue->lock);
    return result;
}

int message_dequeue_no_lock(MessageQueue *queue, Message *message) {
    int result = 0;

    if (queue->current == 0) {
        result = -1;
    } else {
        memcpy(message, &queue->data[queue->current - 1], sizeof(Message));
        queue->current--;
    }
    return result;
}

int message_dequeue_all(MessageQueue *queue) {
    queue->current = 0;
    return 0;
}