//
// Created by Ben Li on 2020-03-20.
//

#ifndef SERVER_CLIENT_CLIENT_H
#define SERVER_CLIENT_CLIENT_H

#include "common.h"
int get_client_name(int argc, char **argv, const char * client_name);
int client_init(Arguments* arguments, char *client_name);
#endif //SERVER_CLIENT_CLIENT_H
