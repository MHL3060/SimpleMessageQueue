//
// Created by Ben Li on 2020-03-19.
//

//message type
#include <stdio.h>
#include "common.h"
#include "message.h"
#include "log.h"

int prepare_message(char *header, char *data, message_t *message) {
    sprintf(message->header, "%s", header);
    sprintf(message->data, "%s", data);
    return 0;
}

int print_message(message_t *message) {
    log_info("Message: \"%s: %s\"", message->header, message->data);
    return 0;
}

// message queue --------------------------------------------------------------

