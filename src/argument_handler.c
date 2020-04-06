//
// Created by Ben Li on 2020-03-20.
//
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "common.h"


int parse_args(Arguments *arguments, int argc, char **argv) {

    int opt;

    arguments->port = (short)SERVER_LISTEN_PORT;
    arguments->hostName = SERVER_IPV4_ADDR;
    arguments->logLevel = LOG_DEBUG;

    while ((opt = getopt(argc, argv, "f:h:p:l:d:v:t:")) != -1) {
        switch (opt) {

            case 'f':
                log_info("%s", optarg);
                arguments->fileName = strdup(optarg);
                break;
            case 'h':
                arguments->hostName = strdup(optarg);
                break;
            case 'p':
                arguments->port = atoi(optarg);
                break;
            case 'l':
                arguments->logLevel = atoi(optarg);
                break;
            case 't':
                arguments->dataType = atoi(optarg);
                break;

            default:
                // help(argv[0]);
                return 1;
        }
    }
    return 0;
}
