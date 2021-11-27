add_library(frontend STATIC frontend.cpp imgui_sdl/imgui_impl_sdl.cpp imgui_sdl/imgui_impl_opengl3.cpp)

if (NOT WIN32)
    target_link_libraries(Simplex PRIVATE -ldl -lpthread)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # copy over SDL2.dll, does not happen automatically for me for some reason
    message("Copying SDL2.dll to ${CMAKE_BINARY_DIR}")
    file(COPY ${CMAKE_SOURCE_DIR}/files/SDL2.dll
            DESTINATION ${CMAKE_BINARY_DIR})
endif()