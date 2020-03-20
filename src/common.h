//
// Created by Ben Li on 2020-03-19.
//

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "log.h"

/* Maximum bytes that can be send() or recv() via net by one call.
 * It's a good idea to test sending one byte by one.
 */


#define SERVER_IPV4_ADDR "127.0.0.1"
#define SERVER_LISTEN_PORT 33235

#define NO_SOCKET -1

#define MAX_SEND_SIZE 1024

/* Size of send queue (messages). */
#define MAX_MESSAGES_BUFFER_SIZE 100

#define SENDER_MAXSIZE 128
#define DATA_MAXSIZE 512
typedef struct {
    char type;
    char version;
    char header[SENDER_MAXSIZE];
    char data[DATA_MAXSIZE];

} message_t;

typedef struct {
    int size;
    message_t *data;
    int current;
    pthread_mutex_t lock;
} message_queue_t;

typedef struct {
    int socket;
    struct sockaddr_in addres;
    /* Messages that waiting for send. */
    message_queue_t send_buffer;

    /* Buffered sending message.
     * In case we doesn't send whole message per one call send().
     * And current_sending_byte is a pointer to the part of data that will be send next call.
     */
    message_t sending_buffer;
    size_t current_sending_byte;

    /* The same for the receiving message. */
    message_t receiving_buffer;
    size_t current_receiving_byte;
} peer_t;

#endif /* COMMON_H */