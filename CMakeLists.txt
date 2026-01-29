cmake_minimum_required(VERSION 3.15)
project(HTTPServer)

add_executable(${PROJECT_NAME}
    src/main.c
    src/global.c
    src/helper.c
    src/server.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
)
