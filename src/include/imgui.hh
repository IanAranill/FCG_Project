#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <SFML/Window.hpp>

#include "lighting.hh"
#include "mesh.hh"
#include "mirror.hh"
#include "mouse.hh"

class ImguiWrap {
   public:
    bool wantImgui = false;

    ImguiWrap(sf::Window& window) {
        if (!ImGui::SFML::Init(window, sf::Vector2f(window.getSize()))) {
            std::cerr << "Failure: could not init ImGui::SFML." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        ImGui_ImplOpenGL3_Init("#version 410 core");
    }

    void update_ImGui(const sf::Window& window, const sf::Time& elapsed) {
        ImGuiIO& io = ImGui::GetIO();
        if (wantImgui) {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        } else {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
        ImGui::SFML::Update(sf::Mouse::getPosition(window), sf::Vector2f(window.getSize()),
                            elapsed);
    }

    void draw_ImGui(Light& scene_light, Mesh& corner, Mesh& bunny, Mirror& mirror) {
        ImGui_ImplOpenGL3_NewFrame();

        ImGui::Begin("Impostazioni");
        if (ImGui::CollapsingHeader("Luce")) {
            ImGui::DragFloat3("Posizione luce", glm::value_ptr(scene_light.direct_pos), 0.1f);
            ImGui::ColorEdit3("Colore luce", glm::value_ptr(scene_light.direct_val));

            ImGui::ColorEdit3("Colore luce ambientale", glm::value_ptr(scene_light.ambient_val));
        }

        if (ImGui::CollapsingHeader("Bunny")) {
            ImGui::DragFloat3("Ambiente bunny", glm::value_ptr(bunny.material_ambient), 0.01f, 0.0f,
                              1.0f);
            ImGui::DragFloat3("Diffuso bunny", glm::value_ptr(bunny.material_diffuse), 0.01f, 0.0f,
                              1.0f);
            ImGui::DragFloat3("Speculare bunny", glm::value_ptr(bunny.material_specular), 0.01f,
                              0.0f, 1.0f);
            ImGui::DragFloat("Shininess bunny", &bunny.material_shininess, 0.1f);
            ImGui::DragFloat3("Posizione bunny", glm::value_ptr(bunny.position), 0.1f);
            ImGui::DragFloat3("Rotazione bunny", glm::value_ptr(bunny.rotation), 0.5f, -180.0f,
                              180.0f);
            float uniform_scale = bunny.scale.x;
            if (ImGui::DragFloat("Scala bunny", &uniform_scale, 0.1f)) {
                bunny.scale = glm::vec3(uniform_scale);
            }
        }

        if (ImGui::CollapsingHeader("Stanza")) {
            ImGui::DragFloat3("Ambiente stanza", glm::value_ptr(corner.material_ambient), 0.01f,
                              0.0f, 1.0f);
            ImGui::DragFloat3("Diffuso stanza", glm::value_ptr(corner.material_diffuse), 0.01f,
                              0.0f, 1.0f);
            ImGui::DragFloat3("Speculare stanza", glm::value_ptr(corner.material_specular), 0.01f,
                              0.0f, 1.0f);
            ImGui::DragFloat("Shininess stanza", &corner.material_shininess, 0.1f);
            ImGui::DragFloat3("Posizione stanza", glm::value_ptr(corner.position), 0.1f);
            ImGui::DragFloat3("Rotazione stanza", glm::value_ptr(corner.rotation), 0.5f, -180.0f,
                              180.0f);

            float uniform_scale = corner.scale.x;
            if (ImGui::DragFloat("Scala stanza", &uniform_scale, 0.1f)) {
                corner.scale = glm::vec3(uniform_scale);
            }
        }

        if (ImGui::CollapsingHeader("Specchio")) {
            float mirror_scale = mirror.mesh.scale.x;
            if (ImGui::DragFloat("Scala specchio", &mirror_scale, 0.1f)) {
                mirror.mesh.scale = glm::vec3(mirror_scale);
            }

            ImGui::DragFloat3("Posizione specchio", glm::value_ptr(mirror.position), 0.1f);
            ImGui::DragFloat3("Rotazione specchio", glm::value_ptr(mirror.mesh.rotation), 0.5f,
                              -180.0f, 180.0f);
            ImGui::DragFloat3("Normale specchio", glm::value_ptr(mirror.normal), 0.05f, -1.0f,
                              1.0f);

            mirror.normal = glm::normalize(mirror.normal);
            mirror.update_mesh_transform();  // Aggiorna sia il vetro che il frame
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
