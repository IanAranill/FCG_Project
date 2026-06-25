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
    // --- Proprietà della camera ---
    glm::vec3 cam_pos{0.0f, 0.0f, 3.0f};
    glm::mat4 vp;
    GLint cam_pos_loc;
    GLint vp_loc;

    // --- Vettori Direzionali ---
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

    Camera(GLuint program, glm::vec3 start_position = glm::vec3(0.0f, 0.0f, 3.0f))
        : cam_pos(start_position) {
        cam_pos_loc = glGetUniformLocation(program, "cam_pos");
        vp_loc = glGetUniformLocation(program, "vp");
        update_camera_vectors();
    }

    // --- Generazione matrici ---
    [[nodiscard]] glm::mat4 get_view_matrix() const {
        return glm::lookAt(cam_pos, cam_pos + front, up);
    }

    [[nodiscard]] glm::mat4 get_projection_matrix(float aspect_ratio) const {
        return glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 100.0f);
    }

    // --- Gestione Shader ---
    // Invio dei parametri della camera allo shader
    void push_to_shader(float aspect_ratio) {
        glm::mat4 view = get_view_matrix();
        glm::mat4 projection = get_projection_matrix(aspect_ratio);
        vp = projection * view;

        glUniformMatrix4fv(vp_loc, 1, GL_FALSE, glm::value_ptr(vp));
        glUniform3fv(cam_pos_loc, 1, glm::value_ptr(cam_pos));
    }

    // --- Gestione input esterno ---
    void process_keyboard(CameraMovement direction, float delta_time) {
        float velocity = movement_speed * delta_time;
        if (direction == CameraMovement::FORWARD) cam_pos += front * velocity;
        if (direction == CameraMovement::BACKWARD) cam_pos -= front * velocity;
        if (direction == CameraMovement::LEFT) cam_pos -= right * velocity;
        if (direction == CameraMovement::RIGHT) cam_pos += right * velocity;
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