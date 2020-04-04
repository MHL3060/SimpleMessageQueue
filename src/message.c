//
// Created by Ben Li on 2020-03-19.
//

//message type
#include <stdio.h>
#include <avro/io.h>
#include <avro/schema.h>
#include <avro.h>
#include <avro/data.h>
#include "common.h"
#include "message.h"
#include "log.h"

int32_t prepare_message(char *header, char *data, size_t size,  Message *message) {
    memcpy(message->header, header, HEADER_SIZE);
    memcpy(message->data, data, size);
    message->data_size = size;
    return 0;
}

int32_t validate_schema(avro_schema_t * message_schema) {

    if (avro_schema_from_json_literal(SCHEMA, message_schema)) {
        log_error("unable to parse message schema");
        return -1;
    }
    return 0;
}

int32_t message_convert_message_to_avro_record(Message * message, avro_datum_t * record) {
    avro_datum_t type = avro_int32(message->type);
    avro_datum_t payload = avro_givebytes(message->data, message->data_size, free);

    char * buffer[DATA_MAXSIZE];
    int64_t size;
    avro_bytes_get(payload, buffer,  &size);

    avro_schema_t schema = avro_schema_map(avro_schema_long());
    avro_datum_t header = avro_map(schema);
    avro_map_set(header, "header", "heartbeat");
    if (avro_record_set(*record, "type", type) ||
        avro_record_set(*record, "payload", payload) ||
        avro_record_set(*record, "header", header)) {
        log_error("%s", avro_strerror());
        return -1;
    } else {
        avro_datum_decref(type);
        avro_datum_decref(payload);
        avro_datum_decref(header);
        return 0;
    }
}

int avro_datum_value_get_bytes(const void *vself, Message * message)
{
    const avro_datum_t  self = (const avro_datum_t) vself;
    char  *bytes;
    int64_t  sz;
    avro_bytes_get(self, &bytes, &sz);
    memcpy(message->data, bytes, sz);
    return 0;
}

int32_t message_convert_datum_to_message(avro_datum_t * avro_message_record, Message * message) {

    avro_datum_t  type;
    avro_datum_t payload;
    if (avro_record_get(*avro_message_record, "type", &type) == 0) {
        avro_int32_get(type, &message->type);
        avro_datum_decref(type);
    } else {
        log_error("%s", avro_strerror());
        return -1;
    }
    if(avro_record_get(*avro_message_record, "payload", &payload) == 0) {
        avro_datum_value_get_bytes(payload, message);
        avro_datum_decref(payload);
    } else {
        log_error("%s", avro_strerror());
        return -1;
    }
    return 0;
}

int32_t message_to_bytes(Message * message, unsigned char * byteArrayResult, size_t * byte_array_size) {
    avro_schema_t schema;
    validate_schema(&schema);
    avro_datum_t avro_message = avro_record(schema);
    message_convert_message_to_avro_record(message, &avro_message);
    char payload_buffer[AVRO_PAYLOAD_SIZE];

    memset(payload_buffer, '\0', sizeof(payload_buffer));
    avro_writer_t  writer = avro_writer_memory(payload_buffer, sizeof(payload_buffer));
    if (avro_write_data(writer, NULL, avro_message) !=0) {
        log_error("error when converting to avro %s",avro_strerror());
        return -1;
    }
    int64_t size = avro_writer_tell(writer);


    int32_t networkOrderInt = htonl(size);
    memcpy(byteArrayResult, &networkOrderInt, HEADER_SIZE);
    memcpy(byteArrayResult + HEADER_SIZE, payload_buffer, size);
    memcpy(byteArrayResult + HEADER_SIZE + size, END_OF_MESSAGE_PAYLOAD, END_OF_MESSAGE_PAYLOAD_SIZE);

    * byte_array_size = HEADER_SIZE + size + END_OF_MESSAGE_PAYLOAD_SIZE;
    avro_schema_decref(schema);
    avro_writer_free(writer);
    return 0;
}

int32_t message_bytes_to_message(unsigned char avro_byte_stream[], int32_t size_to_read, Message * message) {
    avro_schema_t  schema;
    validate_schema(&schema);
    avro_datum_t avro_message = avro_record(schema);
    avro_reader_t reader = avro_reader_memory(avro_byte_stream, size_to_read);
    if( avro_read_data(reader,  schema, schema, &avro_message) == EINVAL) {
       log_error('%s', avro_strerror());
       return -1;
    }
    message_convert_datum_to_message(&avro_message, message);
    avro_schema_decref(schema);
    avro_reader_free(reader);
    return 0;
}

// message queue --------------------------------------------------------------

