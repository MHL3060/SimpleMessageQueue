//
// Created by Ben Li on 2020-03-19.
//

#ifndef SERVER_CLIENT_MESSAGE_H
#define SERVER_CLIENT_MESSAGE_H

#define  TYPE_HEART_BEAT 1;
#define  TYPE_DATA 2
#define  TYPE_OK 3

#include "common.h"
#define SCHEMA "{" \
                "  \"namespace\": \"simple.message.queue.avro\"," \
                "  \"type\": \"record\"," \
                "  \"name\" : \"message\"," \
                "  \"fields\": [" \
                "    {" \
                "      \"name\": \"header\"," \
                "      \"type\": {\"type\": \"map\", \"values\": \"string\"}," \
                "      \"values\": \"string\"" \
                "    }," \
                "    {" \
                "      \"name\": \"type\"," \
                "      \"type\": \"string\"" \
                "    }," \
                "    {" \
                "      \"name\": \"payload\"," \
                "      \"type\": \"bytes\"" \
                "    }" \
                "  ]" \
                "}"
// message --------------------------------------------------------------------

int prepare_message(char *header, char *data, message_t *message);
int print_message(message_t *message);
int validateSchema();
#endif //SERVER_CLIENT_MESSAGE_H
