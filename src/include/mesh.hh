#pragma once
#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Mesh {
   public:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // --- Variabili di Trasformazione (Aggiunte per compatibilità col Main) ---
    unsigned int VAO, VBO, EBO;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::mat4 model_matrix = glm::mat4(1.0f);

    // --- Costruttore ---
    Mesh(const std::string& filename, bool smooth_normals = true) {
        if (!load_off(filename)) {
            std::cerr << "Errore critico durante il caricamento della mesh.\n";
            exit(1);
        }

        normalize_mesh();
        if (smooth_normals) {
            compute_smooth_normals();
        } else {
            compute_flat_normals();
        }
        pack_for_gpu();
    }

   private:
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> temp_indices;

    // --- Parsing File OFF ---
    bool load_off(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Errore: Impossibile aprire il file: " << filename << std::endl;
            return false;
        }

        std::string line;

        std::getline(file, line);
        if (line.find("OFF") == std::string::npos) {
            std::cerr << "Errore: Il file non ha l'intestazione OFF.\n";
            return false;
        }

        int vnum = 0, fnum = 0, enum_edges = 0;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);
            ss >> vnum >> fnum >> enum_edges;
            break;
        }

        temp_positions.reserve(vnum);
        for (int i = 0; i < vnum; ++i) {
            std::getline(file, line);
            std::stringstream ss(line);
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }

        for (int i = 0; i < fnum; ++i) {
            std::getline(file, line);
            std::stringstream ss(line);
            int num_verts;
            ss >> num_verts;

            std::vector<unsigned int> face_verts(num_verts);
            for (int j = 0; j < num_verts; ++j) {
                ss >> face_verts[j];
            }

            for (int j = 1; j < num_verts - 1; ++j) {
                temp_indices.push_back(face_verts[0]);
                temp_indices.push_back(face_verts[j]);
                temp_indices.push_back(face_verts[j + 1]);
            }
        }
        return true;
    }

    // --- Normalizzazione (Bounding Box) ---
    void normalize_mesh() {
        if (temp_positions.empty()) return;

        // Trova i limiti minimi e massimi della mesh
        glm::vec3 min_bounds = temp_positions[0];
        glm::vec3 max_bounds = temp_positions[0];

        for (const auto& pos : temp_positions) {
            min_bounds = glm::min(min_bounds, pos);
            max_bounds = glm::max(max_bounds, pos);
        }

        // Calcola il centro esatto dell'oggetto
        glm::vec3 center = (min_bounds + max_bounds) * 0.5f;

        // Calcola l'estensione massima per mantenere le proporzioni originali
        glm::vec3 extents = max_bounds - min_bounds;
        float max_extent = std::max({extents.x, extents.y, extents.z});

        // Applica la trasformazione: Sposta al centro e scala a grandezza massima 1.0
        for (auto& pos : temp_positions) {
            pos = (pos - center) / max_extent;
        }
    }

    // --- Smooth Shading ---
    void compute_smooth_normals() {
        temp_normals.assign(temp_positions.size(), glm::vec3(0.0f));

        for (size_t i = 0; i < temp_indices.size(); i += 3) {
            unsigned int i0 = temp_indices[i];
            unsigned int i1 = temp_indices[i + 1];
            unsigned int i2 = temp_indices[i + 2];

            glm::vec3 v0 = temp_positions[i0];
            glm::vec3 v1 = temp_positions[i1];
            glm::vec3 v2 = temp_positions[i2];

            glm::vec3 face_normal = glm::cross(v1 - v0, v2 - v0);

            temp_normals[i0] += face_normal;
            temp_normals[i1] += face_normal;
            temp_normals[i2] += face_normal;
        }

        for (auto& normal : temp_normals) {
            normal = glm::normalize(normal);
        }
    }

   void compute_flat_normals() {
        // Creiamo nuovi vettori "srotolati" per non sovrascrivere i vertici condivisi
        std::vector<glm::vec3> new_positions;
        std::vector<glm::vec3> new_normals;
        std::vector<unsigned int> new_indices;

        for (size_t i = 0; i < temp_indices.size(); i += 3) {
            unsigned int i0 = temp_indices[i];
            unsigned int i1 = temp_indices[i + 1];
            unsigned int i2 = temp_indices[i + 2];

            glm::vec3 v0 = temp_positions[i0];
            glm::vec3 v1 = temp_positions[i1];
            glm::vec3 v2 = temp_positions[i2];

            // 1. Calcoliamo la normale della faccia
            glm::vec3 face_normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            // 3. Duplichiamo i vertici "scollandoli" dagli altri triangoli
            new_positions.push_back(v0);
            new_positions.push_back(v1);
            new_positions.push_back(v2);

            // Assegniamo la stessa normale piatta a tutti e 3 i vertici
            new_normals.push_back(face_normal);
            new_normals.push_back(face_normal);
            new_normals.push_back(face_normal);

            // 4. Aggiorniamo gli indici per puntare ai nuovi vertici appena creati
            unsigned int current_idx = new_positions.size() - 3;
            new_indices.push_back(current_idx);
            new_indices.push_back(current_idx + 1);
            new_indices.push_back(current_idx + 2);
        }

        // Sovrascriviamo i vecchi vettori: ora pack_for_gpu() funzionerà perfettamente!
        temp_positions = new_positions;
        temp_normals = new_normals;
        temp_indices = new_indices;
    }

    // --- GPU Packing ---
    void pack_for_gpu() {
        vertices.reserve(temp_positions.size() * 6);
        for (size_t i = 0; i < temp_positions.size(); ++i) {
            vertices.push_back(temp_positions[i].x);
            vertices.push_back(temp_positions[i].y);
            vertices.push_back(temp_positions[i].z);
            vertices.push_back(temp_normals[i].x);
            vertices.push_back(temp_normals[i].y);
            vertices.push_back(temp_normals[i].z);
        }
        indices = temp_indices;
    }
};