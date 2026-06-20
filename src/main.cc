#define GLAD_GL_IMPLEMENTATION
#include <SFML/Window.hpp>
#include <SFML/System/Clock.hpp> // Aggiunto per il Delta Time!
#include <cstdlib>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>

#ifndef GLAD_GL
#define GLAD_GL
#include "glad/gl.h"
#endif

// Modules
#include "include/camera.hh"
#include "include/light.hh"
#include "include/materials.hh"
#include "include/hotshaders.hh"
#include "include/scene.hh" 


/////////////////////////////
// Window and OpenGL setup //
/////////////////////////////

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

        window = new sf::Window(sf::VideoMode(sf::Vector2u(window_width, window_height)),
                                "Progetto FCG - Specchio FPS", sf::Style::Default,
                                sf::State::Windowed, settings);

        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window->setPosition(sf::Vector2i((desktop.size.x - window_width) / 2,
                                         (desktop.size.y - window_height) / 2));

        window->setVerticalSyncEnabled(true);

        if (!window->setActive(true)) {
            std::cerr << "Failure: error during SFML OpenGL Activation." << std::endl;
            exit(1);
        }
        sf::ContextSettings gotten = window->getSettings();

        std::cout << "depth bits: " << gotten.depthBits << std::endl;
        std::cout << "stencil bits: " << gotten.stencilBits << std::endl;
        std::cout << "antialiasing level: " << gotten.antiAliasingLevel << std::endl;
        std::cout << "SFML GL version: " << gotten.majorVersion << "." << gotten.minorVersion
                  << std::endl;

        int version = gladLoadGL(sf::Context::getFunction);
        if (!version) {
            std::cerr << "Failure: error during glad loading." << std::endl;
            exit(1);
        }
        std::cout << "GLAD GL version: " << GLAD_VERSION_MAJOR(version) << "."
                  << GLAD_VERSION_MINOR(version) << std::endl;
    }

    ~Setup() { delete window; }
};

////////////////////
// SFML Callbacks //
////////////////////

void handle_keyboard(const sf::Event::KeyPressed& key, bool& running) {
    if (key.scancode == sf::Keyboard::Scancode::Escape) {
        running = false;
    }
}

void handle_mouse(const sf::Event::MouseMoved& mouse, Camera& camera) {
    static float prev_x = 0.0f;
    static float prev_y = 0.0f;
    static bool first_mouse = true; 

    if (first_mouse) {
        prev_x = static_cast<float>(mouse.position.x);
        prev_y = static_cast<float>(mouse.position.y);
        first_mouse = false;
    }

    float x_offset = static_cast<float>(mouse.position.x) - prev_x;
    float y_offset = static_cast<float>(mouse.position.y) - prev_y;

    prev_x = static_cast<float>(mouse.position.x);
    prev_y = static_cast<float>(mouse.position.y);

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        camera.process_mouse_drag(x_offset, y_offset);
    }
}

//////////
// Main //
//////////

int main(int argc, char* argv[]) {
    Setup setup;
    sf::Window& window = *setup.window;

    // Inizializzazione Camera FPS
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

    Shaders shaders("vertex.vert", "fragment.frag");
    shaders.use ();
    // TODO: Inizializzare Stanza e Oggetto 3D

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    // Main Loop //
    sf::Clock delta_clock;
    bool running = true;
    
    while (running) {
        float dt = delta_clock.restart().asSeconds();

        // Gestione Input
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                running = false;
            } else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                handle_keyboard(*key_pressed, running);
            } else if (const auto* mouse_moved = event->getIf<sf::Event::MouseMoved>()) {
                handle_mouse(*mouse_moved, camera);
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) camera.process_keyboard(CameraMovement::FORWARD, dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) camera.process_keyboard(CameraMovement::BACKWARD, dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) camera.process_keyboard(CameraMovement::LEFT, dt);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) camera.process_keyboard(CameraMovement::RIGHT, dt);

        // TODO: Aggiornare la logica della scena (es. far ruotare l'oggetto)

        // Calcolo Matrici
        float aspect_ratio = static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y);
        glm::mat4 view = camera.get_view_matrix();
        glm::mat4 proj = camera.get_projection_matrix(aspect_ratio);
        glm::mat4 vp = proj * view;

        // Rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // TODO: glUniformMatrix4fv(glGetUniformLocation(shader_program, "vp"), 1, GL_FALSE, &vp[0][0]);
        // TODO: Disegnare la Stanza
        // TODO: Disegnare la logica dello Specchio
        // TODO: Disegnare l'oggetto 3D

        window.display();
    }

    return 0;
}