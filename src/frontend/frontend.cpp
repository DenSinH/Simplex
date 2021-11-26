#include "frontend.h"

#include <SDL.h>

#include "shaders.inl"

constexpr float pi = 3.14159265;

void Frontend::InitSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    // Decide GL+GLSL versions
    const char* glsl_version = "#version 330";
#if __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    auto window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | // SDL_WINDOW_RESIZABLE |
                                           SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Simplex", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, window_flags);
    gl_context = SDL_GL_CreateContext((SDL_Window*)window);
    SDL_GL_MakeCurrent((SDL_Window*)window, gl_context);
    SDL_GL_SetSwapInterval(1); // vsync

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
}

void Frontend::InitOGL() {
    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glUseProgram(program);
    view = glGetUniformLocation(program, "view");
    proj = glGetUniformLocation(program, "projection");
    glUseProgram(0);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point3d) * simplex.points.size(), simplex.points.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void Frontend::HandleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
//            ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
            case SDL_QUIT: {
                shutdown = true;
                break;
            }
            case SDL_WINDOWEVENT: {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID((SDL_Window*)window)) {
                    shutdown = true;
                }
                break;
            }
            case SDL_MOUSEMOTION: {
                if (trap_mouse) {
                    camera.Look(event.motion.xrel * LookSensitivity, -event.motion.yrel * LookSensitivity);
                }
                break;
            }
            case SDL_KEYDOWN: {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    trap_mouse ^= true;
                    SDL_SetRelativeMouseMode((SDL_bool)trap_mouse);
                    SDL_ShowCursor(!trap_mouse);
                }
                break;
            }
            default:
                break;
        }
    }

    auto keyboard = SDL_GetKeyboardState(nullptr);
    if (keyboard[SDL_SCANCODE_W]) camera.pos += MoveSensitivity * camera.front;
    if (keyboard[SDL_SCANCODE_S]) camera.pos -= MoveSensitivity * camera.front;
    if (keyboard[SDL_SCANCODE_D]) camera.pos += glm::normalize(glm::cross(camera.front, camera.up)) * MoveSensitivity;
    if (keyboard[SDL_SCANCODE_A]) camera.pos -= glm::normalize(glm::cross(camera.front, camera.up)) * MoveSensitivity;
    if (keyboard[SDL_SCANCODE_SPACE]) camera.pos += glm::vec3(0, MoveSensitivity, 0);
    if (keyboard[SDL_SCANCODE_LSHIFT]) camera.pos -= glm::vec3(0, MoveSensitivity, 0);
}

void Frontend::Run() {
    InitSDL();
    InitOGL();

    SDL_SetRelativeMouseMode((SDL_bool)trap_mouse);
    SDL_ShowCursor(!trap_mouse);
    
    while (!shutdown) {
        HandleInput();

            // Start the Dear ImGui frame
//        ImGui_ImplOpenGL3_NewFrame();
//        ImGui_ImplSDL2_NewFrame(window);
//        ImGui::NewFrame();

            // ImGui::ShowDemoWindow();

            // Rendering
//        ImGui::Render();

#ifdef SHOW_EXAMPLE_MENU
            ImGui::ShowDemoWindow(NULL);
#endif

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 1280, 720);

        glUseProgram(program);

        glm::mat4 proj_mat = glm::perspective(glm::radians(camera.fov), (float)1280 / (float)720, 0.1f, 100.0f);
        glm::mat4 view_mat = glm::lookAt(camera.pos, camera.pos + camera.front, camera.up);
        glUniformMatrix4fv(proj, 1, false, (const float*)glm::value_ptr(proj_mat));
        glUniformMatrix4fv(view, 1, false, (const float*)glm::value_ptr(view_mat));

        glBindVertexArray(vao);

        glDrawArrays(GL_POINTS, 0, simplex.points.size());

        glBindVertexArray(0);
        glUseProgram(0);

        // then draw the imGui stuff over it
//        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // frameswap
        SDL_GL_SwapWindow((SDL_Window*)window);
    }

    // Cleanup
//    ImGui_ImplOpenGL3_Shutdown();
//    ImGui_ImplSDL2_Shutdown();
//    ImGui::DestroyContext();

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow((SDL_Window*)window);
    SDL_Quit();
}