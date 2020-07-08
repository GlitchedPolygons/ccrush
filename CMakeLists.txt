cmake_minimum_required(VERSION 3.16)
project(ccrush C)

set(CMAKE_C_STANDARD 11)

add_library(${PROJECT_NAME}
        ${CMAKE_CURRENT_LIST_DIR}/lib/miniz/miniz.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/miniz/miniz.h
        ${CMAKE_CURRENT_LIST_DIR}/src/ccrush.c
        ${CMAKE_CURRENT_LIST_DIR}/include/ccrush.h
        )

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/chillbuff chillbuff)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/miniz
        )

target_link_libraries(${PROJECT_NAME}
        PRIVATE chillbuff
        )