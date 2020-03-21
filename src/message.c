//
// Created by Ben Li on 2020-03-19.
//

//message type
#include <stdio.h>
#include <avro/io.h>
#include "common.h"
#include "message.h"
#include "log.h"

avro_schema_t person_schema;
int prepare_message(char *header, char *data, message_t *message) {
    sprintf(message->header, "%s", header);
    sprintf(message->data, "%s", data);
    return 0;
}

int print_message(message_t *message) {
    log_info("Message: \"%s: %s\"", message->header, message->data);
    return 0;
}

int validateSchema() {
    avro_schema_t person_schema;
    if (avro_schema_from_json_literal(SCHEMA, &person_schema)) {
        log_error("unable to parse message schema");
        return -1;
    }
    return 0;
}

int message_convert_message_to_avro_record(message_t * message, avro_datum_t * datum) {
    avro_datum_t type = avro_int32(message->type);
    avro_datum_t data = avro_bytes(message->data, message->data_size);

    if (avro_record_set(datum, "type", type) ||
        avro_record_set(datum, "data", data)) {
        return -1;
    } else {
        avro_datum_decref(type);
        avro_datum_decref(data);
        return 0;
    }
}

int message_convert_datum_to_message(avro_datum_t * datum, message_t * message) {


    return 0;
}

// message queue --------------------------------------------------------------

