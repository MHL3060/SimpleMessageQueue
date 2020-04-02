
set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
message("feefe ${APACHE_AVRO_INCLUDE_DIRS}")
message("lib ${APACHE_AVRO_LIBRARIES}")
include_directories(
        ${APACHE_AVRO_INCLUDE_DIRS}
        ${JANSSON_INCLUDES_DIRS}
        ${AO_INCLUDE_DIRS}
)
link_directories(
        ${JANSSON_LIBRARIES}
        ${APACHE_AVRO_LIBRARIES}
        ${AO_LIBRARY_DIRS}

)

add_library(common STATIC
        log.c
        argument_handler.c
        util.c
        message.c
        message_queue.c
        peer.c
        message_handler.c
)

add_executable(server server.c server_main.c)
add_executable(client client.c client_main.c)

target_link_libraries(server jansson avro ao Threads::Threads common )
target_link_libraries(client jansson avro ao Threads::Threads common )
message ("dest ${PROJECT_BINARY_DIR}")
