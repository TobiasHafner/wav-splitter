cmake_minimum_required(VERSION 3.25)
project(wav-splitter C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(wav-splitter
    src/main.c
    src/wav-header.c
    src/utils.c
    src/processing.c
)

if (MSVC)
    target_compile_options(wav-splitter PRIVATE /W4)
else()
    target_compile_options(wav-splitter PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(wav-splitter PRIVATE include)
