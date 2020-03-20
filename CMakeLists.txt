cmake_minimum_required(VERSION 3.12)

include (ExternalProject)
include (FindPkgConfig)

project(server_client C)

set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(common STATIC src/log.c src/util.c src/message.c src/message_queue.c src/peer.c)

add_executable(server src/server.c src/server_main.c)
add_executable(client src/client.c src/client_main.c)
target_link_libraries(server Threads::Threads common)
target_link_libraries(client Threads::Threads common)
find_library (APACHE_AVRO_LIBRARIES avro)

if (NOT APACHE_AVRO_LIBRARIES)
    set (AVRO "${CMAKE_BINARY_DIR}/avro-prefix")
    ExternalProject_Add (
            avro_external
            URL "http://apache.mirror.rafal.ca/avro/stable/c/avro-c-1.9.2.tar.gz"
            DOWNLOAD_NAME avro-c.gz
            DOWNLOAD_NO_EXTRACT false

            UPDATE_COMMAND ""
            CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:STRING=${RTLSDR_PREFIX}
            -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
            -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
            ${RTLSDR_CMAKE_ARGS}
    )

    if (LIBUSB_BUILTIN)
        add_dependencies (rtlsdr_external ${LIBUSB_BUILTIN})
    endif ()

    add_library (rtlsdr STATIC IMPORTED)
    set_property (TARGET rtlsdr PROPERTY IMPORTED_LOCATION "${RTLSDR_PREFIX}/lib/librtlsdr_static.a")
    add_dependencies (rtlsdr rtlsdr_external)

    set (RTL_SDR_INCLUDE_DIRS "${RTLSDR_PREFIX}/include")
    set (RTL_SDR_LIBRARIES rtlsdr ${LIBUSB_LIBRARIES})
    set (RTL_SDR_BUILTIN rtlsdr)
endif ()