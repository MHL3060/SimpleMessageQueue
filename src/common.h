//
// Created by Ben Li on 2020-03-19.
//
#pragma  once

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
#define DATA_MAXSIZE 8192

static const int HEADER_SIZE = sizeof(int32_t);

static const unsigned char END_OF_MESSAGE_PAYLOAD[] = { 0xBA, 0xDB, 0xEE, 0xFB, 0xAD, 0xF0, 0x0D};
static const int END_OF_MESSAGE_PAYLOAD_SIZE = sizeof(END_OF_MESSAGE_PAYLOAD);

typedef struct {
    int type;
    char version;
    unsigned char header[SENDER_MAXSIZE];
    unsigned char data[DATA_MAXSIZE];
    int data_size;

} Message;

typedef struct {
    int size;
    Message *data;
    int current;
    pthread_mutex_t lock;
} MessageQueue;

typedef struct {
    int socket;
    struct sockaddr_in addres;
    /* Messages that waiting for send. */
    MessageQueue send_buffer;

    /* Buffered sending message.
     * In case we doesn't send whole message per one call send().
     * And current_sending_byte is a pointer to the part of data that will be send next call.
     */
    unsigned char * sending_buffer[DATA_MAXSIZE];
    size_t total_sending_buffer_size;
    size_t current_sending_byte;

    /* The same for the receiving message. */
    unsigned char receiving_buffer[DATA_MAXSIZE];
    int32_t receiving_header;
    unsigned char receiving_tail[END_OF_MESSAGE_PAYLOAD_SIZE];
    size_t current_receiving_byte;
    size_t total_received_buffer_size;
} peer_t;

typedef struct {
    char * hostName;
    unsigned short port;
    int dataType;
    int logLevel;
} Arguments;



#endif /* COMMON_H */