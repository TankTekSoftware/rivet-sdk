#include <RivetPong/objects/Ball.hpp>

#include <algorithm>
#include <cmath>

Ball::Ball()
{
    this->reset();

    this->velocity.x = 600.0f;
    this->velocity.y = 400.0f;

    this->radius = 16.0f;
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
    const float viewport_width = static_cast<float>(Rivet::Viewport::get_width());

    this->position.x += this->velocity.x * this->speed_multiplier * delta;
    this->position.y += this->velocity.y * this->speed_multiplier * delta;

    // The ball only bounces off the side walls. Leaving through the top or bottom is a point,
    //      which `PongApp` scores.
    if (this->position.x - this->radius < 0.0f) {
        this->position.x = this->radius;
        this->velocity.x = -this->velocity.x;
    }
    else if (this->position.x + this->radius > viewport_width) {
        this->position.x = viewport_width - this->radius;
        this->velocity.x = -this->velocity.x;
    }
}

void Ball::render(Rivet::RenderPass pass)
{
    if (pass != Rivet::RenderPass::VIEWPORT) {
        return;
    }

    // A 2px ring rather than a 1px circle outline, so the outline survives the viewport being
    //      scaled down to the window.
    const float line_thickness = 2.0f;

    DrawRing(this->position, this->radius - line_thickness, this->radius, 0.0f, 360.0f, 0, WHITE);
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
    this->position.x = Rivet::Viewport::get_width() / 2.0f;
    this->position.y = Rivet::Viewport::get_height() / 2.0f;

    // Each serve starts back at the base speed.
    this->speed_multiplier = 1.0f;
}
