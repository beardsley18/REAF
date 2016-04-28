cmake_minimum_required(VERSION 2.8.4)
project(proglib-c C)

set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c90")
endif()

option(BUILD_TESTS "Build tests." OFF)

set(SOURCE_FILES
    include/vn/util/compiler.h
    include/vn/util/export.h
    include/vn/bool.h
    include/vn/error.h
    include/vn/error_detection.h
    include/vn/int.h
    include/vn/math.h
    include/vn/math/vector.h
    include/vn/types.h
    include/vn/protocol/common.h
    include/vn/protocol/uart.h
    include/vn/protocol/spi.h
    include/vn/util.h
    include/vn/sensors.h
    include/vn/xplat.h
    include/vn/xplat/serialport.h
    include/vn/xplat/thread.h
    include/vn/xplat/criticalsection.h
    include/vn/xplat/event.h
    include/vn/xplat/time.h
    include/vn/sensors/ezasyncdata.h
    include/vn/sensors/compositedata.h
    include/vn/sensors/searcher.h
    src/vn/error_detection.c
    src/vn/protocol/uart.c
    src/vn/protocol/spi.c
    src/vn/util.c
    src/vn/math.c
    src/vn/math/vector.c
    src/vn/sensors.c
    src/vn/xplat.c
    src/vn/error.c
    src/vn/xplat/serialport.c
    src/vn/xplat/thread.c
    src/vn/xplat/criticalsection.c
    src/vn/xplat/event.c
    src/vn/xplat/time.c
    src/vn/sensors/searcher.c
    src/vn/sensors/ezasyncdata.c
	src/vn/sensors/compositedata.c)

file(GLOB_RECURSE TEST_SOURCE_FILES src/vn/**/*.test.cpp)
set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} src/vn/main.cpp)

include_directories(include)

add_library(proglib-c ${SOURCE_FILES})

if (BUILD_TESTS)

    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries (DLLs).")
    add_subdirectory(../libs/googletest gtest)

    include_directories(../libs/googletest/include)

    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

    add_executable(proglib-c-test ${TEST_SOURCE_FILES})

    target_link_libraries(proglib-c-test LINK_PUBLIC gtest)
    target_link_libraries(proglib-c-test LINK_PUBLIC proglib-c)

endif()
