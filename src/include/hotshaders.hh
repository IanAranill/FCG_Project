#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

class Shaders {
   public:
    // Cache per memorizzare le posizioni delle variabili uniform
    GLuint program{0};

    Shaders(const std::string& vertexPath, const std::string& fragmentPath) {
        reload(vertexPath, fragmentPath);
    }

    // Distruttore: pulisce la memoria GPU
    ~Shaders() {
        if (program != 0) {
            glDeleteProgram(program);
        }
    }

    // Evita copie accidentali che causerebbero "double-free" sulla GPU
    Shaders(const Shaders&) = delete;
    Shaders& operator=(const Shaders&) = delete;

    void use() const {
        if (program != 0) {
            glUseProgram(program);
        }
    }

    void reload(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexCode = read_file(vertexPath);
        std::string fragmentCode = read_file(fragmentPath);

        if (vertexCode.empty() || fragmentCode.empty()) return;

        GLuint vertex = compile_shader(GL_VERTEX_SHADER, vertexCode, vertexPath);
        GLuint fragment = compile_shader(GL_FRAGMENT_SHADER, fragmentCode, fragmentPath);

        if (!vertex || !fragment) return;

        GLuint new_program = glCreateProgram();
        glAttachShader(new_program, vertex);
        glAttachShader(new_program, fragment);
        glLinkProgram(new_program);

        int success;
        glGetProgramiv(new_program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(new_program, 512, NULL, infoLog);
            std::cerr << "ERRORE LINKING SHADER:\n" << infoLog << std::endl;
            glDeleteProgram(new_program);
        } else {
            if (program != 0) glDeleteProgram(program);
            program = new_program;
            std::cout << "Shader ricaricati con successo: " << vertexPath << " & " << fragmentPath
                      << std::endl;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

   private:
    std::string read_file(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Impossibile aprire il file shader: " << filepath << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    GLuint compile_shader(GLenum type, const std::string& source, const std::string& name) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERRORE COMPILAZIONE SHADER (" << name << "):\n" << infoLog << std::endl;
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }
};