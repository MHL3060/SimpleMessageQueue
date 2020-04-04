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
#define HEART_BEAT_TIME_IN_SEC 60

#define TYPE_HEART_BEAT 1
#define TYPE_DATA 2
#define TYPE_AUDIO 3

#define  END_OF_MESSAGE_PAYLOAD_SIZE 7 // = sizeof(END_OF_MESSAGE_PAYLOAD);
//Message maps to Avro Message.


#define AVRO_PAYLOAD_SIZE 12000

typedef struct {
    unsigned char type; //                     1
    unsigned char version; //                  1
    int32_t data_size;     //                  4
    unsigned char header[SENDER_MAXSIZE]; // 128
    unsigned char data[DATA_MAXSIZE];     //8192
                                        //------
                                        //  8326
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
    unsigned char sending_buffer[AVRO_PAYLOAD_SIZE];  //same as the message size
    size_t total_sending_buffer_size;
    size_t current_sending_byte;

    /* The same for the receiving message. */
    unsigned char receiving_buffer[AVRO_PAYLOAD_SIZE]; //same as the message
    int32_t receiving_header;
    unsigned char receiving_tail[7];
    size_t current_receiving_byte;
    size_t total_received_buffer_size;
} peer_t;

typedef struct {
    char * hostName;
    unsigned short port;
    int dataType;
    int logLevel;
} Arguments;


// header is only for the size of the payload.
static const int HEADER_SIZE = sizeof(int32_t);
// end of the message magic number. maybe we should change this one to do the crc instead.
static const unsigned char END_OF_MESSAGE_PAYLOAD[] = { 0xBA, 0xDB, 0xEE, 0xFB, 0xAD, 0xF0, 0x0D};

static const int32_t  MESSAGE_SIZE = sizeof(Message);  //8326

#endif /* COMMON_H */
