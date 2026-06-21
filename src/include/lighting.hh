#pragma once
#include <glm/glm.hpp>
#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

class Lighting {
   public:
    // --- Proprietà Esplicite della Luce ---
    glm::vec3 light_position;
    glm::vec3 light_color;
    glm::vec3 light_ambient;

    // --- Proprietà Esplicite del Materiale ---
    glm::vec3 material_diffuse;
    glm::vec3 material_specular;
    glm::vec3 material_ambient;
    float material_shininess;

    Lighting() {
        // Valori iniziali Luce
        light_position = glm::vec3(0.0f, 5.0f, 0.0f);
        light_color = glm::vec3(1.0f, 1.0f, 1.0f);
        light_ambient = glm::vec3(0.2f, 0.2f, 0.2f);

        // Valori iniziali Materiale
        material_diffuse = glm::vec3(0.1f, 0.7f, 0.8f);
        material_specular = glm::vec3(0.5f, 0.5f, 0.5f);
        material_ambient = glm::vec3(0.1f, 0.7f, 0.8f);
        material_shininess = 64.0f;
    }

    // Invia i dati di Luce e Materiale allo shader
    // (I nomi tra virgolette corrispondono alle variabili uniform nel fragment.frag)
    void push_to_shader(GLuint program) const {
        glUniform3fv(glGetUniformLocation(program, "light.direct_pos"), 1, &light_position[0]);
        glUniform3fv(glGetUniformLocation(program, "light.direct_val"), 1, &light_color[0]);
        glUniform3fv(glGetUniformLocation(program, "light.ambient_val"), 1, &light_ambient[0]);

        glUniform3fv(glGetUniformLocation(program, "material.diffuse"), 1, &material_diffuse[0]);
        glUniform3fv(glGetUniformLocation(program, "material.specular"), 1, &material_specular[0]);
        glUniform3fv(glGetUniformLocation(program, "material.ambient"), 1, &material_ambient[0]);
        glUniform1f(glGetUniformLocation(program, "material.shininess"), material_shininess);
    }
};