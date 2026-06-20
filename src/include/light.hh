#pragma once
#include <glm/glm.hpp>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 ambient_val;

    Light() {
        position = glm::vec3(0.0f, 3.0f, 0.0f);
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        ambient_val = glm::vec3(0.2f, 0.2f, 0.2f);
    }

    void push_to_shader(GLuint program) const {
        glUniform3fv(glGetUniformLocation(program, "light.direct_pos"), 1, &position[0]);
        glUniform3fv(glGetUniformLocation(program, "light.direct_val"), 1, &color[0]);
        glUniform3fv(glGetUniformLocation(program, "light.ambient_val"), 1, &ambient_val[0]);
    }
};