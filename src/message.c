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
    avro_datum_t payload = avro_givebytes(message->data, message->data_size, NULL);

    //debug
    char * buffer[8192];
    size_t size;
    avro_bytes_get(payload, buffer,  &size);

    log_info(avro_datum_to_bytes(payload)->bytes);
    avro_schema_t schema = avro_schema_map(avro_schema_long());
    avro_datum_t header = avro_map(schema);
    avro_map_set(header, "header", "heartbeat");
    if (avro_record_set(*datum, "type", type) ||
        avro_record_set(*datum, "payload", payload) ||
        avro_record_set(*datum, "header", header)) {
        return -1;
    } else {
        avro_datum_decref(type);
        avro_datum_decref(payload);
        avro_datum_decref(header);
        return 0;
    }
}

int message_convert_datum_to_message(avro_datum_t * datum, message_t * message) {

    avro_datum_t  type;
    avro_datum_t payload;
    unsigned char * buffer[1];
    if (avro_record_get(*datum, "type", &type) == 0) {
        avro_int32_get(type, &message->type);
    } else {
        log_error("%s", avro_strerror());
        return -1;
    }

    if(avro_record_get(*datum, "payload", &payload) ==0) {
        int size;
        if (avro_bytes_get(payload, buffer, &size) == 0) {
            memcpy(message->data, buffer[0],size);
            free(buffer[0]);
        }
    } else {
        log_error("%s", avro_strerror());
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
    memset(buffer, '1', sizeof(buffer));
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

int message_bytes_to_message(unsigned char * avroByteStream, size_t sizeToRead, message_t * message) {
    avro_schema_t  schema;
    validate_schema(&schema);
    avro_datum_t avro_message = avro_record(schema);
    avro_reader_t reader = avro_reader_memory(avroByteStream, sizeToRead);
    if( avro_read_data(reader,  schema, schema, &avro_message) == EINVAL) {
       log_error('%s', avro_strerror());
       return -1;
    }
    message_convert_datum_to_message(&avro_message, message);

    return 0;
}

// message queue --------------------------------------------------------------

