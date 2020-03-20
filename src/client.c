//
// Created by Ben Li on 2020-03-19.
//

// Simple example of client.
// Client prints received messages to stdout and sends from stdin.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

#include "common.h"
#include "log.h"

peer_t server;
pthread_t message_producer;
void shutdown_properly(int code);

void handle_signal_action(int sig_number) {
    if (sig_number == SIGINT) {
        log_debug("SIGINT was catched!\n");
        shutdown_properly(EXIT_SUCCESS);
    } else if (sig_number == SIGPIPE) {
        log_debug("SIGPIPE was catched!\n");
        shutdown_properly(EXIT_SUCCESS);
    }
}

int setup_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_signal_action;
    if (sigaction(SIGINT, &sa, 0) != 0) {
        perror("sigaction()");
        return -1;
    }
    if (sigaction(SIGPIPE, &sa, 0) != 0) {
        perror("sigaction()");
        return -1;
    }

    return 0;
}

int get_client_name(int argc, char **argv, char *client_name) {
    if (argc > 1)
        strcpy(client_name, argv[1]);
    else
        strcpy(client_name, "no name");

    return 0;
}

int connect_server(peer_t *server) {
    // create socket
    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket < 0) {
        perror("socket()");
        return -1;
    }

    // set up addres
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IPV4_ADDR);
    server_addr.sin_port = htons(SERVER_LISTEN_PORT);

    server->addres = server_addr;

    if (connect(server->socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) != 0) {
        perror("connect()");
        return -1;
    }

    log_debug("Connected to %s:%d.\n", SERVER_IPV4_ADDR, SERVER_LISTEN_PORT);

    return 0;
}

int build_fd_sets(peer_t *server, fd_set *read_fds, fd_set *write_fds, fd_set *except_fds) {
    FD_ZERO(read_fds);
    FD_SET(STDIN_FILENO, read_fds);
    FD_SET(server->socket, read_fds);

    FD_ZERO(write_fds);
    // there is smth to send, set up write_fd for server socket
    if (server->send_buffer.current > 0)
        FD_SET(server->socket, write_fds);

    FD_ZERO(except_fds);
    FD_SET(STDIN_FILENO, except_fds);
    FD_SET(server->socket, except_fds);

    return 0;
}

int handle_read_from_stdin(peer_t *server, char *client_name) {
    char read_buffer[DATA_MAXSIZE]; // buffer for stdin
    if (read_from_stdin(read_buffer, DATA_MAXSIZE) != 0) {
        return -1;
    }

    // Create new message and enqueue it.
    message_t new_message;
    prepare_message(client_name, read_buffer, &new_message);
    print_message(&new_message);

    if (peer_add_to_send(server, &new_message) != 0) {
        log_debug("Send buffer is overflowed, we lost this message!\n");
        return 0;
    }
    log_debug("New message to send was enqueued right now.\n");

    return 0;
}

/* You should be careful when using this function in multythread program.
 * Ensure that server is thread-safe. */
void shutdown_properly(int code) {
    delete_peer(&server);
    log_debug("Shutdown client properly.\n");
    exit(code);
}

int handle_received_message(message_t *message) {
    log_debug("Received message from server.\n");
    print_message(message);
    return 0;
}

void init_heart_beat(char * client_name) {
    enqueue_heart_beat_message(&server, client_name, true, 3000);
}

int init_client(char *client_name) {

    if (setup_signals() != 0)
        exit(EXIT_FAILURE);

    log_debug("Client '%s' start.\n", client_name);

    create_peer(&server);
    if (connect_server(&server) != 0)
        shutdown_properly(EXIT_FAILURE);

    /* Set nonblock for stdin. */
    int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flag);

    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    log_debug("Waiting for server message or stdin input. Please, type text to send:");

    // server socket always will be greater then STDIN_FILENO
    int maxfd = server.socket;
    pthread_create(&message_producer, NULL, &init_heart_beat, client_name);
    while (1) {
        // Select() updates fd_set's, so we need to build fd_set's before each select()call.
        build_fd_sets(&server, &read_fds, &write_fds, &except_fds);

        int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, NULL);

        switch (activity) {
            case -1:
                perror("select()");
                shutdown_properly(EXIT_FAILURE);

            case 0:
                // you should never get here
                log_debug("select() returns 0.\n");
                shutdown_properly(EXIT_FAILURE);

            default:
                /* All fd_set's should be checked. */
                if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                    if (handle_read_from_stdin(&server, client_name) != 0)
                        shutdown_properly(EXIT_FAILURE);
                }

                if (FD_ISSET(STDIN_FILENO, &except_fds)) {
                    log_debug("except_fds for stdin.\n");
                    shutdown_properly(EXIT_FAILURE);
                }

                if (FD_ISSET(server.socket, &read_fds)) {
                    if (receive_from_peer(&server, &handle_received_message) != 0)
                        shutdown_properly(EXIT_FAILURE);
                }

                if (FD_ISSET(server.socket, &write_fds)) {
                    if (send_to_peer(&server) != 0)
                        shutdown_properly(EXIT_FAILURE);
                }

                if (FD_ISSET(server.socket, &except_fds)) {
                    log_debug("except_fds for server.\n");
                    shutdown_properly(EXIT_FAILURE);
                }
        }

        log_debug("And we are still waiting for server or stdin activity. You can type something to send:\n");
    }
}

int main(int argc, char **argv) {
    init_log(LOG_DEBUG, "client");
    pthread_mutex_init(&mutex, NULL);
    


    char client_name[256];
    get_client_name(argc, argv, client_name);
    init_client(client_name);
    return 0;
}