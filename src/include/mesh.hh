#pragma once

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

// --- Struttura dati vertice ---
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

class Mesh {
   public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // --- Identificatori risorse OpenGL ---
    GLuint VAO{0}, VBO{0}, EBO{0};

    // --- Parametri di trasformazione affine ---
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    glm::vec3 material_diffuse;
    glm::vec3 material_specular;
    glm::vec3 material_ambient;
    float material_shininess;
    GLint material_diffuse_loc;
    GLint material_specular_loc;
    GLint material_ambient_loc;
    GLint material_shininess_loc;

    // --- Ciclo di vita e gestione della memoria ---
    Mesh() = default;

    Mesh(GLuint program, const std::string& filename, bool smooth_normals = true) {
        if (!load_off(filename)) {
            std::cerr << ">>> [ERRORE CRITICO MESH] Impossibile caricare: " << filename << "\n";
            return;
        }

        normalize_mesh();

        if (smooth_normals) {
            compute_smooth_normals();
        } else {
            compute_flat_normals();
        }

        // Inizializzazione parametri del materiale
        material_diffuse = glm::vec3(0.1f, 0.7f, 0.8f);
        material_specular = glm::vec3(0.5f, 0.5f, 0.5f);
        material_ambient = glm::vec3(0.1f, 0.7f, 0.8f);
        material_shininess = 64.0f;

        // Memorizzazione (caching) delle location Uniform per l'ottimizzazione delle chiamate GPU
        material_diffuse_loc = glGetUniformLocation(program, "material.diffuse");
        material_specular_loc = glGetUniformLocation(program, "material.specular");
        material_ambient_loc = glGetUniformLocation(program, "material.ambient");
        material_shininess_loc = glGetUniformLocation(program, "material.shininess");

        build_vertices();
        setup_gl_resources();
    }

    ~Mesh() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // --- Funzioni di interfaccia rendering ---
    [[nodiscard]] glm::mat4 get_model_matrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }

    void draw() const {
        if (VAO == 0) return;
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void push_material_to_shader() const {
        glUniform3fv(material_diffuse_loc, 1, glm::value_ptr(material_diffuse));
        glUniform3fv(material_specular_loc, 1, glm::value_ptr(material_specular));
        glUniform3fv(material_ambient_loc, 1, glm::value_ptr(material_ambient));

        glUniform1f(material_shininess_loc, material_shininess);
    }

   private:
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> temp_indices;

    // --- Funzioni I/O file system ---
    bool load_off(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        std::string header;
        file >> header;
        if (header.substr(0, 3) != "OFF") return false;

        int num_vertices = 0, num_faces = 0, num_edges = 0;
        if (header.length() > 3) {
            num_vertices = std::stoi(header.substr(3));
            file >> num_faces >> num_edges;
        } else {
            file >> num_vertices >> num_faces >> num_edges;
        }

        temp_positions.reserve(num_vertices);
        for (int i = 0; i < num_vertices; ++i) {
            glm::vec3 pos;
            file >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }

        temp_indices.reserve(num_faces * 3);
        for (int i = 0; i < num_faces; ++i) {
            int n;
            file >> n;
            if (n != 3) {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            unsigned int idx0, idx1, idx2;
            file >> idx0 >> idx1 >> idx2;

            temp_indices.push_back(idx0);
            temp_indices.push_back(idx1);
            temp_indices.push_back(idx2);

            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        return true;
    }

    // --- Pipeline elaborazione geometria ---
    void normalize_mesh() {
        if (temp_positions.empty()) return;

        glm::vec3 min_bounds = temp_positions[0];
        glm::vec3 max_bounds = temp_positions[0];

        for (const auto& pos : temp_positions) {
            min_bounds = glm::min(min_bounds, pos);
            max_bounds = glm::max(max_bounds, pos);
        }

        glm::vec3 center = (min_bounds + max_bounds) / 2.0f;
        glm::vec3 dimensions = max_bounds - min_bounds;
        float max_dim = std::max({dimensions.x, dimensions.y, dimensions.z});

        if (max_dim == 0.0f) max_dim = 1.0f;

        for (auto& pos : temp_positions) {
            pos = (pos - center) / max_dim;
        }
    }

    // --- Pipeline calcolo normali ---
    void compute_smooth_normals() {
        temp_normals.assign(temp_positions.size(), glm::vec3(0.0f));

        for (size_t i = 0; i < temp_indices.size(); i += 3) {
            unsigned int i0 = temp_indices[i];
            unsigned int i1 = temp_indices[i + 1];
            unsigned int i2 = temp_indices[i + 2];

            glm::vec3 face_normal = glm::cross(temp_positions[i1] - temp_positions[i0],
                                               temp_positions[i2] - temp_positions[i0]);

            temp_normals[i0] += face_normal;
            temp_normals[i1] += face_normal;
            temp_normals[i2] += face_normal;
        }

        for (auto& normal : temp_normals) {
            if (glm::length(normal) > 1e-6f) {
                normal = glm::normalize(normal);
            } else {
                normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
        }
    }

    void compute_flat_normals() {
        std::vector<glm::vec3> new_positions;
        std::vector<glm::vec3> new_normals;
        std::vector<unsigned int> new_indices;

        size_t new_size = temp_indices.size();
        new_positions.reserve(new_size);
        new_normals.reserve(new_size);
        new_indices.reserve(new_size);

        for (size_t i = 0; i < temp_indices.size(); i += 3) {
            unsigned int i0 = temp_indices[i];
            unsigned int i1 = temp_indices[i + 1];
            unsigned int i2 = temp_indices[i + 2];

            glm::vec3 cross_prod = glm::cross(temp_positions[i1] - temp_positions[i0],
                                              temp_positions[i2] - temp_positions[i0]);

            glm::vec3 face_normal;
            if (glm::length(cross_prod) > 1e-6f) {
                face_normal = glm::normalize(cross_prod);
            } else {
                face_normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            for (unsigned int idx : {i0, i1, i2}) {
                new_positions.push_back(temp_positions[idx]);
                new_normals.push_back(face_normal);
                new_indices.push_back(static_cast<unsigned int>(new_positions.size() - 1));
            }
        }

        temp_positions = std::move(new_positions);
        temp_normals = std::move(new_normals);
        temp_indices = std::move(new_indices);
    }

    // --- Pipeline elaborazione memoria video ---
    void build_vertices() {
        vertices.reserve(temp_positions.size());
        for (size_t i = 0; i < temp_positions.size(); ++i) {
            Vertex vertex;
            vertex.position = temp_positions[i];
            vertex.normal = temp_normals[i];
            vertices.push_back(vertex);
        }
        indices = std::move(temp_indices);

        temp_positions.clear();
        temp_positions.shrink_to_fit();
        temp_normals.clear();
        temp_normals.shrink_to_fit();
    }

    void setup_gl_resources() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(),
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};