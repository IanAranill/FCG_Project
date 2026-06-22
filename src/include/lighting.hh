#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

class Lighting {
   public:
    // --- Mappe per le proprietà Uniform ---
    std::unordered_map<std::string, glm::vec3> vec3_uniforms;
    std::unordered_map<std::string, float> float_uniforms;

    Lighting() {
        // Valori iniziali Luce
        vec3_uniforms["light.direct_pos"] = glm::vec3(0.0f, 1.5f, 0.0f);
        vec3_uniforms["light.direct_val"] = glm::vec3(1.0f, 1.0f, 1.0f);
        vec3_uniforms["light.ambient"] = glm::vec3(0.2f, 0.2f, 0.2f);

        // Valori iniziali Materiale
        vec3_uniforms["material.diffuse"] = glm::vec3(0.1f, 0.7f, 0.8f);
        vec3_uniforms["material.specular"] = glm::vec3(0.5f, 0.5f, 0.5f);
        vec3_uniforms["material.ambient"] = glm::vec3(0.1f, 0.7f, 0.8f);

        float_uniforms["material.shininess"] = 64.0f;
    }

    void push_to_shader(GLuint program) const {
        for (const auto& [name, value] : vec3_uniforms) {
            glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value));
        }

        for (const auto& [name, value] : float_uniforms) {
            glUniform1f(glGetUniformLocation(program, name.c_str()), value);
        }
    }
};