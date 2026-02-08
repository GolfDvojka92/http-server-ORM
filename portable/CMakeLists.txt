cmake_minimum_required(VERSION 3.10)

project(portable_client_server)

add_executable(server
    server.c
    ../src/global.c
)

add_executable(client
    client.c
)

target_compile_options(server PRIVATE)
target_compile_options(client PRIVATE)

target_include_directories(server PRIVATE include)
target_include_directories(client PRIVATE include)