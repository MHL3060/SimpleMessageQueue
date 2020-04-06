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
#include <stdbool.h>
#include <netdb.h>
#include <pthread.h>

#include "common.h"
#include "log.h"
#include "message_queue.h"
#include "peer.h"
#include "util.h"
#include "client.h"
#include "message_handler.h"

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

int get_client_name(int argc, char **argv, const char *client_name) {
    if (argc > 1)
        strcpy(client_name, argv[1]);
    else
        strcpy(client_name, "no name");

    return 0;
}
/**
 * the host need to be resolve.
 * @param host
 * @param res - address for the result if return is = 0;
 * @return
 */
int lookup_host(const char *host, char * addrstr) {
    struct addrinfo hints, *res;
    int errcode;

    void *ptr;

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo(host, NULL, &hints, &res);
    if (errcode != 0) {
        perror("getaddrinfo");
        return -1;
    }

    printf("Host: %s\n", host);
    while (res) {
        inet_ntop(res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family) {
            case AF_INET:
                ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                break;
        }
        inet_ntop(res->ai_family, ptr, addrstr, 100);
        log_info("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4,
               addrstr, res->ai_canonname);
        res = res->ai_next;
    }
    freeaddrinfo(res);
    return 0;
}


int connect_server(Arguments* arguments, peer_t *server) {
    // create socket
    char ipAddress[100];
    int error = lookup_host(arguments->hostName, ipAddress);
    if (error) {
        return -1;
    }

    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket < 0) {
        perror("socket()");
        return -1;
    }

    // set up address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipAddress);
    server_addr.sin_port = htons(arguments->port);

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
    if (server->send_buffer.front >= 0)
        FD_SET(server->socket, write_fds);

    FD_ZERO(except_fds);
    FD_SET(STDIN_FILENO, except_fds);
    FD_SET(server->socket, except_fds);

    return 0;
}

static int handle_read_from_stdin(peer_t *server, const char *client_name) {
    char read_buffer[DATA_MAXSIZE]; // buffer for stdin
    int received_size = 0;
    char header[256];
    strncpy(header, client_name, strlen(client_name) + 1);
    while (read_from_stdin(read_buffer, DATA_MAXSIZE, &received_size) > 0) {
        Message new_message;
        prepare_message(header, read_buffer, received_size, &new_message);
        //print_message(&new_message);
        new_message.type = TYPE_AUDIO;

        if (peer_add_to_send(server, &new_message) != 0) {
            log_debug("Send buffer is overflowed, we lost this message!\n");
            return 0;
        }
        log_debug("New message to send was enqueued right now.\n");
    }
    return 0;
}

/* You should be careful when using this function in multythread program.
 * Ensure that server is thread-safe. */
void shutdown_properly(int code) {
    peer_delete(&server);
    log_debug("Shutdown client properly.\n");
    exit(code);
}

void init_heart_beat(char *client_name) {
    peer_enqueue_heart_beat(&server, client_name, false);
}

int client_init(Arguments * arguments, char *client_name) {

    if (setup_signals() != 0)
        exit(EXIT_FAILURE);

    log_debug("Client '%s' start.\n", client_name);

    peer_create(&server);
    if (connect_server(arguments, &server) != 0)
        shutdown_properly(EXIT_FAILURE);

    /* Set nonblock for stdin. */
    int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flag);

    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    log_debug("Waiting for server message or stdin input. Please, type text to send:");
    if (arguments->fileName) {
        readFile(arguments->fileName, arguments->dataType, &server.send_buffer);
    }
    // server socket always will be greater then STDIN_FILENO
    int maxfd = server.socket;
    pthread_create(&message_producer, NULL, (void *)&init_heart_beat, client_name);
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
                    if (peer_receive_from_peer(&server, &handle_message) != 0)
                        shutdown_properly(EXIT_FAILURE);
                }

                if (FD_ISSET(server.socket, &write_fds)) {
                    if (peer_send_to_peer(&server) != 0)
                        shutdown_properly(EXIT_FAILURE);
                }

                if (FD_ISSET(server.socket, &except_fds)) {
                    log_debug("except_fds for server.\n");
                    shutdown_properly(EXIT_FAILURE);
                }
        }
    }
}
