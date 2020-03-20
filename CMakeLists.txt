cmake_minimum_required(VERSION 3.12)

include (ExternalProject)
include (FindPkgConfig)

project(server_client C)

find_library (APACHE_AVRO_LIBRARIES avro)

if (NOT APACHE_AVRO_LIBRARIES)
    set (AVRO_PREFIX "${CMAKE_BINARY_DIR}/avro-prefix")
    ExternalProject_Add (
            avro_external
            URL "http://apache.mirror.rafal.ca/avro/stable/c/avro-c-1.9.2.tar.gz"
            DOWNLOAD_NAME avro-c.gz
            PREFIX ${AVRO_PREFIX}

            UPDATE_COMMAND ""
            CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:STRING=${AVRO_PREFIX}
            -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
            -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
    )

    add_library (avro STATIC IMPORTED)
    set_property (TARGET avro PROPERTY IMPORTED_LOCATION "${AVRO_PREFIX}/lib/libavro.a")
    set(APACHE_AVRO_LIBRARIES  avro ${AVRO_PREFIX}/lib)
    set (AVRO_INCLUDE_DIRS "${AVRO_PREFIX}/include")
    message("${AVRO_PREFIX}/src/avro_external")
    add_dependencies(avro avro_external)
endif ()

add_subdirectory (src)