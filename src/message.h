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
int32_t prepare_message(char *header, char *data, Message *message);
int32_t print_message(Message *message);

int32_t message_to_bytes(Message * message, unsigned char * byte_array_result, int32_t * byte_array_size);
int32_t message_bytes_to_message(unsigned char * avro_byte_stream, int32_t bytesToRead, Message * message);
#endif //SERVER_CLIENT_MESSAGE_H
