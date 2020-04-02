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

    return read_count;
}
