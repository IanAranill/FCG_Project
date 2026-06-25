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
        // FASE 1: Disegno della Scena Reale
        // ==========================================
        // Disabilitazione dello Stencil Test e abilitazione della scrittura su Color e Depth
        // Buffer. Impostazione del Culling standard per nascondere le facce posteriori.
        glDisable(GL_STENCIL_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);

        camera.push_to_shader(aspect_ratio);
        scene_light.push_to_shader();

        main_scene.draw();

        // ==========================================
        // FASE 2: Rendering del Retro dello Specchio
        // ==========================================
        // Inversione del Culling per renderizzare esclusivamente la faccia posteriore dello
        // specchio.
        glCullFace(GL_FRONT);

        glm::vec3 orig_diff = mirror.mesh.material_diffuse;
        glm::vec3 orig_amb = mirror.mesh.material_ambient;
        glm::vec3 orig_spec = mirror.mesh.material_specular;

        // Applicazione di un materiale scuro per modellare la scocca fisica.
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

        mirror.mesh.draw();

        // ==========================================
        // FASE 3: Mappatura Stencil (Vetro = 1, Bordo = 2) e Creazione Tela Scura
        // ==========================================
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glCullFace(GL_BACK);

        glm::vec3 orig_scale = mirror.mesh.scale;

        // --- Step A: Tracciamento dell'ID 2 (Bordo esterno) in modo invisibile ---
        // Disabilitazione della scrittura dei colori e della profondità. Tracciamento dell'ingombro
        // totale mascherato con ID 2.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        glStencilFunc(GL_ALWAYS, 2, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        mirror.mesh.scale = orig_scale * 1.05f;
        glm::mat4 border_model_matrix = mirror.mesh.get_model_matrix();
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(border_model_matrix));
        mirror.mesh.draw();

        // --- Step B: Tracciamento dell'ID 1 (Vetro interno) e creazione della tela scura ---
        // Ripristino della scrittura di colore e profondità.
        // Sovrascrittura della parte centrale dello stencil con l'ID 1 e applicazione di un
        // materiale scuro per occludere la stanza retrostante.
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        mirror.mesh.material_diffuse = glm::vec3(0.05f, 0.05f, 0.05f);
        mirror.mesh.material_ambient = glm::vec3(0.02f, 0.02f, 0.02f);
        mirror.mesh.material_specular = glm::vec3(0.01f, 0.01f, 0.01f);
        mirror.mesh.push_material_to_shader();

        mirror.mesh.scale = orig_scale;
        glm::mat4 glass_model_matrix = mirror.mesh.get_model_matrix();
        glm::mat3 glass_normal_matrix = glm::transpose(glm::inverse(glm::mat3(glass_model_matrix)));

        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(glass_model_matrix));
        glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, glm::value_ptr(glass_normal_matrix));

        mirror.mesh.draw();

        // ==========================================
        // FASE 4: Rendering della Scena Riflessa
        // ==========================================
        // Limitazione del rendering esclusivamente all'area mascherata con ID 1.
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Attivazione dell'Hardware Clipping Plane per tagliare la geometria oltre lo specchio.
        clip_plane = glm::vec4(mirror.normal, -glm::dot(mirror.normal, mirror.position));
        glUniform4fv(clip_plane_loc, 1, glm::value_ptr(clip_plane));
        glEnable(GL_CLIP_DISTANCE0);

        // Modifica dei parametri della telecamera in base alla matrice di riflessione.
        glm::mat4 reflection_matrix = mirror.get_reflection_matrix();
        glm::mat4 view = camera.get_view_matrix();
        glm::mat4 projection = camera.get_projection_matrix(aspect_ratio);

        glm::mat4 reflected_view = view * reflection_matrix;
        glm::mat4 reflected_vp = projection * reflected_view;
        glUniformMatrix4fv(camera.vp_loc, 1, GL_FALSE, glm::value_ptr(reflected_vp));

        glFrontFace(GL_CW);

        glm::vec3 orig_light_pos = scene_light.direct_pos;
        scene_light.direct_pos = glm::vec3(reflection_matrix * glm::vec4(orig_light_pos, 1.0f));
        scene_light.push_to_shader();

        glm::vec3 orig_cam_pos = camera.cam_pos;
        glUniform3fv(camera.cam_pos_loc, 1,
                     glm::value_ptr(glm::vec3(reflection_matrix * glm::vec4(orig_cam_pos, 1.0f))));

        main_scene.draw();

        // ==========================================
        // FASE 5: Rendering del Bordo Esterno
        // ==========================================
        glDisable(GL_CLIP_DISTANCE0);
        glFrontFace(GL_CCW);

        // Limitazione del rendering esclusivamente all'area mascherata con ID 2.
        glStencilFunc(GL_EQUAL, 2, 0xFF);

        // Applicazione di un materiale nero sulla mesh scalata per generare la cornice.
        mirror.mesh.material_diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
        mirror.mesh.material_ambient = glm::vec3(0.0f, 0.0f, 0.0f);
        mirror.mesh.material_specular = glm::vec3(0.0f, 0.0f, 0.0f);
        mirror.mesh.push_material_to_shader();

        mirror.mesh.scale = orig_scale * 1.05f;
        glm::mat4 final_border_model = mirror.mesh.get_model_matrix();
        glm::mat3 final_border_normal = glm::transpose(glm::inverse(glm::mat3(final_border_model)));

        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(final_border_model));
        glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, glm::value_ptr(final_border_normal));

        mirror.mesh.draw();

        // ==========================================
        // FASE 6: Ripristino Finale Globale
        // ==========================================
        // Ripristino dei materiali originali e pulizia della macchina a stati di OpenGL.
        mirror.mesh.scale = orig_scale;
        mirror.mesh.material_diffuse = orig_diff;
        mirror.mesh.material_ambient = orig_amb;
        mirror.mesh.material_specular = orig_spec;

        glDisable(GL_STENCIL_TEST);

        scene_light.direct_pos = orig_light_pos;
        scene_light.push_to_shader();
        glUniform3fv(camera.cam_pos_loc, 1, glm::value_ptr(orig_cam_pos));
    }
};