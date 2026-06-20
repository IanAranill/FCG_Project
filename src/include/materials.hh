#pragma once
#include <glm/glm.hpp>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

struct Material {
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    float shininess;

    Material() {
        diffuse = glm::vec3(0.1f, 0.7f, 0.8f);
        specular = glm::vec3(0.5f, 0.5f, 0.5f);
        ambient = glm::vec3(0.1f, 0.7f, 0.8f);
        shininess = 64.0f;
    }

    void push_to_shader(GLuint program) const {
        glUniform3fv(glGetUniformLocation(program, "material.diffuse"), 1, &diffuse[0]);
        glUniform3fv(glGetUniformLocation(program, "material.specular"), 1, &specular[0]);
        glUniform3fv(glGetUniformLocation(program, "material.ambient"), 1, &ambient[0]);
        glUniform1f(glGetUniformLocation(program, "material.shininess"), shininess);
    }
};