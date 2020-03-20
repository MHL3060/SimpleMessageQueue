//
// Created by Ben Li on 2020-03-20.
//
#include "log.h"
#include "client.h"
int main(int argc, char **argv) {
    init_log(LOG_DEBUG, "client");

    char client_name[256];
    get_client_name(argc, argv, client_name);
    client_init(client_name);
    return 0;
}

