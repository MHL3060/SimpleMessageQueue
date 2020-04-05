//
// Created by Ben Li on 2020-04-04.
//
#include "common.h"
#ifndef SERVER_MESSAGE_QUEUE_NET_UTIL_H
#define SERVER_MESSAGE_QUEUE_NET_UTIL_H

int start_listen_socket(int listeningPort, int *listen_sock);
int set_nonblock(int fd);
void handle_signal_action(int sig_number);
int handle_new_connection(peer_t * connection_list, int max_client);

#endif //SERVER_MESSAGE_QUEUE_NET_UTIL_H
