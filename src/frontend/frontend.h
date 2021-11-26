#pragma once

#include "camera.h"
#include "default.h"
#include "simplex/simplex.h"


struct Frontend {
    static constexpr int Width = 1280;
    static constexpr int Height = 720;

    static constexpr float LookSensitivity = 0.2;
    static constexpr float MoveSensitivity = 0.1;

    Simplex simplex;

    Frontend(Simplex&& simplex) : simplex(std::move(simplex)) {

    }

    void InitSDL();
    void InitOGL();
    void HandleInput();
    void Run();

private:
    bool shutdown = false;
    bool trap_mouse = true;

    Camera camera;

    void* window;
    void* gl_context;
    u32 program;
    u32 view;
    u32 proj;
    u32 vao;
    u32 vbo;
};