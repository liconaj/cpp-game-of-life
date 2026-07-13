cmake_minimum_required(VERSION 3.28)

project(GameOfLife LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external)
add_subdirectory(src)

add_custom_target(copy_compile_commands ALL
    COMMAND
        "${CMAKE_COMMAND}" -E copy_if_different
        "${PROJECT_BINARY_DIR}/compile_commands.json"
        "${PROJECT_SOURCE_DIR}/compile_commands.json"
    VERBATIM
)
