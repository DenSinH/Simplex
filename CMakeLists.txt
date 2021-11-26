cmake_minimum_required(VERSION 3.19)
project(Simplex)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)

add_executable(Simplex
        modules/glad/glad.c
        main.cpp
        )

include_directories(Simplex PUBLIC modules ${SDL2_INCLUDE_DIR} modules modules/glm include src)

target_link_libraries(Simplex PRIVATE ${SDL2_LIBRARY})

add_subdirectory(src/frontend)

target_link_libraries(Simplex PRIVATE frontend)