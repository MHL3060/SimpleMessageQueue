//
// Created by Ben Li on 2020-03-19.
//

//message type
#include <stdio.h>
#include <avro/io.h>
#include "common.h"
#include "message.h"
#include "log.h"

int prepare_message(char *header, char *data, message_t *message) {
    sprintf(message->header, "%s", header);
    sprintf(message->data, "%s", data);
    return 0;
}

int print_message(message_t *message) {
    log_info("Message: \"%s: %s\"", message->header, message->data);
    return 0;
}


int readSchema(char * fileName, char* content) {
    FILE * file = fopen(fileName, "r");
    if (file == NULL) {
        return -1;
    }
    int size = fread(file, sizeof(&content),1, content);
    fclose(file);
    if (size <= 0) {
        return -1;
    } else {
        return size;
    }
}

int validateSchema() {
    avro_schema_t person_schema;
    if (avro_schema_from_json_literal(SCHEMA, &person_schema)) {
        log_error("unable to parse message schema");
        return -1;
    }
    return 0;
}

// message queue --------------------------------------------------------------

