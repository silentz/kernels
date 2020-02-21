cmake_minimum_required(VERSION 3.0)

project(minifs)
include_directories("src/")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lreadline")

set(SRC_LIST src/main.c)
add_subdirectory(src/internal/utils internal/utils)
add_subdirectory(src/internal/commands internal/commands)

add_executable(${PROJECT_NAME} ${SRC_LIST})
