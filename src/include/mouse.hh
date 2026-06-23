#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#ifdef __APPLE__
#include <CoreGraphics/CGRemoteOperation.h>

#include <cstdint>
#endif

class Mouse {
   private:
#ifdef __APPLE__
    bool first_call = true;

#else
    sf::Vector2f accumulated_delta{0.f, 0.f};
#endif

   public:
#ifdef __APPLE__

    void event(const sf::Event::MouseMovedRaw& /* event */) {
        // No-op: on macOS deltas are polled inside consumeDelta(), not fed from events.
    }

    sf::Vector2f delta() {
        int32_t dx = 0;
        int32_t dy = 0;
        CGGetLastMouseDelta(&dx, &dy);

        // First-call quirk: CGGetLastMouseDelta reports the
        // displacement between the mouse position at program
        // launch and the current position on its very first
        // invocation. We discard that value and return zero to
        // prevent a violent camera jump at startup.
        if (first_call) {
            first_call = false;
            return sf::Vector2f{0.f, 0.f};
        }

        return sf::Vector2f{static_cast<float>(dx), static_cast<float>(dy)};
    }

    void setPosition(sf::Vector2i position) const {
        CGPoint warpPoint;
        warpPoint.x = static_cast<CGFloat>(position.x);
        warpPoint.y = static_cast<CGFloat>(position.y);

        CGWarpMouseCursorPosition(warpPoint);
    }

#else

    void event(const sf::Event::MouseMovedRaw& e) {
        accumulated_delta += sf::Vector2f(e.delta);
    };

    sf::Vector2f delta() {
        sf::Vector2f delta = accumulated_delta;
        accumulated_delta = sf::Vector2f{0.f, 0.f};
        return delta;
    }

    void setPosition(sf::Vector2i position) const {
        sf::Mouse::setPosition(position);
    }

#endif
};