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

        mesh.material_diffuse = glm::vec3(0.05f, 0.05f, 0.05f);
        mesh.material_ambient = glm::vec3(0.02f, 0.02f, 0.02f);
        mesh.material_specular = glm::vec3(0.01f, 0.01f, 0.01f);
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

    void update_mesh_transform(bool rotated) {
        mesh.position = position;
        if (!rotated) return;
        glm::vec3 local_normal(0.f);
        glm::mat4 model_matrix = mesh.get_model_matrix();
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));

        for (auto v : mesh.vertices) local_normal += v.normal;

        normal = glm::normalize(normal_matrix * local_normal);
    }

    void render_scene_with_mirror(GLuint shader_program, Camera& camera, Scene& main_scene,
                                  Light& scene_light, float aspect_ratio) {
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

        glm::vec3 orig_scale = mesh.scale;

        // Applicazione di un materiale scuro per modellare la scocca fisica.
        mesh.push_material_to_shader();

        mesh.scale = orig_scale * 1.05f;
        glm::mat4 mirror_model = mesh.get_model_matrix();
        glm::mat3 mirror_normal_back = -glm::transpose(glm::inverse(glm::mat3(mirror_model)));

        glUniformMatrix4fv(main_scene.model_loc, 1, GL_FALSE, glm::value_ptr(mirror_model));
        glUniformMatrix3fv(main_scene.normal_mat_loc, 1, GL_FALSE,
                           glm::value_ptr(mirror_normal_back));

        mesh.draw();

        // ==========================================
        // FASE 3: Mappatura Stencil (Vetro = 1, Bordo = 2) e Creazione Tela Scura
        // ==========================================
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glCullFace(GL_BACK);

        // --- Step A: Tracciamento dell'ID 2 (Bordo esterno) in modo invisibile ---
        // Disabilitazione della scrittura dei colori e della profondità. Tracciamento dell'ingombro
        // totale mascherato con ID 2.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        glStencilFunc(GL_ALWAYS, 2, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        glm::mat3 mirror_normal_front = -mirror_normal_back;
        glUniformMatrix4fv(main_scene.model_loc, 1, GL_FALSE, glm::value_ptr(mirror_model));
        glUniformMatrix3fv(main_scene.normal_mat_loc, 1, GL_FALSE,
                           glm::value_ptr(mirror_normal_front));
        mesh.draw();

        // --- Step B: Tracciamento dell'ID 1 (Vetro interno) e creazione della tela scura ---
        // Ripristino della scrittura di colore e profondità.
        // Sovrascrittura della parte centrale dello stencil con l'ID 1 e applicazione di un
        // materiale scuro per occludere la stanza retrostante.
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        mesh.scale = orig_scale;
        glm::mat4 glass_model_matrix = mesh.get_model_matrix();
        glm::mat3 glass_normal_matrix = glm::transpose(glm::inverse(glm::mat3(glass_model_matrix)));

        glUniformMatrix4fv(main_scene.model_loc, 1, GL_FALSE, glm::value_ptr(glass_model_matrix));
        glUniformMatrix3fv(main_scene.normal_mat_loc, 1, GL_FALSE,
                           glm::value_ptr(glass_normal_matrix));

        mesh.draw();

        // ==========================================
        // FASE 4: Rendering della Scena Riflessa
        // ==========================================
        // Limitazione del rendering esclusivamente all'area mascherata con ID 1.
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Attivazione dell'Hardware Clipping Plane per tagliare la geometria oltre lo specchio.
        clip_plane = glm::vec4(normal, -glm::dot(normal, position));
        glUniform4fv(clip_plane_loc, 1, glm::value_ptr(clip_plane));
        glEnable(GL_CLIP_DISTANCE0);

        // Modifica dei parametri della telecamera in base alla matrice di riflessione.
        glm::mat4 reflection_matrix = get_reflection_matrix();
        glm::mat4 view = camera.get_view_matrix();
        glm::mat4 projection = camera.get_projection_matrix(aspect_ratio);

        glm::mat4 reflected_view = view * reflection_matrix;
        glm::mat4 reflected_vp = projection * reflected_view;
        glUniformMatrix4fv(camera.vp_loc, 1, GL_FALSE, glm::value_ptr(reflected_vp));

        glFrontFace(GL_CW);

        glm::vec3 orig_light_pos = scene_light.direct_pos;
        scene_light.direct_pos = glm::vec3(reflection_matrix * glm::vec4(orig_light_pos, 1.0f));
        scene_light.push_to_shader();

        glUniform3fv(
            camera.cam_pos_loc, 1,
            glm::value_ptr(glm::vec3(reflection_matrix * glm::vec4(camera.cam_pos, 1.0f))));

        main_scene.draw();

        // ==========================================
        // FASE 5: Rendering del Bordo Esterno
        // ==========================================
        glDisable(GL_CLIP_DISTANCE0);
        glFrontFace(GL_CCW);

        // Limitazione del rendering esclusivamente all'area mascherata con ID 2.
        glStencilFunc(GL_EQUAL, 2, 0xFF);

        mesh.scale = orig_scale * 1.05f;
        glm::mat4 final_border_model = mesh.get_model_matrix();
        glm::mat3 final_border_normal = glm::transpose(glm::inverse(glm::mat3(final_border_model)));

        glUniformMatrix4fv(main_scene.model_loc, 1, GL_FALSE, glm::value_ptr(final_border_model));
        glUniformMatrix3fv(main_scene.normal_mat_loc, 1, GL_FALSE,
                           glm::value_ptr(final_border_normal));

        mesh.draw();

        // ==========================================
        // FASE 6: Ripristino Finale Globale
        // ==========================================
        // Ripristino dei materiali originali e pulizia della macchina a stati di OpenGL.
        mesh.scale = orig_scale;

        glDisable(GL_STENCIL_TEST);

        scene_light.direct_pos = orig_light_pos;
        scene_light.push_to_shader();
        glUniform3fv(camera.cam_pos_loc, 1, glm::value_ptr(camera.cam_pos));
    }
};