//
// Created by Ben Li on 2020-03-19.
//

#include <avro/schema.h>
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
                "      \"type\": {\"type\": \"map\", \"values\": \"string\"}" \
                "    }," \
                "    {" \
                "      \"name\": \"type\"," \
                "      \"type\": \"int\"" \
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
int validate_schema(avro_schema_t * message_schema);

int message_to_bytes(message_t * message, unsigned char byteArrayResult[], int * byte_array_size);
int bytes_to_message(char ** avroByteStream, message_t * message);
#endif //SERVER_CLIENT_MESSAGE_H
