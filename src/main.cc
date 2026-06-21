#define GLAD_GL_IMPLEMENTATION
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>

#ifndef GLAD_GL
#define GLAD_GL
#include "glad/gl.h"
#endif

// --- Moduli ---
#include "include/camera.hh"
#include "include/hotshaders.hh"
#include "include/lighting.hh"
#include "include/mesh.hh"
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

        int version = gladLoadGL(sf::Context::getFunction);
        if (!version) {
            std::cerr << "Failure: error during glad loading." << std::endl;
            exit(1);
        }
    }

    ~Setup() { delete window; }
};

//////////////////////////
// Gestione Input       //
//////////////////////////

void handle(const sf::Event::KeyPressed& key, bool& running) {
    if (key.scancode == sf::Keyboard::Scancode::Escape) {
        running = false;
    }
}

void handle(const sf::Event::MouseMoved& mouse, Camera& camera) {
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

void handle_events(sf::Window& window, bool& running, Camera& camera, float dt) {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            running = false;
        } else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
            handle(*key_pressed, running);
        } else if (const auto* mouse_moved = event->getIf<sf::Event::MouseMoved>()) {
            handle(*mouse_moved, camera);
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        camera.process_keyboard(CameraMovement::FORWARD, dt);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        camera.process_keyboard(CameraMovement::BACKWARD, dt);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        camera.process_keyboard(CameraMovement::LEFT, dt);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        camera.process_keyboard(CameraMovement::RIGHT, dt);
}

/////////////////////////////////
// Helper di Scena e Shading   //
/////////////////////////////////

std::unique_ptr<Scene> load_scene(const std::string& filepath) {
    std::cout << "Caricamento " << filepath << "..." << std::endl;
    Mesh data(filepath);
    return std::make_unique<Scene>(data.vertices, data.indices);
}

// Utilizziamo la nuova classe Lighting
void prepare_shader_and_camera(const Shaders& shaders, const Camera& camera,
                               const Lighting& lighting, float aspect_ratio) {
    shaders.use();
    glm::mat4 view = camera.get_view_matrix();
    glm::mat4 proj = camera.get_projection_matrix(aspect_ratio);
    glm::mat4 vp = proj * view;

    glUniformMatrix4fv(glGetUniformLocation(shaders.program, "vp"), 1, GL_FALSE, &vp[0][0]);
    glUniform3fv(glGetUniformLocation(shaders.program, "cam_pos"), 1, &camera.position[0]);

    lighting.push_to_shader(shaders.program);
}

void draw_object(const std::unique_ptr<Scene>& scene, GLuint shader_program,
                 const glm::mat4& model_matrix) {
    GLint model_loc = glGetUniformLocation(shader_program, "model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, &model_matrix[0][0]);
    scene->draw();
}

//////////
// Main //
//////////

int main(int argc, char* argv[]) {
    Setup setup;
    sf::Window& window = *setup.window;

    // --- Inizializzazione Logica ---
    Camera camera(glm::vec3(0.0f, 1.5f, 5.0f));
    Shaders shaders("resources/shaders/vertex.vert", "resources/shaders/fragment.frag");

    // Nuova classe unificata
    Lighting scene_lighting;

    // --- Caricamento Geometrie ---
    auto corner = load_scene("resources/meshes/corner.off");
    auto bunny = load_scene("resources/meshes/bunny.off");
    // TODO: Inizializzazione Specchio

    // --- Setup OpenGL ---
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    // --- Main Loop ---
    sf::Clock delta_clock;
    bool running = true;

    while (running) {
        float dt = delta_clock.restart().asSeconds();

        // --- Gestione Input ---
        handle_events(window, running, camera, dt);

        // TODO: Aggiornamento Logica Scena

        // --- Clear Buffer ---
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // --- Setup Shaders ---
        float aspect_ratio =
            static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y);
        prepare_shader_and_camera(shaders, camera, scene_lighting, aspect_ratio);

        // --- Rendering ---

        glm::mat4 model_corner = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
        draw_object(corner, shaders.program, model_corner);

        // TODO: Maschera Specchio (Stencil Buffer)
        // TODO: Rendering Oggetti Riflessi
        // TODO: Rendering Vetro Specchio

        glm::mat4 model_bunny = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -2.0f));
        model_bunny = glm::scale(model_bunny, glm::vec3(15.0f));
        draw_object(bunny, shaders.program, model_bunny);

        // --- Swap Buffer ---
        window.display();
    }

    return 0;
}