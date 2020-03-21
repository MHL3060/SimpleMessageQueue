//
// Created by Ben Li on 2020-03-20.
//
#include "log.h"
#include "server.h"
#include "message.h"
int main() {
    Arguments arguments;
    init_log(LOG_DEBUG, "server");
    int returnCode;
    if (validateSchema() != 0) {
        log_error("validation error");
        return -1;
    } else {
        return server_init(&returnCode);
    }

}
