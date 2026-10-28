#include <RivetPong/objects/PlayerPaddle.hpp>

PlayerPaddle::PlayerPaddle()
    : Paddle(GetScreenHeight() - 128.0f, 600.0f)
{
}

void PlayerPaddle::update(float delta)
{
    // Touch (or the mouse on desktop) pulls the paddle toward the pointer. raylib reports the
    //      first touch as the left mouse button, so this covers both.
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        this->move_toward(GetMousePosition().x, delta);
        return;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        this->move_toward(0.0f, delta);
    }
    else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        this->move_toward(static_cast<float>(GetScreenWidth()), delta);
    }
}
