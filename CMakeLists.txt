cmake_minimum_required(VERSION 3.12)

include (ExternalProject)
include (FindPkgConfig)

project(server_message_queue C)



find_library(JANSSON jansson)
if (NOT JANSSON)
    set (JANSSON_PREFIX "${CMAKE_BINARY_DIR}/jansson-prefix")
    ExternalProject_Add(
            jansson_external
            GIT_REPOSITORY "https://github.com/akheron/jansson.git"
            GIT_TAG 71c4e8ec215afa225ac20eed269a14963cd37b50
            PREFIX ${JANSSON_PREFIX}

            UPDATE_COMMAND ""
            CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:STRING=${JANSSON_PREFIX}
            -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
            -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
    )

    add_library (jansson STATIC IMPORTED)
    set_property (TARGET jansson PROPERTY IMPORTED_LOCATION "${JANSSON_PREFIX}/lib/libjansson.a")
    set(JANSSON_LIBRARIES ${JANSSON_PREFIX}/lib)
    set(JANSSON_INCLUDE_DIRS "${JANSSON_PREFIX}/include")
    add_dependencies(jansson jansson_external)
    set(AVRO_CMAKE_ARGS
            -DINCLUDE_DIRS:STRING=${JANSSON_INCLUDE_DIRS}
            -DLIBRARIES:STRING=${JANSSON_PREFIX}/lib/libjansson.a
            )
endif()


find_library (APACHE_AVRO_LIBRARIES avro)
if (NOT APACHE_AVRO_LIBRARIES)
    set (AVRO_PREFIX "${CMAKE_BINARY_DIR}/avro-prefix")
    message("avro cmake ${AVRO_CMAKE_ARGS}")
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
            ${AVRO_CMAKE_ARGS}
    )

    add_library (avro STATIC IMPORTED)
    include_directories(
            ${JANSSON_PREFIX}
    )
    set_property (TARGET avro PROPERTY IMPORTED_LOCATION "${AVRO_PREFIX}/lib/libavro.a")
    set(APACHE_AVRO_LIBRARIES ${AVRO_PREFIX}/lib)
    set(APACHE_AVRO_INCLUDE_DIRS "${AVRO_PREFIX}/include")
    add_dependencies(avro avro_external)
endif ()

add_subdirectory (src)
