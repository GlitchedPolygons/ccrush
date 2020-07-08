cmake_minimum_required(VERSION 3.7)
project(ccrush C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

option(CCRUSH_ENABLE_TESTS "Build unit tests." OFF)

set(${PROJECT_NAME}_SRC_FILES
        ${CMAKE_CURRENT_LIST_DIR}/lib/miniz/miniz.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/miniz/miniz.h
        ${CMAKE_CURRENT_LIST_DIR}/src/ccrush.c
        ${CMAKE_CURRENT_LIST_DIR}/include/ccrush.h
        )

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRC_FILES})

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

    find_package(cmocka)

    if (NOT ${cmocka_FOUND})
        message(STATUS "cmocka not found; using included version inside lib/cmocka/ folder...")
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/cmocka cmocka)
    else ()
        message(STATUS "Found cmocka!")
    endif ()

    add_executable(run_tests
            ${CMAKE_CURRENT_LIST_DIR}/tests/tests.c
            ${${PROJECT_NAME}_SRC_FILES}
            )

    target_include_directories(run_tests
            PUBLIC ${cmocka_INCLUDE_DIR}
            PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
            PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/miniz
            )

    target_link_libraries(run_tests
            PUBLIC ${${PROJECT_NAME}_DEPS_TARGETS}
            PUBLIC cmocka
            )

    if (ENABLE_COVERAGE)
        find_package(codecov)
        add_coverage(${PROJECT_NAME})
        add_coverage(run_tests)
        coverage_evaluate()
    endif ()
endif ()