cmake_minimum_required(VERSION 3.12)

include (CheckLibraryExists)
include (CheckSymbolExists)
include (ExternalProject)
include (FindPkgConfig)

project(server_client C)
project(server_message_queue C)

find_library (APACHE_AVRO_LIBRARIES avro)
if (USE_SYSTEM_LIBAO)
    find_library (AO_LIBRARIES ao)
    pkg_search_module(AO ao)
    if (NOT AO_LIBRARIES)
        message (WARNING " libao not found. Building from source.")
        set (USE_SYSTEM_LIBAO OFF)
    endif ()
endif ()

if (USE_SYSTEM_LIBEVENT)
    find_library (EVENT_LIBRARIES event)
    pkg_search_module(EVENT event)
    if (NOT EVENT_LIBRARIES)
        message (WARNING " libevent not found. Building from source.")
        set (USE_SYSTEM_LIBEVENT OFF)
    endif ()
endif ()

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
    set(APACHE_AVRO_LIBRARIES ${AVRO_PREFIX}/lib)
    set(APACHE_AVRO_INCLUDE_DIRS "${AVRO_PREFIX}/include")
    add_dependencies(avro avro_external)
endif ()

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
endif()

if (NOT USE_SYSTEM_LIBAO)

    set (LIBAO_PREFIX "${CMAKE_BINARY_DIR}/libao-prefix")
    ExternalProject_Add (
            libao_external
            GIT_REPOSITORY "https://git.xiph.org/libao.git"
            GIT_TAG d5221655dfd1a2156aa6be83b5aadea7c1e0f5bd
            PREFIX ${LIBAO_PREFIX}

            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ${LIBAO_PREFIX}/src/libao_external/configure ${HOST_TRIPLE_ARG} --prefix=${LIBAO_PREFIX} --enable-static --disable-shared --disable-pulse
    )
    ExternalProject_Add_Step (
            libao_external
            bootstrap
            COMMAND sh ./autogen.sh
            DEPENDEES patch
            DEPENDERS configure
            WORKING_DIRECTORY <SOURCE_DIR>
    )

    add_library (libao STATIC IMPORTED)
    set_property (TARGET libao PROPERTY IMPORTED_LOCATION "${LIBAO_PREFIX}/lib/libao.a")
    add_dependencies (libao libao_external)

    set (AO_INCLUDE_DIRS "${LIBAO_PREFIX}/include")
    set (AO_LIBRARIES libao ksuser winmm)
    set (AO_BUILTIN libao)
endif ()

##[[if (NOT USE_SYSTEM_LIBEVENT)
#
#    set (LIBEVENT_PREFIX "${CMAKE_BINARY_DIR}/libevent-prefix")
#    ExternalProject_Add (
#            libevent_external
#            URL "https://github.com/libevent/libevent/releases/download/release-2.1.11-stable/libevent-2.1.11-stable.tar.gz"
#            DOWNLOAD_NAME libevent.tar.gz
#            PREFIX ${LIBEVENT_PREFIX}
#
#            UPDATE_COMMAND ""
#            CMAKE_ARGS
#            -DCMAKE_INSTALL_PREFIX:STRING=${LIBEVENT_PREFIX}
#           # CONFIGURE_COMMAND ${LIBAO_PREFIX}/src/libao_external/configure ${HOST_TRIPLE_ARG} --prefix=${LIBAO_PREFIX} --enable-static --disable-shared --disable-pulse
#    )
##    ExternalProject_Add_Step (
##            libao_external
##            bootstrap
##            COMMAND sh ./autogen.sh
##            DEPENDEES patch
##            DEPENDERS configure
##            WORKING_DIRECTORY <SOURCE_DIR>
##    )
#
#    add_library (libevent STATIC IMPORTED)
#    set_property (TARGET libevent PROPERTY IMPORTED_LOCATION "${LIBEVENT_PREFIX}/lib/libevent.a")
#    add_dependencies (libevent libevent_external)
#
#    set (EVENT_INCLUDE_DIRS "${LIBEVENT_PREFIX}/include")
#    set (EVENT_LIBRARIES libevent)
#    set (EVENT_BUILTIN libevent)
#endif ()]]


add_subdirectory (src)

