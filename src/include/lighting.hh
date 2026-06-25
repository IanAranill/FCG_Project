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
    // --- Proprietà uniform ---
    glm::vec3 direct_pos;
    glm::vec3 direct_val;
    glm::vec3 ambient_val;
    GLint direct_pos_loc;
    GLint direct_val_loc;
    GLint ambient_val_loc;

    Light(GLuint program) {
        // Impostazione dei valori iniziali della luce
        direct_pos = glm::vec3(0.0f, 1.5f, 0.0f);
        direct_val = glm::vec3(1.0f, 1.0f, 1.0f);
        ambient_val = glm::vec3(0.2f, 0.2f, 0.2f);
        direct_pos_loc = glGetUniformLocation(program, "light.direct_pos");
        direct_val_loc = glGetUniformLocation(program, "light.direct_val");
        ambient_val_loc = glGetUniformLocation(program, "light.ambient_val");
    }

    void push_to_shader() const {
        glUniform3fv(direct_pos_loc, 1, glm::value_ptr(direct_pos));
        glUniform3fv(direct_val_loc, 1, glm::value_ptr(direct_val));
        glUniform3fv(ambient_val_loc, 1, glm::value_ptr(ambient_val));
    }
};