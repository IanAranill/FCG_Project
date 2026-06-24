#define GLAD_GL_IMPLEMENTATION
#ifndef GLAD_GL
#define GLAD_GL
#include "glad/gl.h"
#endif

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <optional>

#include "include/camera.hh"
#include "include/hotshaders.hh"
#include "include/lighting.hh"
#include "include/mesh.hh"
#include "include/mirror.hh"
#include "include/mouse.hh"
#include "include/scene.hh"

// --- Setup Contesto Finestra ---
class Setup {
   public:
    sf::Window* window;
    int window_width = 1024;
    int window_height = 768;

    Setup() {
        sf::ContextSettings settings;
        settings.depthBits = 24;
        settings.stencilBits = 8;
        settings.antiAliasingLevel = 4;
        settings.attributeFlags = sf::ContextSettings::Attribute::Core;
        settings.majorVersion = 4;
        settings.minorVersion = 1;

        window = new sf::Window(sf::VideoMode(sf::Vector2u(window_width, window_height)),
                                "Progetto FCG", sf::State::Windowed, settings);

        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window->setPosition(sf::Vector2i((desktop.size.x - window_width) / 2,
                                         (desktop.size.y - window_height) / 2));

        if (window->getSettings().stencilBits == 0) {
            std::cerr << ">>> [ERRORE] Il sistema non ha allocato lo Stencil Buffer! I riflessi "
                         "non verranno mascherati.\n";
        }

        window->setFramerateLimit(60);
        window->requestFocus();
        window->setMouseCursorGrabbed(true);
        window->setMouseCursorVisible(false);

        if (!ImGui::SFML::Init(*window, {(float)window_width, (float)window_height})) {
            std::cerr << "Failure: could not init ImGui::SFML." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        ImGui_ImplOpenGL3_Init("#version 410 core");
        if (!window->setActive(true)) {
            std::cerr
                << ">>> [ERRORE CRITICO] Impossibile attivare il contesto OpenGL sulla finestra!\n";
            std::exit(EXIT_FAILURE);
        }
    }

    ~Setup() { delete window; }
};

// --- Polling Eventi OS ---
void handle_events(sf::Window& window, bool& running, Camera& camera, float dt, Mouse& mouse,
                   bool& wantImGui) {
    static sf::Vector2i last_pos;

    while (const std::optional<sf::Event> event = window.pollEvent()) {
        ImGui::SFML::ProcessEvent(window, *event);

        if (event->is<sf::Event::Closed>()) {
            running = false;
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                   keyPressed && !ImGui::GetIO().WantCaptureKeyboard) {
            if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                running = false;
            } else if (keyPressed->scancode == sf::Keyboard::Scancode::RShift) {
                wantImGui = !wantImGui;
                window.setMouseCursorVisible(wantImGui);
                window.setMouseCursorGrabbed(!wantImGui);
            }
        } else if (const auto* mouse_raw = event->getIf<sf::Event::MouseMovedRaw>()) {
            mouse.event(*mouse_raw);
        } else if (event->is<sf::Event::FocusLost>()) {
            window.setMouseCursorGrabbed(false);
            window.setMouseCursorVisible(true);
        } else if (event->is<sf::Event::FocusGained>()) {
            if (!wantImGui) {
                window.setMouseCursorGrabbed(true);
                window.setMouseCursorVisible(false);
            }
        }
    }

    if (window.hasFocus() && !wantImGui) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W))
            camera.process_keyboard(CameraMovement::FORWARD, dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S))
            camera.process_keyboard(CameraMovement::BACKWARD, dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A))
            camera.process_keyboard(CameraMovement::LEFT, dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D))
            camera.process_keyboard(CameraMovement::RIGHT, dt);
    }
}

// --- ImGui Helper ---
void update_ImGui(const sf::Window& window, const sf::Time& elapsed, bool wantImGui) {
    ImGuiIO& io = ImGui::GetIO();
    if (wantImGui) {
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    } else {
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    }
    ImGui::SFML::Update(sf::Mouse::getPosition(window), sf::Vector2f(window.getSize()), elapsed);
}

void draw_ImGui(Light& scene_light, Mesh& corner, Mesh& bunny, Mirror& mirror) {
    ImGui_ImplOpenGL3_NewFrame();

    ImGui::Begin("Impostazioni");
    if (ImGui::CollapsingHeader("Luce")) {
        ImGui::DragFloat3("Posizione luce",
                          glm::value_ptr(scene_light.vec3_uniforms["light.direct_pos"]), 0.1f);
        ImGui::ColorEdit3("Colore luce",
                          glm::value_ptr(scene_light.vec3_uniforms["light.direct_val"]));

        ImGui::ColorEdit3("Colore luce ambientale",
                          glm::value_ptr(scene_light.vec3_uniforms["light.ambient_val"]));
    }

    if (ImGui::CollapsingHeader("Bunny")) {
        ImGui::DragFloat3("Ambiente bunny", glm::value_ptr(bunny.vec3_uniforms["material.ambient"]),
                          0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Diffuso bunny", glm::value_ptr(bunny.vec3_uniforms["material.diffuse"]),
                          0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Speculare bunny",
                          glm::value_ptr(bunny.vec3_uniforms["material.specular"]), 0.01f, 0.0f,
                          1.0f);
        ImGui::DragFloat("Shininess bunny", &bunny.float_uniforms["material.shininess"], 0.1f);
        ImGui::DragFloat3("Posizione bunny", glm::value_ptr(bunny.position), 0.1f);
        float uniform_scale = bunny.scale.x;
        if (ImGui::DragFloat("Scala bunny", &uniform_scale, 0.1f)) {
            bunny.scale = glm::vec3(uniform_scale);
        }
    }

    if (ImGui::CollapsingHeader("Stanza")) {
        ImGui::DragFloat3("Ambiente stanza",
                          glm::value_ptr(corner.vec3_uniforms["material.ambient"]), 0.01f, 0.0f,
                          1.0f);
        ImGui::DragFloat3("Diffuso stanza",
                          glm::value_ptr(corner.vec3_uniforms["material.diffuse"]), 0.01f, 0.0f,
                          1.0f);
        ImGui::DragFloat3("Speculare stanza",
                          glm::value_ptr(corner.vec3_uniforms["material.specular"]), 0.01f, 0.0f,
                          1.0f);
        ImGui::DragFloat("Shininess stanza", &corner.float_uniforms["material.shininess"], 0.1f);
        ImGui::DragFloat3("Posizione stanza", glm::value_ptr(corner.position), 0.1f);

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
        ImGui::DragFloat3("Normale specchio", glm::value_ptr(mirror.normal), 0.05f, -1.0f, 1.0f);

        mirror.normal = glm::normalize(mirror.normal);
        mirror.update_mesh_transform();  // Aggiorna sia il vetro che il frame
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main() {
    Setup setup;
    sf::Window& window = *setup.window;

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(sf::Context::getFunction))) {
        std::cerr << "Errore nell'inizializzazione di GLAD\n";
        return -1;
    }

    Shaders shaders("resources/shaders/vertex.vert", "resources/shaders/fragment.frag");
    Camera camera(glm::vec3(0.0f, 1.0f, 5.0f));
    Light scene_light;
    Mouse mouse;

    Mesh corner("resources/meshes/corner.off", false);
    corner.position = glm::vec3(0.0f, -1.0f, 0.0f);
    corner.scale = glm::vec3(25.0f);

    Mesh bunny("resources/meshes/bunny.off", true);
    bunny.position = glm::vec3(0.0f, 0.0f, 0.0f);
    bunny.scale = glm::vec3(1.5f);

    Scene scene;
    scene.add_mesh(&corner);
    scene.add_mesh(&bunny);

    Mirror mirror("resources/meshes/mirror.off", glm::vec3(0.0f, 1.0f, -3.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));
    mirror.mesh.scale = glm::vec3(3.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    sf::Clock delta_clock;
    bool running = true;
    bool wantImGui = false;

    while (running) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        sf::Vector2u window_size = window.getSize();

        float aspect_ratio = static_cast<float>(window_size.x) / static_cast<float>(window_size.y);
        sf::Time elapsed = delta_clock.restart();
        float dt = elapsed.asSeconds();

        handle_events(window, running, camera, dt, mouse, wantImGui);

        if (sf::Vector2f mDelta = mouse.delta();
            !wantImGui && (std::abs(mDelta.x) > 0.0f || std::abs(mDelta.y) > 0.0f)) {
            camera.process_mouse_drag(mDelta.x, mDelta.y);
        }

        shaders.use();
        camera.push_to_shader(shaders.program, aspect_ratio);
        scene_light.push_to_shader(shaders.program);

        mirror.render_scene_with_mirror(shaders.program, camera, scene, mirror, scene_light, aspect_ratio);

        update_ImGui(window, elapsed, wantImGui);
        draw_ImGui(scene_light, corner, bunny, mirror);

        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::SFML::Shutdown();
    return 0;
}