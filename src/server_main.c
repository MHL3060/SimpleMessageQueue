//
// Created by Ben Li on 2020-03-20.
//
#include "log.h"
#include "server.h"
#include "message.h"
#include "libevent_server.h"
int main() {
    Arguments arguments;
    init_log(LOG_DEBUG, "server");
    int returnCode;

    return server_init(&returnCode);

}
