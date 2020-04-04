//
// Created by Ben Li on 2020-03-20.
//
#include "log.h"
#include "client.h"
#include "argument_handler.h"
#include "common.h"

int main(int argc, char **argv) {

    Arguments arguments;
    parse_args(&arguments, argc, argv);

    init_log(arguments.logLevel, "client");

    log_info("connect to host: %s, port : %d ", arguments.hostName, arguments.port);
    const char client_name[256];
    get_client_name(argc, argv, client_name);
    client_init(&arguments, client_name);
    return 0;
}

