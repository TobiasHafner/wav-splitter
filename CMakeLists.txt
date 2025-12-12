cmake_minimum_required(VERSION 3.25)
project(wav-splitter C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(wav-splitter
    main.c
    wav-header.c
    processing-utils.c
)

if (MSVC)
    target_compile_options(wav-splitter PRIVATE /W4)
else()
    target_compile_options(wav-splitter PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(wav-splitter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
