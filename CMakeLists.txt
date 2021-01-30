cmake_minimum_required(VERSION 3.7)
project(ccrush C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(${PROJECT_NAME}_MAJOR 2)
set(${PROJECT_NAME}_MINOR 0)
set(${PROJECT_NAME}_PATCH 0)
set(${PROJECT_NAME}_VERSION_STRING "${${PROJECT_NAME}_MAJOR}.${${PROJECT_NAME}_MINOR}.${${PROJECT_NAME}_PATCH}")

option(${PROJECT_NAME}_DLL "Use as a DLL." OFF)
option(${PROJECT_NAME}_BUILD_DLL "Build as a DLL." OFF)
option(${PROJECT_NAME}_ENABLE_TESTS "Build unit tests." OFF)
option(${PROJECT_NAME}_PACKAGE "Build the library and package it into a .tar.gz after successfully building." OFF)

set(${PROJECT_NAME}_SRC_FILES
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/adler32.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/compress.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/crc32.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/crc32.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/deflate.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/deflate.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/gzclose.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/gzguts.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/gzlib.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/gzread.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/gzwrite.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/infback.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/inffast.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/inffast.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/inffixed.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/inflate.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/inflate.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/inftrees.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/inftrees.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/trees.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/trees.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/uncompr.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/zutil.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/zutil.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/zconf.h
        ${CMAKE_CURRENT_LIST_DIR}/lib/zlib/zlib.h
        ${CMAKE_CURRENT_LIST_DIR}/src/ccrush.c
        ${CMAKE_CURRENT_LIST_DIR}/include/ccrush.h
        )

if (${${PROJECT_NAME}_BUILD_DLL})
    add_compile_definitions("CCRUSH_BUILD_DLL=1")
    set(${PROJECT_NAME}_DLL ON)
endif ()

if (${${PROJECT_NAME}_DLL})
    add_compile_definitions("CCRUSH_DLL=1")
endif ()

if (NOT WIN32)
    add_compile_definitions("HAVE_UNISTD_H=1")
endif ()

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRC_FILES})

if (${${PROJECT_NAME}_BUILD_DLL} OR BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            VERSION ${${PROJECT_NAME}_VERSION_STRING}
            SOVERSION ${${PROJECT_NAME}_MAJOR}
            )
endif ()

if (${${PROJECT_NAME}_PACKAGE})
    set_target_properties(${PROJECT_NAME} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin"

            ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin/debug"
            LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin/debug"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin/debug"

            ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin/release"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin/release"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/bin/release"
            )

    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD COMMAND ${CMAKE_COMMAND}
            -E copy ${CMAKE_CURRENT_LIST_DIR}/NOTICE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/NOTICE"
            )

    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD COMMAND ${CMAKE_COMMAND}
            -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/include "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/include"
            )

    string(TOLOWER "${PROJECT_NAME}-${${PROJECT_NAME}_VERSION_STRING}-${CMAKE_SYSTEM_NAME}-${CMAKE_HOST_SYSTEM_PROCESSOR}.tar.gz" ${PROJECT_NAME}_OUTPUT_ARCHIVE_FILENAME)

    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD COMMAND ${CMAKE_COMMAND}
            -E tar "czvf" ${${PROJECT_NAME}_OUTPUT_ARCHIVE_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
            )
endif ()

if (NOT TARGET chillbuff)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/chillbuff chillbuff)
endif ()

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/zlib
        )

target_link_libraries(${PROJECT_NAME}
        PRIVATE chillbuff
        )

get_target_property(${PROJECT_NAME}_DEPS_TARGETS ${PROJECT_NAME} LINK_LIBRARIES)

if (${${PROJECT_NAME}_ENABLE_TESTS})

    add_executable(run_tests
            ${CMAKE_CURRENT_LIST_DIR}/tests/tests.c
            ${${PROJECT_NAME}_SRC_FILES}
            )

    target_include_directories(run_tests
            PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
            PUBLIC ${CMAKE_CURRENT_LIST_DIR}/lib/acutest/include
            PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/zlib
            )

    target_link_libraries(run_tests
            PUBLIC ${${PROJECT_NAME}_DEPS_TARGETS}
            )

    if (ENABLE_COVERAGE)
        find_package(codecov)
        add_coverage(${PROJECT_NAME})
        add_coverage(run_tests)
        coverage_evaluate()
    endif ()
endif ()