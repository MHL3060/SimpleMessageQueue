//
// Created by Ben Li on 2020-03-19.
//

//message type
#include <stdio.h>
#include <avro.h>
#include <avro/io.h>
#include "common.h"
#include "message.h"
#include "log.h"


#define SCHEMA_PATH "../schema/message.json"
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
    char schema[8192];
    FILE * file = fopen(SCHEMA_PATH, "r");
    avro_reader_file(file);
    avro_reader_t * reader = avro_reader_file(file);
}

// message queue --------------------------------------------------------------

