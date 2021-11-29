#pragma once

#include "camera.h"
#include "default.h"
#include "compute/compute.h"

#include <future>
#include <optional>
#include <chrono>


struct Frontend {
    static constexpr int Width = 1280;
    static constexpr int Height = 720;

    static constexpr float LookSensitivity = 0.2;
    static constexpr float MoveSensitivity = 0.1;

    Compute<MAX_POINTS> compute;

    Frontend(Compute<MAX_POINTS>&& simplex);

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
    std::vector<i32> (Compute<MAX_POINTS>::*command)(float);
    size_t no_vertices;
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::duration<double> duration = std::chrono::milliseconds(0);
    std::future<std::vector<i32>> simplex_indices_future{};
    void CheckSimplexIndexCommand();

    int homology_dim = 0;
    std::vector<typename Compute<MAX_POINTS>::simplex_t> homology_basis{};
    std::future<std::vector<typename Compute<MAX_POINTS>::simplex_t>> homology_basis_future{};
    void CheckHomologyBasisCommand();
};