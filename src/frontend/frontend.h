#pragma once

#include "camera.h"
#include "default.h"
#include "simplex/simplex.h"

#include <future>
#include <optional>


struct Frontend {
    static constexpr int Width = 1280;
    static constexpr int Height = 720;

    static constexpr float LookSensitivity = 0.2;
    static constexpr float MoveSensitivity = 0.1;

    Simplex simplex;

    Frontend(Simplex&& simplex);

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
    u32 ebo;

    void InitSDL();
    void InitOGL();
    void InitImGui();
    void HandleInput();

    bool menu_open = true;
    int dimension = 0;
    float epsilon = 0.1;
    void DrawMenu();

    u32 draw_type;
    u32 next_draw_type;
    std::future<std::vector<i32>> command{};
    void CheckCommand();
};