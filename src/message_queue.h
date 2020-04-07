//
// Created by Ben Li on 2020-03-19.
//

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include "common.h"
#include "message.h"




int message_create_queue(int queue_size, MessageQueue *queue);

void delete_message_queue(MessageQueue *queue);

int message_enqueue(MessageQueue *queue, Message *message);
/**
 * this method must run inside the pthread_mutex_lock or it cannot guarantee the position.
 * @param queue
 * @param message
 * @return
 */
int message_peak(MessageQueue * queue, Message *message);
int message_dequeue_no_lock(MessageQueue *queue, Message *message);
int message_dequeue(MessageQueue *queue, Message *message);
int message_dequeue_all(MessageQueue *queue);
int message_enqueue_with_retry(MessageQueue *queue, Message *message, int retry_time);


#endif // MESSAGE_QUEUE_H
