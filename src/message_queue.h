//
// Created by Ben Li on 2020-03-19.
//

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include "common.h"
#include "message.h"




int message_create_queue(int queue_size, message_queue_t *queue);

void delete_message_queue(message_queue_t *queue);

int message_enqueue(message_queue_t *queue, message_t *message);

int message_dequeue(message_queue_t *queue, message_t *message);

int message_dequeue_all(message_queue_t *queue);



#endif // MESSAGE_QUEUE_H
