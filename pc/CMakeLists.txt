cmake_minimum_required(VERSION 3.10)
project(ps2pc_hub C)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)

add_executable(pong_pc
    main.c
    net_pc.c
    ../games/pong.c
)

target_include_directories(pong_pc PRIVATE ${SDL2_INCLUDE_DIRS} ..)
target_link_libraries(pong_pc ${SDL2_LIBRARIES})
