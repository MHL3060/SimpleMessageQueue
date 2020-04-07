//
// Created by Ben Li on 2020-03-19.
//

#pragma once
#ifndef SERVER_CLIENT_UTIL_H
#define SERVER_CLIENT_UTIL_H
int read_from_stdin(char *read_buffer, size_t max_len, size_t * received_size);
void init_sockaddr(int port, struct sockaddr_in * sockaddr);
        int is_array_equals(unsigned char * first, unsigned char * second, size_t size);
int copy_last_elements(unsigned char * array, size_t total_array_size, unsigned char * last_elements);

void init_termios(struct termios * old, struct termios * current);
void reset_termios(struct termios * old);
int read_file(char * fileName, int dataType,  MessageQueue * queue);
#endif //SERVER_CLIENT_UTIL_H
