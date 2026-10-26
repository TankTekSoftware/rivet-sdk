#include <RivetPong/objects/Paddle.hpp>

#include <algorithm>

Paddle::Paddle(float y, float speed)
{
    this->position.x = GetScreenWidth() / 2.0f;
    this->position.y = y;

    this->size.x = 80.0f;
    this->size.y = 12.0f;

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

    DrawRectangleLinesEx(this->get_rect(), 1.0f, WHITE);
}

void Paddle::move_toward(float target_x, float delta)
{
    const float step = this->speed * delta;
    const float half_width = this->size.x / 2.0f;
    const float display_width = static_cast<float>(GetScreenWidth());

    // Move at most one step, so the paddle settles on the target instead of jittering past it.
    this->position.x += std::clamp(target_x - this->position.x, -step, step);
    this->position.x = std::clamp(this->position.x, half_width, display_width - half_width);
}
