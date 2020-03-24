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
typedef void
(*avro_free_func_t)(void *ptr, size_t sz);
struct avro_bytes_datum_t {
    struct avro_obj_t obj;
    char *bytes;
    int64_t size;
    avro_free_func_t  free;
};
#define check_param(result, test, name)					\
	{								\
		if (!(test)) {						\
			avro_set_error("Invalid " name " in %s",	\
				       __FUNCTION__);			\
			return result;					\
		}							\
	}
#define container_of(ptr_, type_, member_)  \
    ((type_ *)((char *)ptr_ - (size_t)&((type_ *)0)->member_))
#define avro_datum_to_bytes(datum_)     (container_of(datum_, struct avro_bytes_datum_t, obj))
// --above from garbage
int prepare_message(char *header, char *data, message_t *message);
int print_message(message_t *message);
int validate_schema(avro_schema_t * message_schema);

int message_to_bytes(message_t * message, unsigned char * byte_array_result, int * byte_array_size);
int message_bytes_to_message(unsigned char * avro_byte_stream, size_t bytesToRead, message_t * message);
#endif //SERVER_CLIENT_MESSAGE_H
