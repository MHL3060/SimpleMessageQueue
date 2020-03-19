cmake_minimum_required(VERSION 3.14)
project(server_client C)

set(CMAKE_C_STANDARD 11)
add_executable(server src/log.c src/server.c)
add_executable(client src/log.c src/client.c)