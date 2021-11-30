#pragma once

#include "glad/glad.h"

#include <vector>


const char* vertex_shader_source = R"(
    #version 330 core

    layout (location = 0) in vec3 pos;

    out vec3 color;

    uniform mat4 view;
    uniform mat4 projection;

    float rand(vec2 co){
        return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
    }

    void main()
    {
        gl_Position = projection * view * vec4(pos, 1.0);
        gl_PointSize = 4.0;

        color = vec3(0.2, 0.2, 0.2) + 0.8 * vec3(rand(pos.xy), rand(pos.yx), rand(pos.xz));
    }
)";

const char* fragment_shader_source = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 color;

    uniform float alpha = 0.5;

    void main()
    {
        FragColor = vec4(color, alpha);
    }
)";

static GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
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