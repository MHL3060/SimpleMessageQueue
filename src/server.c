//
// Created by Ben Li on 2020-03-19.
//


#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <event2/event.h>
#include <termios.h>

#include "common.h"
#include "log.h"
#include "peer.h"
#include "util.h"
#include "message_handler.h"

#define MAX_CLIENTS 10

#define SERVER_NAME "server"

int listen_sock;
peer_t connection_list[MAX_CLIENTS];
char read_buffer[1024]; // buffer for stdin
struct termios old, current;
pthread_t message_producer;

void shutdown_properly(int code);

void handle_signal_action(int sig_number) {
    if (sig_number == SIGINT) {
        log_info("SIGINT was catched!");
        shutdown_properly(EXIT_SUCCESS);
    }
   /* else if (sig_number == SIGPIPE) {
        log_info("SIGPIPE was catched!");
         shutdown_properly(EXIT_SUCCESS);
    }*/
}

void send_heart_beat_messages() {

    for (int i = 0; i < MAX_CLIENTS; i++) {
       peer_enqueue_heart_beat(&connection_list[i], SERVER_NAME, false);
    }

}
int setup_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_signal_action;
    if (sigaction(SIGINT, &sa, 0) != 0) {
        perror("sigaction()");
        return -1;
    }
    /*if (sigaction(SIGPIPE, &sa, 0) != 0) {
        log_info("sigpipe catch");
        perror("sigaction()");
        return -1;
    }*/

    return 0;
}

/* Start listening socket listen_sock. */
int start_listen_socket(int *listen_sock) {
    // Obtain a file descriptor for our "listening" socket.
    *listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*listen_sock < 0) {
        perror("socket");
        return -1;
    }

    int reuse = 1;
    if (setsockopt(*listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
        perror("setsockopt");
        return -1;
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl (INADDR_ANY); //inet_addr(INADDR_ANY);
    my_addr.sin_port = htons(SERVER_LISTEN_PORT);

    if (bind(*listen_sock, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) != 0) {
        perror("bind");
        return -1;
    }

    // start accept client connections
    if (listen(*listen_sock, 10) != 0) {
        perror("listen");
        return -1;
    }
    log_info("Accepting connections on port %d.", (int) SERVER_LISTEN_PORT);

    return 0;
}

void shutdown_properly(int code) {
    int i;

    close(listen_sock);

    for (i = 0; i < MAX_CLIENTS; ++i)
        if (connection_list[i].socket != NO_SOCKET)
            close(connection_list[i].socket);

    log_info("Shutdown server properly.\n");
    resetTermios(&old);
    exit(code);
}

int build_fd_sets(fd_set *read_fds, fd_set *write_fds, fd_set *except_fds) {
    int i;

    FD_ZERO(read_fds);
    FD_SET(STDIN_FILENO, read_fds);

    FD_SET(listen_sock, read_fds);
    for (i = 0; i < MAX_CLIENTS; ++i)
        if (connection_list[i].socket != NO_SOCKET)
            FD_SET(connection_list[i].socket, read_fds);

    FD_ZERO(write_fds);

    for (i = 0; i < MAX_CLIENTS; ++i)
        if (connection_list[i].socket != NO_SOCKET && connection_list[i].send_buffer.front >= 0)
            FD_SET(connection_list[i].socket, write_fds);

    FD_ZERO(except_fds);
    FD_SET(STDIN_FILENO, except_fds);
    FD_SET(listen_sock, except_fds);
    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (connection_list[i].socket != NO_SOCKET) {
            FD_SET(connection_list[i].socket, except_fds);
        }
    }

    return 0;
}

int handle_new_connection() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_len = sizeof(client_addr);
    int new_client_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_len);
    if (new_client_sock < 0) {
        perror("accept()");
        return -1;
    }

    char client_ipv4_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ipv4_str, INET_ADDRSTRLEN);

    log_info("Incoming connection from %s:%d.\n", client_ipv4_str, client_addr.sin_port);

    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (connection_list[i].socket == NO_SOCKET) {
            connection_list[i].socket = new_client_sock;
            connection_list[i].addres = client_addr;
            connection_list[i].current_sending_byte = -1;
            connection_list[i].current_receiving_byte = 0;
            return 0;
        }
    }

    log_info("There is too much connections. Close new connection %s:%d.\n", client_ipv4_str, client_addr.sin_port);
    close(new_client_sock);
    return -1;
}

void close_client_connection(peer_t *client) {
    log_info("Close client socket for %s.\n", peer_get_addres_str(client));

    close(client->socket);
    client->socket = NO_SOCKET;
    message_dequeue_all(&client->send_buffer);
    client->current_sending_byte = -1;
    client->current_receiving_byte = 0;
}

/* Reads from stdin and create new message. This message enqueues to send queueu. */
int handle_read_from_stdin() {
    char read_buffer[DATA_MAXSIZE]; // buffer for stdin
    int received_sized = 0;
    while (read_from_stdin(read_buffer, MAX_SEND_SIZE, &received_sized) > 0) {
        // Create new message and enqueue it.
        Message new_message;
        new_message.type = TYPE_AUDIO;
        prepare_message(SERVER_NAME, read_buffer, received_sized, &new_message);

        /* enqueue message for all clients */
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

int server_init(int *returnCode) {
    initTermios(&old, &current);
    if (setup_signals() != 0)
        return EXIT_FAILURE;

    if (start_listen_socket(&listen_sock) != 0)
        return EXIT_FAILURE;

    /* Set nonblock for stdin. */
    int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flag);

    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
        connection_list[i].socket = NO_SOCKET;
        peer_create(&connection_list[i]);
    }

    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    int high_sock = listen_sock;

    log_info("Waiting for incoming connections.\n");
    // pthread_create(&message_producer, NULL, (void *)&send_heart_beat_messages, NULL);
    while (1) {
        build_fd_sets(&read_fds, &write_fds, &except_fds);

        high_sock = listen_sock;
        for (i = 0; i < MAX_CLIENTS; ++i) {
            if (connection_list[i].socket > high_sock)
                high_sock = connection_list[i].socket;
        }
        struct timeval timeval;
        timeval.tv_sec = 10000000;
        timeval.tv_usec = 0;
        int activity = select(high_sock + 1, &read_fds, &write_fds, &except_fds, &timeval);

        switch (activity) {
            case -1:
                perror("select()");
                shutdown_properly(EXIT_FAILURE);

            case 0:
                // you should never get here
                log_info("select() returns 0.\n");
                shutdown_properly(EXIT_FAILURE);

            default:
                /* All set fds should be checked. */
                if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                    if (handle_read_from_stdin() != 0)
                        shutdown_properly(EXIT_FAILURE);
                }

                if (FD_ISSET(listen_sock, &read_fds)) {
                    handle_new_connection();
                }

                if (FD_ISSET(STDIN_FILENO, &except_fds)) {
                    log_info("except_fds for stdin.\n");
                    shutdown_properly(EXIT_FAILURE);
                }

                if (FD_ISSET(listen_sock, &except_fds)) {
                    log_info("Exception listen socket fd.\n");
                    shutdown_properly(EXIT_FAILURE);
                }

                for (i = 0; i < MAX_CLIENTS; ++i) {
                    if (connection_list[i].socket != NO_SOCKET && FD_ISSET(connection_list[i].socket, &read_fds)) {
                        if (peer_receive_from_peer(&connection_list[i], &handle_message) != 0) {
                            close_client_connection(&connection_list[i]);
                            continue;
                        }
                    }
                    if (connection_list[i].socket != NO_SOCKET && FD_ISSET(connection_list[i].socket, &write_fds)) {
                        log_info("now it is our turn to send");
                        if (peer_send_to_peer(&connection_list[i]) != 0) {
                            close_client_connection(&connection_list[i]);
                            continue;
                        }
                    }

                    if (connection_list[i].socket != NO_SOCKET && FD_ISSET(connection_list[i].socket, &except_fds)) {
                        log_info("Exception client fd.\n");
                        close_client_connection(&connection_list[i]);
                        continue;
                    }
                }

        }

       // log_debug("And we are still waiting for clients' or stdin activity. You can type something to send:\n");
    }

    return 0;
}
