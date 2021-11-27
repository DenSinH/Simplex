cmake_minimum_required(VERSION 3.19)
project(Simplex)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)
message(STATUS "SDL2 at ${SDL2_INCLUDE_DIR}")

find_package(Boost 1.75.0 REQUIRED)
message(STATUS "Found Boost at ${Boost_INCLUDE_DIR}")

add_compile_options("-Ofast -fno-fast-math -march=native")

add_executable(Simplex
        modules/glad/glad.c
        main.cpp
)

file(GLOB imgui_src "modules/imgui/**.cpp")
add_library(imgui ${imgui_src})

include_directories(Simplex PUBLIC modules modules/glm include src)
include_directories(Simplex PUBLIC ${SDL2_INCLUDE_DIR} ${Boost_INCLUDE_DIR})


target_link_libraries(Simplex PRIVATE ${Boost_LIBRARY})
target_link_libraries(Simplex PRIVATE ${SDL2_LIBRARY})

add_subdirectory(src/frontend)
add_subdirectory(src/simplex)

target_link_libraries(Simplex PRIVATE frontend simplex imgui)