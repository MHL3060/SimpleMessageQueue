//
// Created by Ben Li on 2020-03-19.
//

//message type
#include <stdio.h>
#include <avro/io.h>
#include <avro/schema.h>
#include <avro.h>
#include "common.h"
#include "message.h"
#include "log.h"

int prepare_message(char *header, char *data, message_t *message) {
    sprintf(message->header, "%s", header);
    sprintf(message->data, "%s", data);
    message->data_size = strlen(message->data) + 1;
    return 0;
}

int prepare_avro_message(char *header, char *data, avro_datum_t * result) {
    message_t message;
    prepare_message(header, data, &message);
    message_convert_message_to_avro_record(&message, result);
}

int print_message(message_t *message) {
    log_info("Message: \"%s: %s\"", message->header, message->data);
    return 0;
}

int validate_schema(avro_schema_t * message_schema) {

    if (avro_schema_from_json_literal(SCHEMA, message_schema)) {
        log_error("unable to parse message schema");
        return -1;
    }
    return 0;
}

int message_convert_message_to_avro_record(message_t * message, avro_datum_t * datum) {
    avro_datum_t type = avro_int32(message->type);
    avro_datum_t data = avro_bytes(message->data, message->data_size);
    avro_schema_t schema = avro_schema_map(avro_schema_long());
    avro_datum_t header = avro_map(schema);
    avro_map_set(header, "header", "heartbeat");
    if (avro_record_set(*datum, "type", type) ||
        avro_record_set(*datum, "payload", data) ||
        avro_record_set(*datum, "header", header)) {
        return -1;
    } else {
        avro_datum_decref(type);
        avro_datum_decref(data);
        avro_datum_decref(header);
        return 0;
    }
}

int message_convert_datum_to_message(avro_datum_t * datum, message_t * message) {

    avro_datum_t  type;
    avro_datum_t data;
    if (avro_record_get(datum, "type", &type) == 0) {
        avro_int32_get(type, &message->type);
    } else {
        return -1;
    }
    if(avro_record_get(datum, "data", &data) ==0) {
        avro_bytes_get(data, &message->data, &message->data_size);
    } else {
        return -1;
    }
    return 0;
}

int message_to_bytes(message_t * message, unsigned char * byteArrayResult, int * byte_array_size) {
    avro_schema_t schema;
    validate_schema(&schema);
    avro_datum_t avroMessage = avro_record(schema);
    message_convert_message_to_avro_record(message, &avroMessage);
    char buffer[8192];
    avro_writer_t  writer = avro_writer_memory(buffer, sizeof(buffer));
    if (avro_write_data(writer, NULL, avroMessage) !=0) {
        log_error("%s",avro_strerror());
        return -1;
    }
    size_t size = avro_writer_tell(writer);
    memcpy(byteArrayResult, buffer, size);
    avro_datum_decref(avroMessage);
    * byte_array_size = size;
    return 0;
}

int message_bytes_to_message(char ** avroByteStream, message_t * message) {
    avro_schema_t  schema;
    validate_schema(&schema);
    avro_datum_t avroMessage = avro_record(schema);
    avro_reader_t reader = avro_reader_memory(avroByteStream, MAX_MESSAGES_BUFFER_SIZE);
    avro_read_data(reader,  &schema, &schema, &avroMessage);
    message_convert_datum_to_message(&avroMessage, message);

    return 0;
}

// message queue --------------------------------------------------------------

