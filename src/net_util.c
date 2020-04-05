//
// Created by Ben Li on 2020-04-04.
//
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "common.h"
/* Start listening socket listen_sock. */
int start_listen_socket(int listening_port, int *listen_sock) {
    // Obtain a file descriptor for our "listening" socket.
    *listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(INADDR_ANY);
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

int handle_new_connection(int listen_sock, peer_t * connection_list, int max_client) {
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
    for (i = 0; i < max_client; ++i) {
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

int set_nonblock(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        log_error("Error getting flags on fd %d", fd);
        return -1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags)) {
        log_error("Error setting non-blocking I/O on fd %d", fd);
        return -1;
    }

    return 0;
}


