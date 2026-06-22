#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef GLAD_GL
#define GLAD_GL
#include "../glad/gl.h"
#endif

enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
   public:
    // --- Vettori Direzionali ---
    glm::vec3 position{0.0f, 0.0f, 3.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    glm::vec3 world_up{0.0f, 1.0f, 0.0f};

    // --- Angoli di Eulero ---
    float yaw{-90.0f};
    float pitch{0.0f};

    // --- Impostazioni ---
    float movement_speed{3.0f};
    float mouse_sensitivity{0.05f};
    float fov{45.0f};

    Camera(glm::vec3 start_position = glm::vec3(0.0f, 0.0f, 3.0f)) : position(start_position) {
        update_camera_vectors();
    }

    // --- Matrici ---
    [[nodiscard]] glm::mat4 get_view_matrix() const {
        return glm::lookAt(position, position + front, up);
    }

    [[nodiscard]] glm::mat4 get_projection_matrix(float aspect_ratio) const {
        return glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 100.0f);
    }

    // --- Gestione Shader ---
    void push_to_shader(GLuint program, float aspect_ratio) const {
        glm::mat4 view = get_view_matrix();
        glm::mat4 projection = get_projection_matrix(aspect_ratio);
        glm::mat4 vp = projection * view;

        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));

        // RIPRISTINATO: Matrice VP fondamentale per la coerenza geometrica
        glUniformMatrix4fv(glGetUniformLocation(program, "vp"), 1, GL_FALSE, glm::value_ptr(vp));
        glUniform3fv(glGetUniformLocation(program, "camera_pos"), 1, glm::value_ptr(position));
    }

    // --- Gestione Input ---
    void process_keyboard(CameraMovement direction, float delta_time) {
        float velocity = movement_speed * delta_time;
        if (direction == CameraMovement::FORWARD) position += front * velocity;
        if (direction == CameraMovement::BACKWARD) position -= front * velocity;
        if (direction == CameraMovement::LEFT) position -= right * velocity;
        if (direction == CameraMovement::RIGHT) position += right * velocity;
    }

    void process_mouse_drag(float x_offset, float y_offset) {
        yaw += x_offset * mouse_sensitivity;
        pitch -= y_offset * mouse_sensitivity;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        update_camera_vectors();
    }

   private:
    void update_camera_vectors() {
        glm::vec3 new_front;
        new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        new_front.y = sin(glm::radians(pitch));
        new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(new_front);
        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
    }
};