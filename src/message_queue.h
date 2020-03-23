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
/**
 * this method must run inside the pthread_mutex_lock or it cannot guarantee the position.
 * @param queue
 * @param message
 * @return
 */
int message_peak(message_queue_t * queue, message_t *message);
int message_dequeue_no_lock(message_queue_t *queue, message_t *message);
int message_dequeue(message_queue_t *queue, message_t *message);
int message_dequeue_to_byte_stream(message_queue_t *queue, unsigned char * avro_stream_bytes, int * byteSize);
int message_dequeue_all(message_queue_t *queue);



#endif // MESSAGE_QUEUE_H
