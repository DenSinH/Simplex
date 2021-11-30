#pragma once

#include "camera.h"
#include "default.h"
#include "compute/compute.h"

#include <future>
#include <optional>
#include <chrono>
#include <boost/container/static_vector.hpp>


struct Frontend {
    static constexpr int Width = 1280;
    static constexpr int Height = 720;

    static constexpr float LookSensitivity = 0.2;
    static constexpr float MoveSensitivity = 0.1;

    std::unique_ptr<ComputeBase> compute;

    Frontend(std::unique_ptr<ComputeBase>&& simplex);

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
    std::array<u32, 3> ebo;
    u32 hebo;

    void InitSDL();
    void InitOGL();
    void InitImGui();
    void HandleInput();

    bool menu_open = true;
    int projection = 0;

    int dimension = 0;
    float epsilon = 0.1;
    void DrawMenu();

    std::array<size_t, 3> no_vertices;
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::duration<double> duration = std::chrono::milliseconds(0);
    std::future<boost::container::static_vector<std::vector<i32>, 3>> simplex_indices_future{};
    void CheckSimplexIndexCommand();


    using point_t = typename ComputeBase::point_t;

    int homology_dim = 0;
    bool show_homology = false;
    size_t h_basis_size = 0;
    size_t h_basis_vertices = 0;
    std::future<std::pair<size_t, std::vector<i32>>> h_basis_future{};
    void CheckHomologyBasisCommand();
};