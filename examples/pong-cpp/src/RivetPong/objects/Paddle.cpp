#include <RivetPong/objects/Paddle.hpp>

#include <algorithm>

Paddle::Paddle(float center_offset, float speed)
{
    this->center_offset = center_offset;

    this->position.x = Rivet::Viewport::get_width() / 2.0f;
    this->follow_center();

    this->size.x = 160.0f;
    this->size.y = 24.0f;

    this->speed = speed;
}

Rectangle Paddle::get_rect() const
{
    return {
        this->position.x - this->size.x / 2.0f,     // x
        this->position.y - this->size.y / 2.0f,     // y
        this->size.x,                               // width
        this->size.y                                // height
    };
}

void Paddle::render(Rivet::RenderPass pass)
{
    if (pass != Rivet::RenderPass::VIEWPORT) {
        return;
    }

    DrawRectangleLinesEx(this->get_rect(), 2.0f, WHITE);
}

void Paddle::follow_center()
{
    // The viewport's height follows the screen's shape, so it changes when the window is resized
    //      or the device is rotated. Placing the paddle from the center line rather than an edge
    //      keeps both paddles the same distance apart at any size.
    this->position.y = Rivet::Viewport::get_height() / 2.0f + this->center_offset;
}

void Paddle::move_toward(float target_x, float delta)
{
    const float step = this->speed * delta;
    const float half_width = this->size.x / 2.0f;
    const float viewport_width = static_cast<float>(Rivet::Viewport::get_width());

    // Move at most one step, so the paddle settles on the target instead of jittering past it.
    this->position.x += std::clamp(target_x - this->position.x, -step, step);
    this->position.x = std::clamp(this->position.x, half_width, viewport_width - half_width);
}
