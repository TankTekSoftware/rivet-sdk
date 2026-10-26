#include <RivetPong/objects/Ball.hpp>

#include <algorithm>
#include <cmath>

Ball::Ball()
{
    this->reset();

    this->velocity.x = 300.0f;
    this->velocity.y = 200.0;

    this->radius = 8.0f;
}

Vector2 Ball::get_position() const
{
    return this->position;
}

Vector2 Ball::get_velocity() const
{
    return {
        this->velocity.x * this->speed_multiplier,
        this->velocity.y * this->speed_multiplier
    };
}

void Ball::update(float delta)
{
    const float display_width = static_cast<float>(GetScreenWidth());

    this->position.x += this->velocity.x * this->speed_multiplier * delta;
    this->position.y += this->velocity.y * this->speed_multiplier * delta;

    // The ball only bounces off the side walls. Leaving through the top or bottom is a point,
    //      which `PongApp` scores.
    if (this->position.x - this->radius < 0.0f) {
        this->position.x = this->radius;
        this->velocity.x = -this->velocity.x;
    }
    else if (this->position.x + this->radius > display_width) {
        this->position.x = display_width - this->radius;
        this->velocity.x = -this->velocity.x;
    }
}

void Ball::render(Rivet::RenderPass pass)
{
    if (pass != Rivet::RenderPass::VIEWPORT) {
        return;
    }

    DrawCircleLinesV(this->position, this->radius, WHITE);
}

void Ball::bounce_off(Rectangle rect)
{
    if (!CheckCollisionCircleRec(this->position, this->radius, rect)) {
        return;
    }

    // Push the ball out of the side it hit and send it away from that side, so it can't
    //      collide again on the next frame and get stuck inside the rectangle.
    if (this->position.y < rect.y + rect.height / 2.0f) {
        this->position.y = rect.y - this->radius;
        this->velocity.y = -std::fabs(this->velocity.y);
    }
    else {
        this->position.y = rect.y + rect.height + this->radius;
        this->velocity.y = std::fabs(this->velocity.y);
    }

    // Every hit speeds the ball up, so rallies get harder the longer they last. The cap keeps
    //      the ball from moving far enough in one frame to pass through a paddle.
    const float speed_increase = 1.1f;
    const float max_speed_multiplier = 3.0f;

    this->speed_multiplier = std::min(this->speed_multiplier * speed_increase, max_speed_multiplier);
}

void Ball::reset()
{
    this->position.x = GetScreenWidth() / 2.0f;
    this->position.y = GetScreenHeight() / 2.0f;

    // Each serve starts back at the base speed.
    this->speed_multiplier = 1.0f;
}
