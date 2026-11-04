#include <RivetPong/objects/PlayerPaddle.hpp>

// 384 below the center line is 256 from the bottom of the 1280 tall court.
PlayerPaddle::PlayerPaddle() : Paddle(384.0f, 1200.0f)
{
}

void PlayerPaddle::update(float delta)
{
    this->follow_center();

    // Touch (or the mouse on desktop) pulls the paddle toward the pointer. raylib reports the
    //      first touch as the left mouse button, so this covers both.
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        this->move_toward(Rivet::Viewport::screen_to_viewport_x(GetMousePosition().x), delta);
        return;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        this->move_toward(0.0f, delta);
    }
    else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        this->move_toward(static_cast<float>(Rivet::Viewport::get_width()), delta);
    }
}
