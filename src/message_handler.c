//
// Created by Ben Li on 2020-04-01.
//

#include "message_handler.h"
#include "common.h"


void wire_tap(const unsigned char *message) {
    log_info("Message: \"%s: %s\"", message);
    printf("%s", message);
}


int32_t handle_message(Message * message) {
    //if (message->type == TYPE_DATA) {
        wire_tap(message->data);
    //}

    return 0;
}
