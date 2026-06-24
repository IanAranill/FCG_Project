#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "mesh.hh"

class Mirror {
   public:
    Mesh mesh;
    glm::vec3 position;
    glm::vec3 normal;

    Mirror(const std::string& mesh_path, glm::vec3 pos, glm::vec3 norm)
        : mesh(mesh_path, false), position(pos), normal(glm::normalize(norm)) {
        mesh.position = pos;
    }

    glm::mat4 get_reflection_matrix() const {
        glm::vec3 N = normal;
        glm::vec3 P = position;
        float d = glm::dot(P, N);

        return glm::mat4(1.0f - 2.0f * N.x * N.x, -2.0f * N.y * N.x, -2.0f * N.z * N.x, 0.0f,
                         -2.0f * N.x * N.y, 1.0f - 2.0f * N.y * N.y, -2.0f * N.z * N.y, 0.0f,
                         -2.0f * N.x * N.z, -2.0f * N.y * N.z, 1.0f - 2.0f * N.z * N.z, 0.0f,
                         2.0f * d * N.x, 2.0f * d * N.y, 2.0f * d * N.z, 1.0f);
    }

    void update_mesh_transform() { mesh.position = position; }
};