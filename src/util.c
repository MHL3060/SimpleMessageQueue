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
#include "log.h"
/* Reads from stdin and create new message. This message enqueues to send queueu. */
int read_from_stdin(char *read_buffer, size_t max_len) {
    memset(read_buffer, 0, max_len);

    ssize_t read_count = 0;
    ssize_t total_read = 0;

    do {
        read_count = read(STDIN_FILENO, read_buffer, max_len);
        if (read_count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read()");
            return -1;
        } else if (read_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        } else if (read_count > 0) {
            total_read += read_count;
            if (total_read > max_len) {
                log_debug("Message too large and will be chopped. Please try to be shorter next time.");
                fflush(STDIN_FILENO);
                break;
            }
        }
    } while (read_count > 0);

    size_t len = strlen(read_buffer);
    if (len > 0 && read_buffer[len - 1] == '\n')
        read_buffer[len - 1] = '\0';

    log_debug("Read from stdin %zu bytes. Let's prepare message to send.", strlen(read_buffer));

    return 0;
}
