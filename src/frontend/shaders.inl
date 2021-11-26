#pragma once

#include "glad/glad.h"

#include <vector>


const char* vertex_shader_source = R"(
    #version 330 core

    layout (location = 0) in vec3 pos;

    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * vec4(pos, 1.0);
        gl_PointSize = 4.0;
    }
)";

const char* fragment_shader_source = R"(
    #version 330 core
    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
)";

static GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> log(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, log.data());

        glDeleteShader(shader);
        std::printf("Shader compilation failed: %s\n", log.data());
    }
    return shader;
}