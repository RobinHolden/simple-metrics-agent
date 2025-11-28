cmake_minimum_required(VERSION 3.20)

project(simple-metrics-agent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(simple-metrics-agent
    src/main.cpp
	src/config.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wconversion
        -Wsign-conversion
    )
	target_compile_options(simple-metrics-agent PRIVATE ${WARNINGS})
endif()
