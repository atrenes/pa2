cmake_minimum_required(VERSION 3.26)
project(raspred2 C)

set(CMAKE_C_STANDARD 99)
link_directories(/Users/danil/CLionProjects/raspred2)
#target_link_libraries()

add_executable(pa2 main.c pa2345_starter_code/pa2345.h common/common.h common/ipc.h
        ipc.c
        utility.c
        utility.h
        logger.c
        logger.h)
