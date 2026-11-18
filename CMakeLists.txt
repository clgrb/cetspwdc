cmake_minimum_required(VERSION 3.21)

project(codetest LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)

include_directories(AFTER lib .)
add_subdirectory(lib)
add_subdirectory(src)



