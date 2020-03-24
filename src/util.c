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

bool is_array_equals(unsigned char * first, unsigned char * second, size_t size) {
    bool equals = true;
    for (int i = 0; i < size; i++) {
        if (first[i] != second[i]) {
            equals = false;
            break;
        }
    }
    return equals;
}

/*
int copy_last_elements(unsigned char * array, size_t total_array_size, unsigned char * last_elements) {
    size_t size = sizeof(&last_elements);

    int elemnent_addr = last_elements;

    for (int i = total_array_size - size; i < total_array_size; i++) {
        &elemnent_addr = array[i];
        elemnent_addr++;
    }
    return 0;
}*/
