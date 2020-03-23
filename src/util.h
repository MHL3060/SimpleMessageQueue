//
// Created by Ben Li on 2020-03-19.
//

#pragma once
#ifndef SERVER_CLIENT_UTIL_H
#define SERVER_CLIENT_UTIL_H
int read_from_stdin(char *read_buffer, size_t max_len);
int is_array_equals(unsigned char * first, unsigned char * second, size_t size);
int copy_last_elements(unsigned char * array, size_t total_array_size, unsigned char * last_elements);
#endif //SERVER_CLIENT_UTIL_H
