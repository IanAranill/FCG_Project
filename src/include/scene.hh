#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

#include "mesh.hh"

class Scene {
   private:
    std::vector<Mesh*> meshes;
    GLint model_loc;
    GLint normal_mat_loc;

   public:
    Scene(GLuint program) {
        model_loc = glGetUniformLocation(program, "model");
        normal_mat_loc = glGetUniformLocation(program, "normal_matrix");
    }

    // --- Gestione della Scena ---

    void add_mesh(Mesh* mesh) {
        if (mesh) {
            meshes.push_back(mesh);
        }
    }

    void clear() { meshes.clear(); }

    // --- Rendering ---

    void draw() const {
        // Cicla  attraverso tutte le mesh registrate nella scena
        for (const Mesh* mesh : meshes) {
            if (mesh) {
                // Generazione e Push della Model Matrix e della Normal Matrix per ogni singolo
                // oggetto
                mesh->push_material_to_shader();
                glm::mat4 model_matrix = mesh->get_model_matrix();
                glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));

                glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
                glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, glm::value_ptr(normal_matrix));

                mesh->draw();
            }
        }
    }
};