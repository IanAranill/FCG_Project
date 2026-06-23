#define GLAD_GL_IMPLEMENTATION

#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Context.hpp>
#include <SFML/Window/Event.hpp>
#include <cstdlib>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <optional>

#ifndef GLAD_GL
#define GLAD_GL
#include "glad/gl.h"
#endif

#include "include/camera.hh"
#include "include/hotshaders.hh"
#include "include/lighting.hh"
#include "include/mesh.hh"
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
        settings.depthBits = 32;
        settings.stencilBits = 8;
        settings.antiAliasingLevel = 4;
        settings.attributeFlags = sf::ContextSettings::Attribute::Core;
        settings.majorVersion = 4;
        settings.minorVersion = 1;

        window =
            new sf::Window(sf::VideoMode(sf::Vector2u(window_width, window_height)),
                           "Progetto FCG", sf::State::Windowed, settings);

        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window->setPosition(sf::Vector2i((desktop.size.x - window_width) / 2,
                                         (desktop.size.y - window_height) / 2));

        if (!window->setActive(true)) {
            std::cerr
                << ">>> [ERRORE CRITICO] Impossibile attivare il contesto OpenGL sulla finestra!\n";
            std::exit(EXIT_FAILURE);
        }

        window->setFramerateLimit(60);
        window->setMouseCursorGrabbed(true);
        window->setMouseCursorVisible(false);
    }

    ~Setup() { delete window; }
};

// --- Polling Eventi OS ---
void handle_events(sf::Window& window, bool& running, Camera& camera, float dt, Mouse& mouse) {
    static sf::Vector2i last_pos;

    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            running = false;
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                running = false;
            }
        } else if (const auto* mouse_raw = event->getIf<sf::Event::MouseMovedRaw>()) {
            mouse.event(*mouse_raw);
        }
    }

    if (window.hasFocus()) {
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
    Setup setup;
    sf::Window& window = *setup.window;

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(sf::Context::getFunction))) {
        std::cerr << "Errore nell'inizializzazione di GLAD\n";
        return -1;
    }

    // --- Inizializzazione Sottosistemi ---
    Shaders shaders("resources/shaders/vertex.vert", "resources/shaders/fragment.frag");
    Camera camera(glm::vec3(0.0f, 1.0f, 5.0f));
    Lighting scene_lighting;
    Mouse mouse;

    // --- Allocazione Geometrie ---
    Mesh corner("resources/meshes/corner.off", false);
    corner.position = glm::vec3(0.0f, -1.0f, 0.0f);
    corner.scale = glm::vec3(25.0f);

    Mesh bunny("resources/meshes/bunny.off", true);
    bunny.position = glm::vec3(0.0f, 0.0f, 0.0f);
    bunny.scale = glm::vec3(1.5f);

    Scene scene;
    scene.add_mesh(&corner);
    scene.add_mesh(&bunny);

    // --- Configurazione Stato OpenGL ---
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    sf::Clock delta_clock;
    bool running = true;

    // --- Main Rendering Loop ---
    while (running) {
        float dt = delta_clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        handle_events(window, running, camera, dt, mouse);

        if (sf::Vector2f mDelta = mouse.delta();
            std::abs(mDelta.x) > 0.0f || std::abs(mDelta.y) > 0.0f) {
            camera.process_mouse_drag(mDelta.x, mDelta.y);
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        float aspect_ratio =
            static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y);

        shaders.use();
        camera.push_to_shader(shaders.program, aspect_ratio);
        scene_lighting.push_to_shader(shaders.program);

        scene.draw(shaders.program);

        window.display();
    }

    return 0;
}