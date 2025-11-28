cmake_minimum_required(VERSION 3.20)

project(simple-metrics-agent LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(simple-metrics-agent
    src/main.cpp
	src/config.cpp
	src/agent.cpp
)

target_compile_features(simple-metrics-agent PRIVATE cxx_std_20)

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
