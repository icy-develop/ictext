cmake_minimum_required(VERSION 3.30)
project(ictext C)

set(CMAKE_C_STANDARD 23)

add_executable(ictext src/main.c
        src/include/error_handling.h
        src/include/raw_mode.h
        src/raw_mode.c
        src/error_handling.c)
