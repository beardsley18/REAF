cmake_minimum_required(VERSION 2.8.4)
project(ez_async_data)

add_subdirectory(../.. proglib-c)

set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c90")

set(SOURCE_FILES
    main.c)

include_directories("../../include")

add_executable(ez_async_data ${SOURCE_FILES})

target_link_libraries(ez_async_data LINK_PUBLIC proglib-c)

if (UNIX OR APPLE)
	target_link_libraries(ez_async_data LINK_PUBLIC pthread)
endif()
