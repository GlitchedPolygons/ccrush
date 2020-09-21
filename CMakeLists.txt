cmake_minimum_required(VERSION 3.7)
project(ccrush C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CCRUSH_MAJOR 1)
set(CCRUSH_MINOR 0)
set(CCRUSH_PATCH 1)

option(CCRUSH_ENABLE_TESTS "Build unit tests." OFF)

set(${PROJECT_NAME}_SRC_FILES
        ${CMAKE_CURRENT_LIST_DIR}/lib/miniz/miniz.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/miniz/miniz.h
        ${CMAKE_CURRENT_LIST_DIR}/src/ccrush.c
        ${CMAKE_CURRENT_LIST_DIR}/include/ccrush.h
        )

if (CCRUSH_BUILD_DLL)
    add_compile_definitions("CCRUSH_BUILD_DLL=1")
    set(CCRUSH_DLL ON)
endif ()

if (CCRUSH_DLL)
    add_compile_definitions("CCRUSH_DLL=1")
endif ()

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRC_FILES})

if (CCRUSH_BUILD_DLL OR BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            VERSION ${CCRUSH_MAJOR}.${CCRUSH_MINOR}.${CCRUSH_PATCH}
            SOVERSION ${CCRUSH_MAJOR}
            )
endif ()

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        )

if (NOT TARGET chillbuff)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/chillbuff chillbuff)
endif ()

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/miniz
        )

target_link_libraries(${PROJECT_NAME}
        PRIVATE chillbuff
        )

get_target_property(${PROJECT_NAME}_DEPS_TARGETS ${PROJECT_NAME} LINK_LIBRARIES)

if (CCRUSH_ENABLE_TESTS)

    add_executable(run_tests
            ${CMAKE_CURRENT_LIST_DIR}/tests/tests.c
            ${${PROJECT_NAME}_SRC_FILES}
            )

    target_include_directories(run_tests
            PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
            PUBLIC ${CMAKE_CURRENT_LIST_DIR}/lib/acutest/include
            PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/miniz
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