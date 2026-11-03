#include <RivetPong/objects/PlayerPaddle.hpp>

#include <algorithm>
#include <cmath>

namespace
{
    // The pointer is reported in window coordinates, but the game is laid out in viewport
    //      coordinates. This undoes the scale and letterbox offset the viewport applies when it
    //      is drawn to the window, so it must stay in step with `Rivet::Viewport::update()`.
    float pointer_to_viewport_x(float pointer_x)
    {
        const float display_width = static_cast<float>(Rivet::Display::get_width());
        const float display_height = static_cast<float>(Rivet::Display::get_height());
        const float viewport_width = static_cast<float>(Rivet::Viewport::get_width());
        const float viewport_height = static_cast<float>(Rivet::Viewport::get_height());

        float scale = std::min(display_width / viewport_width, display_height / viewport_height);
        if (Rivet::Viewport::is_integer_scaling()) {
            scale = std::floor(scale);
        }

        // A zero scale means the viewport isn't drawn at all, so there is nothing to map onto.
        if (scale <= 0.0f) {
            return pointer_x;
        }

        const float offset_x = (display_width - viewport_width * scale) * 0.5f;
        return (pointer_x - offset_x) / scale;
    }
}

PlayerPaddle::PlayerPaddle()
    : Paddle(Rivet::Viewport::get_height() - 256.0f, 1200.0f)
{
}

void PlayerPaddle::update(float delta)
{
    // Touch (or the mouse on desktop) pulls the paddle toward the pointer. raylib reports the
    //      first touch as the left mouse button, so this covers both.
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        this->move_toward(pointer_to_viewport_x(GetMousePosition().x), delta);
        return;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        this->move_toward(0.0f, delta);
    }
    else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        this->move_toward(static_cast<float>(Rivet::Viewport::get_width()), delta);
    }
}
