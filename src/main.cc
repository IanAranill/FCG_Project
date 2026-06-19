#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#include <SFML/Window.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include <iostream>
#include <cstdlib>


/////////////////////////////
// Window and OpenGL setup //
/////////////////////////////

class Setup
{
public:
    sf::Window* window;
    int window_width = 1024;
    int window_height = 768;

    Setup() {
        sf::ContextSettings settings;
        settings.depthBits = 32;
        settings.stencilBits = 8; // FONDAMENTALE per lo specchio futuro!
        settings.antiAliasingLevel = 4;
        settings.attributeFlags = sf::ContextSettings::Attribute::Core;
        settings.majorVersion = 4;
        settings.minorVersion = 1;

        window = new sf::Window (
                                 sf::VideoMode(sf::Vector2u(window_width, window_height)),
                                 "Progetto FCG - Specchio FPS",
                                 sf::Style::Default,
                                 sf::State::Windowed,
                                 settings
                                 );

        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window->setPosition(sf::Vector2i(
            (desktop.size.x - window_width) / 2,
            (desktop.size.y - window_height) / 2
        ));

        window->setVerticalSyncEnabled (true);

        if (!window->setActive (true)) {
            std::cerr << "Failure: error during SFML OpenGL Activation." << std::endl;
            exit (1);
        }
        sf::ContextSettings gotten = window->getSettings ();

        std::cout << "depth bits: " << gotten.depthBits << std::endl;
        std::cout << "stencil bits: " << gotten.stencilBits << std::endl;
        std::cout << "antialiasing level: " << gotten.antiAliasingLevel << std::endl;
        std::cout << "SFML GL version: " << gotten.majorVersion << "." << gotten.minorVersion << std::endl;

        int version = gladLoadGL (sf::Context::getFunction);
        if (!version) {
            std::cerr << "Failure: error during glad loading." << std::endl;
            exit (1);
        }
        std::cout << "GLAD GL version: " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;
    }

    ~Setup() {
        delete window;
    }
};

////////////////////
// SFML Callbacks //
////////////////////

void handle_events(sf::Window& window, bool& running)
{
    while (const std::optional event = window.pollEvent ())
    {
        if (event->is<sf::Event::Closed> ()) {
            running = false;
        }
        else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed> ()) {
            if (key_pressed->scancode == sf::Keyboard::Scancode::Escape) {
                running = false;
            }
            // TODO: Aggiungere gestione tasti WASD per la telecamera FPS
        }
        else if (const auto* mouse = event->getIf<sf::Event::MouseMoved> ()) {
            // TODO: Aggiungere gestione movimento mouse per la rotazione della visuale
        }
    }
}

//////////
// Main //
//////////

int main (int argc, char* argv[])
{
    Setup setup;
    sf::Window& window = *setup.window;

    // TODO: Inizializzare Camera FPS
    // TODO: Inizializzare Stanza e Oggetto 3D

    glEnable (GL_CULL_FACE);
    glCullFace (GL_BACK);
    
    glEnable (GL_DEPTH_TEST);
    

    //// 3. Main Loop ////
    bool running = true;
    while (running)
    {
        // A. Gestione Input
        handle_events(window, running);

        // TODO: Aggiornare la logica della scena (es. far ruotare l'oggetto)
        
        // B. Rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // TODO: Disegnare la Stanza
        // TODO: Disegnare la logica dello Specchio
        // TODO: Disegnare l'oggetto 3D

        window.display ();
    }

    return 0;
}