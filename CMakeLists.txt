cmake_minimum_required(VERSION 3.20)

project(simple-metrics-agent LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(simple-metrics-agent
    src/main.cpp
	src/config.cpp
	src/agent.cpp
	src/metrics.cpp
	src/net.cpp
	src/hostname.cpp
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


enable_testing()

add_executable(unit_tests
    tests/unit_tests.cpp
    src/config.cpp
    src/net.cpp
)

target_compile_features(unit_tests PRIVATE cxx_std_20)
target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME unit_tests COMMAND unit_tests)
