#pragma once
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Mesh {
   public:
    // Dati pronti per essere inviati alla GPU (x,y,z, nx,ny,nz)
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Mesh(const std::string& filename) {
        if (!load_off(filename)) {
            std::cerr << "Errore critico durante il caricamento della mesh.\n";
            exit(1);
        }
        compute_smooth_normals();
        pack_for_gpu();
    }

   private:
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> temp_indices;

    // Lettura del file OFF
    bool load_off(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Errore: Impossibile aprire il file: " << filename << std::endl;
            return false;
        }

        std::string line;

        // Cerca intestazione OFF
        std::getline(file, line);
        if (line.find("OFF") == std::string::npos) {
            std::cerr << "Errore: Il file non ha l'intestazione OFF.\n";
            return false;
        }

        // Salta commenti e righe vuote per leggere vnum, fnum, e edges
        int vnum = 0, fnum = 0, enum_edges = 0;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);
            ss >> vnum >> fnum >> enum_edges;
            break;
        }

        // Lettura Posizioni
        temp_positions.reserve(vnum);
        for (int i = 0; i < vnum; ++i) {
            std::getline(file, line);
            std::stringstream ss(line);
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }

        // Lettura Facce (Gestisce triangoli e poligoni a N lati)
        for (int i = 0; i < fnum; ++i) {
            std::getline(file, line);
            std::stringstream ss(line);
            int num_verts;
            ss >> num_verts;

            std::vector<unsigned int> face_verts(num_verts);
            for (int j = 0; j < num_verts; ++j) {
                ss >> face_verts[j];
            }

            // Triangolazione: converte poligoni complessi in triangoli per OpenGL
            for (int j = 1; j < num_verts - 1; ++j) {
                temp_indices.push_back(face_verts[0]);
                temp_indices.push_back(face_verts[j]);
                temp_indices.push_back(face_verts[j + 1]);
            }
        }
        return true;
    }

    // Calcolo Normali (Smooth Shading)
    void compute_smooth_normals() {
        temp_normals.assign(temp_positions.size(), glm::vec3(0.0f));

        // Calcola la normale per ogni triangolo e la accumula ai suoi 3 vertici
        for (size_t i = 0; i < temp_indices.size(); i += 3) {
            unsigned int i0 = temp_indices[i];
            unsigned int i1 = temp_indices[i + 1];
            unsigned int i2 = temp_indices[i + 2];

            glm::vec3 v0 = temp_positions[i0];
            glm::vec3 v1 = temp_positions[i1];
            glm::vec3 v2 = temp_positions[i2];

            // Prodotto vettoriale per trovare la perpendicolare della faccia
            glm::vec3 face_normal = glm::cross(v1 - v0, v2 - v0);

            temp_normals[i0] += face_normal;
            temp_normals[i1] += face_normal;
            temp_normals[i2] += face_normal;
        }

        // Normalizza
        for (auto& normal : temp_normals) {
            normal = glm::normalize(normal);
        }
    }

    // Preparazione per la GPU
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