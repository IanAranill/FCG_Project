#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

class Light {
   public:
    // --- Mappa per le proprietà Uniform ---
    std::unordered_map<std::string, glm::vec3> vec3_uniforms;

    Light() {
        // Valori iniziali Luce
        vec3_uniforms["light.direct_pos"] = glm::vec3(0.0f, 1.5f, 0.0f);
        vec3_uniforms["light.direct_val"] = glm::vec3(1.0f, 1.0f, 1.0f);
        vec3_uniforms["light.ambient_val"] = glm::vec3(0.2f, 0.2f, 0.2f);
    }

    void push_to_shader(GLuint program) const {
        for (const auto& [name, value] : vec3_uniforms) {
            glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value));
        }
    }
};