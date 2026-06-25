#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "camera.hh"
#include "lighting.hh"
#include "mesh.hh"
#include "scene.hh"

class Mirror {
   private:
    glm::vec4 clip_plane;
    GLint clip_plane_loc;

   public:
    Mesh mesh;
    glm::vec3 position;
    glm::vec3 normal;

    Mirror(GLuint program, const std::string& mesh_path, glm::vec3 pos, glm::vec3 norm)
        : mesh(program, mesh_path, false), position(pos), normal(glm::normalize(norm)) {
        mesh.position = pos;
        clip_plane_loc = glGetUniformLocation(program, "clip_plane");
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

    void render_scene_with_mirror(GLuint shader_program, Camera& camera, Scene& main_scene,
                                  Mirror& mirror, Light& scene_light, float aspect_ratio) {
        // ==========================================
        // FASE 1: Disegna la scena reale
        // ==========================================
        glDisable(GL_STENCIL_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);

        camera.push_to_shader(aspect_ratio);
        scene_light.push_to_shader();

        // Disegna Stanza e Coniglio
        main_scene.draw();

        // ==========================================
        // FASE 2: Disegna il Retro dello Specchio
        // ==========================================
        // Inverte il Culling per vedere SOLO la faccia posteriore dello specchio
        glCullFace(GL_FRONT);

        glm::vec3 orig_diff = mirror.mesh.material_diffuse;
        glm::vec3 orig_amb = mirror.mesh.material_ambient;
        glm::vec3 orig_spec = mirror.mesh.material_specular;

        // Crea un materiale scuro per il "corpo" dello specchio
        mirror.mesh.material_diffuse = glm::vec3(0.05f, 0.05f, 0.05f);
        mirror.mesh.material_ambient = glm::vec3(0.02f, 0.02f, 0.02f);
        mirror.mesh.material_specular = glm::vec3(0.01f, 0.01f, 0.01f);
        mirror.mesh.push_material_to_shader();

        GLint model_loc = glGetUniformLocation(shader_program, "model");
        GLint normal_mat_loc = glGetUniformLocation(shader_program, "normal_matrix");

        glm::mat4 mirror_model = mirror.mesh.get_model_matrix();
        glm::mat3 mirror_normal_back = -glm::transpose(glm::inverse(glm::mat3(mirror_model)));

        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(mirror_model));
        glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, glm::value_ptr(mirror_normal_back));

        // Disegnia la faccia posteriore (se visibile)
        mirror.mesh.draw();

        // ==========================================
        // FASE 3: Tela Scura e Maschera Stencil (Fronte dello Specchio)
        // ==========================================
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        glCullFace(GL_BACK);  // Torna a guardare il fronte dello specchio

        // Disegna fisicamente un rettangolo nero solido sul fronte dello specchio che verrà poi
        // sostituito dall'immagine da specchiare.
        glm::mat3 mirror_normal_front = glm::transpose(glm::inverse(glm::mat3(mirror_model)));
        glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, glm::value_ptr(mirror_normal_front));

        mirror.mesh.draw();

        // Ripristiniamo il materiale originale per ImGui
        mirror.mesh.material_diffuse = orig_diff;
        mirror.mesh.material_ambient = orig_amb;
        mirror.mesh.material_specular = orig_spec;

        // ==========================================
        // FASE 4: Scena Riflessa (con Hardware Clipping)
        // ==========================================
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glClear(GL_DEPTH_BUFFER_BIT);

        // --- CALCOLO DEL PIANO DI TAGLIO ---
        // Imposta il piano di taglio in World Space. La normale dello specchio
        // punta verso la stanza, quindi taglierà tutto ciò che si trova "dietro".
        clip_plane = glm::vec4(mirror.normal, -glm::dot(mirror.normal, mirror.position));
        glUniform4fv(clip_plane_loc, 1, glm::value_ptr(clip_plane));

        // Attiva il taglio hardware
        glEnable(GL_CLIP_DISTANCE0);

        // Setup della Seconda Camera
        glm::mat4 reflection_matrix = mirror.get_reflection_matrix();
        glm::mat4 view = camera.get_view_matrix();
        glm::mat4 projection = camera.get_projection_matrix(aspect_ratio);

        glm::mat4 reflected_view = view * reflection_matrix;
        glm::mat4 reflected_vp = projection * reflected_view;
        glUniformMatrix4fv(camera.vp_loc, 1, GL_FALSE, glm::value_ptr(reflected_vp));

        glFrontFace(GL_CW);

        glm::vec3 orig_light_pos = scene_light.direct_pos;
        glm::vec3 ref_light_pos = glm::vec3(reflection_matrix * glm::vec4(orig_light_pos, 1.0f));
        scene_light.direct_pos = ref_light_pos;
        scene_light.push_to_shader();

        glm::vec3 orig_cam_pos = camera.cam_pos;
        glm::vec3 ref_cam_pos = glm::vec3(reflection_matrix * glm::vec4(orig_cam_pos, 1.0f));
        glUniform3fv(camera.cam_pos_loc, 1, glm::value_ptr(ref_cam_pos));

        // Quando la scena viene disegnata, gli oggetti reali che si trovano
        // fisicamente "oltre" il vetro verranno automaticamente eliminati dalla GPU!
        main_scene.draw();

        // ==========================================
        // FASE 5: Ripristino Finale
        // ==========================================
        // Disattiva il piano di taglio per rimettere a posto la telecamera!
        glDisable(GL_CLIP_DISTANCE0);

        glFrontFace(GL_CCW);
        glDisable(GL_STENCIL_TEST);

        scene_light.direct_pos = orig_light_pos;
        scene_light.push_to_shader();
        glUniform3fv(camera.cam_pos_loc, 1, glm::value_ptr(orig_cam_pos));
    }
};