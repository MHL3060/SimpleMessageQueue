//
// Created by Ben Li on 2020-03-20.
//
#include "log.h"
#include "server.h"

int main() {
    init_log(LOG_DEBUG, "server");
    int returnCode;

    return server_init(&returnCode);
}
