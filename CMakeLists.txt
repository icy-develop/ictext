cmake_minimum_required(VERSION 3.30)
project(ictext C)

set(CMAKE_C_STANDARD 23)

add_executable(ictext src/main.c
        src/include/error_handling.h
        src/include/editor_config.h
        src/editor_config.c
        src/error_handling.c
        src/include/defines.h
        src/editor.c
        src/include/editor.h)
