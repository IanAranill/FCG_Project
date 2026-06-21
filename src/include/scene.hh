#pragma once
#include <vector>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

class Scene {
   private:
    GLuint vbo;
    GLuint ebo;
    GLuint vao;
    std::size_t index_count;

   public:
    Scene(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        index_count = indices.size();
        setup_mesh(vertices, indices);
    }

    ~Scene() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    void draw() const {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

   private:
    void setup_mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(),
                     GL_STATIC_DRAW);

        // Attributo 0: Posizione (x, y, z)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Attributo 1: Normale (nx, ny, nz)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};