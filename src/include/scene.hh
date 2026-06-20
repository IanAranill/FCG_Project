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
    size_t index_count;

public:
    // Costruttore che accetta direttamente i dati grezzi (vertici e indici)
    // Nessuna dipendenza da file esterni!
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
        glBindVertexArray(0); // Pulizia dello stato
    }

private:
    void setup_mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        // Generazione dei buffer
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        // Binding del VAO (registra tutte le configurazioni successive)
        glBindVertexArray(vao);

        // Caricamento Vertici (Posizione + Normale)
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Caricamento Indici (L'ordine in cui unire i vertici per formare i triangoli)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Attributo 0: Posizione (x, y, z)
        // Lo stride è 6 * sizeof(float) perché ogni vertice ha 3 coordinate per la posizione e 3 per la normale
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Attributo 1: Normale (nx, ny, nz)
        // Inizia dopo i primi 3 float
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Unbind del VAO per evitare modifiche accidentali
        glBindVertexArray(0); 
    }
};