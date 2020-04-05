//
// Created by Ben Li on 2020-03-19.
//

// common ---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <termios.h>
#include "log.h"
/* Reads from stdin and create new message. This message enqueues to send queueu. */
int read_from_stdin(char *read_buffer, size_t max_len, size_t * received_size) {
    memset(read_buffer, 0, max_len);

    ssize_t read_count = 0;
    ssize_t total_read = 0;


        read_count = read(STDIN_FILENO, read_buffer, max_len);

        if (read_count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read()");
            return -1;
        } else if (read_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return -1;
        } else if (read_count > 0) {
            total_read += read_count;
            if (total_read > max_len) {
                log_info("Message too large and will be chopped. Please try to be shorter next time.");
            }
        }

    log_info("Read from stdin %d bytes. Let's prepare message to send.", read_count);
    * received_size = read_count;
    return read_count;
}

void init_sockaddr(int port, struct sockaddr_in * sockaddr) {
    memset(sockaddr, 0, sizeof(sockaddr));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = htonl (INADDR_ANY); //inet_addr(INADDR_ANY);
    sockaddr->sin_port = htons(port);
}

void initTermios(struct termios * old, struct termios * current)
{
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    current = old; /* make new settings same as old settings */
    current->c_lflag &= ~ICANON; /* disable buffered i/o */
   // if (echo) {
        current->c_lflag |= ECHO; /* set echo mode */
   // } else {
     //   current->c_lflag &= ~ECHO; /* set no echo mode */
   // }
    tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(struct termios * old)
{
    tcsetattr(0, TCSANOW, old);
}
