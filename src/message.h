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
int32_t prepare_message(char *header, char *data, message_t *message);
int32_t print_message(message_t *message);
int32_t validate_schema(avro_schema_t * message_schema);

int32_t message_to_bytes(message_t * message, unsigned char * byte_array_result, int32_t * byte_array_size);
int32_t message_bytes_to_message(unsigned char * avro_byte_stream, int32_t bytesToRead, message_t * message);
#endif //SERVER_CLIENT_MESSAGE_H
