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
#include <fcntl.h>
#include "log.h"
#include "common.h"
#include "message_queue.h"

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

    if (read_count > 0) {
        log_info("Read from stdin %d bytes. Let's prepare message to send.", read_count);
    }
    * received_size = read_count;
    return read_count;
}

void init_sockaddr(int port, struct sockaddr_in * sockaddr) {
    memset(sockaddr, 0, sizeof(sockaddr));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = htonl (INADDR_ANY); //inet_addr(INADDR_ANY);
    sockaddr->sin_port = htons(port);
}

void init_termios(struct termios * old, struct termios * current)
{
    tcgetattr(STDIN_FILENO, old); /* grab old terminal i/o settings */
    current = old; /* make new settings same as old settings */
    current->c_lflag &= ~(ICANON); /* disable buffered i/o */

    current->c_lflag |= ECHO; /* set echo mode */

    tcsetattr(STDIN_FILENO, TCSANOW, current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void reset_termios(struct termios * old)
{
    tcsetattr(STDIN_FILENO, TCSANOW, old);
}

int read_file(char * fileName, int dataType,  MessageQueue * queue) {

    FILE * file = fopen(fileName, "rb");
    unsigned char buffer[DATA_MAXSIZE];
    int32_t size;
    Message message;
    if (file != NULL) {
        while ((size = fread(buffer, 1, DATA_MAXSIZE, file)) > 0) {

            memset(&message, 0, sizeof(Message));
            memcpy(&message.data, buffer, size);
            message.data_size = size;
            message.type = dataType;
            while(message_enqueue(queue, &message) == -1) {
                usleep(100);
            }
        }
        return 0;
    } else {
        log_fatal("file '%s' not found. ", fileName);
        return -1;
    }

}
