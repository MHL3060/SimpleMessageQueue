#[[cmake_minimum_required(VERSION 3.12)

include (ExternalProject)
include (FindPkgConfig)

project(server_client C)]]

set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
message("feefe ${AVRO_INCLUDE_DIRS}")
message ("lib ${APACHE_AVRO_LIBRARIES}")
include_directories(
        ${AVRO_INCLUDE_DIRS}
)
link_directories(
        ${APACHE_AVRO_LIBRARIES}
)

add_library(common STATIC log.c util.c message.c message_queue.c peer.c)

add_executable(server server.c server_main.c)
add_executable(client client.c client_main.c)
target_link_libraries(server avro Threads::Threads common )
target_link_libraries(client avro Threads::Threads common )