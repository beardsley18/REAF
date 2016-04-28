cmake_minimum_required(VERSION 2.8.4)
project(getting_started C)

add_subdirectory(../.. proglib-c)

set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c90")

set(SOURCE_FILES
    main.c)

include_directories("../../include")

add_executable(getting_started ${SOURCE_FILES})

target_link_libraries(getting_started LINK_PUBLIC proglib-c)

if (UNIX OR APPLE)
	target_link_libraries(getting_started LINK_PUBLIC pthread)
endif()
