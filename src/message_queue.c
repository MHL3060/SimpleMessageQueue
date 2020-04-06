//
// Created by Ben Li on 2020-03-19.
//

#include <pthread.h>
#include <stdio.h>
#include <avro.h>

#include "common.h"
#include "message_queue.h"
#include "peer.h"


int message_create_queue(int queue_size, MessageQueue *queue) {
    queue->data = calloc(queue_size, sizeof(Message));
    queue->size = queue_size;
    queue->front = -1;
    queue->rear = -1;
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
    if ((queue->front == 0 && queue->rear == queue->size -1) || (queue->front == queue->rear + 1)) {
        result = -1;
    } else {
        if (queue->front == -1) {
            queue->front = 0;
            queue->rear = 0;
        } else {
            if (queue->rear == queue->size -1) {
                queue->rear = 0;
            }else {
                queue->rear++;
            }
        }
        memcpy(&queue->data[queue->rear], message, sizeof(Message));

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
    if (queue->front == queue->rear) {
        return -1;
    } else {
        memcpy(message, &queue->data[queue->front], sizeof(Message));
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

    if (queue->front == -1) {
        result =  -1;
    } else {
        memcpy(message, &queue->data[queue->front], sizeof(Message));
        if (queue->front == queue->rear) {
            queue->front = -1;
            queue->rear = -1;
        } else {
            if (queue->front == queue->size -1) {
                queue->front = 0;
            } else {
                queue->front++;
            }
        }
    }
    return 0;
}

int message_dequeue_all(MessageQueue *queue) {
    queue->front = -1;
    queue->rear = -1;

    return 0;
}






