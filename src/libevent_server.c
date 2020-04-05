/*
//
// Created by Ben Li on 2020-04-04.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <event.h>

#include "net_util.h"
#include "common.h"


*/
/*
void cmd_connect(int listenfd, short evtype, void *arg)
{
    struct sockaddr_in6 remote_addr;
    socklen_t addrlen = sizeof(remote_addr);
    int sockfd;
    int i;

    if(!(evtype & EV_READ)) {
        ERROR_OUT("Unknown event type in connect callback: 0x%hx", evtype);
        return;
    }

    // Accept and configure incoming connections (up to 10 connections in one go)
    for(i = 0; i < 10; i++) {
        sockfd = accept(listenfd, (struct sockaddr *)&remote_addr, &addrlen);
        if(sockfd < 0) {
            if(errno != EWOULDBLOCK && errno != EAGAIN) {
                ERRNO_OUT("Error accepting an incoming connection");
            }
            break;
        }

        INFO_OUT("Client connected on fd %d", sockfd);

        setup_connection(sockfd, &remote_addr, (struct event_base *)arg);
    }
}
*//*



*/
/* Reads from stdin and create new message. This message enqueues to send queueu. *//*

int handle_read_from_stdin() {
    char read_buffer[DATA_MAXSIZE]; // buffer for stdin
    int received_sized = 0;
    while (read_from_stdin(read_buffer, DATA_MAXSIZE, &received_sized) > 0) {
        // Create new message and enqueue it.
        Message new_message;
        new_message.type = TYPE_AUDIO;
        prepare_message(SERVER_NAME, read_buffer, received_sized, &new_message);

        */
/* enqueue message for all clients *//*

        int i;
        for (i = 0; i < MAX_CLIENTS; ++i) {
            if (connection_list[i].socket != NO_SOCKET) {
                if (peer_add_to_send(&connection_list[i], &new_message) != 0) {
                    log_info("Send buffer was overflowed, we lost this message!\n");
                    continue;
                }
                log_info("New message to send was enqueued right now.\n");
            }
        }
    }

    return 0;
}

int handle_connection(int listenfd, short evtype, void *arg) {
    handle_new_connection(listenfd, )
}
int server_init_event() {

    struct event_base *evloop;
    struct event connect_event;
    struct event stdin_event;

    struct sockaddr_in my_addr;
    int listenfd;
    int writefd;


    sigset_t sigset;

    sigemptyset(&sigset);

    struct sigaction sigInfo = {
            .sa_handler = handle_signal_action,
            .sa_mask = sigset,
            .sa_flags = SA_RESTART
    };

    sigaction(SIGINT, &sigInfo, NULL);
    sigaction(SIGTERM, &sigInfo, NULL);

    log_info("libevent version: %s", event_get_version());
    evloop = event_base_new();

    if (evloop == NULL) {
        log_error("Error initializing event loop");
        return -1;
    }

    server_loop = evloop;
    log_info("libevent is using %s for events.", event_base_get_method(evloop));

    if (start_listen_socket(SERVER_LISTEN_PORT, &listenfd) == -1) {
        log_error("error starting listening port");
        return -1;
    }

    if (set_nonblock(listenfd)) {
        log_error("error setting listeing socket o non-blocking");
        return -1;
    }

    event_set(&connect_event, listenfd, EV_READ |EV_PERSIST, , evloop);
    event_base_set(evloop, &connect_event);
    event_set(&stdin_event, STDIN_FILENO, EV_READ |EV_PERSIST, handle_read_from_stdin, evloop);
    event_base_set(evloop, &stdin_event);
    if(event_add(&connect_event, NULL)) {
        log_error("Error scheduling connection event on the event loop.");
    }
    if (event_add(&stdin_event, NULL)) {
        log_error("Error scheduling stdin event on the event loop.");
    }



    // Start the event loop
    if(event_base_dispatch(evloop)) {
        log_error("Error running event loop.");
    }



    log_info("Server is shutting down.");

    // Clean up and close open connections
    while(socketlist->next != NULL) {
        free_cmdsocket(socketlist->next);
    }

    // Clean up libevent
    if(event_del(&connect_event)) {
        log_error("Error removing connection event from the event loop.");
    }
    event_base_free(evloop);
    if(close(listenfd)) {
        log_error("Error closing listening socket");
    }

    log_info("Goodbye.");

    return 0;

}*/
