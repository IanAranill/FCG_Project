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
#include "include/imgui.hh"
#include "include/lighting.hh"
#include "include/mesh.hh"
#include "include/mirror.hh"
#include "include/mouse.hh"
#include "include/scene.hh"

// --- Setup function ---
sf::Window setup() {
    int window_width = 1024;
    int window_height = 768;

    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antiAliasingLevel = 4;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;
    settings.majorVersion = 4;
    settings.minorVersion = 1;

    sf::Window window = sf::Window(sf::VideoMode(sf::Vector2u(window_width, window_height)),
                                   "Progetto FCG", sf::State::Windowed, settings);

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(
        sf::Vector2i((desktop.size.x - window_width) / 2, (desktop.size.y - window_height) / 2));

    if (window.getSettings().stencilBits == 0) {
        std::cerr << ">>> [ERRORE] Il sistema non ha allocato lo Stencil Buffer! I riflessi "
                     "non verranno mascherati.\n";
    }

    window.setFramerateLimit(60);
    window.requestFocus();
    window.setMouseCursorGrabbed(true);
    window.setMouseCursorVisible(false);

    if (!window.setActive(true)) {
        std::cerr
            << ">>> [ERRORE CRITICO] Impossibile attivare il contesto OpenGL sulla finestra!\n";
        std::exit(EXIT_FAILURE);
    }

    return window;
}

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

int main() {
    sf::Window window = setup();
    ImguiWrap ImguiWrap(window);

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(sf::Context::getFunction))) {
        std::cerr << "Errore nell'inizializzazione di GLAD\n";
        return -1;
    }

    Shaders shaders("resources/shaders/vertex.vert", "resources/shaders/fragment.frag");
    Camera camera(shaders.program, glm::vec3(0.0f, 1.0f, 5.0f));
    Light scene_light(shaders.program);
    Mouse mouse;

    Mesh corner(shaders.program, "resources/meshes/corner.off", false);
    corner.position = glm::vec3(0.0f, -1.0f, 0.0f);
    corner.scale = glm::vec3(25.0f);

    Mesh bunny(shaders.program, "resources/meshes/bunny.off", true);
    bunny.position = glm::vec3(0.0f, 0.0f, 0.0f);
    bunny.scale = glm::vec3(1.5f);

    Scene scene;
    scene.add_mesh(&corner);
    scene.add_mesh(&bunny);

    Mirror mirror(shaders.program, "resources/meshes/mirror.off", glm::vec3(0.0f, 1.0f, -3.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));
    mirror.mesh.scale = glm::vec3(3.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    sf::Clock delta_clock;
    bool running = true;

    while (running) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        sf::Vector2u window_size = window.getSize();

        float aspect_ratio = static_cast<float>(window_size.x) / static_cast<float>(window_size.y);
        sf::Time elapsed = delta_clock.restart();
        float dt = elapsed.asSeconds();

        handle_events(window, running, camera, dt, mouse, ImguiWrap.wantImgui);

        if (sf::Vector2f mDelta = mouse.delta();
            !ImguiWrap.wantImgui && (std::abs(mDelta.x) > 0.0f || std::abs(mDelta.y) > 0.0f)) {
            camera.process_mouse_drag(mDelta.x, mDelta.y);
        }

        shaders.use();
        camera.push_to_shader(aspect_ratio);
        scene_light.push_to_shader();

        mirror.render_scene_with_mirror(shaders.program, camera, scene, mirror, scene_light,
                                        aspect_ratio);

        ImguiWrap.update_ImGui(window, elapsed);
        ImguiWrap.draw_ImGui(scene_light, corner, bunny, mirror);

        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::SFML::Shutdown();
    return 0;
}